#ifndef VSCODE_INTELLISENSE_SPI_SPIDEV_H
#define VSCODE_INTELLISENSE_SPI_SPIDEV_H

#include <stdint.h>

/*
 * Minimal shim for VS Code IntelliSense on non-Linux hosts.
 * The real build should still use the system's linux/spi/spidev.h.
 */

#define SPI_MODE_0 0

#define SPI_IOC_WR_MODE 0
#define SPI_IOC_WR_BITS_PER_WORD 0
#define SPI_IOC_WR_MAX_SPEED_HZ 0
#define SPI_IOC_MESSAGE(N) 0

struct spi_ioc_transfer {
    uint64_t tx_buf;
    uint64_t rx_buf;
    uint32_t len;
    uint32_t speed_hz;
    uint16_t delay_usecs;
    uint8_t bits_per_word;
    uint8_t cs_change;
    uint32_t pad;
};

#endif
