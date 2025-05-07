#ifndef _SPI_LIB_H_
#define _SPI_LIB_H_

#include <furi_hal.h>
#include <furi_hal_bus.h>
#include <furi_hal_spi.h>
#include <furi_hal_spi_config.h>

#define TIMEOUT_SPI 100
#define CS          &gpio_ext_pa4
#define SCK         &gpio_ext_pb3
#define MOSI        &gpio_ext_pa7
#define MISO        &gpio_ext_pa6

#define BUS        &furi_hal_spi_bus_r
#define SPEED_8MHZ &furi_hal_spi_preset_1edge_low_8m // 8 MHZ
#define SPEED_4MHZ &furi_hal_spi_preset_1edge_low_4m // 4 MHZ
#define SPEED_2MHZ &furi_hal_spi_preset_1edge_low_2m // 2 MHZ

/**
 * Alloc the spi instance
 */
FuriHalSpiBusHandle* spi_alloc();

/**
 * Send data via spi
 */
bool spi_send(FuriHalSpiBusHandle* spi, const uint8_t* buffer, size_t size);

/**
 * Read data via spi
 */
bool spi_send_and_read(
    FuriHalSpiBusHandle* spi,
    const uint8_t* action_address,
    uint8_t* data_read,
    size_t size_to_send,
    size_t size_to_read);

/**
 * Free the spi instance
 */
void spi_free(FuriHalSpiBusHandle* spi);

#endif
