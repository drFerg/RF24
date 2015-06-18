#ifndef GPIO_H
#define	GPIO_H
/* GPIO pin directions */
#define GPIO_IN 0
#define GPIO_OUT 1
/* GPIO pin values */
#define GPIO_LOW 0
#define GPIO_HIGH 1
/* GPIO interrupt edges */
#define GPIO_NO_EDGE 0
#define GPIO_FALLING_EDGE 1
#define GPIO_RISING_EDGE 2
#define GPIO_BOTH_EDGES 3

/* Opens the specified port as an input or output
 * returns 1 if successful, 0 otherwise */
int gpio_open(int port, int dir);

/* Closes the specified port
 * returns 1 if successful, 0 otherwise */
int gpio_close(int port);

/* Reads the specified port into the provided val
 * returns 1 if successful, 0 otherwise */
int gpio_read(int port, int *val);

/* Writes val to the specified port
 * returns 1 if successful, 0 otherwise */
int gpio_write(int port, int val);

int gpio_enable_edge(int port, int edge);

#endif	/* GPIO_H */

