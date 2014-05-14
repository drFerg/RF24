#############################################################################
#
# Makefile for librf24 on Raspberry Pi
#
# License: GPL (General Public License)
# Author:  gnulnulf <arco@appeltaart.mine.nu>
# Date:    2013/02/07 (version 1.0)
#
# Description:
# ------------
# use make all and mak install to install the library 
# You can change the install directory by editing the LIBDIR line
#
LIBDIR=/usr/local/lib
LIBNAME=librf24.so.1.0


# The recommended compiler flags for the Raspberry Pi


CCFLAGS= -Wall -W -fPIC
CFLAGS= -Wall -W -fPIC
# make all
ifeq ($(platform), pi)
	CCFLAGS+=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
endif

all: librf24

# Make the library
librf24: gpio.o RF24.o spi.o compatibility.o
	g++ -shared -Wl,-soname,librf24.so.1 ${CCFLAGS} -o ${LIBNAME} compatibility.o gpio.o spi.o RF24.o

# Library parts
RF24.o: RF24.c RF24.h
gpio.o: gpio.c gpio.h
spi.o: spi.c spi.h

compatibility.o: compatibility.c compatibility.h
# clear build files
clean:
	rm -rf *o ${LIBNAME}

# Install the library to LIBPATH
install: 
	cp librf24.so.1.0 ${LIBDIR}/${LIBNAME}
	ln -sf ${LIBDIR}/${LIBNAME} ${LIBDIR}/librf24.so.1
	ln -sf ${LIBDIR}/${LIBNAME} ${LIBDIR}/librf24.so
	ldconfig
