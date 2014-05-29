#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "gpio.h"
#include "interrupts.h"

#define GPIO_FN_MAXLEN  32
#define POLL_TIMEOUT    1000
#define RDBUF_LEN   5

int interrupt_wait(void *port) {
    char fn[GPIO_FN_MAXLEN];
    int fd,ret;
    struct pollfd pfd;
    char rdbuf[RDBUF_LEN];
    int pin = (int) port;

    memset(rdbuf, 0x00, RDBUF_LEN);
    memset(fn, 0x00, GPIO_FN_MAXLEN);

    snprintf(fn, GPIO_FN_MAXLEN-1, "/sys/class/gpio/gpio%d/value", pin);
    gpio_open(pin, GPIO_IN);
    gpio_enable_edge(pin, 1);
    fd = open(fn, O_RDONLY);
    if(fd < 0) {
        perror(fn);
        return 2;
    }
    pfd.fd = fd;
    pfd.events = POLLPRI;
    
    ret = read(fd, rdbuf, RDBUF_LEN-1);
    if(ret < 0) {
        perror("read()");
        return 4;
    }
    printf("value is: %s\n", rdbuf);
    
    while(1) {
        memset(rdbuf, 0x00, RDBUF_LEN);
        lseek(fd, 0, SEEK_SET);
        ret = poll(&pfd, 1, -1);
        if (ret < 0) {
            perror("poll()");
            close(fd);
            return 3;
        }
        if (ret == 0) {
            printf("timeout\n");
            continue;
        }
        ret = read(fd, rdbuf, RDBUF_LEN-1);
        if(ret < 0) {
            perror("read()");
            return 4;
        }
        printf("interrupt, value is: %s\n", rdbuf);
    }
    close(fd);
    return 0;
}