#include "SocketMulticastTable.hh"

void SocketMulticastTable::addRecord(SocketRecord* requested) {
    // WARNING: DIT IS ALLEMAAL NOG PSEUDOCODE (aka werkende code die ik nog niet getest heb)

    /* If the requested filter mode is INCLUDE *and* the requested source
     * list is empty, then the entry corresponding to the requested
     * interface and multicast address is deleted if present. If no such
     * entry is present, the request is ignored.
     */
    if (requested->is_include() && requested->source_list.empty()){
        delete_if_exists(requested.interface);
    }

    /* If the requested filter mode is EXCLUDE *or* the requested source
     * list is non-empty, then the entry corresponding to the requested
     * interface and multicast address, if present, is changed to contain
     * the requested filter mode and source list. If no such entry is
     * present, a new entry is created, using the parameters specified in
     * the request.
     */
    if (requested->is_exclude() or !requested->source_list.empty()){

        index = get_index_or_create(interface);
        SocketRecord* record = records[index];
        record->filter_mode = requested->filtermode;
        record->source_list = requested->source_list;

    }
}