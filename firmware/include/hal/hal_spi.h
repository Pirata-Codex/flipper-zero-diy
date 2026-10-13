#ifndef HAL_SPI_H
#define HAL_SPI_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    HAL_SPI_1,
    HAL_SPI_2,
} hal_spi_bus_t;

void hal_spi_init(hal_spi_bus_t bus);
void hal_spi_transmit(hal_spi_bus_t bus, const uint8_t *data, size_t length);
void hal_spi_transmit_receive(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, size_t length);

#endif // HAL_SPI_H
