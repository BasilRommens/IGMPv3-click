/**
 * Multicast routers implementing IGMPv3 keep state per group per
 * attached network. This group state consists of a filter-mode, a list
 * of sources, and various timers. For each attached network running
 * IGMP, a multicast router records the desired reception state for that
 * network. That state conceptually consists of a set of records of the
 * form:
 * (multicast address, group timer, filter-mode, (source records))
 * Each source record is of the form:
 * (source address, source timer)
 * If all sources within a given group are desired, an empty source
 * record list is kept with filter-mode set to EXCLUDE. This means
 * hosts on this network want all sources for this group to be
 * forwarded. This is the IGMPv3 equivalent to a IGMPv1 or IGMPv2 group
 * join.
 */

#ifndef CLICK_GroupState_HH
#define CLICK_GroupState_HH

#include <click/string.hh>
#include "constants.hh"

// Isn't used in our implementation, since source list is always empty
class SourceRecord {
public:
    SourceRecord(in_addr source_address, int source_timer)
            : source_address(source_address), source_timer(source_timer) {}

    in_addr source_address;
    int source_timer;
};

// Kept per group per attached network
class GroupState {
public:
    GroupState(in_addr multicast_address)
            : multicast_address(multicast_address) {
        filter_mode = Constants::MODE_IS_INCLUDE;
        source_records = Vector<SourceRecord *>();
    };
    in_addr multicast_address;
    int group_timer;
    int filter_mode;

    /**
     * If all sources within a given group are desired (wat altijd het geval is in onze implementatie), an empty source
     * record list is kept with filter-mode set to EXCLUDE. This means
     * hosts on this network want all sources for this group to be
     * forwarded. This is the IGMPv3 equivalent to a IGMPv1 or IGMPv2 group
     * join.
     */
    Vector<SourceRecord *> source_records;
};

#endif
