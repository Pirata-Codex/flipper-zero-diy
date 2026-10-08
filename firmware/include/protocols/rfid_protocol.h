#ifndef RFID_PROTOCOL_H
#define RFID_PROTOCOL_H

#include <stdint.16.h>
#include <stdint.h>
#include <stdbool.h>

// EM4100 125kHz RFID Tag (64 bits total)
// Header: 9 consecutive '1' bits
// 10 data groups: 4 data bits + 1 parity bit (even) = 50 bits
// Column parity: 4 bits
// Stop bit: 1 '0' bit
typedef struct {
    uint8_t version_or_customer_id; // 8 bits
    uint32_t card_number;           // 32 bits
    uint64_t raw_payload_64;        // Full 64-bit raw frame
} rfid_em4100_t;

bool rfid_decode_em4100_bits(uint64_t raw_bits, rfid_em4100_t *out_card);
uint64_t rfid_encode_em4100_bits(uint8_t version, uint32_t card_number);

#endif // RFID_PROTOCOL_H
