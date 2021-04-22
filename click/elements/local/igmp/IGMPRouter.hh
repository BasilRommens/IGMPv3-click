#ifndef CLICK_IGMPRouter_HH
#define CLICK_IGMPRouter_HH

#include <click/element.hh>
#include <click/ipaddress.hh>

#include "GroupState.hh"

CLICK_DECLS

/**
 * Membership Queries are sent by IP multicast routers to query the
 * multicast reception state of neighboring interfaces. (rfc3376, 4.1)
*/

/**
* De router kan de join en leave verwerken en daarop de routering aanpassen. (opgave, 3.5)
*/

class IGMPRouter : public Element {
public:
    IGMPPacketSource();

    ~IGMPPacketSource();

    Vector<GroupState*> group_states;

    // rfc3376, p.31
    int get_current_state(in_addr multicast_address);

    const char* class_name() const { return "IGMPRouter"; }
    const char* port_count() const { return "-/="; } // Any num input, evenveel output
    const char* processing() const { return AGNOSTIC; }

    void push(int port, Packet *p);

    int configure(Vector<String>& conf, ErrorHandler* errh) { return 0; }
};

CLICK_ENDDECLS
#endif
