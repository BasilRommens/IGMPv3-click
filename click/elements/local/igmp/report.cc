#include "report.hh"

CLICK_DECLS

        uint8_t

GroupRecord::getRecordType()
{
    return record_type;
}

uint16_t GroupRecord::getNumSources()
{
    return ntohs(num_sources);
}

in_addr GroupRecord::getMulticastAddress()
{
    return multicast_address;
}

Vector<in_addr> GroupRecord::getSourceAddresses()
{
    return source_addresses;
}

int GroupRecord::size()
{
    int default_size = 8;
    int source_address_size = source_addresses.size()*4;
    int auxiliary_size = aux_data_len*4;
    return default_size+source_address_size+auxiliary_size;
}

void Report::addGroupRecord(GroupRecord record)
{
    group_records.push_back(record);
    num_group_records += htons(ntohs(num_group_records)+1);
}

void GroupRecord::add_source(in_addr source)
{
    source_addresses.push_back(source);
    num_sources = htons(ntohs(num_sources)+1);
}

WritablePacket* Report::createPacket()
{
    return nullptr;
}

int Report::size()
{
    int default_size = 8;
    int group_record_size = 0;
    for (int i = 0; i<group_records.size(); i++) {
        group_record_size = group_records[i].size();
    }
    return default_size+group_record_size;
}

CLICK_ENDDECLS
