#ifndef CLICK_IGMPPacketSource_HH
#define CLICK_IGMPPacketSource_HH

#include <click/element.hh>
#include <click/ipaddress.hh>

CLICK_DECLS

/**
 * Generates random packets (config determines report or query)
 * Note: These packets won't have an IP header yet.
 * Code sterk gebasseerd op short ICMPPingSource die we in de les gezien hebben
 */
class IGMPPacketSource : public Element {
public:
    IGMPPacketSource();

    ~IGMPPacketSource();

    const char* class_name() const { return "IGMPPacketSource"; }
    const char* port_count() const { return "0/1"; }
    const char* processing() const { return PUSH; }

    int configure(Vector<String>& conf, ErrorHandler* errh);

    void run_timer(Timer*);

private:
    Packet* make_packet();

    Packet* make_query_packet();

    Packet* make_report_packet();

    // Generates reports if true, queries if false
    bool generate_report = true;


};

CLICK_ENDDECLS
#endif
