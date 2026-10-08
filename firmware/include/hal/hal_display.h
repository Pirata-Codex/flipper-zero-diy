#ifndef HAL_DISPLAY_H
#define HAL_DISPLAY_H

#include <stdint.h>

#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 240

void hal_display_init(void);
void hal_display_clear(uint16_t color_rgb565);
void hal_display_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void hal_display_flush(void);

#endif // HAL_DISPLAY_H
