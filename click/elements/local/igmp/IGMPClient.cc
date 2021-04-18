#include <clicknet/ip.h> // Must be above everything else otherwise this wouldn't work

#include <click/config.h>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/timer.hh>

#include "IGMPClient.hh"

#include "SocketMulticastTable.cc"
#include "InterfaceMulticastTable.cc"

CLICK_DECLS

IGMPClient::IGMPClient() {}

IGMPClient::~IGMPClient() {}

int IGMPClient::configure(Vector <String> &conf, ErrorHandler *errh) {
    // TODO: parse config string
    return 0;
}

void IGMPClient::push(int port, Packet *p) {
    return;
}


void IGMPClient::IPMulticastListen(int socket, in_addr interface, in_addr multicast_address, int filter_mode,
                                   Vector <in_addr> source_list) {
    SocketRecord* socketRecord = new SocketRecord(interface, multicast_address, filter_mode, source_list);
    socketMulticastTable->addRecord(socketRecord);
    interfaceMulticastTable->update(socketMulticastTable);
}

int join_leave_handle(int filter_mode, const String &conf, Element *e, void *thunk, ErrorHandler *errh) {

    IGMPClient *igmpClient = (IGMPClient *) e;

    // Read input
    Vector<String> vconf;
    cp_argvec(conf, vconf);
    in_addr multicast_address;
    if(Args(vconf, igmpClient, errh).read_m("ADDRESS", multicast_address).complete() < 0)
        return -1;

    // Handle input
    igmpClient->IPMulticastListen(0, igmpClient->get_identifier(), multicast_address, filter_mode, Vector<in_addr>());

    return 0;
}

int join_handle(const String &conf, Element *e, void *thunk, ErrorHandler *errh) {
    /**
     * The Join operation is equivalent to
     * IPMulticastListen ( socket, interface, multicast-address, EXCLUDE, {} )
     * (rfc3376, 2)
     */
    return join_leave_handle(Constants::MODE_IS_EXCLUDE, conf, e, thunk, errh);
}

int leave_handle(const String &conf, Element *e, void *thunk, ErrorHandler *errh) {
    /**
     * the Leave operation is equivalent to:
     * IPMulticastListen ( socket, interface, multicast-address, INCLUDE, {} )
     * (rfc3376, 2)
     */
    return join_leave_handle(Constants::MODE_IS_INCLUDE, conf, e, thunk, errh);
}

void IGMPClient::add_handlers() {
    add_write_handler("join_handle", &join_handle, (void *) 0);
    add_write_handler("leave_handle", &leave_handle, (void *) 0);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPClient)
