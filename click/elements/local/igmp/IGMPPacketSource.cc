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

IGMPPacketSource::IGMPPacketSource() {}

IGMPPacketSource::~IGMPPacketSource() {}

int IGMPPacketSource::configure(Vector <String> &conf, ErrorHandler *errh) {
    // TODO: parse config string

    // Call function run_timer every second
    Timer *timer = new Timer(this);
    timer->initialize(this);
    timer->schedule_after_msec(1000);

    return 0;
}

Packet *IGMPPacketSource::make_packet() {
    if (generate_report) {
        return make_report_packet();
    } else {
        return make_query_packet();
    }
}

Packet *IGMPPacketSource::make_query_packet() {
    return nullptr;
}

Packet *IGMPPacketSource::make_report_packet() {
    // Room for the IP header and Ether header which must be added later by
    //  another element
    int headroom = sizeof(click_ip) + sizeof(click_ether);

    WritablePacket *q = Packet::make(headroom, 0, sizeof(struct Report), 0);
    if (!q) {
        return 0;
    }

    // Make packet data 0 to prevent weird problems
    memset(q->data(), '\0', sizeof(struct Report));

    // Cast the data to a report and set the attribute values
    Report *report = (Report *) (q->data());

    // htons is host to network server, to prevent problems with big and little
    // endians
    report->type = 0x22;
    GroupRecord group_record = GroupRecord();

    // TODO: For some reason is dit niet gelijk aan 0 in het pakket. Mijn gok is dat dit te maken gaat hebben met
    //  mogelijke extra data die een vector bevat, buiten het element zelf
    group_record.aux_data_len = htons(0x01);

    // TODO fix record type
    group_record.record_type = htons(0x01);
//    click_chatter("%d", sizeof(struct Report));
//    click_chatter("Voor: %d", group_record.getNumSources());
    group_record.add_source(in_addr());
//    click_chatter("Na: %d", group_record.getNumSources());
    group_record.multicast_address = in_addr();

    report->addGroupRecord(group_record);

    report->checksum = click_in_cksum(q->data(), q->length());

    return q;
}

void IGMPPacketSource::run_timer(Timer *timer) {
    if (Packet * q = make_packet()) {
        Report* r = (Report *) q->data();
//        click_chatter("Final: %d", r->group_records[0].getNumSources());
        output(0).push(q);
        timer->reschedule_after_msec(1000);
    }
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPPacketSource)
