#include <clicknet/ip.h> // Must be above everything else otherwise this wouldn't work

#include <click/config.h>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/timer.hh>
#include <time.h>

#include "constants.hh"
#include "report.hh"
#include "query.hh"
#include "helper.hh"
#include "IGMPClient.hh"

CLICK_DECLS

IGMPClient::IGMPClient() {
    interfaceMulticastTable = new InterfaceMulticastTable();
    socketMulticastTable = new SocketMulticastTable();
}

IGMPClient::~IGMPClient() {}

int IGMPClient::configure(Vector <String> &, ErrorHandler *) {
    return 0;
}

void IGMPClient::process_udp(Packet *p) {
    const click_ip *ip_header = p->ip_header();
    in_addr multicast_address = ip_header->ip_dst;
//    click_chatter("Packet for %s", IPAddress(multicast_address).s().c_str());

    in_addr interface; // interface is always 0.0.0.0

    if (interfaceMulticastTable->can_forward(ip_header->ip_src, multicast_address) or
        IPAddress(multicast_address) == IPAddress("224.0.0.1")) {
        // forward packet
        output(2).push(p);
    } else {
        // drop packet
        output(1).push(p);
    }
}

void IGMPClient::process_query(QueryPacket *p, int port) {
    Query *q = p->to_query();
    if (not q->hasCorrectChecksum()) {
        click_chatter("\033[1;93m%-6s\033[0m %-6s", "Warning: ", "Faulty checksum in Query");
        return;
    }
    // References RFC 3376, section 5.2.
    /**
     * When a system receives a Query, it does not respond immediately.
     * Instead, it delays its response by a random amount of time, bounded
     * by the Max Resp Time value derived from the Max Resp Code in the
     * received Query message. A system may receive a variety of Queries on
     * different interfaces and of different kinds (e.g., General Queries,
     * Group-Specific Queries, and Group-and-Source-Specific Queries), each
     * of which may require its own delayed response.
     *
     * The actual time allowed, called the Max
     * Resp Time, is represented in units of 1/10 second (RFC 3376, section 4.1.1.)
     */
    srand48(time(0));
    double delay = drand48() * q->getMaxResponseTime() / 10;

    /**
     * Before scheduling a response to a Query, the system must first
     * consider previously scheduled pending responses and in many cases
     * schedule a combined response. Therefore, the system must be able to
     * maintain the following state:
     * o A timer per interface for scheduling responses to General Queries.
     * o A per-group and interface timer for scheduling responses to Group-
     * Specific and Group-and-Source-Specific Queries.
     */


    /**
     * When a new Query with the Router-Alert option arrives on an
     * interface, provided the system has state to report, a delay for a
     * response is randomly selected in the range (0, [Max Resp Time]) where
     * Max Resp Time is derived from Max Resp Code in the received Query
     * message. The following rules are then used to determine if a Report
     * needs to be scheduled and the type of Report to schedule. The rules
     * are considered in order and only the first matching rule is applied.
     */

    /**
     * 1. If there is a pending response to a previous General Query
     * scheduled sooner than the selected delay, no additional response
     */
    if (isShortestGeneralPendingResponse(port, Timestamp(delay))) {
        return;
    }

    /**
     * 2. If the received Query is a General Query, the interface timer is
     * used to schedule a response to the General Query after the
     * selected delay. Any previously pending response to a General
     * Query is canceled.
     */
    if (q->isGeneralQuery()) {
        QueryResponseArgs *args = new QueryResponseArgs();
        args->query = q;
        args->client = this;
        args->interface = port;

        Timer *timer = new Timer(&IGMPClient::respondToQuery, args);
        timer->initialize(this);
        timer->schedule_after_msec(delay * 1000);

        // Cancel any pending responses to General Queries, there won't be more than one
        // We will delete the general timer on the same interface
        for (Vector < Pair < int, Timer * >> ::iterator it = general_timers.begin(); it != general_timers.end();
        ++it) {
            if (it->first == port) {
                general_timers.erase(it);
                break;
            }
        }

        general_timers.push_back(Pair<int, Timer *>(port, timer));
        return;
    }

    /**
     * 3. If the received Query is a Group-Specific Query or a Group-and-
     * Source-Specific Query and there is no pending response to a
     * previous Query for this group, then the group timer is used to
     * schedule a report. If the received Query is a Group-and-Source-
     * Specific Query, the list of queried sources is recorded to be used
     * when generating a response.
     */
    if (q->isGroupSpecificQuery() and not isPendingResponse(q->groupAddress)) {
        QueryResponseArgs *args = new QueryResponseArgs();
        args->query = q;
        args->client = this;
        args->interface = port;

        Timer *timer = new Timer(&IGMPClient::respondToQuery, args);
        timer->initialize(this);
        // This should be a group timer, but there is no group and interface
        // timer so the delay is used.
        timer->schedule_after_msec(delay * 1000);

        group_timers.push_back(std::make_tuple(port, timer, q->groupAddress));
        return;
    }

    /**
     * 4. If there already is a pending response to a previous Query
     * scheduled for this group, and either the new Query is a Group-
     * Specific Query or the recorded source-list associated with the
     * group is empty, then the group source-list is cleared and a single
     * response is scheduled using the group timer. The new response is
     * scheduled to be sent at the earliest of the remaining time for the
     * pending report and the selected delay.
     */
    if (isPendingResponse(q->groupAddress)
        and (q->isGroupSpecificQuery() or isSourceListEmpty(q->groupAddress, port))) {
        Vector <in_addr>* sourceList = getSourceList(q->groupAddress, port);
        if (not sourceList) {
            sourceList = new Vector<in_addr>{};
        }
        sourceList->clear();

        QueryResponseArgs *args = new QueryResponseArgs();
        args->query = q;
        args->client = this;
        args->interface = port;

        Timer *timer = new Timer(&IGMPClient::respondToQuery, args);
        timer->initialize(this);
        // This should be the earliest of the remaining time for the pending
        // report and the selected delay
        timer->schedule_after_msec(
                std::min(get_sec_before_expiry(getPendingResponseTimer(q->groupAddress)), delay));

        group_timers.push_back(std::make_tuple(port, timer, q->groupAddress));
        return;
    }
    return;
}

void IGMPClient::process_other_packet(Packet *, int) {
    click_chatter("\033[1;93m%-6s\033[0m %-6s", "Warning:",
                  "IGMP type is something else than a query (silently ignoring)");
    return;
}

void IGMPClient::respondToQuery(Timer *timer, void *thunk) {
    QueryResponseArgs *args = static_cast<QueryResponseArgs *>(thunk);
    // retrieve the original query message
    Query *q = args->query;
    IGMPClient *client = args->client;
    int interface = args->interface;
    Vector < GroupRecord * > group_records_to_send{};
    // Decide what to respond
    /**
     * When the timer in a pending response record expires, the system
     * transmits, on the associated interface, one or more Report messages
     * carrying one or more Current-State Records (see section 4.2.12), as
     * follows:
     */

    /**
     * 1. If the expired timer is the interface timer (i.e., it is a pending
     * response to a General Query), then one Current-State Record is
     * sent for each multicast address for which the specified interface
     * has reception state, as described in section 3.2. The Current-
     * State Record carries the multicast address and its associated
     * filter mode (MODE_IS_INCLUDE or MODE_IS_EXCLUDE) and source list.
     * Multiple Current-State Records are packed into individual Report
     * messages, to the extent possible.
     */
    if (client->isInterfaceTimer(timer)) {
        for (auto interface_record: client->interfaceMulticastTable->records) {
            // TODO fix the correct interface records
            // send current state record
            GroupRecord *groupRecord = client->createCurrentStateRecord(interface_record->multicast_address,
                                                                        interface_record->filter_mode,
                                                                        interface_record->source_list);
//            Report *report = new Report();
//            report->addGroupRecord(groupRecord);
//            Packet *p = report->createPacket();
//            client->output(interface).push(p);
            group_records_to_send.push_back(groupRecord);
        }
    }

    /**
     * 2. If the expired timer is a group timer and the list of recorded
     * sources for the that group is empty (i.e., it is a pending
     * response to a Group-Specific Query), then if and only if the
     * interface has reception state for that group address, a single
     * Current-State Record is sent for that address. The Current-State
     * Record carries the multicast address and its associated filter
     * mode (MODE_IS_INCLUDE or MODE_IS_EXCLUDE) and source list.
     */
    if (client->isGroupTimer(timer) and client->isSourceListEmpty(q->groupAddress, interface)) {
        for (auto interface_record: client->interfaceMulticastTable->records) {
            if (interface_record->multicast_address == q->groupAddress) {
                GroupRecord *groupRecord = client->createCurrentStateRecord(q->groupAddress,
                                                                            interface_record->filter_mode,
                                                                            interface_record->source_list);
//                Report *report = new Report();
//                report->addGroupRecord(groupRecord);
//                Packet *p = report->createPacket();
//                client->output(interface).push(p);
                group_records_to_send.push_back(groupRecord);
            }
        }
    }

    /**
     *  If the resulting Current-State Record has an empty set of source
     *  addresses, then no response is sent.
     */
    for (auto group_record: group_records_to_send) {
        if (group_record->isSourceAddressesEmpty() and group_record->record_type == Constants::MODE_IS_INCLUDE) {
            // Do nothing
            continue;
        }
        Report *report = new Report();
        report->addGroupRecord(group_record);
        // Generate the packet
        Packet *p = report->createPacket();
        // Send the packet
        client->output(interface).push(p);
    }

    /**
     * Finally, after any required Report messages have been generated, the
     * source lists associated with any reported groups are cleared.
     */
    for (auto group_record: group_records_to_send) {
        for (auto &interface_record: client->interfaceMulticastTable->records) {
            if (group_record->multicast_address == interface_record->multicast_address) {
                interface_record->source_list.clear();
                break;
            }
        }
    }

    // Remove the timer from the client
    // general
    bool found;
    do {
        found = false;
        for (Vector < Pair < int, Timer * >> ::iterator general_timer = client->general_timers.begin();
                general_timer != client->general_timers.end();
        ++general_timer) {
            if (general_timer->second == timer) {
                client->general_timers.erase(general_timer);
                found = true;
                break;
            }
        }
    } while (found);
    // group
    do {
        found = false;
        for (Vector < std::tuple < int, Timer *, in_addr >> ::iterator it = client->group_timers.begin();
                it != client->group_timers.end();
        ++it) {
            if (std::get<1>(*it) == timer) {
                client->group_timers.erase(it);
                found = true;
                break;
            }
        }
    } while (found);
}

void IGMPClient::respondToStateChange(Timer *timer, void *thunk) {
    StateChangeArgs *args = static_cast<StateChangeArgs *>(thunk);
    IGMPClient *client = args->client;
    int retransmit = args->retransmit - 1;
    args->retransmit = retransmit;
    Report *report = args->report;

    // Create the packet
    Packet *report_packet = report->createPacket();
    // Send report packet
    client->output(0).push(report_packet);

    // If we can't retransmit anymore stop sending
    if (retransmit == 0) {
        // Remove the timer from the list
        for (Vector < std::tuple < int, Timer *, in_addr >> ::iterator it = client->group_timers.begin(); it !=
                                                                                                          client->group_timers.end();
        ++it) {
            if (std::get<0>(*it) == 0 and std::get<1>(*it) == timer) {
                client->group_timers.erase(it);
                break;
            }
        }
        for (Vector < Pair < int, Timer * >> ::iterator it = client->general_timers.begin(); it !=
                                                                                             client->general_timers.end();
        ++it) {
            if (it->first == 0 and it->second == timer) {
                client->general_timers.erase(it);
                break;
            }
        }
        timer->unschedule();
        return;
    }

    // Set a seed
    srand48(time(0));
    double interval = drand48() * Defaults::UNSOLICITED_REPORT_INTERVAL;
    timer->schedule_after_msec(interval * 1000);
}

Timestamp IGMPClient::getShortestGeneralPendingResponse(int interface) {
    Timestamp current_expiry = Timestamp(2147483647U);
    for (auto general_timer : general_timers) {
        if (general_timer.first == interface) {
            continue;
        }
        if (general_timer.second->expiry() < current_expiry) {
            current_expiry = general_timer.second->expiry();
        }
    }
    return current_expiry;
}

bool IGMPClient::isShortestGeneralPendingResponse(int interface, Timestamp delay) {
    return getShortestGeneralPendingResponse(interface) < delay;
}

bool IGMPClient::isPendingResponse(in_addr group_address) {
    for (auto element: group_timers) {
        if (std::get<2>(element) == group_address) {
            return true;
        }
    }
    return false;
}

bool IGMPClient::isSourceListEmpty(in_addr group_address, int) {
    for (auto element: interfaceMulticastTable->records) {
        if (element->multicast_address == group_address) {
            return element->isSourceListEmpty();
        }
    }
    return true;
}

bool IGMPClient::isInterfaceTimer(Timer *timer) {
    for (auto general_timer: general_timers) {
        if (timer == general_timer.second) {
            return true;
        }
    }
    return false;
}

bool IGMPClient::isGroupTimer(Timer *timer) {
    for (auto group_timer: group_timers) {
        if (timer == std::get<1>(group_timer)) {
            return true;
        }
    }
    return false;
}

bool IGMPClient::isChangeInterfaceActive(in_addr interface) {
    for (auto active_interface: change_interface_active) {
        if (active_interface.first->interface == interface) {
            return true;
        }
    }
    return false;
}

bool IGMPClient::hasChangedState(int filter_mode, int old_state) {
    return filter_mode != old_state;
}

Vector <in_addr> *IGMPClient::getSourceList(in_addr group_address, int) {
    for (auto element: interfaceMulticastTable->records) {
        if (element->multicast_address == group_address) {
            return &element->source_list;
        }
    }
    return nullptr;
}

Timer *IGMPClient::getPendingResponseTimer(in_addr group_address) {
    Vector < std::tuple < int, Timer *, in_addr >> ::iterator
    group_timers_iterator;
    for (group_timers_iterator = group_timers.begin();
         group_timers_iterator != group_timers.end();
         ++group_timers_iterator) {
        if (std::get<2>(*group_timers_iterator) == group_address) {
            return std::get<1>(*group_timers_iterator);
        }
    }
    return nullptr;
}

StateChangeArgs *IGMPClient::getChangeInterfaceActiveArgs(in_addr interface) {
    return getChangeInterfaceActiveTuple(interface).first;
}

Timer *IGMPClient::getChangeInterfaceActiveTimer(in_addr interface) {
    return getChangeInterfaceActiveTuple(interface).second;
}

Pair<StateChangeArgs *, Timer *> IGMPClient::getChangeInterfaceActiveTuple(in_addr interface) {
    for (auto active_interface: change_interface_active) {
        if (active_interface.first->interface == interface) {
            return active_interface;
        }
    }
    return Pair<StateChangeArgs *, Timer *>(nullptr, nullptr);
}

void IGMPClient::removePendingResponse(in_addr group_address) {
    Vector < std::tuple < int, Timer *, in_addr >> ::iterator
    group_timers_iterator;
    for (group_timers_iterator = group_timers.begin();
         group_timers_iterator != group_timers.end();
         ++group_timers_iterator) {
        if (std::get<2>(*group_timers_iterator) == group_address) {
            break;
        }
    }
    // If no pending response has been found then skip the removing part
    if (group_timers_iterator == group_timers.end()) {
        return;
    }
    group_timers.erase(group_timers_iterator);
}

void IGMPClient::updateStateChangeReport(in_addr interface, in_addr multicast_addr, int filter_mode,
                                         Vector <in_addr> source_list) {
    // References RFC 3376, section 5.1.
    /**
     * The contents of the new transmitted report are calculated as follows.
     * As was done with the first report, the interface state for the
     * affected group before and after the latest change is compared. The
     * report records expressing the difference are built according to the
     * table above. However these records are not transmitted in a message
     * but instead merged with the contents of the pending report, to create
     * the new State-Change report. The rules for merging the difference
     * report resulting from the state change and the pending report are
     * described below.
     */
    int old_state = interfaceMulticastTable->filter_state(multicast_addr);
    filter_mode = new_filter_mode(old_state, filter_mode);
    // If no change has occurred then quit
    if (not hasChangedState(filter_mode, old_state)) {
        return;
    }

    GroupRecord *record = new GroupRecord(filter_mode, multicast_addr, source_list);
    Report *state_change_report = new Report();
    state_change_report->addGroupRecord(record);

    StateChangeArgs *args = getChangeInterfaceActiveArgs(interface);
    Timer *timer = getChangeInterfaceActiveTimer(interface);
    Report *pending_report = args->report;

    /**
     * The transmission of the merged State-Change Report terminates
     * retransmissions of the earlier State-Change Reports for the same
     * multicast address, and becomes the first of [Robustness Variable]
     * transmissions of State-Change Reports.
     */
    timer->unschedule();
    for (Vector < Pair < StateChangeArgs * , Timer * >> ::iterator it = change_interface_active.begin(); it !=
                                                                                                         change_interface_active.end();
    ++it) {
        if (it->second == timer) {
            change_interface_active.erase(it);
            break;
        }
    }

    /**
     * Each time a source is included in the difference report calculated
     * above, retransmission state for that source needs to be maintained
     * until [Robustness Variable] State-Change reports have been sent by
     * the host. This is done in order to ensure that a series of
     * successive state changes do not break the protocol robustness.
     */
    bool source_is_included = true;
    if (not source_is_included) {
        args->retransmit = Defaults::ROBUSTNESS_VARIABLE - 1;
    }

    /**
     * If the interface reception-state change that triggers the new report
     * is a filter-mode change, then the next [Robustness Variable] State-
     * Change Reports will include a Filter-Mode-Change record. This
     * applies even if any number of source-list changes occur in that
     * period. The host has to maintain retransmission state for the group
     * until the [Robustness Variable] State-Change reports have been sent.
     * When [Robustness Variable] State-Change reports with Filter-Mode-
     * Change records have been transmitted after the last filter-mode
     * change, and if source-list changes to the interface reception have
     * scheduled additional reports, then the next State-Change report will
     * include Source-List-Change records.
     */
    bool filter_mode_change = false;
    if (filter_mode_change) {
        // todo
    }

    /**
     * Each time a State-Change Report is transmitted, the contents are
     * determined as follows. If the report should contain a Filter-Mode-
     * Change record, then if the current filter-mode of the interface is
     * INCLUDE, a TO_IN record is included in the report, otherwise a TO_EX
     * record is included. The contents of these records are built
     * according to the table below.
     */
    /**
     * Record   Sources included
     * ------   ----------------
     * TO_IN    All in the current interface state that must be forwarded
     * TO_EX    All in the current interface state that must be blocked
     */
    Report *report = new Report();
    if (state_change_report->containsFilterModeChangeRecord()) {

        Vector <in_addr> source_list = interfaceMulticastTable->getRecord(multicast_addr)->source_list;
        GroupRecord *newGroupRecord = new GroupRecord(filter_mode, multicast_addr, source_list);
        if (interfaceMulticastTable->is_in(multicast_addr)) {
            newGroupRecord->record_type = Constants::CHANGE_TO_INCLUDE_MODE;
        } else {
            newGroupRecord->record_type = Constants::CHANGE_TO_EXCLUDE_MODE;
        }
        report->addGroupRecord(newGroupRecord);
    }
    return;
}

GroupRecord *
IGMPClient::createCurrentStateRecord(in_addr multicast_addr, int filter_mode, Vector <in_addr> source_list) {
    // Create a group record, with the current state
    GroupRecord *groupRecord = new GroupRecord(filter_mode, multicast_addr, source_list);

    return groupRecord;
}

void IGMPClient::push(int port, Packet *p) {
    if (port == 1) {
        click_chatter("\033[1;32mReceived a UDP packet on port %d\033[0m", port);
        process_udp(p);
        return;
    } else {
        click_chatter("\033[1;32mReceived a query on port %d\033[0m", port);
        QueryPacket *query = (QueryPacket *) (get_data_offset_4(p));
        if (query->type == Constants::QUERY_TYPE) {
            process_query(query, port);
            return;
        }

        /**
         * (Received IGMP messages of types other than Query are silently
         * ignored, except as required for interoperation with earlier versions
         * of IGMP.) (RFC 3376, section 5)
         *
         * We will give a warning that they are ignored, not quite the same
         * as the protocol. But the reasoning is here that we should be not-
         * ified when running, in order to show that it is working
         */
        if (query->type != Constants::QUERY_TYPE) {
            process_other_packet(p, port);
            return;
        }
    }
}

void IGMPClient::IPMulticastListen(int socket, in_addr interface, in_addr multicast_address, int filter_mode,
                                   Vector <in_addr> source_list) {
    // Note: source_list is always empty in the modified version of the rfc

    SocketRecord *socketRecord = new SocketRecord(interface, multicast_address, filter_mode, source_list);

    // required for report
    int old_state = interfaceMulticastTable->filter_state(multicast_address);
//    click_chatter("Old state was %d for ip addres:", old_state);
//    click_chatter(inadress_to_string(multicast_address).c_str());

    // Update it's own records
    // delete previous record if exists
    socketMulticastTable->delete_if_exists(interface, multicast_address);
    socketMulticastTable->addRecord(socketRecord);
    interfaceMulticastTable->update(socketMulticastTable);

    /**
     * If more changes to the same interface state entry occur before all
     * the retransmissions of the State-Change Report for the first change
     * have been completed, each such additional change triggers the
     * immediate transmission of a new State-Change Report.
     *
     * (RFC 3376, section 5.1.)
     */
    // TODO further implement this part, but for now skip
    if (isChangeInterfaceActive(interface)) {
//        updateStateChangeReport(interface, multicast_address, filter_mode, source_list);
//        return;
    }
    // Send report packet
    filter_mode = new_filter_mode(old_state, filter_mode);
    // If no change has occurred then quit
    if (not hasChangedState(filter_mode, old_state)) {
        return;
    }
    GroupRecord *record = new GroupRecord(filter_mode, multicast_address, source_list);
    Report *report = new Report();
    report->addGroupRecord(record);
    // Create the packet
    Packet *report_packet = report->createPacket();

    // Send the packet on port 0
    output(0).push(report_packet);
    /**
     * To cover the possibility of the State-Change Report being missed by
     * one or more multicast routers, it is retransmitted [Robustness
     * Variable] - 1 more times, at intervals chosen at random from the
     * range (0, [Unsolicited Report Interval]).
     *
     * (RFC 3376, section 5.1.)
     */
    // Seeding the random number generator based on the random time
    srand48(time(0));
    double interval = drand48() * Defaults::UNSOLICITED_REPORT_INTERVAL;
    int amount_of_retransmissions = Defaults::ROBUSTNESS_VARIABLE - 1;
    StateChangeArgs *args = new StateChangeArgs();
    args->client = this;
    args->retransmit = amount_of_retransmissions;
    args->report = report;

    Timer *timer = new Timer(&IGMPClient::respondToStateChange, args);
    timer->initialize(this);
    timer->schedule_after_msec(interval * 1000);

    group_timers.push_back(std::make_tuple(socket, timer, multicast_address));
    return;
}

int IGMPClient::new_filter_mode(int old_state, int new_state) {
    /**
     * A change of interface state causes the system to immediately transmit
     * a State-Change Report from that interface. The type and contents of
     * the Group Record(s) in that Report are determined by comparing the
     * filter mode and source list for the affected multicast address before
     * and after the change, according to the table below. If no interface
     * state existed for that multicast address before the change (i.e., the
     * change consisted of creating a new per-interface record), or if no
     * state exists after the change (i.e., the change consisted of deleting
     * a per-interface record), then the "non-existent" state is considered
     * to have a filter mode of INCLUDE and an empty source list.
     */
    if (old_state == Constants::MODE_IS_INCLUDE && new_state == Constants::MODE_IS_EXCLUDE) {
        return Constants::CHANGE_TO_EXCLUDE_MODE;
    } else if (old_state == Constants::MODE_IS_EXCLUDE && new_state == Constants::MODE_IS_INCLUDE) {
        return Constants::CHANGE_TO_INCLUDE_MODE;
    } else {
        return Constants::CHANGE_TO_INCLUDE_MODE;
    }
}

int join_leave_handle(int filter_mode, const String &conf, Element *e, void *, ErrorHandler *errh) {

    IGMPClient *igmpClient = (IGMPClient *) e;

    // Read input
    Vector <String> vconf;
    cp_argvec(conf, vconf);
    in_addr multicast_address;
    // In order to avoid using ADDRESS before the IP Adress we use read_mp
    // for positionally specified arguments
    if (Args(vconf, igmpClient, errh).read_mp("ADDRESS", multicast_address).complete() < 0)
        return -1;

    // Handle input
    igmpClient->IPMulticastListen(0, igmpClient->get_identifier(), multicast_address, filter_mode, Vector<in_addr>());

    return 0;
}

int join_handle(const String &conf, Element *e, void *thunk, ErrorHandler *errh) {
    /**
     * The Join operation is equivalent to
     * IPMulticastListen ( socket, interface, multicast-address, EXCLUDE, {} )
     * (rfc3376, 2)
     */
    return join_leave_handle(Constants::MODE_IS_EXCLUDE, conf, e, thunk, errh);
}

int leave_handle(const String &conf, Element *e, void *thunk, ErrorHandler *errh) {
    /**
     * the Leave operation is equivalent to:
     * IPMulticastListen ( socket, interface, multicast-address, INCLUDE, {} )
     * (rfc3376, 2)
     */
    return join_leave_handle(Constants::MODE_IS_INCLUDE, conf, e, thunk, errh);
}

int crash_handle(const String &, Element *, void *, ErrorHandler *) {
    // "Unexpectedly" crashes the client
    throw "Yeet";
}

String get_tables_handle(Element *e, void *) {
    IGMPClient *igmpClient = (IGMPClient *) e;
    SocketMulticastTable *socketMulticastTable = igmpClient->get_socket_multicast_table();\
    InterfaceMulticastTable *interfaceMulticastTable = igmpClient->get_interface_multicast_table();
    String socketString = socketMulticastTable->to_string();
    String interfaceString = interfaceMulticastTable->to_string();
    return socketString + "\n" + interfaceString;
}

void IGMPClient::add_handlers() {
    add_write_handler("join", &join_handle, (void *) 0);
    add_write_handler("leave", &leave_handle, (void *) 0);
    add_write_handler("crash", &crash_handle, (void *) 0);
    add_read_handler("tables", &get_tables_handle, 0);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPClient)

#include "SocketMulticastTable.cc"
#include "InterfaceMulticastTable.cc"
