/**
 * For each socket on which IPMulticastListen has been invoked, the
 * system records the desired multicast reception state for that socket.
 * That state conceptually consists of a set of records of the form:
 * (interface, multicast-address, filter-mode, source-list) (rfc3376, 3.1)
 */

/**
 * The socket state evolves in response to each invocation of
 * IPMulticastListen on the socket, as follows:
 * - If the requested filter mode is INCLUDE *and* the requested source
 *   list is empty, then the entry corresponding to the requested
 *   interface and multicast address is deleted if present. If no such
 *   entry is present, the request is ignored.
 * - If the requested filter mode is EXCLUDE *or* the requested source
 *   list is non-empty, then the entry corresponding to the requested
 *   interface and multicast address, if present, is changed to contain
 *   the requested filter mode and source list. If no such entry is
 *   present, a new entry is created, using the parameters specified in
 *   the request. (rfc3376, 3.1)
*/

class SocketRecord {
    in_addr interface; // TODO: correct type?
    in_addr multicast_address;
    int filter_mode; // TODO: correct type?
    Vector<in_addr> source_list;

    SocketRecord(in_addr interface_init){
        interface = interface_init;
    }

    bool is_include() {
        return False; // TODO
    }
};
class SocketMulticastTable {
    Vector<SocketRecord*> records;
    void addRecord(SocketRecord* requested);
    int get_index(in_addr interface) {
        // Returnt -1 als er geen entry voor de gegeven interface inzit, anders de index
        int index = -1;
        for (int i = 0; i < records.length(); i++) {
            if (records[i]->interface == interface){
                index = i;
                break;
            }
        }
        return index;
    }

    void delete_if_exists(in_addr interface) {
        int index = get_index(interface);
        if (index >= 0){
            records.erase(index);
        }
    }

    int get_index_or_create(in_addr interface){
        int index = get_index(requested.interface());
        if (index == -1) {
            SocketRecord* record = new SocketRecord(requested.interface);
            records.push_back(record);
            index = records.length() - 1;
        }
        return index;
    }
};