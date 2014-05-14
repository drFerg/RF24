#include "spi.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define SET_SPI(_spi, _mode, _bits, _speed) do {\
	_spi->mode = _mode;\
	_spi->bits = _bits;\
	_spi->speed = _speed;\
} while (0)

//	speed = 24000000; // 24Mhz
//	speed = 16000000; // 16Mhz 
//	speed = 8000000; // 8Mhz
 // uint32_t speed = 2000000; // 2Mhz 
	// uint32_t mode = 0;
	// uint8_t bits = 8;
typedef struct spi_state {
	uint32_t speed;
	uint32_t mode;
	uint8_t bits;
	int fd;
} SPIState;

SPIState *spi_init(char *device, uint32_t mode, uint8_t bits, uint32_t speed) {
	int ret;
	SPIState *spi = (SPIState *) malloc(sizeof(SPIState));
	if (spi == NULL) return NULL;
	SET_SPI(spi, mode, bits, speed);
	spi->fd = open(device, O_RDWR);
	if (spi->fd < 0) {
		perror("ERROR: Can't open SPI device");
		return NULL;
	}

	/* spi mode */
	ret = ioctl(spi->fd, SPI_IOC_WR_MODE, &(spi->mode));
	if (ret == -1) {
		perror("ERROR: Can't set spi wr mode");
		return NULL;		
	}
	ret = ioctl(spi->fd, SPI_IOC_RD_MODE, &(spi->mode));
	if (ret == -1) {
		perror("ERROR: Can't set spi rd mode");
		return NULL;				
	}
	
	/* bits per word */
	ret = ioctl(spi->fd, SPI_IOC_WR_BITS_PER_WORD, &(spi->bits));
	if (ret == -1) {
		perror("ERROR: Can't set bits per word");
		return NULL;				
	}
	ret = ioctl(spi->fd, SPI_IOC_RD_BITS_PER_WORD, &(spi->bits));
	if (ret == -1) {
		perror("ERROR: Can't set bits per word");
		return NULL;						
	}

	/* max speed hz */
	ret = ioctl(spi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &(spi->speed));
	if (ret == -1) {
		perror("ERROR: Can't set max speed hz");
		return NULL;						
	}
	ret = ioctl(spi->fd, SPI_IOC_RD_MAX_SPEED_HZ, &(spi->speed));
	if (ret == -1) {
		perror("ERROR: Can't set max speed hz");
		return NULL;						
	}
	return spi;
}

uint8_t spi_transfer(SPIState *spi, uint8_t val, uint8_t *rx) {
	if (spi == NULL) {
		perror("ERROR: NULL spi state");
		return 0;
	}
	int ret;
	// One byte is transferred at once
	uint8_t tx[] = {0};
	tx[0] = val;

	uint8_t rx_val[ARRAY_SIZE(tx)] = {0};
	struct spi_ioc_transfer tr;
	tr.tx_buf = (unsigned long)tx;
	tr.rx_buf = (unsigned long)rx_val;
	tr.len = ARRAY_SIZE(tx);
	tr.delay_usecs = 0;
//	tr.cs_change = 1;
	tr.speed_hz = spi->speed;
	tr.bits_per_word = spi->bits;

	ret = ioctl(spi->fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
	{
		perror("ERROR: can't send spi message");
		return 0;		
	}
	if (rx != NULL) memcpy(rx, rx_val, 1);
	return 1;
}

void spi_close(SPIState *spi){
	close(spi->fd);
}

