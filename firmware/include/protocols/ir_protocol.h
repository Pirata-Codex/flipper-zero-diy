#ifndef IR_PROTOCOL_H
#define IR_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    IR_PROTOCOL_NEC = 0,
    IR_PROTOCOL_RAW
} ir_protocol_type_t;

typedef struct {
    ir_protocol_type_t type;
    uint16_t address;
    uint8_t command;
    bool is_repeat;
} ir_packet_t;

// NEC Protocol Timing Constants (in microseconds)
#define NEC_HEADER_MARK    9000
#define NEC_HEADER_SPACE   4500
#define NEC_BIT_MARK        560
#define NEC_ONE_SPACE      1690
#define NEC_ZERO_SPACE      560
#define NEC_TRAILER_MARK    560

bool ir_decode_nec(const uint16_t *durations, uint16_t count, ir_packet_t *out_packet);
uint16_t ir_encode_nec(const ir_packet_t *packet, uint16_t *out_durations, uint16_t max_durations);

#endif // IR_PROTOCOL_H
