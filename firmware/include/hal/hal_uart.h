#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    HAL_UART_1, // Primary Console
    HAL_UART_2, // Expansion Port / Debug
} hal_uart_bus_t;

void hal_uart_init(hal_uart_bus_t bus, uint32_t baudrate);
void hal_uart_write_string(hal_uart_bus_t bus, const char *str);
void hal_uart_write_bytes(hal_uart_bus_t bus, const uint8_t *data, size_t len);

#endif // HAL_UART_H
