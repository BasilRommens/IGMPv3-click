//
// Created by basil on 18.05.21.
//

#ifndef TCSP_IGMPV3_HELPER_HH
#define TCSP_IGMPV3_HELPER_HH

inline const unsigned char *
get_data_offset_4(Packet* packet) {
    return packet->data()-4;
}

#endif //TCSP_IGMPV3_HELPER_HH
