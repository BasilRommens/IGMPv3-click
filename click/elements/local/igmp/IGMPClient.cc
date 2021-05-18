#include <clicknet/ip.h> // Must be above everything else otherwise this wouldn't work

#include <click/config.h>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/timer.hh>

#include "IGMPClient.hh"

#include "constants.hh"
#include "report.hh"
#include "query.hh"

CLICK_DECLS

IGMPClient::IGMPClient()
{
    interfaceMulticastTable = new InterfaceMulticastTable();
    socketMulticastTable = new SocketMulticastTable();
}

IGMPClient::~IGMPClient() { }

int IGMPClient::configure(Vector<String>& conf, ErrorHandler* errh)
{
    // TODO: parse config string
    return 0;
}

void IGMPClient::process_udp(Packet* p)
{
    click_chatter("It's UDP :-)");
    in_addr multicast_address;
    if (interfaceMulticastTable->is_ex(multicast_address)) {
        // forward packet
        output(2).push(p);
    }
    else {
        // drop packet
        output(1).push(p);
    }
}

void IGMPClient::process_query(QueryPacket* p, int port)
{
    // References RFC 3376, section 5.2
    /**
     * When a system receives a Query, it does not respond immediately.
     * Instead, it delays its response by a random amount of time, bounded
     * by the Max Resp Time value derived from the Max Resp Code in the
     * received Query message. A system may receive a variety of Queries on
     * different interfaces and of different kinds (e.g., General Queries,
     * Group-Specific Queries, and Group-and-Source-Specific Queries), each
     * of which may require its own delayed response.
     */

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

    /**
     * 2. If the received Query is a General Query, the interface timer is
     * used to schedule a response to the General Query after the
     * selected delay. Any previously pending response to a General
     * Query is canceled.
     */

    /**
     * 3. If the received Query is a Group-Specific Query or a Group-and-
     * Source-Specific Query and there is no pending response to a
     * previous Query for this group, then the group timer is used to
     * schedule a report. If the received Query is a Group-and-Source-
     * Specific Query, the list of queried sources is recorded to be used
     * when generating a response.
     */

    /**
     * 4. If there already is a pending response to a previous Query
     * scheduled for this group, and either the new Query is a Group-
     * Specific Query or the recorded source-list associated with the
     * group is empty, then the group source-list is cleared and a single
     * response is scheduled using the group timer. The new response is
     * scheduled to be sent at the earliest of the remaining time for the
     * pending report and the selected delay.
     */

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

    /**
     * 2. If the expired timer is a group timer and the list of recorded
     * sources for the that group is empty (i.e., it is a pending
     * response to a Group-Specific Query), then if and only if the
     * interface has reception state for that group address, a single
     * Current-State Record is sent for that address. The Current-State
     * Record carries the multicast address and its associated filter
     * mode (MODE_IS_INCLUDE or MODE_IS_EXCLUDE) and source list.
     */

    /**
     *  If the reulting Current-State Record has an empty set of source
     *  addresses, then no response is sent.
     */

    /**
     * Finally, after any required Report messages have been generated, the
     * source lists associated with any reported groups are cleared.
     */
    return;
}

void IGMPClient::process_other_packet(Packet* p, int port)
{
    return;
}

void IGMPClient::push(int port, Packet* p)
{
    click_chatter("Received packet on port %d :-)", port);

    if (p->transport_header()) {
        if (p->udp_header()) {
            process_udp(p);
        }
        else {
            click_chatter("It's not udp here");
        }
    }
    else {
        click_chatter("It doesn't contain a transport header");
        QueryPacket* query = (QueryPacket*) (p->data()-4);
        if (query->type==Constants::QUERY_TYPE) {
            process_query(query, port);
            return;
        }

        /**
         * (Received IGMP messages of types other than Query are silently
         * ignored, except as required for interoperation with earlier versions
         * of IGMP.) (RFC 3376, section 5)
         */
        if (query->type!=Constants::REPORT_TYPE) {
            process_other_packet(p, port);
            return;
        }
    }
    output(port).push(p);
}


void IGMPClient::IPMulticastListen(int socket, in_addr interface, in_addr multicast_address, int filter_mode,
        Vector<in_addr> source_list)
{
    SocketRecord* socketRecord = new SocketRecord(interface, multicast_address, filter_mode, source_list);

    // Update it's own records
    socketMulticastTable->addRecord(socketRecord);
    interfaceMulticastTable->update(socketMulticastTable);

    // Send report packet
    int old_state = interfaceMulticastTable->filter_state(multicast_address);
    filter_mode = new_filter_mode(old_state, filter_mode);
    GroupRecord* record = new GroupRecord(filter_mode, multicast_address, source_list);
    Report report = Report();
    report.addGroupRecord(record);
    Packet* report_packet = report.createPacket();
    IPAddress report_address = IPAddress("224.0.0.22");
    report_packet->set_dst_ip_anno(report_address);
    output(0).push(report_packet);
}

int IGMPClient::new_filter_mode(int old_state, int new_state)
{
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
    if (old_state==Constants::MODE_IS_INCLUDE && new_state==Constants::MODE_IS_EXCLUDE) {
        return Constants::CHANGE_TO_EXCLUDE_MODE;
    }
    else if (old_state==Constants::MODE_IS_EXCLUDE && new_state==Constants::MODE_IS_INCLUDE) {
        return Constants::CHANGE_TO_INCLUDE_MODE;
    }
    else {
        return Constants::CHANGE_TO_INCLUDE_MODE;
    }
}

int join_leave_handle(int filter_mode, const String& conf, Element* e, void* thunk, ErrorHandler* errh)
{

    IGMPClient* igmpClient = (IGMPClient*) e;

    // Read input
    Vector<String> vconf;
    cp_argvec(conf, vconf);
    in_addr multicast_address;
    // In order to avoid using ADDRESS before the IP Adress we use read_mp
    // for positionally specified arguments
    if (Args(vconf, igmpClient, errh).read_mp("ADDRESS", multicast_address).complete()<0)
        return -1;

    // Handle input
    igmpClient->IPMulticastListen(0, igmpClient->get_identifier(), multicast_address, filter_mode, Vector<in_addr>());

    return 0;
}

int join_handle(const String& conf, Element* e, void* thunk, ErrorHandler* errh)
{
    /**
     * The Join operation is equivalent to
     * IPMulticastListen ( socket, interface, multicast-address, EXCLUDE, {} )
     * (rfc3376, 2)
     */
    return join_leave_handle(Constants::MODE_IS_EXCLUDE, conf, e, thunk, errh);
}

int leave_handle(const String& conf, Element* e, void* thunk, ErrorHandler* errh)
{
    /**
     * the Leave operation is equivalent to:
     * IPMulticastListen ( socket, interface, multicast-address, INCLUDE, {} )
     * (rfc3376, 2)
     */
    return join_leave_handle(Constants::MODE_IS_INCLUDE, conf, e, thunk, errh);
}

String get_tables_handle(Element* e, void* thunk)
{
    IGMPClient* igmpClient = (IGMPClient*) e;
    SocketMulticastTable* socketMulticastTable = igmpClient->get_socket_multicast_table();\
    InterfaceMulticastTable* interfaceMulticastTable = igmpClient->get_interface_multicast_table();
    String socketString = socketMulticastTable->to_string();
    String interfaceString = interfaceMulticastTable->to_string();
    return socketString+"\n"+interfaceString;
}

void IGMPClient::add_handlers()
{
    add_write_handler("join", &join_handle, (void*) 0);
    add_write_handler("leave", &leave_handle, (void*) 0);
    add_read_handler("tables", &get_tables_handle, 0);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPClient)

#include "SocketMulticastTable.cc"
#include "InterfaceMulticastTable.cc"
