#include "report.hh"

CLICK_DECLS

        uint8_t

GroupRecord::getRecordType()
{
    return record_type;
}

uint16_t GroupRecord::getNumSources()
{
    return num_sources;
}

in_addr GroupRecord::getMulticastAddress()
{
    return multicast_address;
}

Vector<in_addr> GroupRecord::getSourceAdresses()
{
    return source_adresses;
}

void Report::addGroupRecord(GroupRecord record)
{
    group_records.push_back(record);
    num_group_records += 1;
}

void GroupRecord::add_source(in_addr source)
{
    source_adresses.push_back(source);
    num_sources += 1;
}

CLICK_ENDDECLS
