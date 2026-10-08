#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    EVENT_TYPE_NONE = 0,
    EVENT_TYPE_TICK,
    EVENT_TYPE_INPUT,
    EVENT_TYPE_RADIO_RX,
    EVENT_TYPE_IR_RX,
    EVENT_TYPE_RFID_READ,
    EVENT_TYPE_SYSTEM,
} event_type_t;

typedef enum {
    INPUT_KEY_UP = 0,
    INPUT_KEY_DOWN,
    INPUT_KEY_LEFT,
    INPUT_KEY_RIGHT,
    INPUT_KEY_OK,
    INPUT_KEY_BACK,
    INPUT_KEY_COUNT
} input_key_t;

typedef enum {
    INPUT_TYPE_PRESS,
    INPUT_TYPE_RELEASE,
    INPUT_TYPE_SHORT_CLICK,
    INPUT_TYPE_LONG_PRESS
} input_type_t;

typedef struct {
    input_key_t key;
    input_type_t type;
} input_event_data_t;

typedef struct {
    uint32_t frequency;
    uint16_t length;
    const uint8_t *payload;
} radio_rx_data_t;

typedef struct {
    uint32_t protocol;
    uint32_t address;
    uint32_t command;
} ir_rx_data_t;

typedef struct {
    uint64_t card_id;
} rfid_data_t;

typedef struct {
    event_type_t type;
    uint32_t timestamp_ms;
    union {
        input_event_data_t input;
        radio_rx_data_t radio;
        ir_rx_data_t ir;
        rfid_data_t rfid;
        uint32_t custom_param;
    } data;
} event_t;

#define EVENT_QUEUE_CAPACITY 32

typedef struct {
    event_t queue[EVENT_QUEUE_CAPACITY];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t count;
} event_queue_t;

void event_loop_init(event_queue_t *eq);
bool event_loop_push(event_queue_t *eq, const event_t *evt);
bool event_loop_pop(event_queue_t *eq, event_t *out_evt);
bool event_loop_is_empty(const event_queue_t *eq);
uint8_t event_loop_size(const event_queue_t *eq);

#endif // EVENT_LOOP_H
