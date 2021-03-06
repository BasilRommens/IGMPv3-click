#include <clicknet/ip.h> // Must be above everything else otherwise this wouldn't work

#include <click/config.h>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/packet.hh>
#include <click/timer.hh>

#include "IGMPRouter.hh"
#include "constants.hh"
#include "report.hh"
#include "query.hh"
#include "helper.hh"

/**
* Conceptually, when a group record is received, the router filter-mode
 * for that group is updated to cover all the requested sources using
 * the least amount of state. As a rule, once a group record with a
 * filter-mode of EXCLUDE is received, the router filter-mode for that
 * group will be EXCLUDE.
*/

/**
 * When a router filter-mode for a group is EXCLUDE, the source record
 * list contains two types of sources. The first type is the set which
 * represents conflicts in the desired reception state; this set must be
 * forwarded by some router on the network. The second type is the set
 * of sources which hosts have requested to not be forwarded. Appendix
 * A describes the reasons for keeping this second set when in EXCLUDE
 * mode.
*/

CLICK_DECLS

// TODO: Querier elections to determine who must send general queries

IGMPRouter::IGMPRouter() {
    group_states = Vector < Pair < int, GroupState * >> ();
}

IGMPRouter::~IGMPRouter() {}


int IGMPRouter::configure(Vector <String> & conf, ErrorHandler * errh) {
    // Periodically sent general queries
    GeneralQueryTimerArgs *args = new GeneralQueryTimerArgs();
    args->router = this;

    Timer *timer = new Timer(&IGMPRouter::send_general_queries, args);
    timer->initialize(this);
    timer->schedule_after_msec(Defaults::QUERY_INTERVAL * 1000);

    if (conf.size() != noutputs() - 2)
        return errh->error("need %d arguments, one per output port", noutputs() - 2);

    for (int i = 0; i < conf.size(); i++) {
        int temp;
        sscanf(conf[i].c_str(), "%d", &temp);
        attached_networks.push_back(temp);
    }
    return 0;
}

void IGMPRouter::send_general_queries(Timer *timer, void *thunk) {
    // TODO: Election? -> Skip aangezien er hier maar 1 router per subnet is
    click_chatter("\e[1;35m%-6s\e[m", "Sending general queries");

    GeneralQueryTimerArgs *args = static_cast<GeneralQueryTimerArgs *>(thunk);
    IGMPRouter *router = args->router;

    // Send the queries
    for (auto port: router->get_attached_networks()) {
        Packet *query = router->get_general_query();
        router->output(port).push(query);
    }
    // Restart the timer
    timer->reschedule_after_msec(Defaults::QUERY_INTERVAL * 1000);
}


Vector<int> IGMPRouter::get_attached_networks() {
    // TODO: klopt dit?
    //  smh hoe moet ik een vector met een initializer list aanmaken?
    return attached_networks;
}

Packet *IGMPRouter::get_general_query() {
    Query query = Query();
    query.setMaxRespCode(Defaults::MAX_RESPONSE_CODE);
    query.setQRV(Defaults::ROBUSTNESS_VARIABLE);
    query.setQQICFromValue(Defaults::QUERY_RESPONSE_INTERVAL);

    /**
     * The Group Address field is set to zero when sending a General Query,
     * and set to the IP multicast address being queried when sending a
     * Group-Specific Query or Group-and-Source-Specific Query (see section
     * 4.1.9, below).
     *
     * RFC 3376 4.1.3
     */
    query.setGroupAddress(IPAddress(0));

    Packet *query_packet = query.createPacket();

    return query_packet;
}

Pair<int, GroupState *> IGMPRouter::get_group_state(in_addr multicast_address, int port) {
    for (auto groupState: group_states) {
        if (groupState.first == port && groupState.second->multicast_address == multicast_address) {
            return groupState;
        }
    }
    return Pair<int, GroupState *>(0, nullptr);
}

Vector <Pair<int, GroupState *>> IGMPRouter::get_group_state_list(in_addr multicast_address) {
    Vector < Pair < int, GroupState * >> group_state_list = Vector < Pair < int, GroupState * >> ();
    for (auto groupState: group_states) {
        if (groupState.second->multicast_address == multicast_address) {
            group_state_list.push_back(groupState);
        }
    }
    return group_state_list;
}

int IGMPRouter::get_current_state(in_addr multicast_address, int port) {
    Pair < int, GroupState * > groupState = get_group_state(multicast_address, port);
    if (groupState) {
        return groupState.second->filter_mode;
    } else {
        return Constants::MODE_IS_INCLUDE;
    }
}

void IGMPRouter::to_in(in_addr multicast_address, int port) {
//    click_chatter("\e[1;34m%-6s\e[m", "Changing to in");
    update_filter_mode(multicast_address, Constants::MODE_IS_INCLUDE, port);
}

void IGMPRouter::to_ex(in_addr multicast_address, int port) {
//    click_chatter("\e[1;34m%-6s\e[m", "Changing to ex");
    update_filter_mode(multicast_address, Constants::MODE_IS_EXCLUDE, port);
}

Vector<SourceRecord *> IGMPRouter::to_vector(in_addr in_addr_array[], uint16_t length) {
    Vector < SourceRecord * > array_vector = Vector<SourceRecord *>();
    array_vector.reserve(length);
    for (uint16_t idx = 0; idx < length; idx++) {
        array_vector.push_back(new SourceRecord(in_addr_array[idx], 0));
    }
    return array_vector;
}

Vector<SourceRecord *>
IGMPRouter::vector_union(Vector<SourceRecord *> first, Vector<SourceRecord *> second) {
    Vector < SourceRecord * > union_vector = first;
    for (auto second_addr: second) {
        bool already_in_list = false;
        for (auto first_addr: first) {
            if (first_addr->source_address == second_addr->source_address) {
                already_in_list = true;
                break;
            }
        }
        if (not already_in_list) {
            union_vector.push_back(second_addr);
        }
    }
    return union_vector;
}

Vector<SourceRecord *>
IGMPRouter::vector_difference(Vector<SourceRecord *> first, Vector<SourceRecord *> second) {
    Vector < SourceRecord * > difference_vector = Vector<SourceRecord *>();
    for (auto first_addr: first) {
        bool remove_from_list = false;
        for (auto second_addr: second) {
            if (first_addr->source_address == second_addr->source_address) {
                remove_from_list = true;
                break;
            }
        }
        if (not remove_from_list) {
            difference_vector.push_back(first_addr);
        }
    }
    return difference_vector;
}

Vector<SourceRecord *>
IGMPRouter::vector_intersection(Vector<SourceRecord *> first, Vector<SourceRecord *> second) {
    Vector < SourceRecord * > intersection_vector = Vector<SourceRecord *>();
    for (auto second_addr: second) {
        bool add_to_list = true;
        for (auto first_addr: first) {
            if (first_addr->source_address == second_addr->source_address) {
                add_to_list = false;
                break;
            }
        }
        if (add_to_list) {
            intersection_vector.push_back(second_addr);
        }
    }
    return intersection_vector;
}

Vector<SourceRecord *> IGMPRouter::get_strict_positive_timer(GroupState *groupState) {
    return groupState->source_records;
}

Vector<SourceRecord *> IGMPRouter::get_null_timer(GroupState *groupState) {
    return groupState->source_records;
}

Pair<int, GroupState *> IGMPRouter::get_or_create_group_state(in_addr multicast_address, int port) {
    Pair < int, GroupState * > groupState = get_group_state(multicast_address, port);
    if (!groupState) {
        groupState = Pair<int, GroupState *>(port, new GroupState(multicast_address));
        // Append the new group state because we just created it
        group_states.push_back(groupState);
    }
    return groupState;
}

bool IGMPRouter::is_packet_source_in_group_state(GroupState *group_state, Packet *p) {
    // Fetch the ip src address this is needed
    in_addr ip_src = p->ip_header()->ip_src;
    // Loops over all the source records and checks if the packet source
    // address is found if it is, then return true otherwise continue with
    // the loop. At the end if no ip_src is found return false.
    for (auto source_record: group_state->source_records) {
        if (source_record->source_address == ip_src) {
            return true;
        }
    }
    return false;
}

bool IGMPRouter::should_forward_udp(GroupState *group_state, Packet *p) {
    // Check the current mode and decide whether or not to forward the UPD packet
    if (group_state->filter_mode == Constants::MODE_IS_INCLUDE) {
        return is_packet_source_in_group_state(group_state, p);
    } else if (group_state->filter_mode == Constants::MODE_IS_EXCLUDE) {
        return not is_packet_source_in_group_state(group_state, p);
    }
    // This is just for warnings, this should never be triggered
    click_chatter("\e[1;31m%-6s\e[m", "Group state has wrong state.");
    return false;
}

void IGMPRouter::update_filter_mode(in_addr multicast_address, int filter_mode, int port) {
    Pair < int, GroupState * > groupState = get_or_create_group_state(multicast_address, port);
    groupState.second->filter_mode = filter_mode;
//    click_chatter("\e[1;34m%-6s\e[m", "Updated filter mode");
}

void IGMPRouter::process_udp(Packet *p, int port) {
    click_chatter("\e[1;32m%-6s %d\e[m", "Received UDP packet on port", port);
    const click_ip *ip_header = p->ip_header();
    Vector <Pair<int, GroupState *>> port_groups = get_group_state_list(ip_header->ip_dst);
    for (auto port_group: port_groups) {
        if (should_forward_udp(port_group.second, p)) {
            output(port_group.first).push(p->clone());
            click_chatter("\e[1;35m%-6s %d\e[m", "Forwarded UDP packet on port", port_group.first);
        }
    }
}

void IGMPRouter::process_query(QueryPacket *query, int port) {
    click_chatter("\e[1;32m%-6s %d\e[m", "Received query on port %d", port);
    if (not query->to_query()->hasCorrectChecksum()) {
        click_chatter("\033[1;93m%-6s\033[0m %-6s", "Warning:", "Faulty checksum in Query");
        return;
    }
    // rfc 6.6.1
    /**
     * When a router sends or receives a query with the Suppress Router-Side
     * Processing flag set, it will not update its timers.
     */
    if (query->getSFlag()) {
        return;
    }

    /**
     * When a router sends or receives a query with a clear Suppress
     * Router-Side Processing flag, it must update its timers to reflect the
     * correct timeout values for the group or sources being queried. The
     * following table describes the timer actions when sending or receiving
     * a Group-Specific or Group-and-Source Specific Query with the Suppress
     * Router-Side Processing flag not set.
     *
     * Query | Action
     * --------------
     * Q(G)  | Group Timer is lowered to LMQT
     */
    set_group_timer_lmqt(query->groupAddress, port); // TODO: Enkel naar deze poort? -> Ik gok van wel


}

void
IGMPRouter::process_in_report_in(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record) {
    // New state Include (A+B)
    to_in(groupRecord.multicast_address, port);
    // Router state
    Vector < SourceRecord * > A = router_record.second->source_records;
    // Report record
    Vector < SourceRecord * > B = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
    router_record.second->source_records = vector_union(A, B);
}

void
IGMPRouter::process_in_report_ex(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record) {
    // New state Exclude (A*B, B-A)
    to_ex(groupRecord.multicast_address, port);
    // Router state
    Vector < SourceRecord * > A = router_record.second->source_records;
    // Report record
    Vector < SourceRecord * > B = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
    Vector < SourceRecord * > first = vector_intersection(A, B);
    Vector < SourceRecord * > second = vector_difference(B, A);
}


void
IGMPRouter::process_ex_report_in(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record) {

    // New state Exclude(X+A, Y-A)
    to_ex(groupRecord.multicast_address, port);
    // Router state
    // Timer >0
    Vector < SourceRecord * > X = router_record.second->source_records;
    // Timer =0
    Vector < SourceRecord * > Y = Vector<SourceRecord *>();
    // Report record
    Vector < SourceRecord * > A = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
    Vector < SourceRecord * > first = vector_union(X, A);
    Vector < SourceRecord * > second = vector_difference(Y, A);
}


void
IGMPRouter::process_ex_report_ex(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record) {
    // New state Exclude(A-Y, Y*A)
    to_ex(groupRecord.multicast_address, port);
    // Router state
    // Timer >0
    Vector < SourceRecord * > X = router_record.second->source_records;
    // Timer =0
    Vector < SourceRecord * > Y = Vector<SourceRecord *>();
    // Report record
    Vector < SourceRecord * > A = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
    Vector < SourceRecord * > first = vector_difference(A, Y);
    Vector < SourceRecord * > second = vector_intersection(Y, A);
    // (A-X-Y)=GMI, Delete(X-A), Delete(Y-A), GroupTimer=GMI
}


void
IGMPRouter::process_in_report_cex(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record) {
    // New state Exclude (A*B, B-A)
    to_ex(groupRecord.multicast_address, port);
    // Router state
    Vector < SourceRecord * > A = router_record.second->source_records;
    // Report record
    Vector < SourceRecord * > B = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
    // (B-A)=0, Delete (A-B), Send Q(G, A*B), Group Timer=GMI
}

void
IGMPRouter::process_ex_report_cin(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record) {
    /**
     * When a group membership is terminated at a system or traffic from a
     * particular source is no longer desired,
     */

    /**
     * a multicast router must query
     * for other members of the group or listeners of the source before
     * deleting the group (or source) and pruning its traffic.
     *
     * Group-Specific Queries are
     * sent when a router receives a State-Change record indicating a system
     * is leaving a group.
     */

    /**
     * Similarly, when a router queries a specific group, it lowers its
     * group timer for that group to a small interval of Last Member Query
     * Time seconds. If any group records expressing EXCLUDE mode interest
     * in the group are received within the interval, the group timer for
     * the group is updated and the suggestion to the routing protocol to
     * forward the group stands without any interruption.
     */

    return;

    // Hier nog niet direct to ex gaan, maar eerst een group_specific_query sturen

    // New state Exclude (X+A, Y-A)
    to_ex(groupRecord.multicast_address, port);
    // Router state
    // Timer >0
    Vector < SourceRecord * > X = router_record.second->source_records;
    // Timer =0
    Vector < SourceRecord * > Y = Vector<SourceRecord *>();
    // Report record
    Vector < SourceRecord * > A = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
    // (A)=GMI, Send Q(G, X-A), Send Q(G)
}

void IGMPRouter::handle_expired_group_timer(Timer *timer, void *thunk) {
    /**
     * A group timer expiring when a router filter-mode for the group is
     * EXCLUDE means there are no listeners on the attached network in
     * EXCLUDE mode. At this point, a router will transition to INCLUDE
     * filter-mode. Section 6.5 describes the actions taken when a group
     * timer expires while in EXCLUDE mode. (rfc3376, 6.2.2)
     */
    click_chatter("Group timer possibly expired");
    // Get the args
    GroupTimerArgs *args = static_cast<GroupTimerArgs *>(thunk);

    IGMPRouter *router = args->router;
    int port = args->port;
    in_addr address = args->multicast_address;

    // Get the group state
    Pair < int, GroupState * > router_record = router->get_group_state(address, port);
    GroupState *groupState = router_record.second;

    // Check if this expired timer is the most recent timer
    if (groupState->group_timer != timer) {
        // There has been set a new timer, this one can be ignored
        click_chatter("Not expired, group timer has still %f s left",
                      get_sec_before_expiry(groupState->group_timer));
        return;
    }

    // Timer has expired, group can leave
    groupState->filter_mode = Constants::MODE_IS_INCLUDE;

    click_chatter("Group left :-(");
}

void IGMPRouter::set_group_timer(in_addr multicast_address, int port, int duration) {
    // Updates the timer of the group with given multicast_addrefss on given port to given time
    // Duration should be seconds
    Pair < int, GroupState * > router_record = get_group_state(multicast_address, port);
    GroupState *groupState = router_record.second;

    GroupTimerArgs *timerArgs = new GroupTimerArgs();
    timerArgs->multicast_address = multicast_address;
    timerArgs->port = port;
    timerArgs->router = this;

    Timer *timer = new Timer(&IGMPRouter::handle_expired_group_timer, timerArgs);
    timer->initialize(this);
    timer->schedule_after_msec(duration * 1000);

    groupState->group_timer = timer;
//    click_chatter("Set a new group timer, still %d seconds left", get_sec_before_expiry(timer));
}

Timer *IGMPRouter::get_group_timer(in_addr group_address, int port) {
    Pair<int, GroupState *> groupState = get_group_state(group_address, port);
    GroupState *groupRecord = groupState.second;
    return groupRecord->group_timer;
}

void IGMPRouter::set_group_timer_lmqt(in_addr multicast_address, int port) {
    // Updates the timer of the group with given multicast_addrefss on given port to the last member query time
    int lmqt = Defaults::LAST_MEMBER_QUERY_TIME;
    set_group_timer(multicast_address, port, lmqt);
    click_chatter("\033[1;34mSet group timer to LMQT (still %f seconds left)\033[0m",
                  get_sec_before_expiry(get_group_timer(multicast_address, port)));
}

void IGMPRouter::set_group_timer_gmi(in_addr multicast_address, int port) {
    // Updates the timer of the group with given multicast_addrefss on given port to the Group Membership Interval
    int gmi = Defaults::GROUP_MEMBERSHIP_INTERVAL;
    set_group_timer(multicast_address, port, gmi);
    click_chatter("\033[1;34mSet group timer to GMI (still %f seconds left)\033[0m",
                  get_sec_before_expiry(get_group_timer(multicast_address, port)));
}

void IGMPRouter::update_router_state(GroupRecordPacket &groupRecord, int port) {

    int report_recd_mode = groupRecord.record_type;
    Pair < int, GroupState * > router_record = get_or_create_group_state(groupRecord.multicast_address, port);
    int router_state = get_current_state(groupRecord.multicast_address, port);

    // Action table rfc3376, p.31
    if (router_state == Constants::MODE_IS_INCLUDE && report_recd_mode == Constants::MODE_IS_INCLUDE) {
        process_in_report_in(groupRecord, port, router_record);
    } else if (router_state == Constants::MODE_IS_INCLUDE && report_recd_mode == Constants::MODE_IS_EXCLUDE) {
        process_in_report_ex(groupRecord, port, router_record);
        set_group_timer_gmi(groupRecord.multicast_address, port);
    } else if (router_state == Constants::MODE_IS_EXCLUDE && report_recd_mode == Constants::MODE_IS_INCLUDE) {
        process_ex_report_in(groupRecord, port, router_record);
    } else if (router_state == Constants::MODE_IS_EXCLUDE && report_recd_mode == Constants::MODE_IS_EXCLUDE) {
        process_in_report_ex(groupRecord, port, router_record);
        set_group_timer_gmi(groupRecord.multicast_address, port);
    } // RFC 3376 section 6.4
    else if (router_state == Constants::MODE_IS_INCLUDE && report_recd_mode == Constants::CHANGE_TO_EXCLUDE_MODE) {
        process_in_report_cex(groupRecord, port, router_record);
        set_group_timer_gmi(groupRecord.multicast_address, port);
    } else if (router_state == Constants::MODE_IS_EXCLUDE &&
               report_recd_mode == Constants::CHANGE_TO_INCLUDE_MODE) {
        // TODO: Merge queries if already pending queries
        click_chatter("\033[1;34mReceived request to leave\033[0m");
//        process_ex_report_cin(groupRecord, port, router_record);
        send_group_specific_query(groupRecord.multicast_address, port);
        set_group_timer_lmqt(groupRecord.multicast_address, port);
    } else {
//        click_chatter("\e[1;93m%-6s %d %-6s %d \e[m",
//                      "Hmmm, not found. Router is in state",
//                      router_state,
//                      "and report contains state ",
//                      report_recd_mode);
    }
}

Vector<int> IGMPRouter::get_group_members(in_addr multicast_address) {
    Vector <Pair<int, GroupState *>> port_groups = get_group_state_list(multicast_address);
    Vector<int> members;
    for (auto port_group: port_groups) {
        if (port_group.second->filter_mode == Constants::MODE_IS_EXCLUDE) {
            // still interested, so it's a member
            members.push_back(port_group.first);
        }
    }
    return members;
}

Packet *
IGMPRouter::create_group_specific_query_packet(in_addr multicast_address, bool suppress_flag, int time_until_send) {
    Query query = Query();
    query.setGroupAddress(multicast_address);
    query.setSFlag(suppress_flag);
    query.setMaxRespCodeFromTime(Defaults::QUERY_RESPONSE_INTERVAL - time_until_send);
    query.setQRV(Defaults::ROBUSTNESS_VARIABLE);
    query.setQQICFromValue(Defaults::QUERY_RESPONSE_INTERVAL);

    Packet *query_packet = query.createPacket();

    IPAddress query_address = IPAddress(multicast_address);
    query_packet->set_dst_ip_anno(query_address);
    // TODO: if group_timer larger than LMQT, set suppress router side processing bit (6.6.3.1)

    return query_packet;
}

void IGMPRouter::send_group_specific_query(in_addr multicast_address, int port) {
    click_chatter("Sending group specific query...");
    /**
     * When a table action "Send Q(G)" is encountered, then the group timer
     * must be lowered to LMQT. The router must then immediately send a
     * group specific query as well as schedule [Last Member Query Count -
     * 1] query retransmissions to be sent every [Last Member Query
     * Interval] over [Last Member Query Time].
     * When transmitting a group specific query, if the group timer is
     * larger than LMQT, the "Suppress Router-Side Processing" bit is set in
     * the query message.
     */
    Pair<int, GroupState *> group_state_pair = get_group_state(multicast_address, port);

    /**
     * When transmitting a group specific query, if the group timer is
     * larger than LMQT, the "Suppress Router-Side Processing" bit is set in
     * the query message.
     */
    bool suppress_flag = false;
    GroupState *groupState = group_state_pair.second;
    Timer *group_timer = groupState->group_timer;
    if (group_timer != nullptr) {
        int time_left = get_sec_before_expiry(group_timer);
        // click_chatter("Group timer firing in %d msec", time_left);
        if ((time_left / 10) > Defaults::LAST_MEMBER_QUERY_INTERVAL) {
            suppress_flag = true;
        }
    }

    // Set group timer to LMQT
    // Commented because not always LMQT, sometimes GMI -> Responsibility of the caller to update this
    // set_group_timer_lmqt(multicast_address, group_state_pair.first);


    // Merge with previous queries -> A.k.a. remove previously scheduled queries
    stop_scheduled_retransmissions(multicast_address, port);

    // Schedule query retransmissions
    for (int query_num = 0; query_num < Defaults::LAST_MEMBER_QUERY_COUNT; ++query_num) {

        int time_until_send = Defaults::LAST_MEMBER_QUERY_INTERVAL * query_num; // in 1/10 seconds

        ScheduledQueryTimerArgs *timerArgs = new ScheduledQueryTimerArgs();
        timerArgs->multicast_address = multicast_address;
        timerArgs->port = port;
        timerArgs->packet_to_send = create_group_specific_query_packet(multicast_address, suppress_flag,
                                                                       time_until_send);
        timerArgs->router = this;

        Timer *timer = new Timer(&IGMPRouter::send_scheduled_query, timerArgs);
        timer->initialize(this);
        timer->schedule_after_msec(time_until_send * 100);
        Pair<in_addr,int> identifier = Pair<in_addr,int>(multicast_address, port);
        query_timers.push_back(Pair<Pair<in_addr,int>, Timer *>(identifier, timer));
    }

}

void IGMPRouter::send_to_all_group_members(Packet *packet, in_addr group_address) {
    for (int port: get_group_members(group_address)) {
        output(port).push(packet);
//        click_chatter("\033[1;35mQuery sent on port %d\033[0m", port);
    }
}

void IGMPRouter::send_scheduled_query(Timer *timer, void *thunk) {
    click_chatter("\033[1;35mSending scheduled query\033[0m");

    ScheduledQueryTimerArgs *args = static_cast<ScheduledQueryTimerArgs *>(thunk);

    IGMPRouter *router = args->router;
    Packet *packet = args->packet_to_send;
    in_addr address = args->multicast_address;
    int port = args->port;


    if (router->is_timer_canceled(timer)) {
        return;
    }

    router->output(port).push(packet);
}


void IGMPRouter::process_group_record(GroupRecordPacket &groupRecord, int port) {

    // TODO check if this is the right implementation
    int report_recd_mode = groupRecord.record_type;
    update_router_state(groupRecord, port);


//        // Action tabel rfc3376 p.29 (dingen die rekenen op timer nog niet geimplementeerd)
//        if(groupRecord.report_type == Constants::MODE_IS_INCLUDE && groupRecord.num_sources == 0){
//            // suggest to not forward source
//        } else if (groupRecord.report_type == Constants::MODE_IS_EXCLUDE && groupRecord.num_sources == 0) {
//            // suggest to forward traffic from source
//        }
}

void IGMPRouter::process_report(ReportPacket *report, int port) {
    if (not report->hasCorrectChecksum()) {
        click_chatter("\033[1;93m%-6s\033[0m %-6s", "Warning:", "Faulty checksum in Report");
        return;
    }
    click_chatter("\e[1;32m%-6s %d\e[m", "Received report on port", port);

    for (int i = 0; i < ntohs(report->num_group_records); i++) {
        // click_chatter("\e[1;34m%-6s %d\e[m", "Processing group record", i);
        GroupRecordPacket groupRecord = report->group_records[i];
        process_group_record(groupRecord, port);
    }

//    click_chatter("\e[1;34m%-6s\e[m", "Done processing report");
}

void IGMPRouter::push(int port, Packet *p) {
    /**
     * When a multicast router receives a datagram from a source destined to
     * a particular group, a decision has to be made whether to forward the
     * datagram onto an attached network or not. The multicast routing
     * protocol in use is in charge of this decision, and should use the
     * IGMPv3 information to ensure that all sources/groups desired on a
     * subnetwork are forwarded to that subnetwork. IGMPv3 information does
     * not override multicast routing information; for example, if the
     * IGMPv3 filter-mode group for G is EXCLUDE, a router may still forward
     * packets for excluded sources to a transit subnet.
     */

    // Poort 3 -> UDP
    // Andere poort -> Onderscheid maken tussen query en report
    // Poort 0: Server
    // Poort 1: Eerste client
    // Poort 2: Tweede client

//    click_chatter("\e[1;32m%-6s %d\e[m", "Received packet on port", port);

    if (port == 3) {
        process_udp(p, port);
        return;
    }

    ReportPacket *report = (ReportPacket *) (get_data_offset_4(p));

    if (report->type == Constants::REPORT_TYPE) {
        process_report(report, port);
        return;
    }

    QueryPacket *query = (QueryPacket *) (get_data_offset_4(p));
    if (query->type == Constants::QUERY_TYPE) {
        process_query(query, port);
        return;
    }

    // silently ignore other packets
}

void IGMPRouter::change_group_to_exclude(int port, in_addr group_addr) {
    Pair < int, GroupState * > groupState = get_group_state(group_addr, port);
    groupState.second->filter_mode = Constants::MODE_IS_EXCLUDE;
}


void IGMPRouter::stop_scheduled_retransmissions(in_addr multicast_address, int port) {
    for (int i = 0; i < query_timers.size(); ++i) {
        Pair<in_addr, int> identifier = query_timers[i].first;
        in_addr group_address = identifier.first;
        int port_timer = identifier.second;
        if (group_address == multicast_address && port == port_timer) {
            query_timers.erase(query_timers.begin() + i);
            --i;
        }
    }
}

bool IGMPRouter::is_timer_canceled(Timer *timer) {
    for (auto query_timer: query_timers) {
        if (query_timer.second == timer) {
            return false;
        }
    }
    click_chatter("\033[1;93m%-6s\033[0m %-6s", "Warning:", "Timer not triggered because merged with other query");
    return true;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPRouter)

#include "constants.cc"