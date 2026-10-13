#ifndef ST7789_H
#define ST7789_H

#include "hal_gpio.h"
#include "hal_spi.h"

typedef struct {
    hal_spi_bus_t spi_bus;
    hal_gpio_pin_t cs_pin;
    hal_gpio_pin_t dc_pin;
    hal_gpio_pin_t reset_pin;
} st7789_config_t;

void st7789_init(const st7789_config_t *cfg);
void st7789_fill(uint16_t color);
void st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

#endif // ST7789_H
