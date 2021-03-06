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

#ifndef CLICK_SocketMulticastTable_HH
#define CLICK_SocketMulticastTable_HH

#include <click/string.hh>

class SocketRecord {
public:
    in_addr interface; // TODO: correct type?, wordt al bijgehouden in interface, dus mss enkel socket bijhouden en interface niet?
    in_addr multicast_address;
    int filter_mode; // rfc p. 16: 1 if include, 2 if exclude
    Vector<in_addr> source_list;

    SocketRecord(in_addr interface_init) {
        interface = interface_init;
    }

    SocketRecord(in_addr interface, in_addr multicast_address, int filter_mode, Vector <in_addr> source_list)
            : interface(interface), multicast_address(multicast_address), filter_mode(filter_mode),
              source_list(source_list) {
    }

    bool is_include() {
        return filter_mode == 1;
    }

    bool is_exclude() {
        return filter_mode == 2;
    }

    String to_string();
};

class SocketMulticastTable {
    // Update for each socket on which IPMulticastListen has been invoked
    // Must be kept per socket -> Maybe use a map instead? Or keep it as attribute in interface?
public:
    SocketMulticastTable();

    Vector<SocketRecord *> records;

    void addRecord(SocketRecord* requested);

    int get_index(in_addr interface, in_addr multicast_address);

    void delete_if_exists(in_addr interface, in_addr multicast_address);

    int get_index_or_create(in_addr interface, in_addr multicast_address);

    String to_string();
};

String inadress_to_string(in_addr addr);

#endif


