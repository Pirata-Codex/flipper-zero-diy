#include "st7789.h"
#include "hal_gpio.h"
#include "hal_spi.h"

void st7789_init(const st7789_config_t *cfg) {
    if (!cfg) return;
    hal_gpio_init(cfg->cs_pin, HAL_GPIO_MODE_OUTPUT);
    hal_gpio_init(cfg->dc_pin, HAL_GPIO_MODE_OUTPUT);
    hal_gpio_init(cfg->reset_pin, HAL_GPIO_MODE_OUTPUT);

    hal_gpio_write(cfg->reset_pin, false);
    // Reset delay simulation
    hal_gpio_write(cfg->reset_pin, true);
}

void st7789_fill(uint16_t color) {
    (void)color;
}

void st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    (void)x;
    (void)y;
    (void)color;
}
