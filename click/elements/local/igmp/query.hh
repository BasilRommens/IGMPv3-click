#ifndef TCSP_IGMPV3_QUERY_HH
#define TCSP_IGMPV3_QUERY_HH

#include <click/packet.hh>
#include <click/vector.hh>

struct Query {
    // The type indicates what the packet is, this means that it is a
    // query packet
    // 8 bits
    uint8_t type = 0x11;
    // The Max Resp Code field specifies the maximum time allowed before
    // sending a responding report. The actual time allowed, called the Max
    // Resp Time, is represented in units of 1/10 second
    // 8 bits
    uint8_t maxRespCode;
    // The Checksum is the 16-bit one’s complement of the one’s complement
    // sum of the whole IGMP message (the entire IP payload). For computing
    // the checksum, the Checksum field is set to zero. When receiving
    // packets, the checksum MUST be verified before processing a packet.
    // 16 bits
    uint16_t checksum;
    //  The Group Address field is set to zero when sending a General Query,
    // and set to the IP multicast address being queried when sending a
    // Group-Specific Query or Group-and-Source-Specific Query (see section
    // 4.1.9, below).
    // 32 bits, IP address
    in_addr groupAddress;
    // The Resv field is set to zero on transmission, and ignored on
    // reception.
    // 4 bits, the last 4 bits are ignored
    uint8_t resv = 0;
    //  When set to one, the S Flag indicates to any receiving multicast
    // routers that they are to suppress the normal timer updates they
    // perform upon hearing a Query. It does not, however, suppress the
    // querier election or the normal "host-side" processing of a Query that
    // a router may be required to perform as a consequence of itself being
    // a group member.
    // 1 bit, the last 7 bits are ignored
    uint8_t s;
    // If non-zero, the QRV field contains the [Robustness Variable] value
    // used by the querier, i.e., the sender of the Query. If the querier’s
    // [Robustness Variable] exceeds 7, the maximum value of the QRV field,
    // the QRV is set to zero. Routers adopt the QRV value from the most
    // recently received Query as their own [Robustness Variable] value,
    // unless that most recently received QRV was zero, in which case the
    // receivers use the default [Robustness Variable] value specified in
    // section 8.1 or a statically configured value.
    // 3 bits, the last 5 bits are ignored
    uint8_t QRV;
    // The Querier’s Query Interval Code field specifies the [Query
    // Interval] used by the querier. The actual interval, called the
    // Querier’s Query Interval (QQI), is represented in units of seconds
    // 8 bits
    uint8_t QQIC;
    // The Number of Sources (N) field specifies how many source addresses
    // are present in the Query. This number is zero in a General Query or
    // a Group-Specific Query, and non-zero in a Group-and-Source-Specific
    // Query. This number is limited by the MTU of the network over which
    // the Query is transmitted. For example, on an Ethernet with an MTU of
    // 1500 octets, the IP header including the Router Alert option consumes
    // 24 octets, and the IGMP fields up to including the Number of Sources
    // (N) field consume 12 octets, leaving 1464 octets for source
    // addresses, which limits the number of source addresses to 366
    // (1464/4).
    // 16 bits
    uint16_t numberOfSources;
    // The Source Address [i] fields are a vector of n IP unicast addresses,
    // where n is the value in the Number of Sources (N) field
    // Vector of IP Addresses
    Vector<in_addr> sourceAddress;

    int getMaxResponseTime();
    void setMaxResponseTime(uint8_t);
    void setCheckSum();
    uint16_t getCheckSum();
    void setGroupAddress(in_addr);
    in_addr getGroupAddress();
    void setReservationField(uint8_t);
    uint8_t getReservationField();
    void setSFlag();
    uint8_t getSFlag();
    void setQRV(uint8_t);
    uint8_t getQRV();
    void setQQIC(uint8_t);
    uint8_t getQQIC();
    void setNumberOfSources(uint16_t);
    uint16_t getNumberOfSources();
    void addSourceAddress(in_addr);
    void removeSourceAddress(in_addr);
    void removeAllSourceAddress();
    in_addr getSourceAddress(uint16_t);
    Vector<in_addr> getAllSourceAddresses();
private:
    Vector<uint8_t> getEntirePacket();
};

#endif //TCSP_IGMPV3_QUERY_HH
