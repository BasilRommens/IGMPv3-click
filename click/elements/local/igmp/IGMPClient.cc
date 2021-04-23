#include <clicknet/ip.h> // Must be above everything else otherwise this wouldn't work

#include <click/config.h>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/timer.hh>

#include "IGMPClient.hh"

#include "constants.hh"
#include "report.hh"

CLICK_DECLS

IGMPClient::IGMPClient()
{
    interfaceMulticastTable = new InterfaceMulticastTable();
    socketMulticastTable = new SocketMulticastTable();
}

IGMPClient::~IGMPClient() { }

int IGMPClient::configure(Vector<String>& conf, ErrorHandler* errh)
{
    // TODO: parse config string
    return 0;
}

void IGMPClient::push(int port, Packet* p)
{
    output(port).push(p);
}

void IGMPClient::IPMulticastListen(int socket, in_addr interface, in_addr multicast_address, int filter_mode,
        Vector<in_addr> source_list)
{
    SocketRecord* socketRecord = new SocketRecord(interface, multicast_address, filter_mode, source_list);

    // Update it's own records
    socketMulticastTable->addRecord(socketRecord);
    interfaceMulticastTable->update(socketMulticastTable);

    // Send report packet
    int old_state = interfaceMulticastTable->filter_state(multicast_address);
    filter_mode = new_filter_mode(old_state, filter_mode);
    GroupRecord* record = new GroupRecord(filter_mode, multicast_address, source_list);
    Report report = Report();
    report.addGroupRecord(record);
    Packet* report_packet = report.createPacket();
    IPAddress report_address = IPAddress("224.0.0.22");
    report_packet->set_dst_ip_anno(report_address);
    output(0).push(report_packet);
}


int IGMPClient::new_filter_mode(int old_state, int new_state)
{
    /**
     * A change of interface state causes the system to immediately transmit
     * a State-Change Report from that interface. The type and contents of
     * the Group Record(s) in that Report are determined by comparing the
     * filter mode and source list for the affected multicast address before
     * and after the change, according to the table below. If no interface
     * state existed for that multicast address before the change (i.e., the
     * change consisted of creating a new per-interface record), or if no
     * state exists after the change (i.e., the change consisted of deleting
     * a per-interface record), then the "non-existent" state is considered
     * to have a filter mode of INCLUDE and an empty source list.
     */
    if (old_state==Constants::MODE_IS_INCLUDE && new_state==Constants::MODE_IS_EXCLUDE) {
        return Constants::CHANGE_TO_EXCLUDE_MODE;
    }
    else if (old_state==Constants::MODE_IS_EXCLUDE && new_state==Constants::MODE_IS_INCLUDE) {
        return Constants::CHANGE_TO_INCLUDE_MODE;
    }
    else {
        // not supported
    }
}

int join_leave_handle(int filter_mode, const String& conf, Element* e, void* thunk, ErrorHandler* errh)
{

    IGMPClient* igmpClient = (IGMPClient*) e;

    // Read input
    Vector<String> vconf;
    cp_argvec(conf, vconf);
    in_addr multicast_address;
    if (Args(vconf, igmpClient, errh).read_m("ADDRESS", multicast_address).complete()<0)
        return -1;

    // Handle input
    igmpClient->IPMulticastListen(0, igmpClient->get_identifier(), multicast_address, filter_mode, Vector<in_addr>());

    return 0;
}

int join_handle(const String& conf, Element* e, void* thunk, ErrorHandler* errh)
{
    /**
     * The Join operation is equivalent to
     * IPMulticastListen ( socket, interface, multicast-address, EXCLUDE, {} )
     * (rfc3376, 2)
     */
    return join_leave_handle(Constants::MODE_IS_EXCLUDE, conf, e, thunk, errh);
}

int leave_handle(const String& conf, Element* e, void* thunk, ErrorHandler* errh)
{
    /**
     * the Leave operation is equivalent to:
     * IPMulticastListen ( socket, interface, multicast-address, INCLUDE, {} )
     * (rfc3376, 2)
     */
    return join_leave_handle(Constants::MODE_IS_INCLUDE, conf, e, thunk, errh);
}

String get_tables_handle(Element* e, void* thunk)
{
    IGMPClient* igmpClient = (IGMPClient*) e;
    SocketMulticastTable* socketMulticastTable = igmpClient->get_socket_multicast_table();\
    InterfaceMulticastTable* interfaceMulticastTable = igmpClient->get_interface_multicast_table();
    String socketString = socketMulticastTable->to_string();
    String interfaceString = interfaceMulticastTable->to_string();
    return socketString+"\n"+interfaceString;
}

void IGMPClient::add_handlers()
{
    add_write_handler("join", &join_handle, (void*) 0);
    add_write_handler("leave", &leave_handle, (void*) 0);
    add_read_handler("tables", &get_tables_handle, 0);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPClient)

#include "SocketMulticastTable.cc"
#include "InterfaceMulticastTable.cc"
