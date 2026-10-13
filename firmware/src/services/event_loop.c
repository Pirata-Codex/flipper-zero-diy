#include "event_loop.h"
#include <string.h>

void event_loop_init(event_queue_t *eq) {
    if (!eq) return;
    eq->head = 0;
    eq->tail = 0;
    eq->count = 0;
    memset((void*)eq->queue, 0, sizeof(eq->queue));
}

bool event_loop_push(event_queue_t *eq, const event_t *evt) {
    if (!eq || !evt) return false;
    if (eq->count >= EVENT_QUEUE_CAPACITY) {
        return false; // Queue full
    }
    eq->queue[eq->head] = *evt;
    eq->head = (eq->head + 1) % EVENT_QUEUE_CAPACITY;
    eq->count++;
    return true;
}

bool event_loop_pop(event_queue_t *eq, event_t *out_evt) {
    if (!eq || !out_evt) return false;
    if (eq->count == 0) {
        return false; // Queue empty
    }
    *out_evt = eq->queue[eq->tail];
    eq->tail = (eq->tail + 1) % EVENT_QUEUE_CAPACITY;
    eq->count--;
    return true;
}

bool event_loop_is_empty(const event_queue_t *eq) {
    return (eq == NULL) || (eq->count == 0);
}

uint8_t event_loop_size(const event_queue_t *eq) {
    return eq ? eq->count : 0;
}
