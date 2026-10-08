#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "event_loop.h"
#include "input_service.h"
#include "subghz_protocol.h"
#include "ir_protocol.h"
#include "rfid_protocol.h"

// Simple Lightweight Unit Test Harness
static int tests_run = 0;
static int tests_passed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("  [FAIL] %s (%s:%d)\n", msg, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        tests_run++; \
        printf("[%d] Running %s...", tests_run, #test_func); \
        if (test_func()) { \
            tests_passed++; \
            printf(" [PASS]\n"); \
        } \
    } while(0)

// ----------------------------------------------------------------------------
// 1. EVENT LOOP UNIT TESTS
// ----------------------------------------------------------------------------
static bool test_event_loop_basic(void) {
    event_queue_t eq;
    event_loop_init(&eq);

    TEST_ASSERT(event_loop_is_empty(&eq), "Queue should be empty on init");

    event_t e1 = { .type = EVENT_TYPE_TICK, .timestamp_ms = 100 };
    event_t e2 = { .type = EVENT_TYPE_INPUT, .timestamp_ms = 200 };

    TEST_ASSERT(event_loop_push(&eq, &e1), "Push e1 failed");
    TEST_ASSERT(event_loop_push(&eq, &e2), "Push e2 failed");
    TEST_ASSERT(event_loop_size(&eq) == 2, "Size should be 2");

    event_t out;
    TEST_ASSERT(event_loop_pop(&eq, &out), "Pop e1 failed");
    TEST_ASSERT(out.type == EVENT_TYPE_TICK && out.timestamp_ms == 100, "e1 match failed");

    TEST_ASSERT(event_loop_pop(&eq, &out), "Pop e2 failed");
    TEST_ASSERT(out.type == EVENT_TYPE_INPUT && out.timestamp_ms == 200, "e2 match failed");

    TEST_ASSERT(event_loop_is_empty(&eq), "Queue should be empty after pops");
    return true;
}

// ----------------------------------------------------------------------------
// 2. INPUT SERVICE UNIT TESTS
// ----------------------------------------------------------------------------
static bool test_input_service_press_and_release(void) {
    event_queue_t eq;
    event_loop_init(&eq);

    input_service_t input;
    input_service_init(&input, &eq);

    // Simulate Press
    input_service_update_key(&input, INPUT_KEY_OK, true, 1000);
    TEST_ASSERT(event_loop_size(&eq) == 1, "Press should push 1 event");

    event_t evt;
    event_loop_pop(&eq, &evt);
    TEST_ASSERT(evt.type == EVENT_TYPE_INPUT, "Event type must be INPUT");
    TEST_ASSERT(evt.data.input.key == INPUT_KEY_OK, "Key must be OK");
    TEST_ASSERT(evt.data.input.type == INPUT_TYPE_PRESS, "Event subtype must be PRESS");

    // Simulate Release after 100ms (Short Click)
    input_service_update_key(&input, INPUT_KEY_OK, false, 1100);
    TEST_ASSERT(event_loop_size(&eq) == 2, "Release should push RELEASE + SHORT_CLICK");

    event_loop_pop(&eq, &evt);
    TEST_ASSERT(evt.data.input.type == INPUT_TYPE_RELEASE, "First event must be RELEASE");

    event_loop_pop(&eq, &evt);
    TEST_ASSERT(evt.data.input.type == INPUT_TYPE_SHORT_CLICK, "Second event must be SHORT_CLICK");

    return true;
}

static bool test_input_service_long_press(void) {
    event_queue_t eq;
    event_loop_init(&eq);

    input_service_t input;
    input_service_init(&input, &eq);

    // Press at t=1000
    input_service_update_key(&input, INPUT_KEY_BACK, true, 1000);
    TEST_ASSERT(event_loop_size(&eq) == 1, "Press event queued");

    // Hold for 300ms (below threshold)
    input_service_update_key(&input, INPUT_KEY_BACK, true, 1300);
    TEST_ASSERT(event_loop_size(&eq) == 1, "No new event before threshold");

    // Hold for 600ms (above threshold 500ms)
    input_service_update_key(&input, INPUT_KEY_BACK, true, 1600);
    TEST_ASSERT(event_loop_size(&eq) == 2, "Long press event should fire");

    event_t evt;
    event_loop_pop(&eq, &evt); // Pop Press
    event_loop_pop(&eq, &evt); // Pop Long Press
    TEST_ASSERT(evt.data.input.type == INPUT_TYPE_LONG_PRESS, "Must be LONG_PRESS");

    // Release after long press -> NO short click
    input_service_update_key(&input, INPUT_KEY_BACK, false, 1700);
    TEST_ASSERT(event_loop_size(&eq) == 1, "Only RELEASE event should be pushed");

    event_loop_pop(&eq, &evt);
    TEST_ASSERT(evt.data.input.type == INPUT_TYPE_RELEASE, "Must be RELEASE");

    return true;
}

// ----------------------------------------------------------------------------
// 3. SUB-GHZ PROTOCOL UNIT TESTS
// ----------------------------------------------------------------------------
static bool test_subghz_princeton_roundtrip(void) {
    subghz_packet_t original = {
        .type = SUBGHZ_PROTOCOL_PRINCETON,
        .frequency_hz = 433920000,
        .modulation = SUBGHZ_MODULATION_AM_ASK,
        .payload = 0xABCD12, // 24-bit key
        .bit_count = 24,
        .te_us = 350
    };

    uint16_t pulses[60];
    uint16_t count = subghz_encode_princeton(&original, pulses, 60);
    TEST_ASSERT(count > 0, "Princeton encoding failed");

    subghz_packet_t decoded;
    bool status = subghz_decode_princeton(pulses, count, &decoded);
    TEST_ASSERT(status, "Princeton decoding failed");
    TEST_ASSERT(decoded.payload == original.payload, "Payload mismatch");
    TEST_ASSERT(decoded.bit_count == 24, "Bit count mismatch");

    return true;
}

// ----------------------------------------------------------------------------
// 4. INFRARED NEC PROTOCOL UNIT TESTS
// ----------------------------------------------------------------------------
static bool test_ir_nec_roundtrip(void) {
    ir_packet_t original = {
        .type = IR_PROTOCOL_NEC,
        .address = 0x07, // Apple TV remote address
        .command = 0x5A, // Play/Pause
        .is_repeat = false
    };

    uint16_t timings[100];
    uint16_t count = ir_encode_nec(&original, timings, 100);
    TEST_ASSERT(count == 67, "NEC pulse count must be 67");

    ir_packet_t decoded;
    bool status = ir_decode_nec(timings, count, &decoded);
    TEST_ASSERT(status, "NEC decoding failed");
    TEST_ASSERT(decoded.address == original.address, "NEC address mismatch");
    TEST_ASSERT(decoded.command == original.command, "NEC command mismatch");

    return true;
}

// ----------------------------------------------------------------------------
// 5. RFID 125KHZ EM4100 UNIT TESTS
// ----------------------------------------------------------------------------
static bool test_rfid_em4100_encode_decode(void) {
    uint8_t version = 0x12;
    uint32_t card_num = 0x00ABCD12;

    uint64_t raw_bits = rfid_encode_em4100_bits(version, card_num);
    TEST_ASSERT(raw_bits != 0, "RFID encoding failed");

    rfid_em4100_t card;
    bool status = rfid_decode_em4100_bits(raw_bits, &card);
    TEST_ASSERT(status, "RFID decoding & parity check failed");
    TEST_ASSERT(card.version_or_customer_id == version, "Version mismatch");
    TEST_ASSERT(card.card_number == card_num, "Card number mismatch");

    return true;
}

// ----------------------------------------------------------------------------
// MAIN UNIT TEST RUNNER ENTRY POINT
// ----------------------------------------------------------------------------
int main(void) {
    printf("\n==================================================\n");
    printf("     FLIPPER ZERO DIY — CORE LOGIC UNIT TESTS     \n");
    printf("==================================================\n\n");

    RUN_TEST(test_event_loop_basic);
    RUN_TEST(test_input_service_press_and_release);
    RUN_TEST(test_input_service_long_press);
    RUN_TEST(test_subghz_princeton_roundtrip);
    RUN_TEST(test_ir_nec_roundtrip);
    RUN_TEST(test_rfid_em4100_encode_decode);

    printf("\n--------------------------------------------------\n");
    printf("TEST RESULTS: %d / %d PASSED\n", tests_passed, tests_run);
    printf("--------------------------------------------------\n\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
