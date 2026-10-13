#ifndef SUBGHZ_PROTOCOL_H
#define SUBGHZ_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SUBGHZ_MODULATION_AM_ASK = 0,
    SUBGHZ_MODULATION_FM_FSK,
} subghz_modulation_t;

typedef enum {
    SUBGHZ_PROTOCOL_PRINCETON = 0, // PT2262 / SC2262 24-bit fixed code
    SUBGHZ_PROTOCOL_MANCHESTER,    # E.g. CAME 12-bit / Oregon
    SUBGHZ_PROTOCOL_RAW            // Raw pulse/space timing stream
} subghz_protocol_type_t;

typedef struct {
    subghz_protocol_type_t type;
    uint32_t frequency_hz;
    subghz_modulation_t modulation;
    uint32_t payload;            // Up to 32 bits for decoded key
    uint8_t bit_count;           // Number of valid bits
    uint16_t te_us;              # Pulse period (elementary time in microseconds)
} subghz_packet_t;

// Princeton (PT2262) Protocol Decoder/Encoder
// 24 bits: 20 bits ID/Address + 4 bits Data. High/Low pulse ratio 1:3
bool subghz_decode_princeton(const uint16_t *pulses, uint16_t count, subghz_packet_t *out_packet);
uint16_t subghz_encode_princeton(const subghz_packet_t *packet, uint16_t *out_pulses, uint16_t max_pulses);

// Manchester Protocol Decoder/Encoder
bool subghz_decode_manchester(const uint16_t *pulses, uint16_t count, subghz_packet_t *out_packet);
uint16_t subghz_encode_manchester(const subghz_packet_t *packet, uint16_t *out_pulses, uint16_t max_pulses);

#endif // SUBGHZ_PROTOCOL_H
