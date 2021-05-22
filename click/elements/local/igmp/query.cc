#include "query.hh"

CLICK_DECLS
int Query::getMaxResponseTime()
{
    return codeToValue(maxRespCode);
}

void Query::setMaxRespCode(uint8_t _maxRespCode)
{
    maxRespCode = _maxRespCode;
}


void Query::setMaxRespCodeFromTime(int time){
    maxRespCode = valueToCode(time);
}

void Query::setGroupAddress(in_addr _groupAddress)
{
    groupAddress = _groupAddress;
}

in_addr Query::getGroupAddress()
{
    return groupAddress;
}

void Query::setReservationField(uint8_t _resv)
{
    // Set the last 4 bits to
    special = (special << 4 >> 4) | ((_resv << 4) | 0xf0);
}

uint8_t Query::getReservationField()
{
    // return the first 4 bits from the special field in the packet
    return special >> 4;
}

void Query::setSFlag(uint8_t _SFlag)
{
    // take the first bit of the s flag and set it to the 5th bit of the special field
    // in the packet
    click_chatter("%d", special & 0xf7);
    special = special & 0xf7 | _SFlag << 3;
    click_chatter("%d", special);
}

uint8_t Query::getSFlag()
{
    // Take the fifth bit from the special field in the packet
    return (special >> 3) & 0x1;
}

void Query::setQRV(uint8_t _QRV)
{
    // 7 is the maximum value of QRV so if it exceeds we need to set it to 0,
    // otherwise keep it normal.
    if (_QRV>7) {
        special &= 0xf8;
    }
    else {
        special = special & 0xf8 | _QRV;
    }
}

uint8_t Query::getQRV()
{
    // returns the QRV only using the last 3 bits of the special field
    return special & 0x7;
}

void Query::setQQIC(uint8_t _QQIC)
{
    QQIC = _QQIC;
}

void Query::setQQICFromValue(int value)
{
    QQIC = valueToCode(value);
}

uint8_t Query::getQQI()
{
    return codeToValue(QQIC);
}

void Query::setNumberOfSources(int _numberOfSources)
{
    // Ensure that the number of sources never exceeds 366
    if (_numberOfSources<=366) {
        numberOfSources = _numberOfSources;
    }
}

uint16_t Query::getNumberOfSources()
{
    return numberOfSources;
}

void Query::addSourceAddress(in_addr _sourceAddresses)
{
    // Do not exceed 366 number of sources, add if possible +
    // increase number of sources
    if (numberOfSources<366) {
        sourceAddresses.push_back(_sourceAddresses);
        numberOfSources = ntohs(ntohs(numberOfSources)+1);
    }
}

void Query::removeSourceAddress(in_addr)
{
    // TODO
}

void Query::removeAllSourceAddress()
{
    sourceAddresses.clear();
}

in_addr Query::getSourceAddress(uint16_t addressLocation)
{
    return sourceAddresses[addressLocation];
}

Vector<in_addr> Query::getAllSourceAddresses()
{
    return sourceAddresses;
}

// TODO fix this piece of code
int Query::codeToValue(int code)
{
    if (code<128) {
        return code;
    }
    else {
        // 0th bit = 1
        // bit 1-3 exponent
        uint8_t exp = (code >> 4) & 0x7;
        // bit 4-7 mantissa
        uint8_t mant = code & 0xF;
        return (mant | 0x10) << (exp+3);
    }
}

int Query::valueToCode(int code)
{
    if (code<128) {
        return code;
    }
    else {
        uint8_t exp = log2(value);
        uint8_t mant = value/ pow(2, exp);

        mant = mant & 0x0f; // only keep last 4 bits of mant

        exp = exp & 0x07; // exp is 3 bits
        exp = exp << 4;

        uint8_t result = 0x80 | exp | mant; // first bit 1, then exp, then mant

        return result;
    }
}

int Query::size()
{
    int default_size = 12;
    int source_address_size = sourceAddresses.size()*4;
    return default_size+source_address_size;
}

WritablePacket* Query::createPacket()
{
    // Room for the IP header and Ether header which must be added later by
    // another element
    int headroom = sizeof(click_ip)+sizeof(click_ether)+4;

    WritablePacket* q = Packet::make(headroom, 0, this->size()+4, 0);
    if (!q) {
        return 0;
    }

    // Make packet data 0 to prevent weird problems
    memset(q->data(), '\0', q->length());

    // Cast the data to a report and set the attribute values
    QueryPacket* query = (QueryPacket*) (q->data());
    query->type = type;
    query->maxRespCode = maxRespCode;
    query->groupAddress = groupAddress;
    query->special = special;
    query->QQIC = QQIC;
    query->numberOfSources = numberOfSources;

    for (int i = 0; i<htons(numberOfSources); i++) {
        query->sourceAddresses[i] = sourceAddresses[i];
    }

    query->checksum = click_in_cksum(q->data(), q->length());
    query->RouterAlertOption = htonl(0x94040000);

    /**
     * In IGMPv3, General Queries are sent with an IP destination address of
     * 224.0.0.1, the all-systems multicast address. Group-Specific and
     * Group-and-Source-Specific Queries are sent with an IP destination
     * address equal to the multicast address of interest. *However*, a
     * system MUST accept and process any Query whose IP Destination
     * Address field contains *any* of the addresses (unicast or multicast)
     * assigned to the interface on which the Query arrives.
     *
     * RFC 3376, section 4.1.12.
     */
    IPAddress query_address = isGeneralQuery() ? IPAddress("224.0.0.22") : IPAddress(groupAddress);
    q->set_dst_ip_anno(query_address);
    return q;
}

Query* QueryPacket::to_query()
{
    Query* query_to_return = new Query();

    // Transfer all the contents over to the query
    query_to_return->type = this->type;
    query_to_return->maxRespCode = this->maxRespCode;
    query_to_return->checksum = this->checksum;
    query_to_return->groupAddress = this->groupAddress;
    query_to_return->special = this->special;
    query_to_return->QQIC = this->QQIC;
    query_to_return->numberOfSources = this->numberOfSources;
    // make a vector of the array in the query
    for (int idx = 0; idx<this->numberOfSources; idx++) {
        query_to_return->sourceAddresses.push_back(this->sourceAddresses[idx]);
    }

    return query_to_return;
}

bool Query::isGeneralQuery()
{
    return groupAddress==0 and numberOfSources==0;
}

bool Query::isGroupSpecificQuery()
{
    return IPAddress(groupAddress).is_multicast() and numberOfSources==0;
}

bool Query::isSourceListEmpty()
{
    return sourceAddresses.size()==0;
}

bool Query::hasCorrectChecksum()
{
    return getChecksum() == checksum;
}

uint16_t Query::getChecksum()
{
    WritablePacket* q = Packet::make(0, 0, this->size()+4, 0);
    if (!q) {
        return 0;
    }

    // Make packet data 0 to prevent weird problems
    memset(q->data(), '\0', q->length());

    // Cast the data to a report and set the attribute values
    QueryPacket* query = (QueryPacket*) (q->data());
    query->type = type;
    query->maxRespCode = maxRespCode;
    query->groupAddress = groupAddress;
    query->special = special;
    query->QQIC = QQIC;
    query->numberOfSources = numberOfSources;

    for (int i = 0; i<htons(numberOfSources); i++) {
        query->sourceAddresses[i] = sourceAddresses[i];
    }

    uint16_t new_checksum = click_in_cksum(q->data(), q->length());
    q->kill();
    return new_checksum;
}

uint8_t QueryPacket::getSFlag()
{
    // Take the fifth bit from the special field in the packet
    return (special >> 3) & 0x1;
}

CLICK_ENDDECLS
