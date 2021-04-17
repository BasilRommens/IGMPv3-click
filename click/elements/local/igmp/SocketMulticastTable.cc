#include "SocketMulticastTable.hh"

void SocketMulticastTable::addRecord(SocketRecord* requested) {
    // WARNING: DIT IS ALLEMAAL NOG PSEUDOCODE (aka misschien werkende code die ik nog niet getest heb)

    /* If the requested filter mode is INCLUDE *and* the requested source
     * list is empty, then the entry corresponding to the requested
     * interface and multicast address is deleted if present. If no such
     * entry is present, the request is ignored.
     */
    if (requested->is_include() && requested->source_list.empty()){
        delete_if_exists(requested->interface);
    }

    /* If the requested filter mode is EXCLUDE *or* the requested source
     * list is non-empty, then the entry corresponding to the requested
     * interface and multicast address, if present, is changed to contain
     * the requested filter mode and source list. If no such entry is
     * present, a new entry is created, using the parameters specified in
     * the request.
     */
    if (requested->is_exclude() or !requested->source_list.empty()){
        index = get_index_or_create(requested->interface, requested->multicast_address);
        SocketRecord* record = records[index];
        record->filter_mode = requested->filter_mode;
        record->source_list = requested->source_list;
    }
}

int SocketMulticastTable::get_index(in_addr interface, in_addr multicast_address) {
    // Returnt -1 als er geen entry voor de gegeven interface inzit, anders de index
    int index = -1;
    for (int i = 0; i < records.size(); i++) {
        if (records[i]->interface == interface && records[i]->multicast_address == multicast_address){
            index = i;
            break;
        }
    }
    return index;
}

void SocketMulticastTable::delete_if_exists(in_addr interface, in_addr multicast_address) {
    int index = get_index(interface, multicast_address);
    if (index >= 0){
        records.erase(records.begin() + index);
    }
}

int SocketMulticastTable::get_index_or_create(in_addr interface, in_addr multicast_address){
    int index = get_index(interface, multicast_address);
    if (index == -1) {
        SocketRecord* record = new SocketRecord(interface);
        records.push_back(record);
        index = records.size() - 1;
    }
    return index;
}