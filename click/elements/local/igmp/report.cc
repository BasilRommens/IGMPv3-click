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

void Report::addGroupRecord(GroupRecord *record)
{
    group_records.push_back(record);
    num_group_records = htons(ntohs(num_group_records)+1);
}

void GroupRecord::add_source(in_addr source)
{
    source_addresses.push_back(source);
    num_sources = htons(ntohs(num_sources)+1);
}

WritablePacket* Report::createPacket()
{
    // Room for the IP header and Ether header which must be added later by
    //  another element
    int headroom = sizeof(click_ip)+sizeof(click_ether);

    WritablePacket* q = Packet::make(headroom, 0, this->size(), 0);
//    click_chatter("%d", this->size());
    if (!q) {
        return 0;
    }

    // Make packet data 0 to prevent weird problems
    memset(q->data(), '\0', q->length());

    // Cast the data to a report and set the attribute values
    ReportPacket* report = (ReportPacket*) (q->data());
    report->type = type;
    report->reserved1 = reserved1;
    report->reserved2 = reserved2;
    report->num_group_records = num_group_records;

    // create a pointer to the beginning of the group records memory location
    uint32_t* record_ptr = reinterpret_cast<uint32_t*>(&report->group_records);

    for (int i = 0; i<htons(num_group_records); i++) {
        GroupRecord* cur_group_record = group_records[i];
        GroupRecordPacket* new_group_record = (GroupRecordPacket*) (record_ptr);

        new_group_record->record_type = cur_group_record->getRecordType();
        new_group_record->aux_data_len = cur_group_record->aux_data_len;
        new_group_record->num_sources = htons(cur_group_record->getNumSources());
        new_group_record->multicast_address = cur_group_record->getMulticastAddress();
        for (int j = 0; j<cur_group_record->getNumSources(); j++) {
            new_group_record->source_addresses[j] = cur_group_record->source_addresses[j];
        }
        click_chatter("group_records %d", cur_group_record->size());
        // increase the record pointer and shift it by 4 bytes not 1
        record_ptr += cur_group_record->size()/4;
    }

    report->checksum = click_in_cksum(q->data(), q->length());
    return q;
}

int Report::size()
{
    int default_size = 8;
    int group_record_size = 0;
    for (int i = 0; i<group_records.size(); i++) {
        group_record_size += group_records[i]->size();
    }
    return default_size+group_record_size;
}

CLICK_ENDDECLS
