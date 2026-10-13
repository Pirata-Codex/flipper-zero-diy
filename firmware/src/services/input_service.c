#include "input_service.h"
#include <string.h>

void input_service_init(input_service_t *srv, event_queue_t *eq) {
    if (!srv) return;
    srv->event_queue = eq;
    for (int i = 0; i < INPUT_KEY_COUNT; i++) {
        srv->buttons[i].is_pressed = false;
        srv->buttons[i].press_start_time = 0;
        srv->buttons[i].long_press_fired = false;
    }
}

void input_service_update_key(input_service_t *srv, input_key_t key, bool is_pressed, uint32_t current_time_ms) {
    if (!srv || key >= INPUT_KEY_COUNT) return;

    button_state_t *btn = &srv->buttons[key];

    // State transition: Released -> Pressed
    if (is_pressed && !btn->is_pressed) {
        btn->is_pressed = true;
        btn->press_start_time = current_time_ms;
        btn->long_press_fired = false;

        // Fire PRESS event
        event_t evt;
        evt.type = EVENT_TYPE_INPUT;
        evt.timestamp_ms = current_time_ms;
        evt.data.input.key = key;
        evt.data.input.type = INPUT_TYPE_PRESS;
        event_loop_push(srv->event_queue, &evt);
    }
    // Held down check for long press
    else if (is_pressed && btn->is_pressed) {
        if (!btn->long_press_fired && (current_time_ms - btn->press_start_time >= LONG_PRESS_THRESHOLD_MS)) {
            btn->long_press_fired = true;

            event_t evt;
            evt.type = EVENT_TYPE_INPUT;
            evt.timestamp_ms = current_time_ms;
            evt.data.input.key = key;
            evt.data.input.type = INPUT_TYPE_LONG_PRESS;
            event_loop_push(srv->event_queue, &evt);
        }
    }
    // State transition: Pressed -> Released
    else if (!is_pressed && btn->is_pressed) {
        uint32_t duration = current_time_ms - btn->press_start_time;
        btn->is_pressed = false;

        // Fire RELEASE event
        event_t evt;
        evt.type = EVENT_TYPE_INPUT;
        evt.timestamp_ms = current_time_ms;
        evt.data.input.key = key;
        evt.data.input.type = INPUT_TYPE_RELEASE;
        event_loop_push(srv->event_queue, &evt);

        // If not long press, fire SHORT_CLICK event
        if (!btn->long_press_fired && duration >= DEBOUNCE_THRESHOLD_MS) {
            evt.data.input.type = INPUT_TYPE_SHORT_CLICK;
            event_loop_push(srv->event_queue, &evt);
        }
    }
}
