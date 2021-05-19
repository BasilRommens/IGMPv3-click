#ifndef TCSP_IGMPV3_QUERY_HH
#define TCSP_IGMPV3_QUERY_HH

#include <click/packet.hh>
#include <click/vector.hh>

struct Query {
    /**
     * The type indicates what the packet is, this means that it is a
     * query packet
     * 8 bits
     */
    uint8_t type = 0x11;

    /**
     * The Max Resp Code field specifies the maximum time allowed before
     * sending a responding report. The actual time allowed, called the Max
     * Resp Time, is represented in units of 1/10 second
     * (More at RFC 3376 Section 4.1.1)
     * 8 bits
     */
    uint8_t maxRespCode;

    /** The Checksum is the 16-bit one’s complement of the one’s complement
     * sum of the whole IGMP message (the entire IP payload). For computing
     * the checksum, the Checksum field is set to zero. When receiving
     * packets, the checksum MUST be verified before processing a packet.
     * (RFC 3376 Section 4.1.2)
     *
     * 16 bits
     */
    uint16_t checksum;

    /**
     * The Group Address field is set to zero when sending a General Query,
     * and set to the IP multicast address being queried when sending a
     * Group-Specific Query or Group-and-Source-Specific Query (see section
     * 4.1.9, below).
     * (RFC 3376 section 4.1.3)
     * 32 bits, IP address
     */
    in_addr groupAddress;

    /**
     * This is a combination of 3 different parts in the packet, namely
     * Resv (first 4 bits), S (next 1 bit) and QRV (last 3 bits),
     * these are combined in one single byte
     * See RFC 3376 section 4.1.(4-6). for more information
     */
    uint8_t special = 0;

    /**
     * The Querier’s Query Interval Code field specifies the [Query
     * Interval] used by the querier. The actual interval, called the
     * Querier’s Query Interval (QQI), is represented in units of seconds
     * (more at RFC 3376 section 4.1.7)
     * 8 bits
     */
    uint8_t QQIC;

    /**
     * The Number of Sources (N) field specifies how many source addresses
     * are present in the Query. This number is zero in a General Query or
     * a Group-Specific Query, and non-zero in a Group-and-Source-Specific
     * Query. This number is limited by the MTU of the network over which
     * the Query is transmitted. For example, on an Ethernet with an MTU of
     * 1500 octets, the IP header including the Router Alert option consumes
     * 24 octets, and the IGMP fields up to including the Number of Sources
     * (N) field consume 12 octets, leaving 1464 octets for source
     * addresses, which limits the number of source addresses to 366
     * (1464/4).
     * (RFC 3376 section 4.1.8)
     * 16 bits
     */
    uint16_t numberOfSources;

    /**
     * The Source Address [i] fields are a vector of n IP unicast addresses,
     * where n is the value in the Number of Sources (N) field
     * (RFC 3376 section 4.1.9)
     * Vector of IP Addresses (32 bit each)
     */
    Vector<in_addr> sourceAddresses;

    int getMaxResponseTime();

    void setMaxRespCode(uint8_t);

    void setGroupAddress(in_addr);

    in_addr getGroupAddress();

    void setReservationField(uint8_t = 0);

    uint8_t getReservationField();

    void setSFlag(uint8_t);

    uint8_t getSFlag();

    void setQRV(uint8_t);

    uint8_t getQRV();

    void setQQIC(uint8_t);

    uint8_t getQQI();

    void setNumberOfSources(int);

    uint16_t getNumberOfSources();

    void addSourceAddress(in_addr);

    void removeSourceAddress(in_addr);

    void removeAllSourceAddress();

    in_addr getSourceAddress(uint16_t);

    Vector<in_addr> getAllSourceAddresses();

    WritablePacket* createPacket();

    /**
     * A "General Query" is sent by a multicast router to learn the
     * complete multicast reception state of the neighboring interfaces
     * (that is, the interfaces attached to the network on which the
     * Query is transmitted). In a General Query, both the Group Address
     * field and the Number of Sources (N) field are zero.
     *
     * (RFC 3376, section 4.1.11.)
     * @return if this query is general query
     */
    bool isGeneralQuery();

    /**
     * A "Group-Specific Query" is sent by a multicast router to learn
     * the reception state, with respect to a *single* multicast address,
     * of the neighboring interfaces. In a Group-Specific Query, the
     * Group Address field contains the multicast address of interest,
     * and the Number of Sources (N) field contains zero.
     *
     * (RFC 3376, section 4.1.11.)
     * @return if this query is a group specific query
     */
    bool isGroupSpecificQuery();

    bool isSourceListEmpty();

    int size();

private:
    /**
     * Converts a code to a value as described in RFC 3376 section 4.1.7. and 4.1.1.
     */
    int codeToValue(int code);
};

struct QueryPacket {
    uint32_t RouterAlertOption;
    uint8_t type = 0x11;
    uint8_t maxRespCode;
    uint16_t checksum;
    in_addr groupAddress;
    uint8_t special = 0;
    uint8_t QQIC;
    uint16_t numberOfSources;
    in_addr sourceAddresses[0];

    Query* to_query();
    uint8_t getSFlag();

};

#endif //TCSP_IGMPV3_QUERY_HH
