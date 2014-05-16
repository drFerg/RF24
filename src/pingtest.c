#include <stdlib.h>
#include <stdio.h>
#include "RF24.h"

typedef struct result {
    uint8_t pass;
    uint8_t fail;
} Result;


void assert(uint8_t check, uint8_t value, Result *r){
    if (check == value){
        r->pass++;
        printf("PASS\n");
    }
    else{
        r->fail++;
        printf("FAIL\n");
    }
}

void test_data_rate(Result *r){
    printf("[TEST] Performing Data rate tests...\n");
    printf("[TEST]\tSetting data rate to 250Kbps...");
    rf24_setDataRate(RF24_250KBPS);
    assert(RF24_250KBPS, rf24_getDataRate(), r);
    printf("[TEST]\tSetting data rate to 1Mbps...");
    rf24_setDataRate(RF24_1MBPS);
    assert(RF24_1MBPS, rf24_getDataRate(), r);
    printf("[TEST]\tSetting data rate to 2Mbps...");
    rf24_setDataRate(RF24_2MBPS);
    assert(RF24_2MBPS, rf24_getDataRate(), r);
}

void test_power_level(Result *r){
    printf("[TEST] Performing power level tests...\n");
    printf("[TEST]\tSetting power level to min...");
    rf24_setPALevel(RF24_PA_MIN);
    assert(RF24_PA_MIN, rf24_getPALevel, r);
    printf("[TEST]\tSetting power level to low...");
    rf24_setPALevel(RF24_PA_LOW);
    assert(RF24_PA_LOW, rf24_getPALevel, r);
    printf("[TEST]\tSetting power level to high...");
    rf24_setPALevel(RF24_PA_HIGH);
    assert(RF24_PA_HIGH, rf24_getPALevel, r);
    printf("[TEST]\tSetting power level to max...");
    rf24_setPALevel(RF24_PA_MAX);
    assert(RF24_PA_MAX, rf24_getPALevel, r);
}

void test_crc_length(Result *r){
    printf("[TEST] Performing crc tests...\n");
    printf("[TEST]\tDisabling CRC...");
    rf24_setCRCLength(RF24_CRC_DISABLED);
    assert(RF24_CRC_DISABLED, rf24_getCRCLength, r);
    printf("[TEST]\tSetting CRC length to 8bits...");
    rf24_setCRCLength(RF24_CRC_8);
    assert(RF24_CRC_8, rf24_getCRCLength, r);
    printf("[TEST]\tSetting CRC length to 16bits...");
    rf24_setCRCLength(RF24_CRC_16);
    assert(RF24_CRC_16, rf24_getCRCLength, r);
}

void run_test_suite(Result *r){
    test_data_rate(r);
    test_power_level(r);
    printf("***********************************\n");
    printf("*          Test summary           *\n");
    printf("*                                 *\n");
    printf("*   > Tests passed: %d            *\n", r->pass);
    printf("*   > Tests failed: %d            *\n", r->failed);
    printf("*                                 *\n");
    printf("***********************************\n");
}

void setup(void)
{
    Result r;
    // init radio for reading
  // spi device, spi speed, ce gpio pin
    uint8_t status = rf24_init_radio("/dev/spidev0.0", 8000000, 25);
    if (status == 0) exit(-1);
    rf24_enableDynamicPayloads();
    rf24_setAutoAckOnPipe(1, 0);
    rf24_setRetries(15,15);
    rf24_setDataRate(RF24_1MBPS);
    rf24_setPALevel(RF24_PA_MAX);
    rf24_setChannel(76);
    rf24_setCRCLength(RF24_CRC_16);
    rf24_openReadingPipe(1,0xF0F0F0F0E1LL);
    rf24_startListening();
    rf24_printDetails();
    run_test_suite(&r);
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
        printf("Recvd pkt - len: %d : %s\n", len, receivePayload);
    }
}
 
int main(int argc, char** argv) 
{
    setup();
    while(1)
        loop();
 
    return 0;
}