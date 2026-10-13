#include <stdio.h>
#include <string.h>
#include "hal_uart.h"
#include "hal_gpio.h"
#include "hal_display.h"
#include "gui.h"
#include "event_loop.h"
#include "input_service.h"
#include "subghz_protocol.h"
#include "ir_protocol.h"
#include "rfid_protocol.h"

int main(void) {
    // 1. Initialize Console UART
    hal_uart_init(HAL_UART_1, 115200);
    hal_uart_write_string(HAL_UART_1, "\n==================================================\n");
    hal_uart_write_string(HAL_UART_1, "  Flipper Zero DIY Firmware v1.0.0 (Renode Core)\n");
    hal_uart_write_string(HAL_UART_1, "==================================================\n\n");

    // 2. Initialize Event Queue & Input Service
    static event_queue_t main_eq;
    static input_service_t input_srv;
    event_loop_init(&main_eq);
    input_service_init(&input_srv, &main_eq);
    hal_uart_write_string(HAL_UART_1, "[INIT] Event Queue & Input Service Ready\n");

    // 3. Test Sub-GHz Princeton Protocol Engine
    subghz_packet_t subghz_tx = {
        .type = SUBGHZ_PROTOCOL_PRINCETON,
        .frequency_hz = 433920000,
        .payload = 0xABC123,
        .bit_count = 24,
        .te_us = 350
    };
    uint16_t subghz_pulses[60];
    uint16_t p_count = subghz_encode_princeton(&subghz_tx, subghz_pulses, 60);

    subghz_packet_t subghz_rx;
    if (subghz_decode_princeton(subghz_pulses, p_count, &subghz_rx)) {
        char buf[80];
        snprintf(buf, sizeof(buf), "[SUB-GHZ] Princeton 433MHz Key Decoded: 0x%06X (Bits: %d)\n",
                 (unsigned int)subghz_rx.payload, subghz_rx.bit_count);
        hal_uart_write_string(HAL_UART_1, buf);
    }

    // 4. Test IR NEC Protocol Engine
    ir_packet_t ir_tx = {
        .type = IR_PROTOCOL_NEC,
        .address = 0x0007, // Apple TV
        .command = 0x5A    // Play/Pause
    };
    uint16_t ir_durations[100];
    uint16_t ir_count = ir_encode_nec(&ir_tx, ir_durations, 100);

    ir_packet_t ir_rx;
    if (ir_decode_nec(ir_durations, ir_count, &ir_rx)) {
        char buf[80];
        snprintf(buf, sizeof(buf), "[IR] NEC Frame Decoded: Addr=0x%04X, Cmd=0x%02X\n",
                 ir_rx.address, ir_rx.command);
        hal_uart_write_string(HAL_UART_1, buf);
    }

    // 5. Test RFID 125kHz EM4100 Engine
    uint64_t raw_card = rfid_encode_em4100_bits(0x12, 0x00ABCD12);
    rfid_em4100_t card_rx;
    if (rfid_decode_em4100_bits(raw_card, &card_rx)) {
        char buf[80];
        snprintf(buf, sizeof(buf), "[RFID 125kHz] EM4100 Tag Read: Version=0x%02X, ID=0x%08X\n",
                 card_rx.version_or_customer_id, (unsigned int)card_rx.card_number);
        hal_uart_write_string(HAL_UART_1, buf);
    }

    // 6. Initialize GUI
    gui_init();
    gui_render_home_screen();
    hal_uart_write_string(HAL_UART_1, "\n[SYSTEM] System Ready! Waiting for button events...\n");

    // Main Loop
    uint32_t fake_timer = 0;
    while (1) {
        fake_timer += 10;

        // Process any queued events
        event_t evt;
        while (event_loop_pop(&main_eq, &evt)) {
            if (evt.type == EVENT_TYPE_INPUT) {
                const char *key_names[] = {"UP", "DOWN", "LEFT", "RIGHT", "OK", "BACK"};
                const char *evt_names[] = {"PRESS", "RELEASE", "SHORT_CLICK", "LONG_PRESS"};
                char buf[80];
                snprintf(buf, sizeof(buf), "[EVENT] Key %s -> %s (t=%dms)\n",
                         key_names[evt.data.input.key],
                         evt_names[evt.data.input.type],
                         (int)evt.timestamp_ms);
                hal_uart_write_string(HAL_UART_1, buf);
            }
        }

        for (volatile int i = 0; i < 50000; i++) {
            __asm__("nop");
        }
    }

    return 0;
}
