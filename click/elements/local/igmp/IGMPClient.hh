/**
 * IGMP is an asymmetric protocol, specifying separate behaviors for
 * group members -- that is, hosts or routers that wish to receive
 * multicast packets -- and multicast routers. This section describes
 * the part of IGMPv3 that applies to all group members. (rfc3376, 5)
*/

/**
 * A system performs the protocol described in this section over all
 * interfaces on which multicast reception is supported, even if more
 * than one of those interfaces is connected to the same network. (rfc3376, 5)
*/

/**
 * The all-systems multicast address, 224.0.0.1, is handled as a special
 * case. On all systems -- that is all hosts and routers, including
 * multicast routers -- reception of packets destined to the all-systems
 * multicast address, from all sources, is permanently enabled on all
 * interfaces on which multicast reception is supported. No IGMP
 * messages are ever sent regarding the all-systems multicast address. (rfc3376, 5)
*/

/**
 * There are two types of events that trigger IGMPv3 protocol actions on
 * an interface:
 * - change of the interface reception state, caused by a local
 *   invocation of IPMulticastListen.
 * - reception of a Query.
 * (Received IGMP messages of types other than Query are silently
 * ignored, except as required for interoperation with earlier versions
 * of IGMP.) (rfc3376, 5)
*/

/**
 *  Action on change of interface state
 *
 *  A change of interface state causes the system to immediately transmit
 *  a State-Change Report from that interface. The type and contents of
 *  the Group Record(s) in that Report are determined by comparing the
 *  filter mode and source list for the affected multicast address before
 *  and after the change, according to the table below. If no interface
 *  state existed for that multicast address before the change (i.e., the
 *  change consisted of creating a new per-interface record), or if no
 *  state exists after the change (i.e., the change consisted of deleting
 *  a per-interface record), then the "non-existent" state is considered
 *  to have a filter mode of INCLUDE and an empty source list. (rfc3376, 5)
*/

/**
 * To cover the possibility of the State-Change Report being missed by
 * one or more multicast routers, it is retransmitted [Robustness
 * Variable] - 1 more times, at intervals chosen at random from the
 * range (0, [Unsolicited Report Interval]).
*/

/**
 * If more changes to the same interface state entry occur before all
 * the retransmissions of the State-Change Report for the first change
 * have been completed, each such additional change triggers the
 * immediate transmission of a new State-Change Report.
*/

/**
* Een client kan een join en een leave doen. (opgave, 3.5)
*/

// handle join -> Send report

#ifndef CLICK_IGMPClient_HH
#define CLICK_IGMPClient_HH

#include <click/element.hh>
#include <click/ipaddress.hh>

#include "SocketMulticastTable.hh"
#include "InterfaceMulticastTable.hh"

CLICK_DECLS
//
class IGMPClient : public Element {
public:
    IGMPClient();

    ~IGMPClient();

    const char *class_name() const { return "IGMPClient"; }

    const char *port_count() const { return "1/1"; }

    const char *processing() const { return PUSH; }

    int configure(Vector <String> &conf, ErrorHandler *errh);

    void push(int port, Packet *p);

    void IPMulticastListen(int socket, in_addr interface, in_addr multicast_address, int filter_mode, Vector<in_addr> source_list);


    // Handles
    void add_handlers();

    in_addr get_identifier(){ return identifier; }
    InterfaceMulticastTable* get_interface_multicast_table(){ return interfaceMulticastTable; }
    SocketMulticastTable* get_socket_multicast_table(){ return socketMulticastTable; }

    // RFC3376, 5.1
    int new_filter_mode(int old_state, int new_state);


private:
    SocketMulticastTable *socketMulticastTable;
    InterfaceMulticastTable *interfaceMulticastTable;
    in_addr identifier; // Is used as interface in the tables
};

// Handles
int join_handle(const String &conf, Element *e, void *thunk, ErrorHandler *errh);
int leave_handle(const String &conf, Element *e, void *thunk, ErrorHandler *errh);
int join_leave_handle(int filter_mode, const String &conf, Element *e, void *thunk, ErrorHandler *errh);

String get_tables_handle(Element *e, void *thunk);


CLICK_ENDDECLS
#endif

