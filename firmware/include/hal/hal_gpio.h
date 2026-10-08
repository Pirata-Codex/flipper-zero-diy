#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_GPIO_PORT_A,
    HAL_GPIO_PORT_B,
    HAL_GPIO_PORT_C,
    HAL_GPIO_PORT_D,
    HAL_GPIO_PORT_E,
} hal_gpio_port_t;

typedef enum {
    HAL_GPIO_MODE_INPUT,
    HAL_GPIO_MODE_OUTPUT,
    HAL_GPIO_MODE_ALTERNATE,
    HAL_GPIO_MODE_ANALOG,
} hal_gpio_mode_t;

typedef struct {
    hal_gpio_port_t port;
    uint8_t pin;
} hal_gpio_pin_t;

void hal_gpio_init(hal_gpio_pin_t pin, hal_gpio_mode_t mode);
void hal_gpio_write(hal_gpio_pin_t pin, bool state);
bool hal_gpio_read(hal_gpio_pin_t pin);

#endif // HAL_GPIO_H
