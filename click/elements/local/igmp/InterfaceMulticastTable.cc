#include "InterfaceMulticastTable.hh"

InterfaceRecord::InterfaceRecord(in_addr multicast_address, int filter_mode, Vector <in_addr> source_list) :
        multicast_address(multicast_address), filter_mode(filter_mode), source_list(source_list) {

}

void InterfaceMulticastTable::addToMapVector(RecordMap &multicast_pairs, SocketRecord *record) {
    auto key = Pair<in_addr, in_addr>(record->interface, record->multicast_address);
    if (!containsPair(multicast_pairs, key)) {
        mapAddNew(multicast_pairs, key);
    }

    getMapValue(multicast_pairs, key).push_back(record);

}

Pair <Vector<SocketRecord *>, Vector<SocketRecord *>>
InterfaceMulticastTable::splitIncludeExclude(Vector<SocketRecord *> records) {
    // First item are all include records, second all exclude records
    Vector < SocketRecord * > includes;
    Vector < SocketRecord * > excludes;
    for (auto record: records) {
        if (record->is_include()) {
            includes.push_back(record);
        }
        if (record->is_exclude()) {
            excludes.push_back(record);
        }
    }
    return Pair < Vector < SocketRecord * > , Vector < SocketRecord * >> (includes, excludes);
}

bool InterfaceRecord::is_include() {
    return filter_mode == Constants::MODE_IS_INCLUDE;
}

bool InterfaceRecord::is_exclude() {
    return filter_mode == Constants::MODE_IS_EXCLUDE;
}

InterfaceMulticastTable::InterfaceMulticastTable() {
    records = Vector<InterfaceRecord*>();
}

void InterfaceMulticastTable::update(SocketMulticastTable *table) {
    // Get all (interface, multicast_address) pairs
    RecordMap multicast_pairs; // rfc p.6
    for (SocketRecord *record: table->records) {
        addToMapVector(multicast_pairs, record);
    }

    // Update interface table per pair
    records = Vector<InterfaceRecord *>();
    for (auto const &x : multicast_pairs) {
        Pair <in_addr, in_addr> key = x.first;
        Vector < SocketRecord * > all_records = x.second;

        auto in_ex_splitted = splitIncludeExclude(all_records);
        Vector < SocketRecord * > includes = in_ex_splitted.first;
        Vector < SocketRecord * > excludes = in_ex_splitted.second;

        if (!excludes.empty()) {
            // Contains an exclude
            int filter_mode = Constants::MODE_IS_EXCLUDE;
            Vector <in_addr> source_list = vector_union(get_source_lists(excludes));
            source_list = vector_minus(source_list, vector_union(get_source_lists(includes)));
            records.push_back(new InterfaceRecord(key.first, filter_mode, source_list));
        } else {
            // All includes
            int filter_mode = Constants::MODE_IS_INCLUDE;
            Vector <in_addr> source_list = vector_union(get_source_lists(includes));
            records.push_back(new InterfaceRecord(key.first, filter_mode, source_list));
        }
    }
}


Vector <in_addr> InterfaceMulticastTable::vector_minus(Vector <in_addr> vec_a, Vector <in_addr> vec_b) {
    Vector <in_addr> result = Vector<in_addr>();
    for (auto el: vec_a) {
        if (!contains(vec_b, el)) {
            result.push_back(el);
        }
    }
    return result;
}

Vector<SocketRecord *> InterfaceMulticastTable::vector_union(Vector <Vector<SocketRecord *>> vectors) {
    Vector < SocketRecord * > unioned = Vector<SocketRecord *>();
    for (Vector<SocketRecord*> vector: vectors) {
        for (SocketRecord* el: vector) {
            // TODO? Check if element already in unioned
            unioned.push_back(el);
        }
    }
    return unioned;
}

Vector<in_addr> InterfaceMulticastTable::vector_union(Vector <Vector<in_addr>> vectors) {
    Vector < in_addr > unioned = Vector<in_addr>();
    for (Vector<in_addr> vector: vectors) {
        for (in_addr el: vector) {
            // TODO? Check if element already in unioned
            unioned.push_back(el);
        }
    }
    return unioned;
}

Vector <Vector<in_addr>> InterfaceMulticastTable::get_source_lists(Vector<SocketRecord *> record_list) {
    auto source_lists = Vector < Vector < in_addr >> ();
    for (auto record: record_list) {
        source_lists.push_back(record->source_list);
    }
    return source_lists;
}

bool InterfaceMulticastTable::containsPair(RecordMap &map,
                                           Pair <in_addr, in_addr> key) {
    for (auto& pair: map){
        if (pair.first == key){
            return true;
        }
    }
    return false;
}

bool InterfaceMulticastTable::contains(Vector <in_addr> vector, in_addr value) {
    // src: https://stackoverflow.com/questions/3136520/determine-if-map-contains-a-value-for-a-key
    if (find(vector.begin(), vector.end(), value) == vector.end()) {
        return false;
    }
    return true;
}

void InterfaceMulticastTable::mapAddNew(RecordMap &map, InterfaceMulticastIdentifier &key) {
    auto newEntry = Pair < InterfaceMulticastIdentifier, Vector<SocketRecord *>>
    (key, Vector < SocketRecord * > ());
    map.push_back(newEntry);
}

Vector<SocketRecord *> &InterfaceMulticastTable::getMapValue(RecordMap &map, InterfaceMulticastIdentifier &key) {
    for (auto &pair: map) {
        if (pair.first == key) {
            return pair.second;
        }
    }
}


// Komt niet echt overeen met de IS_IN, IS_EX uit de rfc, want die hebben een source_list als parameter
bool InterfaceMulticastTable::is_in(in_addr multicast_address) {
    return filter_state(multicast_address) == Constants::MODE_IS_INCLUDE;
}

bool InterfaceMulticastTable::is_ex(in_addr multicast_address) {
    return filter_state(multicast_address) == Constants::MODE_IS_EXCLUDE;
}

InterfaceRecord* InterfaceMulticastTable::getRecord(in_addr multicast_address){
    for (auto record: records){
        if (record->multicast_address == multicast_address){
            return record;
        }
    }
    return nullptr;
}

int InterfaceMulticastTable::filter_state(in_addr multicast_address) {
    InterfaceRecord* record = getRecord(multicast_address);
    if (record != nullptr) {
        return record->filter_mode;
    } else {
        // Non-existing entry is interpreted as INCLUDE with empty source list
        return Constants::MODE_IS_INCLUDE;
    }
}

String InterfaceRecord::to_string() {
    String result = "";
    result += inadress_to_string(multicast_address) + "\t";
    result += String(filter_mode) + "\t";
    for (auto source: source_list) {
        result += inadress_to_string(source) + ", ";
    }
    return result;
}

String InterfaceMulticastTable::to_string() {
    String result = "INTERFACE MULTICAST TABLE\n";
    for (int i = 0; i < records.size(); i++) {
        auto record = records[i];
        result += record->to_string() + "\n";
    }
    return result;
}