#include "subghz_protocol.h"

// Princeton (PT2262) timing tolerance and decoding logic
// Typical TE ~ 350us to 500us
// Bit 0: High 1*TE, Low 3*TE, High 1*TE, Low 3*TE
// Bit 1: High 3*TE, Low 1*TE, High 3*TE, Low 1*TE
// Bit F (Float): High 1*TE, Low 3*TE, High 3*TE, Low 1*TE

#define PRINCETON_TE_DEFAULT 350

bool subghz_decode_princeton(const uint16_t *pulses, uint16_t count, subghz_packet_t *out_packet) {
    if (!pulses || !out_packet || count < 48) return false;

    // Princeton 24 bits = 48 pulses (High + Low per bit)
    uint32_t decoded_val = 0;
    uint16_t estimated_te = pulses[0] / 3 > 0 ? pulses[0] / 3 : PRINCETON_TE_DEFAULT;

    for (int i = 0; i < 24; i++) {
        uint16_t high1 = pulses[i * 2];
        uint16_t low1  = pulses[i * 2 + 1];

        bool is_short_high = (high1 < estimated_te * 2);
        bool is_short_low  = (low1 < estimated_te * 2);

        decoded_val <<= 1;
        if (!is_short_high && is_short_low) {
            decoded_val |= 1; // Bit 1 (Long High, Short Low)
        } else if (is_short_high && !is_short_low) {
            decoded_val |= 0; // Bit 0 (Short High, Long Low)
        }
    }

    out_packet->type = SUBGHZ_PROTOCOL_PRINCETON;
    out_packet->payload = decoded_val;
    out_packet->bit_count = 24;
    out_packet->te_us = estimated_te;
    out_packet->modulation = SUBGHZ_MODULATION_AM_ASK;
    return true;
}

uint16_t subghz_encode_princeton(const subghz_packet_t *packet, uint16_t *out_pulses, uint16_t max_pulses) {
    if (!packet || !out_pulses || max_pulses < 50) return 0;

    uint16_t te = packet->te_us > 0 ? packet->te_us : PRINCETON_TE_DEFAULT;
    uint16_t idx = 0;

    for (int i = packet->bit_count - 1; i >= 0; i--) {
        bool bit = (packet->payload >> i) & 1;
        if (bit) {
            // Bit 1: 3*TE High, 1*TE Low
            out_pulses[idx++] = 3 * te;
            out_pulses[idx++] = 1 * te;
        } else {
            // Bit 0: 1*TE High, 3*TE Low
            out_pulses[idx++] = 1 * te;
            out_pulses[idx++] = 3 * te;
        }
    }

    // Sync bit: 1*TE High, 31*TE Low
    out_pulses[idx++] = 1 * te;
    out_pulses[idx++] = 31 * te;

    return idx;
}

bool subghz_decode_manchester(const uint16_t *pulses, uint16_t count, subghz_packet_t *out_packet) {
    if (!pulses || !out_packet || count < 24) return false;

    uint32_t decoded = 0;
    for (int i = 0; i < 12 && (i * 2 + 1) < count; i++) {
        uint16_t h = pulses[i * 2];
        uint16_t l = pulses[i * 2 + 1];

        decoded <<= 1;
        if (h > l) {
            decoded |= 1;
        }
    }

    out_packet->type = SUBGHZ_PROTOCOL_MANCHESTER;
    out_packet->payload = decoded;
    out_packet->bit_count = 12;
    out_packet->te_us = 400;
    out_packet->modulation = SUBGHZ_MODULATION_AM_ASK;
    return true;
}

uint16_t subghz_encode_manchester(const subghz_packet_t *packet, uint16_t *out_pulses, uint16_t max_pulses) {
    if (!packet || !out_pulses || max_pulses < (packet->bit_count * 2)) return 0;

    uint16_t te = packet->te_us > 0 ? packet->te_us : 400;
    uint16_t idx = 0;

    for (int i = packet->bit_count - 1; i >= 0; i--) {
        bool bit = (packet->payload >> i) & 1;
        if (bit) {
            out_pulses[idx++] = te;
            out_pulses[idx++] = te;
        } else {
            out_pulses[idx++] = te;
            out_pulses[idx++] = te * 2;
        }
    }
    return idx;
}
