/*
 * Copyright (C) 2017 Baptiste CLENET
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @file
 * @brief       OpenThread test application
 *
 * @author      Hyung-Sin Kim <hs.kim@cs.berkeley.edu>
 */

#include <stdio.h>
#include <rtt_stdio.h>
#include "ot.h"
#include <openthread/udp.h>
#include <openthread/cli.h>
#include <openthread/openthread.h>
//#include "periph/dmac.h"
#include "sched.h"
#include "periph/adc.h"
#include "periph/i2c.h"
#include "periph/spi.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef SAMPLE_INTERVAL
#define SAMPLE_INTERVAL (3000000UL)
#endif
#define SAMPLE_JITTER   (0UL)
#define PAYLOAD_SIZE (189)

#define ENABLE_DEBUG (1)
#include "debug.h"

struct benchmark_stats {
    uint64_t time_micros;
    uint32_t hamilton_tcp_segs_sent;
    uint32_t hamilton_tcp_segs_received; /* Not including bad checksum. */

    /* These three at the end of the experiment. */
    uint32_t hamilton_tcp_srtt;
    uint32_t hamilton_tcp_rttdev;
    uint32_t hamilton_tcp_rttlow;
    uint32_t hamilton_tcp_cwnd;
    uint32_t hamilton_tcp_ssthresh;
    uint32_t hamilton_tcp_total_dupacks;

    uint32_t hamilton_slp_packets_sent;
    uint32_t hamilton_slp_frags_received;

    uint32_t hamilton_slp_snd_packets_singlefrag;
    uint32_t hamilton_slp_frags_sent;
    uint32_t hamilton_slp_rcv_packets_singlefrag;
    uint32_t hamilton_slp_packets_reassembled;

    uint32_t hamilton_ll_frames_received;

    uint32_t hamilton_ll_retries_required[12];
    uint32_t hamilton_ll_frames_send_fail;

    uint8_t hamilton_max_retries;
} __attribute__((packed));

uint16_t myRloc = 0;

#ifdef CPU_DUTYCYCLE_MONITOR
volatile uint64_t cpuOnTime = 0;
volatile uint64_t cpuOffTime = 0;
volatile uint32_t contextSwitchCnt = 0;
volatile uint32_t preemptCnt = 0;
#endif
#ifdef RADIO_DUTYCYCLE_MONITOR
uint64_t radioOnTime = 0;
uint64_t radioOffTime = 0;
#endif

uint32_t packetSuccessCnt = 0;
uint32_t packetFailCnt = 0;
uint32_t packetBusyChannelCnt = 0;
uint32_t broadcastCnt = 0;
uint32_t queueOverflowCnt = 0;

uint16_t nextHopRloc = 0;
uint8_t borderRouterLC = 0;
uint8_t borderRouterRC = 0;
uint32_t routeChangeCnt = 0;
uint32_t borderRouteChangeCnt = 0;

uint32_t totalIpv6MsgCnt = 0;
uint32_t Ipv6TxSuccCnt = 0;
uint32_t Ipv6TxFailCnt = 0;
uint32_t Ipv6RxSuccCnt = 0;
uint32_t Ipv6RxFailCnt = 0;

uint32_t pollMsgCnt = 0;
uint32_t mleMsgCnt = 0;

uint32_t mleRouterMsgCnt = 0;
uint32_t addrMsgCnt = 0;
uint32_t netdataMsgCnt = 0;

uint32_t meshcopMsgCnt = 0;
uint32_t tmfMsgCnt = 0;

uint32_t totalSerialMsgCnt = 0;

#define I_AM_RECEIVER

#define RPI_PORT 4992
#define RECEIVER_PORT 40000
#define SENDER_PORT 40240

/* Controls sender. */
#define BLOCK_SIZE 2048
#define NUM_BLOCKS 50

#define TOTAL_BYTES (BLOCK_SIZE * NUM_BLOCKS)

/* Size of blocks in which the receiver consumes the data. */
#define READ_SIZE 512

extern int sent_pkts;
extern int recv_pkts;

void print_tcp_stats(void) {
    printf("Sent %d TCP segments\n", (int) sent_pkts);
    printf("Received %d TCP segments\n", (int) recv_pkts);
#ifdef CPU_DUTYCYCLE_MONITOR
    printf("CPU: on = %llu, off = %llu\n", cpuOnTime, cpuOffTime);
#endif
#ifdef RADIO_DUTYCYCLE_MONITOR
    printf("Radio: on = %llu, off = %llu\n", radioOnTime, radioOffTime);
#endif
}

int tcp_receiver(void (*onaccept)(void), void (*onfinished)(int)) {
    static char recvbuffer[READ_SIZE];

    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in6 local;
    local.sin6_family = AF_INET6;
    local.sin6_addr = in6addr_any;
    local.sin6_port = htons(RECEIVER_PORT);

    int rv = bind(sock, (struct sockaddr*) &local, sizeof(struct sockaddr_in6));
    if (rv == -1) {
        perror("bind");
        return -1;
    }

    rv = listen(sock, 3);
    if (rv == -1) {
        perror("listen");
        return -1;
    }

    for (;;) {
        size_t total_received = 0;
        //memset(&stats, 0x00, sizeof(stats));
        int fd = accept(sock, NULL, 0);
        if (fd == -1) {
            perror("accept");
            break;
        } else {
            printf("Accepted connection\n");

            if (onaccept != NULL) {
                onaccept();
            }
            volatile uint64_t t1 = xtimer_now_usec64();
            printf("Time = %llu, Received: 0\n", xtimer_now_usec64());
            while (total_received != TOTAL_BYTES) {
                size_t readsofar = 0;
                ssize_t r;
                while (readsofar != READ_SIZE) {
                    r = recv(fd, &recvbuffer[readsofar], READ_SIZE - readsofar, 0);
                    if (r < 0) {
                        perror("read");
                        return -1;
                    } else if (r == 0) {
                        printf("read() reached EOF");
                        return -1;
                    }
                    readsofar += r;
                }

                total_received += READ_SIZE;

                /* Print for every 1024 bytes received. */
                if ((total_received & 0x3FF) == 0) {
                    printf("Time = %llu, Received: %d\n", xtimer_now_usec64(), (int) total_received);
                }
            }
            volatile uint64_t t2 = xtimer_now_usec64();
            printf("Total time is %d\n", (int) ((t2 - t1) / 1000));
            print_tcp_stats();

            if (onfinished != NULL) {
                onfinished(fd);
            }

            rv = close(fd);
            if (rv == -1) {
                perror("close");
                return -1;
            }
        }
    }

    close(sock);
    return -1;
}

int tcp_sender(const char* receiver_ip, void (*ondone)(int)) {
    static char sendbuffer[BLOCK_SIZE];
    static char* data = "The Internet is the global system of interconnected computer networks that use the Internet protocol suite (TCP/IP) to link billions of devices worldwide. It is a network of networks that consists of millions of private, public, academic, business, and government networks of local to global scope, linked by a broad array of electronic, wireless, and optical networking technologies. The Internet carries an extensive range of information resources and services, such as the inter-linked hypertext documents and applications of the World Wide Web (WWW), electronic mail, telephony, and peer-to-peer networks for file sharing.\nAlthough the Internet protocol suite has been widely used by academia and the military industrial complex since the early 1980s, events of the late 1980s and 1990s such as more powerful and affordable computers, the advent of fiber optics, the popularization of HTTP and the Web browser, and a push towards opening the technology to commerce eventually incorporated its services and technologies into virtually every aspect of contemporary life.\nThe impact of the Internet has been so immense that it has been referred to as the '8th continent'.\nThe origins of the Internet date back to research and development commissioned by the United States government, the Government of the UK and France in the 1960s to build robust, fault-tolerant communication via computer networks. This work, led to the primary precursor networks, the ARPANET, in the United States, the Mark 1 NPL network in the United Kingdom and CYCLADES in France. The interconnection of regional academic networks in the 1980s marks the beginning of the transition to the modern Internet. From the late 1980s onward, the network experienced sustained exponential growth as generations of institutional, personal, and mobile computers were connected to it.\nInternet use grew rapidly in the West from the mid-1990s and from the late 1990s in the developing world. In the 20 years since 1995, Internet use has grown 100-times, measured for the period of one ";

    /* Fill up the sendbuffer. */
    strncpy(sendbuffer, data, BLOCK_SIZE);

    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in6 receiver;
    receiver.sin6_family = AF_INET6;
    receiver.sin6_port = htons(RECEIVER_PORT);

    int rv = inet_pton(AF_INET6, receiver_ip, &receiver.sin6_addr);
    if (rv == -1) {
        perror("invalid address family in inet_pton");
        return -1;
    } else if (rv == 0) {
        perror("invalid ip address in inet_pton");
        return -1;
    }

    struct sockaddr_in6 local;
    local.sin6_family = AF_INET6;
    local.sin6_addr = in6addr_any;
    local.sin6_port = htons(SENDER_PORT);

    rv = bind(sock, (struct sockaddr*) &local, sizeof(struct sockaddr_in6));
    if (rv == -1) {
        perror("bind");
        return -1;
    }

    rv = connect(sock, (struct sockaddr*) &receiver, sizeof(struct sockaddr_in6));
    if (rv == -1) {
        perror("connect");
        return -1;
    }

    int i;
    size_t total_sent = 0;
    printf("Time = %llu, Sent: 0\n", xtimer_now_usec64());
    for (i = 0; i != NUM_BLOCKS; i++) {
        rv = send(sock, sendbuffer, BLOCK_SIZE, 0);
        if (rv == -1) {
            perror("send");
            return -1;
        }
        total_sent += BLOCK_SIZE;
        if ((total_sent & 0x3FF) == 0) {
            printf("Time = %llu, Sent: %d\n", xtimer_now_usec64(), (int) total_sent);
        }
    }

    if (ondone != NULL) {
        ondone(sock);
    }

    rv = close(sock);
    if (rv == -1) {
        perror("close");
        return -1;
    }

    printf("Closed socket\n");

    return 0;
}

#include "../../RIOT-OS/pkg/openthread/contrib/tcp_freebsd/bsdtcp/tcp_var.h"
extern struct tcpcb tcbs[1];
struct benchmark_stats stats;

void ondone(int fd) {
    stats.hamilton_tcp_srtt = (uint32_t) tcbs[0].t_srtt;
    stats.hamilton_tcp_rttdev = (uint32_t) tcbs[0].t_rttvar;
    stats.hamilton_tcp_rttlow = (uint32_t) tcbs[0].t_rttlow;
    stats.hamilton_tcp_cwnd = (uint32_t) tcbs[0].snd_cwnd;
    stats.hamilton_tcp_ssthresh = (uint32_t) tcbs[0].snd_ssthresh;
    //stats.hamilton_tcp_total_dupacks = (uint32_t) tcbs[0].t_total_dupacks;
    /* Rest of the variables are filled in already ... */

    int rv = send(fd, &stats, sizeof(stats), 0);
    if (rv != sizeof(stats)) {
        perror("Could not send stats");
        return;
    }
}

#ifdef I_AM_RECEIVER
xtimer_ticks64_t start = {0};
void onaccept(void) {
    start = xtimer_now64();
}
void onfinished(int fd) {
    xtimer_ticks64_t end = xtimer_now64();
    /* Now, compute and print out stats. */
    xtimer_ticks64_t total_time = xtimer_diff64(end, start);
    uint64_t total_micros = xtimer_usec_from_ticks64(total_time);
    stats.time_micros = total_micros;

#ifdef PRINT_STATS
    printf("Total microseconds: %d\n", (int) total_micros);
    struct benchmark_stats mystats;
    memcpy(&mystats, &stats, sizeof(struct benchmark_stats));
    print_stats(&mystats, false);
    xtimer_usleep(1000000);

    struct benchmark_stats senderstats;
    if (read_stats(&senderstats, fd) == 0) {
        print_stats(&senderstats, false);
    }
    xtimer_usleep(1000000);

    printf("\n");
    xtimer_usleep(1000000);

    print_stats_plain(&mystats);
    xtimer_usleep(1000000);

    print_stats_plain(&senderstats);
#else
    ondone(fd);
#endif
}
#endif

//#define SENDTO_ADDR "2001:470:4889::114"
// #define SENDTO_ADDR "fdde:ad00:beef:0000:7fbd:d4c2:9fca:c31a"
//#define SENDTO_ADDR "fdde:ad00:beef:0000:9a93:b693:abfa:9def"
#define SENDTO_ADDR "fdde:ad00:beef:0:e9f0:45bc:c507:6f0e"
//#define SENDTO_ADDR "2001:470:4a71:0:b135:ddac:9366:2bb8"

int main(void)
{
#if DMAC_ENABLE
    dmac_init();
    adc_set_dma_channel(DMAC_CHANNEL_ADC);
    i2c_set_dma_channel(I2C_0,DMAC_CHANNEL_I2C);
    spi_set_dma_channel(0,DMAC_CHANNEL_SPI_TX,DMAC_CHANNEL_SPI_RX);
#endif
    DEBUG("This a test for OpenThread\n");
    xtimer_usleep(5000000ul);

    DEBUG("\n[Main] Trying TCP\n");

    int rv;

#ifdef I_AM_RECEIVER
    printf("Running tcp_receiver program\n");
    otIp6AddUnsecurePort(openthread_get_instance(), RECEIVER_PORT);
    rv = tcp_receiver(NULL, NULL);
#else
    printf("Waiting for 5 seconds...\n");
    xtimer_usleep(5000000L);
    printf("Running tcp_sender program\n");
    otIp6AddUnsecurePort(openthread_get_instance(), SENDER_PORT);
    volatile uint64_t t1 = xtimer_now_usec64();
    rv = tcp_sender(SENDTO_ADDR, NULL);
    volatile uint64_t t2 = xtimer_now_usec64();
    printf("Total time is %d\n", (int) ((t2 - t1) / 1000));
    printf("CPU: on = %llu, off = %llu\n", cpuOnTime, cpuOffTime);
#endif

    if (rv == 0) {
        printf("Program is terminating!\n");
    } else {
        printf("Program did not properly terminate!\n");
    }

    while (1) {
        xtimer_usleep(10000000UL);
    }

    return 0;
}
