#include "hal_gpio.h"
#include "hal_spi.h"
#include "hal_uart.h"
#include "hal_display.h"
#include <stdio.h>
#include <string.h>

// Simulated/CMSIS-backed HAL implementation for STM32F4
void hal_gpio_init(hal_gpio_pin_t pin, hal_gpio_mode_t mode) {
    (void)pin;
    (void)mode;
}

void hal_gpio_write(hal_gpio_pin_t pin, bool state) {
    (void)pin;
    (void)state;
}

bool hal_gpio_read(hal_gpio_pin_t pin) {
    (void)pin;
    return false;
}

void hal_spi_init(hal_spi_bus_t bus) {
    (void)bus;
}

void hal_spi_transmit(hal_spi_bus_t bus, const uint8_t *data, size_t length) {
    (void)bus;
    (void)data;
    (void)length;
}

void hal_spi_transmit_receive(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, size_t length) {
    (void)bus;
    (void)tx;
    if (rx) {
        memset(rx, 0, length);
    }
}

void hal_uart_init(hal_uart_bus_t bus, uint32_t baudrate) {
    (void)bus;
    (void)baudrate;
}

void hal_uart_write_string(hal_uart_bus_t bus, const char *str) {
    (void)bus;
    // Print to stdout / Renode USART buffer
    if (str) {
        printf("%s", str);
    }
}

void hal_uart_write_bytes(hal_uart_bus_t bus, const uint8_t *data, size_t len) {
    (void)bus;
    for (size_t i = 0; i < len; i++) {
        putchar(data[i]);
    }
}

void hal_display_init(void) {
    hal_uart_write_string(HAL_UART_1, "[HAL] Display ST7789 initialized\n");
}

void hal_display_clear(uint16_t color_rgb565) {
    (void)color_rgb565;
}

void hal_display_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    (void)x;
    (void)y;
    (void)color;
}

void hal_display_flush(void) {
}
