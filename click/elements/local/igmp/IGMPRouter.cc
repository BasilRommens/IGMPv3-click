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

int IGMPRouter::get_current_state(in_addr multicast_address){
    for(auto groupState: group_states){
        if (groupState->multicast_address == multicast_address) {
            return groupState->filter_mode;
        }
    }
    return Constants::MODE_IS_INCLUDE;
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

    click_chatter("%d", port);
    ReportPacket* report = (ReportPacket*) p->data();

    for (int i=0; i < ntohs(report->num_group_records); i++){
        GroupRecord groupRecord = report->group_records[i];

        // TODO check if this is the right implementation
        int report_recd_mode = groupRecord.record_type;
        int router_state = get_current_state(groupRecord.multicast_address);

        // Action table rfc3376, p.31
        if (router_state == Constants::MODE_IS_INCLUDE && report_recd_mode == Constants::MODE_IS_INCLUDE) {
            // New state Include (A+B)
            // (B)=GMI
        } else if (router_state == Constants::MODE_IS_INCLUDE && report_recd_mode == Constants::MODE_IS_EXCLUDE) {
            // New state Exclude (A*B, B-A)
            // (B-A)-0, Delete(A-B), GroupTimer=GMI
        } else if (router_state == Constants::MODE_IS_EXCLUDE && report_recd_mode == Constants::MODE_IS_INCLUDE) {
            // New state Exclude(X+A, Y-A)
            // (A) = GMI
        } else if (router_state == Constants::MODE_IS_EXCLUDE && report_recd_mode == Constants::MODE_IS_EXCLUDE) {
            // New state Exclude(A-Y, Y*A)
            // (A-X-Y)=GMI, Delete(X-A), Delete(Y-A), GroupTimer=GMI
        }


//        // Action tabel rfc3376 p.29 (dingen die rekenen op timer nog niet geimplementeerd)
//        if(groupRecord.report_type == Constants::MODE_IS_INCLUDE && groupRecord.num_sources == 0){
//            // suggest to not forward source
//        } else if (groupRecord.report_type == Constants::MODE_IS_EXCLUDE && groupRecord.num_sources == 0) {
//            // suggest to forward traffic from source
//        }
    }

}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPRouter)

#include "constants.cc"