#ifndef TCSP_IGMPV3_REPORT_HH
#define TCSP_IGMPV3_REPORT_HH

#include <click/packet.hh>
#include <click/vector.hh>

struct GroupRecord;

/**
 * Version 3 Membership Reports are sent by IP systems to report (to
 *  neighboring routers) the current multicast reception state, or
 *  changes in the multicast reception state, of their interfaces. (rfc3376, 4.2)
 */
struct Report {
    /**
     * Type for a report is always 0x22
     */
    uint8_t type = 0x22;

    /**
     * The Reserved fields are set to zero on transmission, and ignored on
     * reception. (rfc3376, 4.2.1)
     */
    uint8_t reserved1 = 0x0;

    /**
     * The Checksum is the 16-bit one’s complement of the one’s complement
     * sum of the whole IGMP message (the entire IP payload). For computing
     * the checksum, the Checksum field is set to zero. When receiving
     * packets, the checksum MUST be verified before processing a message. (rfc3376, 4.2.2)
     */
    uint16_t checksum;

    /**
     * The Reserved fields are set to zero on transmission, and ignored on
     * reception. (rfc3376, 4.2.1)
     */
    uint16_t reserved2 = 0x0;

    /**
     * The Number of Group Records (M) field specifies how many Group
     * Records are present in this Report. (rfc3376, 4.2.3)
     */
    uint16_t num_group_records;

    /**
     * Each Group Record is a block of fields containing information
     * pertaining to the sender’s membership in a single multicast group on
     * the interface from which the Report is sent. (rfc3376, 4.2.4)
     */
     Vector<GroupRecord> group_records; // length is num_group_records

     void addGroupRecord(GroupRecord);

};


/**
 * Each Group Record is a block of fields containing information
 * pertaining to the sender’s membership in a single multicast group on
 * the interface from which the Report is sent. (rfc3376, 4.2.4)
 */
struct GroupRecord {

    /**
     * One of those:
     * "Current-State Record"
     *      1. MODE_IS_INCLUDE
     *      2. MODE_IS_EXCLUDE
     * "Filter-Mode-Change Record"
     *      3. CHANGE_TO_INCLUDE_MODE
     *      4. CHANGE_TO_EXCLUDE_MODE
     * More info can be found in rfc3376, 4.2.12
     */
    uint8_t record_type;
    uint8_t getRecordType();

    /**
     * The Aux Data Len field contains the length of the Auxiliary Data
     * field in this Group Record, in units of 32-bit words. It may contain
     * zero, to indicate the absence of any auxiliary data. (rfc3376, 4.2.6)
     */
    uint8_t aux_data_len = 0;

    /**
     * The Number of Sources (N) field specifies how many source addresses
     * are present in this Group Record. (rfc3376, 4.2.7)
     */
    uint16_t num_sources;
    uint16_t getNumSources();

    /**
     * The Multicast Address field contains the IP multicast address to
     * which this Group Record pertains. ((rfc3376, 4.2.8)
     */
    in_addr multicast_address;
    in_addr getMulticastAddress();

    /**
     * The Source Address [i] fields are a vector of n IP unicast addresses,
     * where n is the value in this record’s Number of Sources (N) field. ((rfc3376, 4.2.9)
     */
    Vector<in_addr> source_adresses;
    Vector<in_addr> getSourceAdresses();
    void add_source(uint32_t source);

    /**
     * The Auxiliary Data field, if present, contains additional information
     * pertaining to this Group Record. The protocol specified in this
     * document, IGMPv3, does not define any auxiliary data. Therefore,
     * implementations of IGMPv3 MUST NOT include any auxiliary data (i.e.,
     * MUST set the Aux Data Len field to zero) in any transmitted Group
     * Record, and MUST ignore any auxiliary data present in any received
     * Group Record. The semantics and internal encoding of the Auxiliary
     * Data field are to be defined by any future version or extension of
     * IGMP that uses this field. (rfc3376, 4.2.10)
     */
     // TODO fix
//    void auxilary_data; // (aux_data_len als aantal bits)

    /**
     * If the Packet Length field in the IP header of a received Report
     * indicates that there are additional octets of data present, beyond
     * the last Group Record, IGMPv3 implementations MUST include those
     * octets in the computation to verify the received IGMP Checksum, but
     * MUST otherwise ignore those additional octets. When sending a
     * Report, an IGMPv3 implementation MUST NOT include additional octets
     * beyond the last Group Record. (rfc3376, 4.2.11)
     */
};

/**
 * Routers MUST accept a report with a source address of 0.0.0.0. (rfc3376, 4.2.13)
 */

/**
 * Version 3 Reports are sent with an IP destination address of
 * 224.0.0.22, to which all IGMPv3-capable multicast routers listen. (rfc3376, 4.2.14)
 */

/**
 * If the set of Group Records required in a Report does not fit within
 * the size limit of a single Report message (as determined by the MTU
 * of the network on which it will be sent), the Group Records are sent
 * in as many Report messages as needed to report the entire set. (rfc3376, 4.2.16)
 */

#endif //TCSP_IGMPV3_REPORT_HH
