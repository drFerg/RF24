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
#include <pthread.h>
#include "gpio.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define BUF_LEN 1
#define SET_SPI(_spi, _mode, _bits, _speed, _chip_select) do {\
	_spi->mode = _mode;\
	_spi->bits = _bits;\
	_spi->speed = _speed;\
	_spi->chip_select = _chip_select;\
} while (0)

typedef struct spi_state {
	uint32_t speed;
	uint32_t mode;
	uint8_t bits;
	int fd;
	uint8_t chip_select;
	pthread_mutex_t lock;
} SPIState;

SPIState *spi_init(char *device, uint32_t mode, uint8_t bits, uint32_t speed, uint8_t chip_select) {
	int ret;
	SPIState *spi = (SPIState *) malloc(sizeof(SPIState));
	if (spi == NULL) return NULL;
	SET_SPI(spi, mode, bits, speed, chip_select);
	printf("SPI config: mode %d, %d bit, %dMhz\n",spi->mode, spi->bits, (spi->speed)/1000000);
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
	gpio_open(spi->chip_select, GPIO_OUT);
	spi_disable(spi); /* Ensures chip select is pulled up */
	pthread_mutex_init(&(spi->lock), NULL);
	return spi;
}

void spi_enable(SPIState *spi){
	pthread_mutex_lock(&(spi->lock));
	gpio_write(spi->chip_select, GPIO_LOW);
}

uint8_t spi_transfer(SPIState *spi, uint8_t val, uint8_t *rx) {
	if (spi == NULL) {
		perror("ERROR: NULL spi state");
		return 0;
	}
	int ret;
	uint8_t tx[BUF_LEN] = {val};
	uint8_t rx_val[BUF_LEN] = {0};
	struct spi_ioc_transfer tr;
	memset(&tr, 0, sizeof(struct spi_ioc_transfer));
	tr.tx_buf = (unsigned long)tx;
	tr.rx_buf = (unsigned long)rx_val;
	tr.len = BUF_LEN;
	tr.delay_usecs = 0;
	tr.cs_change = 0;
	tr.speed_hz = spi->speed;
	tr.bits_per_word = spi->bits;

	ret = ioctl(spi->fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		perror("ERROR: can't send spi message");
		return 0;		
	}
	if (rx != NULL) memcpy(rx, rx_val, 1);
	return 1;
}

uint8_t spi_transfer_bulk(SPIState *spi, uint8_t *tx, uint8_t *rx, uint8_t len) {
	if (spi == NULL) {
		perror("ERROR: NULL spi state");
		return 0;
	}
	int ret;
	uint8_t rx_val[len];
	memset(rx_val, 0, len);
	struct spi_ioc_transfer tr;
	tr.tx_buf = (unsigned long)tx;
	tr.rx_buf = (unsigned long)rx_val;
	tr.len = len;
	tr.delay_usecs = 0;
	tr.cs_change = 0;
	tr.speed_hz = spi->speed;
	tr.bits_per_word = spi->bits;

	ret = ioctl(spi->fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		perror("ERROR: can't send spi message");
		return 0;		
	}
	if (rx != NULL) memcpy(rx, rx_val, len);
	return 1;
}

void spi_disable(SPIState *spi){
	gpio_write(spi->chip_select, GPIO_HIGH);
	pthread_mutex_unlock(&(spi->lock));
}

void spi_close(SPIState *spi){
	pthread_mutex_lock(&(spi->lock));
	close(spi->fd);
	pthread_mutex_unlock(&(spi->lock));
}

