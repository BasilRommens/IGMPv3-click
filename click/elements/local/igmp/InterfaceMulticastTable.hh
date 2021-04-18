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

#ifndef CLICK_InterfaceMulticastTable_HH
#define CLICK_InterfaceMulticastTable_HH

#include <click/vector.hh>
#include <click/pair.hh>

typedef Pair <in_addr, in_addr> InterfaceMulticastIdentifier;
typedef Vector <Pair<InterfaceMulticastIdentifier, Vector < SocketRecord * >>>
RecordMap; // Maps (interface, multicast) onto list of records

struct InterfaceRecord {
    in_addr multicast_address;
    int filter_mode;
    Vector <in_addr> source_list;

    InterfaceRecord(in_addr multicast_address, int filter_mode, Vector <in_addr> source_list);

    bool is_include();

    bool is_exclude();

    String to_string();
};

class InterfaceMulticastTable {
public:
    InterfaceMulticastTable();

    // Must be kept per interface -> Maybe use a map instead?
    Vector<InterfaceRecord *> records;

    /**
     * Appends the SocketRecord to the list corresponding to the (interface, multicast_adress) pair in the map
     * @param multicast_pairs (interface, multicast_address) pair
     * @param record Recrod to include at the position
     */
    void addToMapVector(RecordMap &multicast_pairs, SocketRecord *record);

    /**
     * Returns two seperate vector, one with the records with filter mode include, the other with the records with
     * filter mode exclude.
     * @param records List of all records that must be splitted
     * @return The given records splitted based on thair filter mode. First are the includes, second the excludes.
     */
    Pair <Vector<SocketRecord *>, Vector<SocketRecord *>> splitIncludeExclude(Vector<SocketRecord *> records);

    /**
     * Updates the interface records, based on a SocketMulticastTable
     * @param table Table containing the socket records
     */
    void update(SocketMulticastTable *table);

    /**
     * Given vector A and vector B, this returns a new vector A-B, which is the vector A with all elements that are in
     * B removed.
     * @param vec_a vector A
     * @param vec_b vector B
     * @return vector A-B
     */
    Vector <in_addr> vector_minus(Vector <in_addr> vec_a, Vector <in_addr> vec_b);

    /**
     * Given a list of vectors, this returns the union of all these vectors.
     * @param vectors List of vectors of socketrecords that must be unioned
     * @return The union of all given vectors
     */
    Vector<SocketRecord *> vector_union(Vector <Vector<SocketRecord *>> vectors);

    Vector <in_addr> vector_union(Vector <Vector<in_addr>> vectors);

    /**
     * Given a vector of socketRecord, return a vector containing all their source_lists
     * @param records A vector containing all SocketRecords
     * @return The source_lists corresponding to the SocketRecords
     */
    Vector <Vector<in_addr>> get_source_lists(Vector<SocketRecord *> records);

    /**
     * Given a Map and an identifier pair (interface, multicast_address), determine whether the pair already exists in
     * the map.
     * @param map A Map which maps an (interface, multicast_address) pair onto a vector of SocketRecords
     * @param key Identifier pair (interface, multicast_address) of which you want to check whether it's in the map
     * @return wether the pair is in the map
     */
    bool containsPair(RecordMap &map, InterfaceMulticastIdentifier key);

    /**
     * Check if a vector contains a given value
     * @param vector The vector to check
     * @param value The value to check
     * @return True if the vector contains the value
     */
    bool contains(Vector <in_addr> vector, in_addr value);

    /**
     * Adds an empty array to the map at index key
     */
    void mapAddNew(RecordMap &map, InterfaceMulticastIdentifier &key);

    Vector<SocketRecord *> &getMapValue(RecordMap &map, InterfaceMulticastIdentifier &key);


    // Komt niet echt overeen met de IS_IN, IS_EX uit de rfc, want die hebben een source_list als parameter
    bool is_in(in_addr multicast_address);

    bool is_ex(in_addr multicast_address);

    InterfaceRecord* getRecord(in_addr multicast_address);

    int filter_state(in_addr multicast_address);

    String to_string();

};

#endif