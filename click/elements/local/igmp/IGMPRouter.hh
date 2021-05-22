#ifndef CLICK_IGMPRouter_HH
#define CLICK_IGMPRouter_HH

#include <click/element.hh>
#include <click/ipaddress.hh>
#include <click/pair.hh>

#include "GroupState.hh"

CLICK_DECLS

/**
 * Membership Queries are sent by IP multicast routers to query the
 * multicast reception state of neighboring interfaces. (rfc3376, 4.1)
*/

/**
* De router kan de join en leave verwerken en daarop de routering aanpassen. (opgave, 3.5)
*/

class QueryPacket;
class ReportPacket;
class GroupRecordPacket;

class IGMPRouter : public Element {
public:
    IGMPRouter();

    ~IGMPRouter();

    Vector<Pair<int, GroupState*>> group_states;

    // rfc3376, p.31
    int get_current_state(in_addr multicast_address, int port);
    void to_in(in_addr multicast_address, int port);
    void to_ex(in_addr multicast_address, int port);
    void update_filter_mode(in_addr multicast_address, int filter_mode, int port);
    Pair<int, GroupState*> get_group_state(in_addr multicast_address, int port);
    Vector<Pair<int, GroupState*>> get_group_state_list(in_addr multicast_address);
    Pair<int, GroupState*> get_or_create_group_state(in_addr multicast_address, int port);
    bool is_packet_source_in_group_state(GroupState*, Packet*);
    bool should_forward_udp(GroupState*, Packet*);
    Vector<SourceRecord*> get_strict_positive_timer(GroupState*);
    Vector<SourceRecord*> get_null_timer(GroupState*);
    Vector<SourceRecord*> to_vector(in_addr[], uint16_t);
    Vector<SourceRecord*> vector_union(Vector<SourceRecord*>, Vector<SourceRecord*>);
    Vector<SourceRecord*> vector_difference(Vector<SourceRecord*>, Vector<SourceRecord*>);
    Vector<SourceRecord*> vector_intersection(Vector<SourceRecord*>, Vector<SourceRecord*>);

    const char* class_name() const { return "IGMPRouter"; }
    const char* port_count() const { return "-/="; } // Any num input, evenveel output
    const char* processing() const { return AGNOSTIC; }

    void push(int port, Packet *p);


    // Removed parameters because they are not used
    int configure(Vector<String>&, ErrorHandler*);

    static void send_scheduled_query(Timer*, void* thunk); // static to make it possible to use it in timers
    void send_to_all_group_members(Packet* packet, in_addr group_address);

    Packet* create_group_specific_query_packet(in_addr multicast_address, bool suppress_flag, int time_until_send);

    static void handle_expired_group_timer(Timer* timer, void* thunk);

    Timer* get_group_timer(in_addr group_address, int port);

private:
    void process_udp(Packet* p);
    void process_query(QueryPacket* query, int port);
    void process_report(ReportPacket* report, int port);

    void process_group_record(GroupRecordPacket &groupRecord, int port);
    void update_router_state(GroupRecordPacket &groupRecord, int port);
    void process_in_report_in(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record);
    void process_in_report_ex(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record);
    void process_ex_report_in(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record);
    void process_ex_report_ex(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record);
    void process_in_report_cex(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record);
    void process_ex_report_cin(GroupRecordPacket &groupRecord, int port, Pair<int, GroupState *> &router_record);

    // Sends general query to all attached network
    static void send_general_queries(Timer *timer, void *thunk);
    // Sends general query to specific group (port)
    Packet* get_general_query();
    // Returns all ports on which someone is interested in reception of given multicast address
    Vector<int> get_group_members(in_addr multicast_address);

    void send_group_specific_query(in_addr multicast_address);

    void change_group_to_exclude(int port, in_addr group_addr);
    Vector<int> get_attached_networks();

    void set_group_timer(in_addr multicast_address, int port, int duration);
    void set_group_timer_gmi(in_addr multicast_address, int port);
    void set_group_timer_lmqt(in_addr multicast_address, int port);

    // Removes scheduled queries, used for "merging" queries
    // Note: Merging is interpreted as stopping the previous retransmissions and only sending new ones
    void stop_scheduled_retransmissions(in_addr multicast_address);
    bool is_timer_canceled(Timer* timer);
    Vector<Pair<in_addr, Timer*>> query_timers;

};


struct ScheduledQueryTimerArgs {
    // Because somehow multiple arguments aren't supported in a timer
    in_addr multicast_address;
    Packet* packet_to_send;
    IGMPRouter* router;
};

struct GroupTimerArgs {
    in_addr multicast_address;
    int port;
    IGMPRouter* router;
};

struct GeneralQueryTimerArgs {
    IGMPRouter* router;
};

CLICK_ENDDECLS
#endif
