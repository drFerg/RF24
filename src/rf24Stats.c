#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "rf24Stats.h"

typedef struct txrx_stats {
  uint32_t bytes_tx; /* bytes tx'd in last second */
  uint32_t bytes_rx; /* bytes rx'd in last second */
  uint64_t total_bytes_tx;
  uint64_t total_bytes_rx;
  uint32_t tx_rate;
  uint32_t rx_rate;
  uint8_t interval;
  struct timespec timer;
  pthread_mutex_t lock;
  pthread_t stats_thread;
} TXRXStats;

TXRXStats *stats_create(uint8_t interval) {
    TXRXStats *s = (TXRXStats *)calloc(1, sizeof(TXRXStats));
    if (s == NULL) return NULL;
    s->interval = interval;
    s->timer.tv_sec = interval;
    pthread_mutex_init(&(s->lock), NULL);
    return s;
}

void stats_increment(TXRXStats *stats, uint8_t bytes, uint8_t io) {
    pthread_mutex_lock(&(stats->lock));
    switch(io) {
        case(STATS_TX): stats->bytes_tx += bytes; break;
        case(STATS_RX): stats->bytes_rx += bytes; break;
        default: break;
    }
    pthread_mutex_unlock(&(stats->lock));
}

void stats_retrieve(TXRXStats *stats, uint32_t *tx_rate, uint32_t *rx_rate, 
                    uint64_t *total_bytes_tx, uint64_t *total_bytes_rx) {
    pthread_mutex_lock(&(stats->lock));
    if (tx_rate) *tx_rate = stats->tx_rate;
    if (rx_rate) *rx_rate = stats->rx_rate;
    if (total_bytes_tx) *total_bytes_tx = stats->total_bytes_tx;
    if (total_bytes_rx) *total_bytes_rx = stats->total_bytes_rx;
    pthread_mutex_unlock(&(stats->lock));
}

void *monitor_thread(void *stats) {
    TXRXStats *s = (TXRXStats *)stats;
    struct timespec t;
    for(;;) {
        pthread_mutex_lock(&(s->lock));
        s->tx_rate = s->bytes_tx / s->interval;
        s->rx_rate = s->bytes_rx / s->interval;
        s->total_bytes_tx += s->bytes_tx;
        s->total_bytes_rx += s->bytes_rx;
        s->bytes_tx = 0;
        s->bytes_rx = 0;
        t.tv_sec = s->timer.tv_sec;
        printf("<TX RATE: %d bytes/s>\n<RX RATE: %d bytes/s>\n", s->tx_rate, s->rx_rate);
        pthread_mutex_unlock(&(s->lock));
        nanosleep(&t, (struct timespec *)NULL);
    }
}

void stats_start_monitor(TXRXStats *stats) {
    pthread_create(&(stats->stats_thread), NULL, monitor_thread, (void *) stats);
}

void stats_destroy(TXRXStats *stats){
    pthread_mutex_destroy(&(stats->lock));
    free(stats);
}