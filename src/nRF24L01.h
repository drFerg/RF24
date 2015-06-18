/*
    Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.
*/

/* SPI Command Mnemonics */
#define R_REGISTER    0x00 /* Read register */
#define W_REGISTER    0x20 /* Write register */
#define REGISTER_MASK 0x1F

#define R_RX_PAYLOAD  0x61 /* Dequeues and reads RX payload */
#define W_TX_PAYLOAD  0xA0 /* Enqueues TX payload */

#define ACTIVATE      0x50 /* Activates 3 commands below */
#define ACTIVATE_2    0x73 /* Always sent after ACTIVATE command */
#define R_RX_PL_WID   0x60 /* Read RX-payload width for top R_RX_PAYLOAD */
#define W_ACK_PAYLOAD 0xA8 /* Write payload with ACK packet i.e. piggyback */
#define W_TX_PAYLOAD_NOACK 0xB0 /* Disables Auto-ACK on this packet */

#define FLUSH_TX      0xE1 /* Flush TX FIFO */
#define FLUSH_RX      0xE2 /* Flush RX FIFO */
#define REUSE_TX_PL   0xE3 /* Reuse last TX payload */
#define NOP           0xFF /* No operation, might be used to read STATUS reg */


/* Register Map */
#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define STATUS      0x07
#define OBSERVE_TX  0x08
#define CD          0x09
#define RX_ADDR_P0  0x0A
#define RX_ADDR_P1  0x0B
#define RX_ADDR_P2  0x0C
#define RX_ADDR_P3  0x0D
#define RX_ADDR_P4  0x0E
#define RX_ADDR_P5  0x0F
#define TX_ADDR     0x10
#define RX_PW_P0    0x11
#define RX_PW_P1    0x12
#define RX_PW_P2    0x13
#define RX_PW_P3    0x14
#define RX_PW_P4    0x15
#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17
#define DYNPD	    0x1C
#define FEATURE	    0x1D

/***** Bit Mnemonics *****/

/* CONFIG register bit fields */
#define MASK_RX_DR  0x40
#define MASK_TX_DS  0x20
#define MASK_MAX_RT 0x10
#define CRC_BITS    0x0C /* CRC bits */
#define EN_CRC      0x08 /* Enable CRC */
#define EN_CRC_8    0x08
#define EN_CRC_16   0x0C
#define CRCO        0x04 /* CRC scheme (1 or 2 bytes) */
#define PWR_UP      0x02 /* POWER UP OR DOWN */
#define PRIM_RX     0x01 /* Switch modes RX/TX */

/* EN_AA Enhanced Shockburst register bit fields */
#define ENAA_P5     0x20 /* Enable Auto-ACK in pipe PX */
#define ENAA_P4     0x10 /* ^ */
#define ENAA_P3     0x08 /* ^ */
#define ENAA_P2     0x04 /* ^ */
#define ENAA_P1     0x02 /* ^ */
#define ENAA_P0     0x01 /* ^ */
#define ENAA_ALL    0x3F /* Enable Auto-ACK in all pipes */
#define ENAA_NONE   0x00 /* Disable Auto-ACK in all pipes */

/* EN_RXADDR register bit fields */
#define ERX_P5      0x20 /* Enable data pipe PX */
#define ERX_P4      0x10 /* ^ */
#define ERX_P3      0x08 /* ^ */
#define ERX_P2      0x04 /* ^ */
#define ERX_P1      0x02 /* ^ */
#define ERX_P0      0x01 /* ^ */

/* SETUP_AW register bit fields */
#define AW          0x01 /* Address field width */

/* SETUP_RETR register bit fields */
#define ARD         0x10 /* Auto re-tx delay */
#define ARD_250u    0x00
#define ARD_500u    0x10
#define ARD_750u    0x20
#define ARD_1000u   0x40
#define ARD_1250u   0x80
#define ARD_1500u   0x50
#define ARD_2000u   0xA0
#define ARD_4000u   0xF0
#define ARC         0x01 /* Auto re-tx count */
#define ARC_5       0x05
#define ARC_10      0x0A
#define ARC_15      0x0F

/* RF_CH register bit fields */
#define RF_CH_CMD   0x01 /* Sets frequency channel nRF24L01 operates on */

/* RF_SETUP register bit fields */
#define PLL_LOCK    0x10 /* Force PLL lock signal, TEST_ONLY */
#define RF_DR       0x28 /* RF data rate */
#define RF_DR_LOW   0x20 /* 250Kbps */
#define RF_DR_HIGH  0x08 /* 2Mbps */
#define RF_DR_250K  0x20
#define RF_DR_1M    0x00
#define RF_DR_2M    0x08
#define RF_PWR      0x06 /* RF power output */
#define RF_PWR_LOW  0x02
#define RF_PWR_HIGH 0x04
#define RF_PWR_MAX  0x06
#define LNA_HCURR   0x01 /* Setup LNA gain */

/* STATUS register bit fields */
#define RX_DR       0x40 /* Data ready in RX FIFO */
#define TX_DS       0x20 /* Transmit data sent, packet has been tx */ 
#define MAX_RT      0x10 /* Maximum number of re-tx */
#define RX_P_NO     0x0E /* Data pipe num of packet available in RX FIFO */
#define TX_FIFO_FULL 0x01 /* TX FIFO full */

/* OBSERVE_TX register bit fields */
#define PLOS_CNT    0xF0 /* Count loss packets, caps at 15 */
#define ARC_CNT     0x0F /* Count re-tx packets, resets when tx new packet */

/* CD register bit fields */
#define CD_CMD      0x01 /* Carrier Detect or Received Power Detector in Plus model*/

/* FIFO_STATUS register bit fields */
#define TX_REUSE    0x40 /* Reuse last TX packet */
#define TX_FULL     0x20 /* TX FIFO full */
#define TX_EMPTY    0x10 /* TX FIFO empty */
#define RX_FULL     0x02 /* RX FIFO full */
#define RX_EMPTY    0x01 /* RX FIFO empty */

/* DYNPD register bit fields */
#define DPL_P5	    0x20 /* Enable Dynamic Payload Length in pipe PX */
#define DPL_P4	    0x10 /* ^ */
#define DPL_P3	    0x08 /* ^ */
#define DPL_P2	    0x04 /* ^ */
#define DPL_P1	    0x02 /* ^ */
#define DPL_P0	    0x01 /* ^ */
#define DPL_ALL     0x3F /* ^ */

/* FEATURE register bit fields */
#define EN_DPL	    0x04 /* Enables Dynamic Payload Length */
#define EN_ACK_PAY  0x02 /* Enables Payload with ACK */
#define EN_DYN_ACK  0x01 /* Enables the W_TX_PAYLOAD_NOACK command */
#define RST_CFG     0x0F

/* Timing references */ 
#define POWER_UP_DELAY 150 /* Tpd2stby - delay before CE can be set high */
#define POWER_DOWN_DELAY 150
#define WRITE_DELAY 10 /* Thce - Minimum delay for 1 packet to be sent */ 
#define TRANSITION_DELAY 130

/* Other defines */
#define MAX_CHANNEL 127
#define MAX_PAYLOAD_LEN 32
#define MIN_ADDR_WIDTH 3
#define MAX_ADDR_WIDTH 5
#define MIN_PIPE_NUM 0
#define MAX_PIPE_NUM 5
#define PIPE0_SET 1
#define PIPE0_AUTO_ACKED 2