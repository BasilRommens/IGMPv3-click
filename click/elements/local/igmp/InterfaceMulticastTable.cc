#include "InterfaceMulticastTable.hh"


void InterfaceMulticastTable::addToMapVector(Map <Pair<in_addr, in_addr>, Vector<SocketRecord *>> multicast_pairs,
                                             SocketRecord *record) {
    auto key = Pair<in_addr, in_addr>(record.multicast_address);
    if (!containsPair(multicast_pairs, key)) {
        multicast_pairs[key] = Vector({record});
    } else {
        multicast_pairs[key].push_back(record);
    }
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


void InterfaceMulticastTable::update(SocketMulticastTable table) {
    // Get all (interface, multicast_address) pairs
    Map <Pair<in_addr, in_addr>, Vector<SocketRecord *>> multicast_pairs; // rfc p.6
    for (SocketRecord *record: table.records) {
        addToMapVector(multicast_pairs, record)
    }

    // Update interface table per pair
    for (auto const &x : multicast_pairs) {
        Pair <in_addr, in_addr> key = x.first;
        Vector < SocketRecord * > all_records = x.second;

        auto in_ex_splitted = splitIncludeExclude(all_records);
        Vector < SocketRecord * > includes = in_ex_splitted.first;
        Vector < SocketRecord * > excludes = in_ex_splitted.second;

        if (!excludes.empty()) {
            // Contains an exclude
            filter_mode = EXCLUDE;
            source_list = vector_union(get_source_lists(excludes));
            source_list = vector_minus(source_list, vector_union(get_source_lists(includes)));
            // TODO: toevoegen aan records
        } else {
            // All includes
            filter_mode = INCLUDE;
            source_list = vector_union(get_source_lists(includes));
            // TODO: toevoegen aan records
        }
    }
}

};

Vector <in_addr> InterfaceMulticastTable::vector_minus(Vector <in_addr> vec_a, Vector <in_addr> vec_b) {
    Vector <in_addr> result = Vector<in_addr>();
    for (auto el: vec_a)
}

Vector<SocketRecord *> InterfaceMulticastTable::vector_union(Vector<Vector<SocketRecord *>> vectors) {
    Vector < SocketRecord * > unioned = Vector<SocketRecord *>();
    for (auto vector: vectors) {
        for (auto el: vector) {
            // TODO? Check if element already in unioned
            unioned.push_back(el);
        }
    }
    return unioned;
}

Vector <Vector<in_addr>> InterfaceMulticastTable::get_source_lists_union(Vector<SocketRecord *> record_list) {
    auto source_lists = Vector<SocketRecord *>();
    for (auto record: record_list) {
        source_lists.push_back(record);
    }
    return source_lists;
}

bool InterfaceMulticastTable::containsPair(Map <Pair<in_addr, in_addr>, Vector<SocketRecord *>> &map,
                                           Pair <in_addr, in_addr> key) {
    // src: https://stackoverflow.com/questions/3136520/determine-if-map-contains-a-value-for-a-key
    if (map.find(key) == map.end()) {
        return false;
    }
    return true;
}

bool InterfaceMulticastTable::contains(Vector <in_addr> vector, in_addr value) {
    // src: https://stackoverflow.com/questions/3136520/determine-if-map-contains-a-value-for-a-key
    if (map.find(key) == map.end()) {
        return false;
    }
    return true;
}