#include "gpio.h"
#include <stdio.h>
/* Status values */
#define ERROR 0
#define OK 1

int gpio_open(int port, int dir) {
	char path[40];
	FILE *f = fopen("/sys/class/gpio/export", "w");
	if (f == NULL) return ERROR;
	fprintf(f, "%d\n", port);
	fclose(f);
	
	sprintf(path, "/sys/class/gpio/gpio%d/direction", port);
	f = fopen(path, "w");
	if (f == NULL) return ERROR;
	fprintf(f, "%s\n", (dir ? "out" : "in"));
	fclose(f);
	return OK;
}

int gpio_close(int port) {
	FILE *f = fopen("/sys/class/gpio/unexport", "w");
	if (f == NULL) return ERROR;
	fprintf(f, "%d\n", port);
	fclose(f);
	return OK;
}

int gpio_read(int port, int *val) {
	FILE *f;
	char path[40];
	sprintf(path, "/sys/class/gpio/gpio%d/value", port);
	f = fopen(path, "r");
	if (f == NULL) return ERROR;
	fscanf(f, "%d", val);
	fclose(f);
	return OK;
}

int gpio_write(int port, int val){
	FILE *f;
	char file[40];
	sprintf(file, "/sys/class/gpio/gpio%d/value", port);
	f = fopen(file, "w");
	if (f == NULL) return ERROR;
	fprintf(f, "%s\n", (val ? "1" : "0"));
	fclose(f);
	return OK;
}

int gpio_enable_edge(int port, int edge){
	FILE *f;
	char file[40];
	sprintf (file, "/sys/class/gpio/gpio%d/edge", port) ;
  	f = fopen(file, "w");
  	if (f == NULL) return ERROR;
  	switch(edge) {
  		case(0): fprintf(f, "none\n"); break;
  		case(1): fprintf(f, "falling\n"); break;
  		case(2): fprintf(f, "rising\n"); break;
  		case(3): fprintf(f, "both\n"); break;
  	}
  	fclose(f);
  	return OK;
}