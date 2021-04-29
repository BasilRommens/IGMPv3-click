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

IGMPRouter::IGMPRouter(){
    group_states = Vector<GroupState*>();
}

IGMPRouter::~IGMPRouter(){

}

GroupState* IGMPRouter::get_group_state(in_addr multicast_address){
    for(auto groupState: group_states){
        if (groupState->multicast_address == multicast_address) {
            return groupState;
        }
    }
    return nullptr;
}

int IGMPRouter::get_current_state(in_addr multicast_address){
    GroupState* groupState = get_group_state(multicast_address);
    if (groupState) {
        return groupState->filter_mode;
    } else {
        return Constants::MODE_IS_INCLUDE;
    }
}

void IGMPRouter::to_in(in_addr multicast_address){
    click_chatter("Changing to in");
    update_filter_mode(multicast_address, Constants::MODE_IS_INCLUDE);
}

void IGMPRouter::to_ex(in_addr multicast_address){
    click_chatter("Changing to ex");
    update_filter_mode(multicast_address, Constants::MODE_IS_INCLUDE);
}

GroupState* IGMPRouter::get_or_create_group_state(in_addr multicast_address){
    GroupState* groupState = get_group_state(multicast_address);
    if (!groupState) {
        groupState = new GroupState(multicast_address);
    }
    return groupState;
}

void IGMPRouter::update_filter_mode(in_addr multicast_address, int filter_mode){
    GroupState* groupState = get_or_create_group_state(multicast_address);
    groupState->filter_mode = filter_mode;
    click_chatter("Updated filter mode");
}

void IGMPRouter::process_udp(Packet* p) {
    click_chatter("Received UDP packet");
}

void IGMPRouter::process_query(QueryPacket* query){
    click_chatter("Received query");
}

void IGMPRouter::process_report(ReportPacket* report) {

    click_chatter("Received report");

    for (int i=0; i < ntohs(report->num_group_records); i++){
        click_chatter("Processing group record %d", i);
        GroupRecord groupRecord = report->group_records[i];

        // TODO check if this is the right implementation
        int report_recd_mode = groupRecord.record_type;
        int router_state = get_current_state(groupRecord.multicast_address);

        // Action table rfc3376, p.31
        if (router_state == Constants::MODE_IS_INCLUDE && report_recd_mode == Constants::CHANGE_TO_INCLUDE_MODE) {
            to_in(groupRecord.multicast_address);
            // New state Include (A+B)
            // (B)=GMI
        } else if (router_state == Constants::MODE_IS_INCLUDE && report_recd_mode == Constants::CHANGE_TO_EXCLUDE_MODE) {
            to_ex(groupRecord.multicast_address);
            // New state Exclude (A*B, B-A)
            // (B-A)-0, Delete(A-B), GroupTimer=GMI
        } else if (router_state == Constants::MODE_IS_EXCLUDE && report_recd_mode == Constants::CHANGE_TO_INCLUDE_MODE) {
            to_ex(groupRecord.multicast_address); // TODO: Report is include, moet dit in ons geval dan geen to_in worden???
            // New state Exclude(X+A, Y-A)
            // (A) = GMI
        } else if (router_state == Constants::MODE_IS_EXCLUDE && report_recd_mode == Constants::CHANGE_TO_EXCLUDE_MODE) {
            to_ex(groupRecord.multicast_address);
            // New state Exclude(A-Y, Y*A)
            // (A-X-Y)=GMI, Delete(X-A), Delete(Y-A), GroupTimer=GMI
        } else {
            click_chatter("Hmmm, not found. Router is in state %d and report contains state %d.", router_state, report_recd_mode);
        }


//        // Action tabel rfc3376 p.29 (dingen die rekenen op timer nog niet geimplementeerd)
//        if(groupRecord.report_type == Constants::MODE_IS_INCLUDE && groupRecord.num_sources == 0){
//            // suggest to not forward source
//        } else if (groupRecord.report_type == Constants::MODE_IS_EXCLUDE && groupRecord.num_sources == 0) {
//            // suggest to forward traffic from source
//        }
    }

    click_chatter("Done processing report");

}

void IGMPRouter::push(int port, Packet *p){
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

    click_chatter("Received packet on port %d", port);

    if (port == 3){
        process_udp(p);
    }

    ReportPacket* report = (ReportPacket*) p->data();

    if (report->type == Constants::REPORT_TYPE){
        process_report(report);
        return;
    }

    QueryPacket* query = (QueryPacket*) p->data();
    if (report->type == Constants::REPORT_TYPE){
        process_query(query);
        return;
    }



}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPRouter)

#include "constants.cc"