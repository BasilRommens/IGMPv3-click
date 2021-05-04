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

IGMPRouter::IGMPRouter()
{
    group_states = Vector<Pair<int, GroupState* >> ();
}

IGMPRouter::~IGMPRouter()
{

}

Pair<int, GroupState*> IGMPRouter::get_group_state(in_addr multicast_address, int port)
{
    for (auto groupState: group_states) {
        if (groupState.first==port && groupState.second->multicast_address==multicast_address) {
            return groupState;
        }
    }
    return Pair<int, GroupState*>(0, nullptr);
}

Vector<Pair<int, GroupState*>> IGMPRouter::get_group_state_list(in_addr multicast_address)
{
    Vector<Pair<int, GroupState* >> group_state_list = Vector<Pair<int, GroupState* >> ();
    for (auto groupState: group_states) {
        if (groupState.second->multicast_address==multicast_address) {
            group_state_list.push_back(groupState);
        }
    }
    return group_state_list;
}

int IGMPRouter::get_current_state(in_addr multicast_address, int port)
{
    Pair<int, GroupState*>groupState = get_group_state(multicast_address, port);
    if (groupState) {
        return groupState.second->filter_mode;
    }
    else {
        return Constants::MODE_IS_INCLUDE;
    }
}

void IGMPRouter::to_in(in_addr multicast_address, int port)
{
    click_chatter("\e[1;34m%-6s\e[m", "Changing to in");
    update_filter_mode(multicast_address, Constants::MODE_IS_INCLUDE, port);
}

void IGMPRouter::to_ex(in_addr multicast_address, int port)
{
    click_chatter("\e[1;34m%-6s\e[m", "Changing to ex");
    update_filter_mode(multicast_address, Constants::MODE_IS_EXCLUDE, port);
}

Vector<SourceRecord*> IGMPRouter::to_vector(in_addr in_addr_array[], uint16_t length)
{
    Vector<SourceRecord*>array_vector = Vector<SourceRecord*>();
    array_vector.reserve(length);
    for (uint16_t idx = 0; idx<length; idx++) {
        array_vector.push_back(new SourceRecord(in_addr_array[idx], 0));
    }
    return array_vector;
}

Vector<SourceRecord*>
IGMPRouter::vector_union(Vector<SourceRecord*> first, Vector<SourceRecord*> second)
{
    Vector<SourceRecord*>union_vector = first;
    for (auto second_addr: second) {
        bool already_in_list = false;
        for (auto first_addr: first) {
            if (first_addr->source_address==second_addr->source_address) {
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

Vector<SourceRecord*>
IGMPRouter::vector_difference(Vector<SourceRecord*> first, Vector<SourceRecord*> second)
{
    Vector<SourceRecord*>difference_vector = Vector<SourceRecord*>();
    for (auto first_addr: first) {
        bool remove_from_list = false;
        for (auto second_addr: second) {
            if (first_addr->source_address==second_addr->source_address) {
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

Vector<SourceRecord*>
IGMPRouter::vector_intersection(Vector<SourceRecord*> first, Vector<SourceRecord*> second)
{
    Vector<SourceRecord*>intersection_vector = Vector<SourceRecord*>();
    for (auto second_addr: second) {
        bool add_to_list = true;
        for (auto first_addr: first) {
            if (first_addr->source_address==second_addr->source_address) {
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

Vector<SourceRecord*> IGMPRouter::get_strict_positive_timer(GroupState* groupState)
{
    return groupState->source_records;
}

Vector<SourceRecord*> IGMPRouter::get_null_timer(GroupState* groupState)
{
    return groupState->source_records;
}

Pair<int, GroupState*> IGMPRouter::get_or_create_group_state(in_addr multicast_address, int port)
{
    Pair<int, GroupState*>groupState = get_group_state(multicast_address, port);
    if (!groupState) {
        groupState = Pair<int, GroupState*>(port, new GroupState(multicast_address));
        // Append the new group state because we just created it
        group_states.push_back(groupState);
    }
    return groupState;
}

bool IGMPRouter::is_packet_source_in_group_state(GroupState* group_state, Packet* p)
{
    // Fetch the ip src address this is needed
    in_addr ip_src = p->ip_header()->ip_src;
    // Loops over all the source records and checks if the packet source
    // address is found if it is, then return true otherwise continue with
    // the loop. At the end if no ip_src is found return false.
    for (auto source_record: group_state->source_records) {
        if (source_record->source_address==ip_src) {
            return true;
        }
    }
    return false;
}

bool IGMPRouter::should_forward_udp(GroupState* group_state, Packet* p)
{
    // Check the current mode and decide whether or not to forward the UPD packet
    if (group_state->filter_mode==Constants::MODE_IS_INCLUDE) {
        return is_packet_source_in_group_state(group_state, p);
    }
    else if (group_state->filter_mode==Constants::MODE_IS_EXCLUDE) {
        return not is_packet_source_in_group_state(group_state, p);
    }
    // This is just for warnings, this should never be triggered
    click_chatter("\e[1;31m%-6s\e[m", "Group state has wrong state.");
    return false;
}

void IGMPRouter::update_filter_mode(in_addr multicast_address, int filter_mode, int port)
{
    Pair<int, GroupState*>groupState = get_or_create_group_state(multicast_address, port);
    groupState.second->filter_mode = filter_mode;
    click_chatter("\e[1;34m%-6s\e[m", "Updated filter mode");
}

void IGMPRouter::process_udp(Packet* p)
{
    const click_ip* ip_header = p->ip_header();
    Vector<Pair<int, GroupState*>> port_groups = get_group_state_list(ip_header->ip_dst);
    for (auto port_group: port_groups) {
        if (should_forward_udp(port_group.second, p)) {
            click_chatter("\e[1;35m%-6s %d\e[m", "Forwarded UDP packet on port", port_group.first);
            output(port_group.first).push(p);
        }
    }
}

void IGMPRouter::process_query(QueryPacket* query, int port)
{
    click_chatter("\e[1;32m%-6s\e[m", "Received query");
}

void IGMPRouter::process_report(ReportPacket* report, int port)
{
    click_chatter("\e[1;32m%-6s %d\e[m", "Received report on port", port);

    for (int i = 0; i<ntohs(report->num_group_records); i++) {
        click_chatter("\e[1;34m%-6s %d\e[m", "Processing group record", i);
        GroupRecordPacket groupRecord = report->group_records[i];

        // TODO check if this is the right implementation
        int report_recd_mode = groupRecord.record_type;
        Pair<int, GroupState*>router_record = get_or_create_group_state(groupRecord.multicast_address, port);
        int router_state = get_current_state(groupRecord.multicast_address, port);

        // Action table rfc3376, p.31
        if (router_state==Constants::MODE_IS_INCLUDE && report_recd_mode==Constants::MODE_IS_INCLUDE) {
            // New state Include (A+B)
            to_in(groupRecord.multicast_address, port);
            // Router state
            Vector<SourceRecord*>A = router_record.second->source_records;
            // Report record
            Vector<SourceRecord*>B = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
            router_record.second->source_records = vector_union(A, B);
            // TODO (B)=GMI
        }
        else if (router_state==Constants::MODE_IS_INCLUDE && report_recd_mode==Constants::MODE_IS_EXCLUDE) {
            // New state Exclude (A*B, B-A)
            to_ex(groupRecord.multicast_address, port);
            // Router state
            Vector<SourceRecord*>A = router_record.second->source_records;
            // Report record
            Vector<SourceRecord*>B = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
            Vector<SourceRecord*>first = vector_intersection(A, B);
            Vector<SourceRecord*>second = vector_difference(B, A);
            // TODO (B-A)-0, Delete(A-B), GroupTimer=GMI
        }
        else if (router_state==Constants::MODE_IS_EXCLUDE && report_recd_mode==Constants::MODE_IS_INCLUDE) {
            // New state Exclude(X+A, Y-A)
            // TODO: Report is include, moet dit in ons geval dan geen to_in worden???
            to_ex(groupRecord.multicast_address, port);
            // Router state
            // Timer >0
            Vector<SourceRecord*>X = router_record.second->source_records;
            // Timer =0
            Vector<SourceRecord*>Y = Vector<SourceRecord*>();
            // Report record
            Vector<SourceRecord*>A = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
            Vector<SourceRecord*>first = vector_union(X, A);
            Vector<SourceRecord*>second = vector_difference(Y, A);
            // TODO (A) = GMI
        }
        else if (router_state==Constants::MODE_IS_EXCLUDE && report_recd_mode==Constants::MODE_IS_EXCLUDE) {
            // New state Exclude(A-Y, Y*A)
            to_ex(groupRecord.multicast_address, port);
            // Router state
            // Timer >0
            Vector<SourceRecord*> X = router_record.second->source_records;
            // Timer =0
            Vector<SourceRecord*>Y = Vector<SourceRecord*>();
            // Report record
            Vector<SourceRecord*>A = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
            Vector<SourceRecord*>first = vector_difference(A, Y);
            Vector<SourceRecord*>second = vector_intersection(Y, A);
            // (A-X-Y)=GMI, Delete(X-A), Delete(Y-A), GroupTimer=GMI
        } // RFC 3376 section 6.4
        else if (router_state==Constants::MODE_IS_INCLUDE && report_recd_mode==Constants::CHANGE_TO_EXCLUDE_MODE) {
            // New state Exclude (A*B, B-A)
            to_ex(groupRecord.multicast_address, port);
            // Router state
            Vector<SourceRecord*>A = router_record.second->source_records;
            click_chatter("\e[1;31m%-6s\e[m", "Group state has wrong state.");
            // Report record
            Vector<SourceRecord*>B = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
            // (B-A)=0, Delete (A-B), Send Q(G, A*B), Group Timer=GMI
        }
        else if (router_state==Constants::MODE_IS_EXCLUDE && report_recd_mode==Constants::CHANGE_TO_INCLUDE_MODE) {
            // New state Exclude (X+A, Y-A)
            to_ex(groupRecord.multicast_address, port);
            // Router state
            // Timer >0
            Vector<SourceRecord*>X = router_record.second->source_records;
            // Timer =0
            Vector<SourceRecord*>Y = Vector<SourceRecord*>();
            // Report record
            Vector<SourceRecord*>A = to_vector(groupRecord.source_addresses, groupRecord.num_sources);
            // (A)=GMI, Send Q(G, X-A), Send Q(G)
        }
        else {
            click_chatter("\e[1;93m%-6s %d %-6s %d \e[m",
                    "Hmmm, not found. Router is in state",
                    router_state,
                    "and report contains state ",
                    report_recd_mode);
        }

//        // Action tabel rfc3376 p.29 (dingen die rekenen op timer nog niet geimplementeerd)
//        if(groupRecord.report_type == Constants::MODE_IS_INCLUDE && groupRecord.num_sources == 0){
//            // suggest to not forward source
//        } else if (groupRecord.report_type == Constants::MODE_IS_EXCLUDE && groupRecord.num_sources == 0) {
//            // suggest to forward traffic from source
//        }
    }

    click_chatter("\e[1;34m%-6s\e[m", "Done processing report");
}

void IGMPRouter::push(int port, Packet* p)
{
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

    click_chatter("\e[1;32m%-6s %d\e[m", "Received packet on port", port);

    if (port==3) {
        process_udp(p);
    }

    ReportPacket* report = (ReportPacket*) p->data();

    if (report->type==Constants::REPORT_TYPE) {
        process_report(report, port);
        return;
    }

    QueryPacket* query = (QueryPacket*) p->data();
    if (report->type==Constants::REPORT_TYPE) {
        process_query(query, port);
        return;
    }
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPRouter)

#include "constants.cc"