#include <clicknet/ip.h> // Must be above everything else otherwise this wouldn't work

#include <click/config.h>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/timer.hh>

#include "constants.hh"
#include "report.hh"
#include "query.hh"

#include "AddIPRouterAlertOption.hh"

CLICK_DECLS

AddIPRouterAlertOption::AddIPRouterAlertOption() { }

AddIPRouterAlertOption::~AddIPRouterAlertOption() { }

int AddIPRouterAlertOption::configure(Vector<String>& conf, ErrorHandler* errh)
{
    return 0;
}

void AddIPRouterAlertOption::push(int port, Packet* p) {
    return;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(AddIPRouterAlertOption)
