#ifndef SPI_H
#define	SPI_H
#include <stdint.h>

typedef struct spi_state SPIState; /* opaque type definition */

SPIState *spi_init(char *device, uint32_t mode, uint8_t bits, uint32_t speed, uint8_t chip_select);
void spi_enable(SPIState *spi); 
uint8_t spi_transfer(SPIState *spi, uint8_t val, uint8_t *rx);
uint8_t spi_transfer_bulk(SPIState *spi, uint8_t *tx, uint8_t *rx, uint8_t len);
void spi_disable(SPIState *spi);
void spi_close(SPIState *spi);

#endif	/* SPI_H */