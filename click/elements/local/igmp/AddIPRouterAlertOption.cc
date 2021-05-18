#include <clicknet/ip.h> // Must be above everything else otherwise this wouldn't work

#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/timer.hh>

#include "AddIPRouterAlertOption.hh"

CLICK_DECLS

AddIPRouterAlertOption::AddIPRouterAlertOption() { }

AddIPRouterAlertOption::~AddIPRouterAlertOption() { }

int AddIPRouterAlertOption::configure(Vector<String>& conf, ErrorHandler* errh)
{
    return 0;
}

void AddIPRouterAlertOption::push(int, Packet* p)
{
    click_ip* ip_header = (click_ip*) p->data();
    ip_header->ip_v += 0x01;
    output(0).push(p);
    return;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(AddIPRouterAlertOption)
