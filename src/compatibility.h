/* 
 * File:   compatiblity.h
 * Author: purinda
 *
 * Created on 24 June 2012, 3:08 PM
 */

#ifndef COMPATIBLITY_H
#define	COMPATIBLITY_H
	
#include <stddef.h>
#include <time.h>
#include <sys/time.h>

void milliSleep(int millisec);
void microSleep(int millisec);
void secSleep(int sec);
void start_timer();
long millis();

#endif	/* COMPATIBLITY_H */