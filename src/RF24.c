/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "nRF24L01.h"
#include "RF24_config.h"
#include "RF24.h"
#define MAX_CHANNEL 127
#define MAX_PAYLOAD_SIZE 32
#define SPI_BITS 8

SPIState *spi;
uint8_t ce_pin; /**< "Chip Enable" pin, activates the RX or TX role, unused on rpi */
char *spidevice;
uint32_t spispeed;
uint8_t csn_pin; /**< SPI Chip select */
bool wide_band; /* 2Mbs data rate in use? */
bool p_variant; /* False for RF24L01 and TRUE for RF24L01P */
uint8_t payload_size; /**< Fixed size of payloads */
bool ack_payload_available; /**< Whether there is an ack payload waiting */
bool dynamic_payloads_enabled; /**< Whether dynamic payloads are enabled. */ 
uint8_t ack_payload_length; /**< Dynamic size of pending ack payload. */
uint64_t pipe0_reading_address; /**< Last address set on pipe 0 for reading. */
/****************************************************************************/

void csn(int mode) {
  // Minimum ideal SPI bus speed is 2x data rate
  // If we assume 2Mbs data rate and 16Mhz clock, a
  // divider of 4 is the minimum we want.
  // CLK:BUS 8Mhz:2Mhz, 16Mhz:4Mhz, or 20Mhz:5Mhz
  digitalWrite(csn_pin, mode);
}

/****************************************************************************/

void ce(int level) {
  digitalWrite(ce_pin, level);
}

/****************************************************************************/

uint8_t read_register_for(uint8_t reg, uint8_t* buf, uint8_t len) {
  uint8_t status;

  csn(LOW);
  spi_transfer(spi, R_REGISTER | (REGISTER_MASK & reg), &status);
  while (len--)
    *buf++ = spi_transfer(spi, 0xff, NULL);

  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t read_register(uint8_t reg) {
  csn(LOW);
  spi_transfer(spi, R_REGISTER | (REGISTER_MASK & reg), NULL);
  uint8_t result = spi_transfer(spi, 0xff, NULL);
  csn(HIGH);
  return result;
}

/****************************************************************************/

uint8_t write_register_for(uint8_t reg, const uint8_t* buf, uint8_t len) {
  uint8_t status;

  csn(LOW);
  spi_transfer(spi, W_REGISTER | (REGISTER_MASK & reg), &status);
  while (len--)
    spi_transfer(spi, *buf++, NULL);

  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t write_register(uint8_t reg, uint8_t value) {
  uint8_t status;

  IF_SERIAL_DEBUG(printf("write_register(%02x, %02x)\r\n", reg, value));

  csn(LOW);
  spi_transfer(spi, W_REGISTER | (REGISTER_MASK & reg), &status);
  spi_transfer(spi, value, NULL);
  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t write_payload(const void* buf, uint8_t len) {
  uint8_t status;
  const uint8_t* current = (const uint8_t*)buf;

  uint8_t data_len = (len < payload_size ? len : payload_size);
  uint8_t blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;
  
  //printf("[Writing %u bytes %u blanks]", data_len, blank_len);
  
  csn(LOW);
  spi_transfer(spi, W_TX_PAYLOAD, &status);
  while (data_len--)
    spi_transfer(spi, *current++, NULL);
  while (blank_len--)
    spi_transfer(spi, 0, NULL);
  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t read_payload(void* buf, uint8_t len) {
  uint8_t status;
  uint8_t* current = (uint8_t*)buf;

  uint8_t data_len = (len < payload_size ? len : payload_size);
  uint8_t blank_len = (dynamic_payloads_enabled ? 0 : payload_size - data_len);
  
  //printf("[Reading %u bytes %u blanks]", data_len, blank_len);
  
  csn(LOW);
  spi_transfer(spi, R_RX_PAYLOAD, &status);
  while (data_len--)
    spi_transfer(spi, 0xff, current++); /* originally *current++ = spi_transfer... */
  while (blank_len--)
    spi_transfer(spi, 0xff, NULL);
  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t flush_rx() {
  uint8_t status;

  csn(LOW);
  spi_transfer(spi, FLUSH_RX, &status);
  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t flush_tx() {
  uint8_t status;

  csn(LOW);
  spi_transfer(spi, FLUSH_TX, &status);
  csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t get_status() {
  uint8_t status;

  csn(LOW);
  spi_transfer(spi, NOP, &status);
  csn(HIGH);

  return status;
}

bool setDataRate(rf24_datarate_e speed) {
  uint8_t setup = read_register(RF_SETUP);
  wide_band = FALSE;
  setup &= ~RF_DR; /* Clear DR bits i.e. 1Mbps is 00 */

  switch(speed){
    case(RF24_250KBPS): {
      setup |= RF_DR_250K; /* Set low speed bit */
      break;
    }
    case(RF24_1MBPS): break; /* Already set */
    case(RF24_2MBPS): {
      wide_band = TRUE;
      setup |= RF_DR_2M; /* Set high speed bit */
      break;
    }
    default: return FALSE;
  }
  write_register(RF_SETUP, setup);
  if (setup == read_register(RF_SETUP)) { /* Verify write */
    return TRUE;
  } 
  else {
    wide_band = FALSE;
    return FALSE;
  }
}

/****************************************************************************/

rf24_datarate_e getDataRate() {
  uint8_t dr = read_register(RF_SETUP) & RF_DR; /* Extract DR bits */
  switch(dr){
    case(RF_DR_250K): return RF24_1MBPS;
    case(RF_DR_1M): return RF24_250KBPS;
    case(RF_DR_2M): return RF24_2MBPS;
    default: return RF24_ERROR;
  }
}

/****************************************************************************/

void setPALevel(rf24_pa_dbm_e level) {
  uint8_t setup = read_register(RF_SETUP);
  setup &= ~(RF_PWR_LOW | RF_PWR_HIGH);

  switch(level){
    case(RF24_PA_MIN): break;
    case(RF24_PA_LOW): setup |= RF_PWR_LOW; break;
    case(RF24_PA_HIGH): setup |= RF_PWR_HIGH; break;
    case(RF24_PA_MAX): setup |= (RF_PWR_LOW | RF_PWR_HIGH); break;
    case(RF24_PA_ERROR): setup |= (RF_PWR_LOW | RF_PWR_HIGH); break;
  }
  write_register(RF_SETUP, setup);
}

/****************************************************************************/

rf24_pa_dbm_e getPALevel() {
  uint8_t power = read_register(RF_SETUP) & (RF_PWR_LOW | RF_PWR_HIGH);
  switch(power){
    case(RF_PWR_LOW | RF_PWR_HIGH): return RF24_PA_MAX;
    case(RF_PWR_HIGH): return RF24_PA_HIGH;
    case(RF_PWR_LOW): return RF24_PA_LOW;
    default: return RF24_PA_MIN;
  }
}

/****************************************************************************/

void setCRCLength(rf24_crclength_e length) {
  uint8_t config = read_register(CONFIG) & ~(CRCO | EN_CRC);
  
  // switch uses RAM (evil!)
  if (length == RF24_CRC_DISABLED)
  {
    // Do nothing, we turned it off above. 
  }
  else if (length == RF24_CRC_8)
  {
    config |= EN_CRC;
  }
  else
  {
    config |= EN_CRC;
    config |= CRCO;
  }
  write_register(CONFIG, config);
}

/****************************************************************************/

rf24_crclength_e getCRCLength() {
  rf24_crclength_e result = RF24_CRC_DISABLED;
  uint8_t config = read_register(CONFIG) & (CRCO | EN_CRC);

  if (config & EN_CRC)
  {
    if (config & CRCO)
      result = RF24_CRC_16;
    else
      result = RF24_CRC_8;
  }

  return result;
}

/****************************************************************************/

bool isPVariant() {
  return p_variant;
}
/****************************************************************************/

void rf24_disableCRC() {
  uint8_t disable = read_register(CONFIG) & ~EN_CRC;
  write_register(CONFIG, disable);
}

/****************************************************************************/
void rf24_setRetries(uint8_t delay, uint8_t count) {
 write_register(SETUP_RETR, (delay&0xf)<<ARD | (count&0xf)<<ARC);
}
/****************************************************************************/

void print_status(uint8_t status) {
  printf("STATUS\t\t = 0x%02x RX_DR=%x TX_DS=%x MAX_RT=%x RX_P_NO=%x TX_FULL=%x\r\n", 
           status, 
           (status & RX_DR)?1:0, 
           (status & TX_DS)?1:0, 
           (status & MAX_RT)?1:0, 
           ((status >> RX_P_NO) & 0b111), 
           (status & TX_FIFO_FULL)?1:0
         );
}

/****************************************************************************/

void print_observe_tx(uint8_t value) {
  printf("OBSERVE_TX=%02x: POLS_CNT=%x ARC_CNT=%x\r\n", 
           value, 
           (value >> PLOS_CNT) & 0b1111, 
           (value >> ARC_CNT) & 0b1111
         );
}

/****************************************************************************/

void print_byte_register(char* name, uint8_t reg) {
  printf("\t%s =", name);
  printf(" 0x%x", read_register(reg++));

}

/****************************************************************************/

void print_address_register(char* name, uint8_t reg, uint8_t qty) {
  printf("\t%s =", name);

  while (qty--)
  {
    uint8_t buffer[5];
    read_register_for(reg++, buffer, sizeof(buffer));

    printf(" 0x");
    uint8_t* bufptr = buffer + sizeof(buffer);
    while(--bufptr >= buffer)
      printf("%02x", *bufptr);
  }

  printf("\r\n");
}

/****************************************************************************/

// rf24_RF24(string _spidevice, uint32_t _spispeed, uint8_t _cepin):
// spidevice(_spidevice) , spispeed(_spispeed), ce_pin(_cepin), wide_band(TRUE), p_variant(FALSE), 
//   payload_size(32), ack_payload_available(FALSE), dynamic_payloads_enabled(FALSE), 
//   pipe0_reading_address(0) {

// }
// **************************************************************************

// rf24_RF24(uint8_t _cepin, uint8_t _cspin):
//   ce_pin(_cepin), csn_pin(_cspin), wide_band(TRUE), p_variant(FALSE), 
//   payload_size(32), ack_payload_available(FALSE), dynamic_payloads_enabled(FALSE), 
//   pipe0_reading_address(0) {
// }

/****************************************************************************/

void setChannel(uint8_t channel) {
  // TODO: This method could take advantage of the 'wide_band' calculation
  // done in setChannel() to require certain channel spacing.
  write_register(RF_CH, (channel < MAX_CHANNEL ? channel : MAX_CHANNEL));
}

/****************************************************************************/

void rf24_setPayloadSize(uint8_t size) {
  payload_size = (size < MAX_PAYLOAD_SIZE ? size : MAX_PAYLOAD_SIZE);
}

/****************************************************************************/

uint8_t rf24_getPayloadSize() {
  return payload_size;
}

/****************************************************************************/

static const char rf24_datarate_e_str_0[] PROGMEM = "1MBPS";
static const char rf24_datarate_e_str_1[] PROGMEM = "2MBPS";
static const char rf24_datarate_e_str_2[] PROGMEM = "250KBPS";
static const char * const rf24_datarate_e_str_P[] PROGMEM = {
  rf24_datarate_e_str_0, 
  rf24_datarate_e_str_1, 
  rf24_datarate_e_str_2, 
};
static const char rf24_model_e_str_0[] PROGMEM = "nRF24L01";
static const char rf24_model_e_str_1[] PROGMEM = "nRF24L01+";
static const char * const rf24_model_e_str_P[] PROGMEM = {
  rf24_model_e_str_0, 
  rf24_model_e_str_1, 
};
static const char rf24_crclength_e_str_0[] PROGMEM = "Disabled";
static const char rf24_crclength_e_str_1[] PROGMEM = "8 bits";
static const char rf24_crclength_e_str_2[] PROGMEM = "16 bits";
static const char * const rf24_crclength_e_str_P[] PROGMEM = {
  rf24_crclength_e_str_0, 
  rf24_crclength_e_str_1, 
  rf24_crclength_e_str_2, 
};
static const char rf24_pa_dbm_e_str_0[] PROGMEM = "PA_MIN";
static const char rf24_pa_dbm_e_str_1[] PROGMEM = "PA_LOW";
static const char rf24_pa_dbm_e_str_2[] PROGMEM = "PA_HIGH";
static const char rf24_pa_dbm_e_str_3[] PROGMEM = "PA_MAX";
static const char * const rf24_pa_dbm_e_str_P[] PROGMEM = { 
  rf24_pa_dbm_e_str_0, 
  rf24_pa_dbm_e_str_1, 
  rf24_pa_dbm_e_str_2, 
  rf24_pa_dbm_e_str_3, 
};

void rf24_printDetails() {

  printf("SPI device\t = %s\r\n", spidevice);
  printf("SPI speed\t = %d\r\n", spispeed);
  printf("CE GPIO\t = %d\r\n", ce_pin);
	
  print_status(get_status());

  print_address_register("RX_ADDR_P0-1", RX_ADDR_P0, 2);
  print_byte_register("RX_ADDR_P2-5", RX_ADDR_P2);
  //print_address_register("TX_ADDR", TX_ADDR);

  print_byte_register("RX_PW_P0-6", RX_PW_P0);
  print_byte_register("EN_AA", EN_AA);
  print_byte_register("EN_RXADDR", EN_RXADDR);
  print_byte_register("RF_CH", RF_CH);
  print_byte_register("RF_SETUP", RF_SETUP);
  print_byte_register("CONFIG", CONFIG);
  print_byte_register("DYNPD/FEATURE", DYNPD);

  printf("Data Rate\t = %s\r\n", pgm_read_word(&rf24_datarate_e_str_P[getDataRate()]));
  printf("Model\t\t = %s\r\n", pgm_read_word(&rf24_model_e_str_P[isPVariant()]));
  printf("CRC Length\t = %s\r\n", pgm_read_word(&rf24_crclength_e_str_P[getCRCLength()]));
  printf("PA Power\t = %s\r\n", pgm_read_word(&rf24_pa_dbm_e_str_P[getPALevel()]));
}

/****************************************************************************/

void rf24_begin() {
  // Initialize pins
  pinMode(ce_pin, OUTPUT);

  if (strncmp(spidevice, "/dev/spidev0.1", 14) == 0) {
  	csn_pin=9;
  } else {
  	csn_pin=8;
  }
  pinMode(csn_pin, OUTPUT);

    // Initialize SPI bus
  spi = spi_init(spidevice, 0, SPI_BITS, spispeed);

  ce(LOW);
  csn(HIGH);

  // Must allow the radio time to settle else configuration bits will not necessarily stick.
  // This is actually only required following power up but some settling time also appears to
  // be required after resets too. For full coverage, we'll always assume the worst.
  // Enabling 16b CRC is by far the most obvious case if the wrong timing is used - or skipped.
  // Technically we require 4.5ms + 14us as a worst case. We'll just call it 5ms for good measure.
  // WARNING: Delay is based on P-variant whereby non-P *may* require different timing.
  delay(5);

  // Adjustments as per gcopeland fork  
  //resetcfg();

  // Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
  // WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
  // sizes must never be used. See documentation for a more complete explanation.
  write_register(SETUP_RETR, (0b0101 << ARD) | (0b1111 << ARC));

  // Restore our default PA level
  setPALevel(RF24_PA_MAX);

  // Determine if this is a p or non-p RF24 module and then
  // reset our data rate back to default value. This works
  // because a non-P variant won't allow the data rate to
  // be set to 250Kbps.
  if(setDataRate(RF24_250KBPS))
  {
    p_variant = TRUE;
  }
  
  // Then set the data rate to the slowest (and most reliable) speed supported by all
  // hardware.
  setDataRate(RF24_1MBPS);

  // Initialize CRC and request 2-byte (16bit) CRC
  setCRCLength(RF24_CRC_16);
  
  // Disable dynamic payloads, to match dynamic_payloads_enabled setting
  write_register(DYNPD, 0);

  // Reset current status
  // Notice reset and flush is the last thing we do
  write_register(STATUS, (RX_DR | TX_DS | MAX_RT));

  // Set up default configuration.  Callers can always change it later.
  // This channel should be universally safe and not bleed over into adjacent
  // spectrum.
  setChannel(76);

  // Flush buffers
  flush_rx();
  flush_tx();
}

/****************************************************************************/


void rf24_resetcfg(){
	write_register(CONFIG, RST_CFG);
}

void rf24_startListening() {
  write_register(CONFIG, (read_register(CONFIG) | PWR_UP | PRIM_RX));
  write_register(STATUS, (RX_DR | TX_DS | MAX_RT));

  // Restore the pipe0 adddress, if exists
  if (pipe0_reading_address)
    write_register_for(RX_ADDR_P0, (const uint8_t*)&pipe0_reading_address, 5);


  // Adjustments as per gcopeland fork  
  // Flush buffers
  //flush_rx();
  //flush_tx();

  // Go!
  ce(HIGH);

  // wait for the radio to come up (130us actually only needed)
  delayMicroseconds(130);
}

/****************************************************************************/

void rf24_stopListening() {
  ce(LOW);
  flush_tx();
  flush_rx();
}

/****************************************************************************/

void rf24_powerDown() {
  write_register(CONFIG, (read_register(CONFIG) & ~PWR_UP));
  delayMicroseconds(150); /* Adjustments as per gcopeland fork */
}

/****************************************************************************/

void rf24_powerUp() {
  write_register(CONFIG, (read_register(CONFIG) | PWR_UP));
  delayMicroseconds(150); /* Adjustments as per gcopeland fork */
}

/****************************************************************************/
void startWrite(const void* buf, uint8_t len) {
  // Transmitter power-up
  write_register(CONFIG, ((read_register(CONFIG) | PWR_UP) & ~PRIM_RX));
// Adjustments as per gcopeland fork  
// delayMicroseconds(150);

  // Send the payload
  write_payload(buf, len);

  // Allons!
  ce(HIGH);
  delayMicroseconds(10);
  ce(LOW);
}
/******************************************************************/

bool rf24_write(const void* buf, uint8_t len) {
  bool result = FALSE;

  // Begin the write
  startWrite(buf, len);

  uint8_t observe_tx;
  uint8_t status;
  uint32_t sent_at = __millis();
  const uint32_t timeout = 500; //ms to wait for timeout
  do
  {
    status = read_register_for(OBSERVE_TX, &observe_tx, 1);
    IF_SERIAL_DEBUG(printf("%x", observe_tx));
  }
  while(! (status & (TX_DS | MAX_RT)) && (__millis() - sent_at < timeout));

  bool tx_ok, tx_fail;
  rf24_whatHappened(&tx_ok, &tx_fail, &ack_payload_available);
  
  //printf("%u%u%u\r\n", tx_ok, tx_fail, ack_payload_available);

  result = tx_ok;
  IF_SERIAL_DEBUG(printf("%s\n", result ? "...OK." : "...Failed"));

  // Handle the ack packet
  if (ack_payload_available)
  {
    ack_payload_length = rf24_getDynamicPayloadSize();
    IF_SERIAL_DEBUG(printf("[AckPacket]/"));
    IF_SERIAL_DEBUG(printf("%i\n", ack_payload_length));
  }


  // Disable powerDown and flush_tx as per gcopeland fork
  //powerDown();
  //flush_tx();

  return result;
}

/****************************************************************************/

uint8_t rf24_getDynamicPayloadSize() {
  uint8_t result = 0;

  csn(LOW);
  spi_transfer(spi, R_RX_PL_WID, NULL);
  spi_transfer(spi, 0xff, &result);
  csn(HIGH);

  return result;
}

/****************************************************************************/

bool rf24_available(uint8_t* pipe_num) {
  uint8_t status = get_status();

  // Too noisy, enable if you really want lots o data!!
  //IF_SERIAL_DEBUG(print_status(status));

  bool result = (status & RX_DR);

  if (result)
  {
    // If the caller wants the pipe number, include that
    if (pipe_num)
      *pipe_num = (status >> RX_P_NO) & 0b111;

    // Clear the status bit

    // ??? Should this REALLY be cleared now?  Or wait until we
    // actually READ the payload?

    write_register(STATUS, RX_DR);

    // Handle ack payload receipt
    if (status & TX_DS)
    {
      write_register(STATUS, TX_DS);
    }
  }

  return result;
}

/****************************************************************************/

bool rf24_read(void* buf, uint8_t len) {
  // Fetch the payload
  read_payload(buf, len);
  // was this the last of the data available?
  return read_register(FIFO_STATUS) & RX_EMPTY;
}

/****************************************************************************/

void rf24_whatHappened(bool *tx_ok, bool *tx_fail, bool *rx_ready) {
  // Read the status & reset the status in one easy call
  // Or is that such a good idea?
  uint8_t status = write_register(STATUS, (RX_DR | TX_DS | MAX_RT));
  // Report to the user what happened
  *tx_ok = status & TX_DS;
  *tx_fail = status & MAX_RT;
  *rx_ready = status & RX_DR;
}

/****************************************************************************/

void rf24_openWritingPipe(uint64_t value) {
  // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
  // expects it LSB first too, so we're good.

  write_register_for(RX_ADDR_P0, (uint8_t*)&value, 5);
  write_register_for(TX_ADDR, (uint8_t*)&value, 5);

  const uint8_t max_payload_size = 32;
  write_register(RX_PW_P0, (payload_size < max_payload_size ? payload_size : max_payload_size));
}

/****************************************************************************/

static const uint8_t child_pipe[] PROGMEM = {
  RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5
};
static const uint8_t child_payload_size[] PROGMEM = {
  RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5
};
static const uint8_t child_pipe_enable[] PROGMEM = {
  ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5
};

void rf24_openReadingPipe(uint8_t child, uint64_t address) {
  // If this is pipe 0, cache the address.  This is needed because
  // openWritingPipe() will overwrite the pipe 0 address, so
  // startListening() will have to restore it.
  if (child == 0)
    pipe0_reading_address = address;

  if (child <= 6)
  {
    // For pipes 2-5, only write the LSB
    if (child < 2)
      write_register_for(pgm_read_byte(&child_pipe[child]), (const uint8_t*)&address, 5);
    else
      write_register_for(pgm_read_byte(&child_pipe[child]), (const uint8_t*)&address, 1);

    write_register(pgm_read_byte(&child_payload_size[child]), payload_size);

    // Note it would be more efficient to set all of the bits for all open
    // pipes at once.  However, I thought it would make the calling code
    // more simple to do it this way.
    write_register(EN_RXADDR, read_register(EN_RXADDR) | _BV(pgm_read_byte(&child_pipe_enable[child])));
  }
}

/****************************************************************************/

void toggle_features() {
  csn(LOW);
  spi_transfer(spi, ACTIVATE, NULL);
  spi_transfer(spi, ACTIVATE_2, NULL);
  csn(HIGH);
}

/****************************************************************************/

void rf24_enableDynamicPayloads() {
  // Enable dynamic payload throughout the system
  write_register(FEATURE, (read_register(FEATURE) | EN_DPL));

  // If it didn't work, the features are not enabled
  if (! read_register(FEATURE))
  {
    // So enable them and try again
    toggle_features();
    write_register(FEATURE, (read_register(FEATURE) | EN_DPL));
  }

  IF_SERIAL_DEBUG(printf("FEATURE=%i\r\n", read_register(FEATURE)));

  // Enable dynamic payload on all pipes
  //
  // Not sure the use case of only having dynamic payload on certain
  // pipes, so the library does not support it.
  write_register(DYNPD, (read_register(DYNPD) | DPL_P5 | DPL_P4 | DPL_P3 | DPL_P2 | DPL_P1 | DPL_P0));

  dynamic_payloads_enabled = TRUE;
}

/****************************************************************************/

void rf24_enableAckPayload() {
  //
  // enable ack payload and dynamic payload features
  //

  write_register(FEATURE, (read_register(FEATURE) | EN_ACK_PAY | EN_DPL));

  // If it didn't work, the features are not enabled
  if (! read_register(FEATURE))
  {
    // So enable them and try again
    toggle_features();
    write_register(FEATURE, (read_register(FEATURE) | EN_ACK_PAY | EN_DPL));
  }

  IF_SERIAL_DEBUG(printf("FEATURE=%i\r\n", read_register(FEATURE)));

  //
  // Enable dynamic payload on pipes 0 & 1
  //

  write_register(DYNPD, (read_register(DYNPD) | DPL_P1 | DPL_P0));
}

/****************************************************************************/

void rf24_writeAckPayload(uint8_t pipe, const void* buf, uint8_t len) {
  const uint8_t* current = (const uint8_t*)buf;

  csn(LOW);
  spi_transfer(spi, W_ACK_PAYLOAD | (pipe & 0b111), NULL);
  const uint8_t max_payload_size = 32;
  uint8_t data_len = (len < max_payload_size ? len : max_payload_size);
  while (data_len--)
    spi_transfer(spi, *current++, NULL);

  csn(HIGH);
}

/****************************************************************************/

bool rf24_isAckPayloadAvailable() {
  bool result = ack_payload_available;
  ack_payload_available = FALSE;
  return result;
}

/****************************************************************************/

void rf24_setAutoAckOnAll(bool enable) {
  if (enable) write_register(EN_AA, ENAA_ALL);
  else write_register(EN_AA, ENAA_NONE);
}

/****************************************************************************/

void rf24_setAutoAckOnPipe(uint8_t pipe, bool enable) {
  if (pipe > 5) return;
  uint8_t en_aa = read_register(EN_AA);
  switch(enable){
    case(TRUE): en_aa |= (1 << pipe); break;
    case(FALSE): en_aa &= (1 << pipe); break;
  }
  write_register(EN_AA, en_aa);
}

/****************************************************************************/

bool rf24_testCarrier() {
  return (read_register(CD) & 1);
}

/****************************************************************************/

bool rf24_testRPD() {
  return (read_register(RPD) & 1);
}

/****************************************************************************/
// vim:ai:cin:sts=2 sw=2 ft=cpp