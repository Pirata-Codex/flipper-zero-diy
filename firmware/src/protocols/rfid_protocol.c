#include "rfid_protocol.h"

// EM4100 64-bit Format:
// 9 header bits: 111111111
// 10 lines of (4 data bits + 1 even parity bit)
// 4 column even parity bits
// 1 stop bit: 0

static bool calculate_even_parity_4bit(uint8_t nibble) {
    uint8_t count = 0;
    for (int i = 0; i < 4; i++) {
        if ((nibble >> i) & 1) count++;
    }
    return (count % 2) != 0; // Even parity -> parity bit is 1 if odd number of 1s
}

bool rfid_decode_em4100_bits(uint64_t raw_bits, rfid_em4100_t *out_card) {
    if (!out_card) return false;

    // Verify 9-bit header (bits 63..55 must be 1s)
    uint64_t header = (raw_bits >> 55) & 0x1FF;
    if (header != 0x1FF) {
        return false;
    }

    // Verify stop bit (bit 0 must be 0)
    if ((raw_bits & 1) != 0) {
        return false;
    }

    uint8_t data_nibbles[10];
    uint8_t col_parity_calc[4] = {0, 0, 0, 0};

    // Extract 10 rows (bits 54 down to 5)
    for (int row = 0; row < 10; row++) {
        uint8_t shift = 50 - (row * 5);
        uint8_t row_5bit = (raw_bits >> shift) & 0x1F;
        uint8_t data_4bit = (row_5bit >> 1) & 0x0F;
        bool parity_bit   = row_5bit & 1;

        // Verify row parity
        if (calculate_even_parity_4bit(data_4bit) != parity_bit) {
            return false;
        }

        data_nibbles[row] = data_4bit;

        // Accumulate column parity
        for (int col = 0; col < 4; col++) {
            if ((data_4bit >> (3 - col)) & 1) {
                col_parity_calc[col]++;
            }
        }
    }

    // Verify 4 column parity bits (bits 4..1)
    uint8_t raw_col_parity = (raw_bits >> 1) & 0x0F;
    for (int col = 0; col < 4; col++) {
        bool expected_col_parity = (col_parity_calc[col] % 2) != 0;
        bool actual_col_parity   = (raw_col_parity >> (3 - col)) & 1;
        if (expected_col_parity != actual_col_parity) {
            return false;
        }
    }

    // Version/Customer ID = nibble 0 & 1
    out_card->version_or_customer_id = (data_nibbles[0] << 4) | data_nibbles[1];

    // Card Number = nibbles 2..9
    uint32_t card_num = 0;
    for (int i = 2; i < 10; i++) {
        card_num = (card_num << 4) | data_nibbles[i];
    }
    out_card->card_number = card_num;
    out_card->raw_payload_64 = raw_bits;

    return true;
}

uint64_t rfid_encode_em4100_bits(uint8_t version, uint32_t card_number) {
    uint64_t raw_bits = 0;

    // Header: 9 ones at MSB (bits 63..55)
    raw_bits |= ((uint64_t)0x1FF << 55);

    uint8_t data_nibbles[10];
    data_nibbles[0] = (version >> 4) & 0x0F;
    data_nibbles[1] = version & 0x0F;

    for (int i = 0; i < 8; i++) {
        data_nibbles[2 + i] = (card_number >> (28 - i * 4)) & 0x0F;
    }

    uint8_t col_parity_count[4] = {0, 0, 0, 0};

    // Pack 10 rows
    for (int row = 0; row < 10; row++) {
        uint8_t nibble = data_nibbles[row];
        bool parity = calculate_even_parity_4bit(nibble);

        uint8_t row_5bit = (nibble << 1) | (parity ? 1 : 0);
        uint8_t shift = 50 - (row * 5);
        raw_bits |= ((uint64_t)row_5bit << shift);

        for (int col = 0; col < 4; col++) {
            if ((nibble >> (3 - col)) & 1) {
                col_parity_count[col]++;
            }
        }
    }

    // Column parity bits (bits 4..1)
    uint8_t col_parity = 0;
    for (int col = 0; col < 4; col++) {
        if (col_parity_count[col] % 2 != 0) {
            col_parity |= (1 << (3 - col));
        }
    }
    raw_bits |= ((uint64_t)col_parity << 1);

    // Stop bit at bit 0 = 0

    return raw_bits;
}
