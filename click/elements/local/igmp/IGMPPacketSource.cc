#include <click/timer.hh>
#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include <clicknet/tcp.h>
#include "report.hh" // TODO: Is da path juist zo of moet daar iets voor?
#include "IGMPPacketSource.hh"

CLICK_DECLS

IGMPPacketSource::IGMPPacketSource() { }

IGMPPacketSource::~IGMPPacketSource() { }

int IGMPPacketSource::configure(Vector<String>& conf, ErrorHandler* errh)
{
    // TODO: parse config string

    // Call function run_timer every second
    Timer* timer = new Timer(this);
    timer->initialize(this);
    timer->schedule_after_msec(1000);

    return 0;
}

Packet* IGMPPacketSource::make_packet()
{
    if (generate_report) {
        return make_report_packet();
    }
    else {
        return make_query_packet();
    }
}

Packet* IGMPPacketSource::make_report_packet()
{
    // Room for the IP header and Ether header which must be added later by
    //  another element
    int headroom = sizeof(click_ip)+sizeof(click_ether);

    WritablePacket* q = Packet::make(headroom, 0, sizeof(struct Report), 0);
    if (!q) {
        return 0;
    }

    // Make packet data 0 to prevent weird problems
    memset(q->data(), '\0', sizeof(struct Report));

    // Cast the data to a report and set the attribute values
    Report* report = (Report*) (q->data());

    // htons is host to network server, to prevent problems with big and little
    //  endians
    report->type = htons(0x22);

    GroupRecord group_record = GroupRecord();
    group_record.record_type = htons(0x01);
    // group_record.add_source("123.456.1.1") // TODO: String to uint32_t

    report->addGroupRecord(group_record);

    // report->checksum = click_in_cksum((const unsigned char*) igmph, sizeof(Report))

    return q;
}

void IGMPPacketSource::run_timer(Timer* timer)
{
    if (Packet*q = make_packet()) {
        output(0).push(q);
        timer->reschedule_after_msec(1000);
    }
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPPacketSource)
