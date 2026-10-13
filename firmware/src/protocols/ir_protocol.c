#include "ir_protocol.h"

// NEC Protocol Frame:
// 1. Header: 9000us mark, 4500us space
// 2. 32 bits data (Address, ~Address, Command, ~Command)
//    - Bit 0: 560us mark, 560us space
//    - Bit 1: 560us mark, 1690us space
// 3. Trailer: 560us mark

static bool is_around(uint16_t val, uint16_t target, uint16_t tolerance) {
    return (val >= (target - tolerance)) && (val <= (target + tolerance));
}

bool ir_decode_nec(const uint16_t *durations, uint16_t count, ir_packet_t *out_packet) {
    // 1 header mark + 1 header space + 32*2 bit mark/space + 1 trailer = 67 timings
    if (!durations || !out_packet || count < 67) return false;

    // Verify header
    if (!is_around(durations[0], NEC_HEADER_MARK, 1000) ||
        !is_around(durations[1], NEC_HEADER_SPACE, 1000)) {
        return false;
    }

    uint32_t data = 0;
    uint16_t idx = 2;

    for (int i = 0; i < 32; i++) {
        uint16_t mark  = durations[idx++];
        uint16_t space = durations[idx++];

        if (!is_around(mark, NEC_BIT_MARK, 300)) {
            return false;
        }

        data <<= 1;
        if (is_around(space, NEC_ONE_SPACE, 400)) {
            data |= 1;
        } else if (is_around(space, NEC_ZERO_SPACE, 300)) {
            data |= 0;
        } else {
            return false;
        }
    }

    uint8_t addr      = (data >> 24) & 0xFF;
    uint8_t addr_inv  = (data >> 16) & 0xFF;
    uint8_t cmd       = (data >> 8)  & 0xFF;
    uint8_t cmd_inv   = data & 0xFF;

    // Verify inverted checksums if standard NEC
    if ((uint8_t)(~addr_inv) == addr && (uint8_t)(~cmd_inv) == cmd) {
        out_packet->address = addr;
    } else {
        out_packet->address = (addr << 8) | addr_inv; // Extended NEC
    }

    out_packet->type = IR_PROTOCOL_NEC;
    out_packet->command = cmd;
    out_packet->is_repeat = false;
    return true;
}

uint16_t ir_encode_nec(const ir_packet_t *packet, uint16_t *out_durations, uint16_t max_durations) {
    if (!packet || !out_durations || max_durations < 68) return 0;

    uint16_t idx = 0;

    // Header
    out_durations[idx++] = NEC_HEADER_MARK;
    out_durations[idx++] = NEC_HEADER_SPACE;

    // Data payload: 16-bit address + 8-bit command + 8-bit inverse command
    uint32_t raw_data = 0;
    uint8_t addr_high = (packet->address >> 8) & 0xFF;
    uint8_t addr_low  = packet->address & 0xFF;
    if (addr_high == 0) {
        // Standard NEC
        raw_data = ((uint32_t)addr_low << 24) |
                   ((uint32_t)(~addr_low & 0xFF) << 16) |
                   ((uint32_t)packet->command << 8) |
                   ((uint32_t)(~packet->command & 0xFF));
    } else {
        // Extended NEC
        raw_data = ((uint32_t)addr_high << 24) |
                   ((uint32_t)addr_low << 16) |
                   ((uint32_t)packet->command << 8) |
                   ((uint32_t)(~packet->command & 0xFF));
    }

    for (int i = 31; i >= 0; i--) {
        bool bit = (raw_data >> i) & 1;
        out_durations[idx++] = NEC_BIT_MARK;
        out_durations[idx++] = bit ? NEC_ONE_SPACE : NEC_ZERO_SPACE;
    }

    // Trailer
    out_durations[idx++] = NEC_TRAILER_MARK;

    return idx;
}
