//
// Created by basil on 18.05.21.
//

#ifndef TCSP_IGMPV3_HELPER_HH
#define TCSP_IGMPV3_HELPER_HH

inline const unsigned char *
get_data_offset_4(Packet* packet) {
    return packet->data()-4;
}

inline double get_sec_before_expiry(Timer *timer) {
    // TODO: KLopt dit? Zou kunnen dat er ergens iets misloopt, dus best eens grondig testen
    //  ik zet ook gewoon die double om in een int, ik hoop da da geen problemen krijgt
    double msec = (timer->expiry_steady() - Timestamp::now_steady()).msecval();
    return msec / 1000.0;
}

#endif //TCSP_IGMPV3_HELPER_HH
