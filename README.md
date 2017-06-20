# A C Raspberry Pi driver for nRF24L01(+) 2.4GHz Wireless Transceiver

Almost complete - WIP
---
Pretty much a complete re-write of the existing code base to convert to C and improve safety. Thus, it may be in a less operational state than the C++ version but should provide the basic features. :D

Design Goals: 

* Maximally compliant with the intended operation of the chip
* Easy for beginners to use
* Consumed with a public interface that's similiar to other Arduino standard libraries
* Built against the standard SPI library. 
* Support popular platform

* Modifications to the RF24 library in this fork is backward compatible. A single
  enhancement which may cause issue, is code which relies on the driver to power down the
  radio, as a side effect. The radio is no longer powered down after each transmit. Rather,
  the application must take responsibility for power management. Normally this is
  achieved by use of powerDown and powerUp. If you wish to maximize power efficiency,
  you must call powerDown after transmit (write, startWrite).

Please refer to links :

* [Blog on setting up both Arduino UNO & Raspberry Pi] (http://arduino-for-beginners.blogspot.com/2013/02/setup-nordic-nrf24l01-rf-modules-to.html)
* [Documentation Main Page](http://maniacbug.github.com/RF24)
* [RF24 Class Documentation](http://maniacbug.github.com/RF24/classRF24.html)
* [Source Code](https://github.com/maniacbug/RF24)
* [Downloads](https://github.com/maniacbug/RF24/archives/master)
* [nRF24L01+ datasheet](http://www.nordicsemi.com/eng/nordic/download_resource/8765/2/27999719 )




Raspberry Pi RF24 library
===========================

This is a C based library for RF24 / NRF24L01 wireless modules on the raspberry pi.

Setup
=====
1. Execute "make" and "sudo make install" to install the shared libraries
2. Change to examples folder, change to the correct connected pins and execute "make"


Known issues
============
- The current bcm2835 drivers still have some minor bugs/errors. If you have errors, use the GPIO or WiringPi version.


Links 
=====
- Forum links : http://www.raspberrypi.org/phpBB3/viewtopic.php?f=45&t=17061
- C library for Broadcom BCM 2835 http://www.open.com.au/mikem/bcm2835/index.html
- Maniacbug RF24 http://maniacbug.github.com/RF24/index.html
- RF24 Class Reference http://maniacbug.github.com/RF24/classRF24.html


Previous Maintainers of C++ versions
=======
Stanley Seow ( stanleyseow@gmail.com )
https://github.com/stanleyseow/RF24

RF24 for RPi using gpio:
Arco van Geest <arco@appeltaart.mine.nu> 
https://github.com/gnulnulf/RF24

RF24 for RPi using bcm2835:
Charles-Henri Hallard http://hallard.me/ 
https://github.com/hallard/RF24

RF24 for RPi using WiringPi:
Trey Keown <jfktrey@gmail.com>
https://github.com/jfktrey/RF24
