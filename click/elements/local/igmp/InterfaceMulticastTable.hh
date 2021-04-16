/**
 * In addition to the per-socket multicast reception state, a system
 * must also maintain or compute multicast reception state for each of
 * its interfaces. That state conceptually consists of a set of
 * records of the form:
 *
 * (multicast-address, filter-mode, source-list)
 *
 * At most one record per multicast-address exists for a given
 * interface. This per-interface state is derived from the per-socket
 * state, but may differ from the per-socket state when different
 * sockets have differing filter modes and/or source lists for the
 * same multicast address and interface. (rfc3376, 3.2)
*/


struct InterfaceRecord {
    auto multicast_address;
    auto filter_mode;
    auto source_list;
};

class InterfaceMulticastTable {
    Vector <InterfaceRecord> records;

    void addToMapVector(Map <Pair<in_addr, in_addr>, Vector<SocketRecord *>> multicast_pairs, SocketRecord *record) {
        auto key = Pair<in_addr, in_addr>(record.multicast_address);
        if (!containsPair(multicast_pairs, key)) {
            multicast_pairs[key] = Vector({record});
        } else {
            multicast_pairs[key].push_back(record);
        }
    }

    Pair<Vector<SocketRecord*>, Vector<SocketRecord*>> splitIncludeExclude(Vector<SocketRecord*> records){
        // First item are all include records, second all exclude records
        Vector<SocketRecord*> includes;
        Vector<SocketRecord*> excludes;
        for (auto record: records){
            if (record->is_include()){
                includes.push_back(record);
            }
            if (record->is_exclude()){
                excludes.push_back(record);
            }
        }
        return Pair<Vector<SocketRecord*>, Vector<SocketRecord*>>(includes, excludes);
    }

    auto sourceListContainsExclude(){

    }

    void update(SocketMulticastTable table) {
        // Get all (interface, multicast_address) pairs
        Map <Pair<in_addr, in_addr>, Vector<SocketRecord *>> multicast_pairs; // rfc p.6
        for (SocketRecord *record: table.records) {
            addToMapVector(multicast_pairs, record)
        }

        // Update interface table per pair
        for (auto const &x : multicast_pairs) {
            Pair<in_addr, in_addr> key = x.first;
            Vector<SocketRecord*> all_records = x.second;

            auto in_ex_splitted = splitIncludeExclude(all_records);
            Vector<SocketRecord*> includes = in_ex_splitted.first;
            Vector<SocketRecord*> excludes = in_ex_splitted.second;

            if (!excludes.empty()) {
                // Contains an exclude
                filter_mode = EXCLUDE;
                source_list = vector_union(get_source_lists(excludes));
                source_list = vector_minus(source_list, get_source_lists(includes));
            } else {
                // All includes
                filter_mode = INCLUDE;
                source_list = vector_union(get_source_lists(includes));
            }
        }
    }
};

bool containsPair(
<Pair<in_addr, in_addr>, Vector<SocketRecord *>> &map, Pair<in_addr, in_addr>
key) {
// src: https://stackoverflow.com/questions/3136520/determine-if-map-contains-a-value-for-a-key
if (map.
find(key)
== map.

end()

) return false;
return true;
}