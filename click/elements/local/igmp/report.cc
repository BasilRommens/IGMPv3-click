#include "report.hh"

uint8_t GroupRecord::getRecordType() {
    return record_type;
}

uint8_t GroupRecord::getNumSources() {
    return num_sources;
}

uint32_t GroupRecord::getMulticastAddress() {
    return multicast_address;
}

Vector<uint32_t> GroupRecord::getSourceAdresses() {
    return source_adresses;
}

void Report::addGroupRecord(GroupRecord record) {
  group_records.push_back(record);
  num_group_records += 1;
}

void GroupRecord::add_source(uint32_t source) {
  sources.push_back(source);
  num_sources += 1;
}
