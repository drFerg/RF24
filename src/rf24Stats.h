#ifndef STATS_H
#define STATS_H
#include <stdint.h>

#define STATS_TX 0
#define STATS_RX 1

typedef struct txrx_stats TXRXStats;

TXRXStats *stats_create(uint8_t interval);

void stats_increment(TXRXStats *stats, uint8_t bytes, uint8_t io);

void stats_retrieve(TXRXStats *stats, uint32_t *tx_rate, uint32_t *rx_rate, 
                    uint64_t *total_bytes_tx, uint64_t *total_bytes_rx);

void stats_start_monitor(TXRXStats *stats);

void stats_destroy(TXRXStats *stats);

#endif /* STATS_H */