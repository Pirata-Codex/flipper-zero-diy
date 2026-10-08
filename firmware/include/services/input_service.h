#ifndef INPUT_SERVICE_H
#define INPUT_SERVICE_H

#include "event_loop.h"
#include <stdbool.h>

#define LONG_PRESS_THRESHOLD_MS 500
#define DEBOUNCE_THRESHOLD_MS   30

typedef struct {
    bool is_pressed;
    uint32_t press_start_time;
    bool long_press_fired;
} button_state_t;

typedef struct {
    button_state_t buttons[INPUT_KEY_COUNT];
    event_queue_t *event_queue;
} input_service_t;

void input_service_init(input_service_t *srv, event_queue_t *eq);
void input_service_update_key(input_service_t *srv, input_key_t key, bool is_pressed, uint32_t current_time_ms);

#endif // INPUT_SERVICE_H
