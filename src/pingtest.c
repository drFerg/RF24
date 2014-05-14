#include <stdlib.h>
#include <stdio.h>
#include "RF24.h"

void setup(void)
{
    // init radio for reading
  // spi device, spi speed, ce gpio pin
    uint8_t status = rf24_init_radio("/dev/spidev0.0",8000000,25);
    if (status == 0) exit(-1);
    rf24_enableDynamicPayloads();
    rf24_setAutoAckOnPipe(1, 1);
    rf24_setRetries(15,15);
    rf24_setDataRate(RF24_1MBPS);
    rf24_setPALevel(RF24_PA_MAX);
    rf24_setChannel(76);
    rf24_setCRCLength(RF24_CRC_16);
    rf24_openReadingPipe(1,0xF0F0F0F0E1LL);
    rf24_startListening();
}
 
void loop(void)
{
    // 32 byte character array is max payload
    char receivePayload[32];
 
    while (rf24_available(NULL))
    {
        // read from radio until payload size is reached
        uint8_t len = rf24_getDynamicPayloadSize();
        rf24_read(receivePayload, len);
 
        // display payload
        printf("Recvd: %s\n", receivePayload);
    }
}
 
int main(int argc, char** argv) 
{
    setup();
    while(1)
        loop();
 
    return 0;
}