#include <clicknet/ip.h> // Must be above everything else otherwise this wouldn't work

#include <click/config.h>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/timer.hh>

#include "report.hh" // TODO: Is da path juist zo of moet daar iets voor?
#include "report.cc"
#include "query.hh"
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

Packet* IGMPPacketSource::make_query_packet()
{
    return nullptr;
}

Packet* IGMPPacketSource::make_report_packet()
{
    // Cast the data to a report and set the attribute values
    Report report = Report();

    // htons is host to network server, to prevent problems with big and little
    // endians
    report.type = 0x22;
    GroupRecord group_record = GroupRecord();

    // TODO: For some reason is dit niet gelijk aan 0 in het pakket. Mijn gok is dat dit te maken gaat hebben met
    //  mogelijke extra data die een vector bevat, buiten het element zelf
    group_record.aux_data_len = htons(0x01);

    // TODO fix record type
    group_record.record_type = 0x01;
    group_record.num_sources = 0x00;
    group_record.add_source(IPAddress(1).in_addr());
    group_record.multicast_address = IPAddress(1).in_addr();

    report.addGroupRecord(&group_record);

    return report.createPacket();
}

void IGMPPacketSource::run_timer(Timer* timer)
{
    if (Packet*q = make_packet()) {
        Report* r = (Report*) q->data();
//        click_chatter("Final: %d", r->group_records[0].getNumSources());
        output(0).push(q);
        timer->reschedule_after_msec(1000);
    }
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPPacketSource)
