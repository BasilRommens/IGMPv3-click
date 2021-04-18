#include "SocketMulticastTable.hh"

SocketMulticastTable::SocketMulticastTable()
{
    records = Vector<SocketRecord*>();
}

void SocketMulticastTable::addRecord(SocketRecord* requested)
{
    // WARNING: DIT IS ALLEMAAL NOG PSEUDOCODE (aka misschien werkende code die ik nog niet getest heb)

    /* If the requested filter mode is INCLUDE *and* the requested source
     * list is empty, then the entry corresponding to the requested
     * interface and multicast address is deleted if present. If no such
     * entry is present, the request is ignored.
     */
    if (requested->is_include() && requested->source_list.empty()) {
        delete_if_exists(requested->interface, requested->multicast_address);
    }

    /* If the requested filter mode is EXCLUDE *or* the requested source
     * list is non-empty, then the entry corresponding to the requested
     * interface and multicast address, if present, is changed to contain
     * the requested filter mode and source list. If no such entry is
     * present, a new entry is created, using the parameters specified in
     * the request.
     */
    if (requested->is_exclude() or !requested->source_list.empty()) {
        int index = get_index(requested->interface, requested->multicast_address);
        if (index==-1) {
            records.push_back(requested);
            index = records.size()-1;
        }
        SocketRecord* record = records[index];
        record->filter_mode = requested->filter_mode;
        record->source_list = requested->source_list;
    }
}

int SocketMulticastTable::get_index(in_addr interface, in_addr multicast_address)
{
    // Returnt -1 als er geen entry voor de gegeven interface inzit, anders de index
    int index = -1;
    click_chatter("test");
    for (int i = 0; i<records.size(); i++) {
        click_chatter("test");
        if (records[i]->interface==interface && records[i]->multicast_address==multicast_address) {
            index = i;
            break;
        }
    }
    return index;
}

void SocketMulticastTable::delete_if_exists(in_addr interface, in_addr multicast_address)
{
    int index = get_index(interface, multicast_address);
    if (index>=0) {
        records.erase(records.begin()+index);
    }
}

int SocketMulticastTable::get_index_or_create(in_addr interface, in_addr multicast_address)
{
    int index = get_index(interface, multicast_address);
    if (index==-1) {
        SocketRecord* record = new SocketRecord(interface);
        records.push_back(record);
        index = records.size()-1;
    }
    return index;
}

String SocketRecord::to_string()
{
    click_chatter("Hallo");
    String result = "";
    result += inadress_to_string(interface)+"\t";
    result += inadress_to_string(multicast_address)+"\t";
    result += String(filter_mode)+"\t";
    for (auto source: source_list) {
        result += inadress_to_string(source)+", ";
    }
    return result;
}

String SocketMulticastTable::to_string()
{
    String result = "SOCKET MULTICAST TABLE\n";
    for (auto record: records) {
        result += record->to_string()+"\n";
    }
    return result;

}

String inadress_to_string(in_addr addr)
{
    return IPAddress(addr).s();
}
