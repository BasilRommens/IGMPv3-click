#include "query.hh"

// TODO fix this piece of code
int Query::getMaxResponseTime() {
    if (maxRespCode < 128) {
        return maxRespCode;
    } else {
        // 0th bit = 1
        // bit 1-3 exponent
        uint8_t exp = (maxRespCode >> 4) & 0x7;
        // bit 4-7 mantissa
        uint8_t mant = maxRespCode & 0xF;
        return (mant | 0x10) << (exp + 3);
    }
}

void Query::setMaxRespCode(uint8_t _maxRespCode) {
    maxRespCode = _maxRespCode;
}

void Query::setCheckSum() {
    // TODO calculate checksum
}

uint8_t Query::getCheckSum() {
    return checksum;
}

void Query::setGroupAddress(in_addr _groupAddress) {
    groupAddress = _groupAddress;
}

in_addr Query::getGroupAddress() {
    return groupAddress;
}

void Query::setReservationField(uint8_t _resv) {
    resv = _resv;
}

uint8_t Query::getReservationField() {
    return resv;
}

void Query::setSFlag(uint8_t) {
    // TODO
}

uint8_t Query::getSFlag() {
    return s;
}

void Query::setQRV(uint8_t _QRV) {
    // TODO
}

uint8_t Query::getQRV() {
    // TODO
}

void Query::setQQIC(uint8_t _QQIC) {
    // TODO
}

uint8_t Query::getQQIC() {
    // TODO
}

void Query::setNumberOfSources(int _numberOfSources) {
    // TODO
    numberOfSources = _numberOfSources
}

uint16_t Query::getNumberOfSources() {
    // TODO
}

void Query::addSourceAddress(in_addr) {
    // TODO
}

void Query::removeSourceAddress(in_addr) {
    // TODO
}

void Query::removeAllSourceAddress() {
    // TODO
}

in_addr Query::getSourceAddress(uint16_t) {
    // TODO
}

Vector<in_addr> Query::getAllSourceAddresses() {
    return sourceAddress;
}

Vector<uint8_t> Query::getEntirePacket() {
    // TODO generate entire packet
    return nullptr
}