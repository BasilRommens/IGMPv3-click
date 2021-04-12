#include "query.hh"

// TODO fix this piece of code
int Query::getMaxResponseTime() {
    if (maxRespCode < 128) {
        return maxRespCode;
    } else {
        // 0de bit 1
        // bit 1-3 exponent
        uint8_t exp = (maxRespCode >> 4) & 0x7;
        // bit 4-7 mantisse
        uint8_t mant = maxRespCode & 0xF;
        return (mant | 0x10) << (exp + 3);
    }
}