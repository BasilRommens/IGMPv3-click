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
* Een client kan een join en een leave doen. (opgave, 3.5)
*/

// handle join -> Send report

#ifndef CLICK_IGMPClient_HH
#define CLICK_IGMPClient_HH

#include <click/timestamp.hh>
#include <click/element.hh>
#include <click/ipaddress.hh>
#include <algorithm>
#include <tuple>
#include <stdlib.h>

#include "SocketMulticastTable.hh"
#include "InterfaceMulticastTable.hh"

CLICK_DECLS

class Query;
class QueryPacket;
class Report;
class ReportPacket;
class GroupRecord;
class GroupRecordPacket;
struct StateChangeArgs;

class IGMPClient : public Element {
public:
    IGMPClient();
    ~IGMPClient();

    const char* class_name() const { return "IGMPClient"; }
    const char* port_count() const { return "2/3"; }
    const char* processing() const { return PUSH; }

    int configure(Vector<String>& conf, ErrorHandler* errh);
    void push(int port, Packet* p);

    void process_udp(Packet* p);
    void process_query(QueryPacket* p, int port);
    void process_other_packet(Packet* p, int port);

    void IPMulticastListen(int socket, in_addr interface, in_addr multicast_address, int filter_mode,
            Vector<in_addr> source_list);

    // Handles
    void add_handlers();
    in_addr get_identifier() { return identifier; }
    InterfaceMulticastTable* get_interface_multicast_table() { return interfaceMulticastTable; }
    SocketMulticastTable* get_socket_multicast_table() { return socketMulticastTable; }

    // RFC3376, 5.1
    int new_filter_mode(int old_state, int new_state);

    bool isShortestGeneralPendingResponse(int, Timestamp);
private:
    bool isPendingResponse(in_addr);
    bool isSourceListEmpty(in_addr, int);
    bool isInterfaceTimer(Timer*);
    bool isGroupTimer(Timer*);
    bool isChangeInterfaceActive(in_addr);
    bool hasChangedState(int filter_mode, int old_state);

    Timestamp getShortestGeneralPendingResponse(int);
    Vector<in_addr>* getSourceList(in_addr, int);
    Timer* getPendingResponseTimer(in_addr);
    StateChangeArgs* getChangeInterfaceActiveArgs(in_addr);
    Timer* getChangeInterfaceActiveTimer(in_addr);
    Pair<StateChangeArgs*, Timer*> getChangeInterfaceActiveTuple(in_addr);

    GroupRecord* createCurrentStateRecord(in_addr, int, Vector<in_addr>);
    void removePendingResponse(in_addr);
    void updateStateChangeReport(in_addr, in_addr, int, Vector<in_addr>);

    static void respondToQuery(Timer*, void*);
    static void respondToStateChange(Timer*, void*);
    static void respondToUpdate(Timer*, void*);

    SocketMulticastTable* socketMulticastTable;
    InterfaceMulticastTable* interfaceMulticastTable;
    in_addr identifier; // Is used as interface in the tables
    Vector<Pair<int, Timer*>> general_timers;
    Vector<std::tuple<int, Timer*, in_addr>> group_timers;
    Vector<Pair<StateChangeArgs*, Timer*>> change_interface_active;
};

// Handles
int join_handle(const String& conf, Element* e, void* thunk, ErrorHandler* errh);

int leave_handle(const String& conf, Element* e, void* thunk, ErrorHandler* errh);

int join_leave_handle(int filter_mode, const String& conf, Element* e, void* thunk, ErrorHandler* errh);

int crash_handle(const String &conf, Element *e, void *thunk, ErrorHandler *errh);

String get_tables_handle(Element* e, void* thunk);

// Args for responding to a query
struct QueryResponseArgs {
    Query* query;
    IGMPClient* client;
    int interface;
};
// Args for marking state-change
struct StateChangeArgs {
    IGMPClient* client;
    int retransmit;
    Report* report;
    in_addr interface;
    in_addr multicast_addr;
};

CLICK_ENDDECLS
#endif

