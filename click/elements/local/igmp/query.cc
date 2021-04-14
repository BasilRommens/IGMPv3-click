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

void Query::setCheckSum()
{
    // TODO calculate checksum
}

uint8_t Query::getCheckSum()
{
    return checksum;
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
    special = special << 4 >> 4 | ((_resv << 4) | 0xf);
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
    special |= _SFlag & 0x1;
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
        special &= 0xf8 | _QRV >> 5 << 5;
    }
}

uint8_t Query::getQRV()
{
    // returns the QRV only using the last 3 bits of the special field
    return special & 0x7;
}

void Query::setQQIC(uint8_t _QQIC)
{
    QQIC = _QQIC
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

void Query::addSourceAddress(in_addr _sourceAddress)
{
    // Do not exceed 366 number of sources, add if possible +
    // increase number of sources
    if (numberOfSources<366) {
        sourceAddress.push_back(_sourceAddress);
        numberOfSources += 1;
    }
}

void Query::removeSourceAddress(in_addr)
{
    // TODO
}

void Query::removeAllSourceAddress()
{
    sourceAddress.clear();
}

in_addr Query::getSourceAddress(uint16_t addressLocation)
{
    return sourceAddress[addressLocation]
}

Vector<in_addr> Query::getAllSourceAddresses()
{
    return sourceAddress;
}

Vector<uint8_t> Query::getEntirePacket()
{
    // TODO generate entire packet
    return nullptr
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

CLICK_ENDDECLS
