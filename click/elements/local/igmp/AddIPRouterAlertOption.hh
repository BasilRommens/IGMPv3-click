#ifndef CLICK_AddIPRouterAlertOption_HH
#define CLICK_AddIPRouterAlertOption_HH

#include <click/element.hh>
#include <click/ipaddress.hh>

CLICK_DECLS

class AddIPRouterAlertOption : public Element {
public:
    AddIPRouterAlertOption();
    ~AddIPRouterAlertOption();

    const char *class_name() const { return "AddIPRouterAlertOption"; }
    const char *port_count() const { return "1"; }
    const char *processing() const { return PUSH; }

    int configure(Vector <String> &conf, ErrorHandler *errh);

    void push(int port, Packet *p);
};

CLICK_ENDDECLS
#endif //CLICK_AddIPRouterAlertOption_HH
