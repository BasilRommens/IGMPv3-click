#include <clicknet/ip.h> // Must be above everything else otherwise this wouldn't work

#include <click/config.h>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/timer.hh>

#include "IGMPClient.hh"

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


int join_handle(const String &conf, Element *e, void *thunk, ErrorHandler *errh) {
    // Moet een record gaan sturen met de nieuwe info en dit updaten in zijn eigen socketMulticastTable
    // Code copy pasta van click coding slides
//    IGMPClient *me = (IGMPClient *) e;
//    Vector<String> vconf;
//    cp_argvec(conf, vconf);
//    if (Args(vconf, me, errh).read(...).complete() < 0)
//        return -1;
    return 0;
}

int leave_handle(const String &conf, Element *e, void *thunk, ErrorHandler *errh) {
    return 0;
}

void IGMPClient::add_handlers(){
    add_write_handler("join_handle", &join_handle, (void *)0);
    add_write_handler("leave_handle", &leave_handle, (void *)0);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPClient)
