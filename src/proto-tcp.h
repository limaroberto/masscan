#ifndef PROTO_TCP_H
#define PROTO_TCP_H

struct Adapter;
struct TCP_Control_Block;
struct TemplatePacket;
#include "packet-queue.h"
#include "output.h"

#define TCP_SEQNO(px,i) (px[i+4]<<24|px[i+5]<<16|px[i+6]<<8|px[i+7])
#define TCP_ACKNO(px,i) (px[i+8]<<24|px[i+9]<<16|px[i+10]<<8|px[i+11])
#define TCP_FLAGS(px,i) (px[(i)+13])
#define TCP_IS_SYNACK(px,i) ((TCP_FLAGS(px,i) & 0x12) == 0x12)
#define TCP_IS_ACK(px,i) ((TCP_FLAGS(px,i) & 0x10) == 0x10)
#define TCP_IS_RST(px,i) ((TCP_FLAGS(px,i) & 0x4) == 0x4)
#define TCP_IS_FIN(px,i) ((TCP_FLAGS(px,i) & 0x1) == 0x1)

/**
 * Create a TCP connection table (to store TCP control blocks) with
 * the desired initial size.
 * 
 * @param entry_count
 *      A hint about the desired initial size. This should be about twice
 *      the number of oustanding connections, so you should base this number
 *      on your transmit rate (the faster the transmit rate, the more 
 *      outstanding connections you'll have). This function will automatically
 *      round this number up to the nearest power of 2, or round it down
 *      if it causes malloc() to not be able to allocate enoug memory.
 */
struct TCP_ConnectionTable *
tcpcon_create_table(    size_t entry_count,
                        struct rte_ring *transmit_queue,
                        struct rte_ring *packet_buffers,
                        struct TemplatePacket *pkt_template,
                        OUTPUT_REPORT_BANNER report_banner,
                        struct Output *out,
                        unsigned timeout
                        );

void
tcpcon_timeouts(struct TCP_ConnectionTable *tcpcon, unsigned secs, unsigned usecs);

enum TCP_What {
    TCP_WHAT_NOTHING,
    TCP_WHAT_TIMEOUT,
    TCP_WHAT_SYNACK,
    TCP_WHAT_RST,
    TCP_WHAT_FIN,
    TCP_WHAT_ACK,
    TCP_WHAT_DATA,
};

void
tcpcon_handle(struct TCP_ConnectionTable *tcpcon, struct TCP_Control_Block *entry,
    int what, const void *p, size_t length,
    unsigned secs, unsigned usecs,
    unsigned seqno_them);


/**
 * Lookup a connection record based on IP/ports.
 */
struct TCP_Control_Block *
tcpcon_lookup_tcb(
    struct TCP_ConnectionTable *tcpcon, 
    unsigned ip_src, unsigned ip_dst,
    unsigned port_src, unsigned port_dst);

/**
 * Create a new TCB (TCP control block)
 */
struct TCP_Control_Block *
tcpcon_create_tcb(
    struct TCP_ConnectionTable *tcpcon, 
    unsigned ip_src, unsigned ip_dst,
    unsigned port_src, unsigned port_dst,
    unsigned my_seqno, unsigned their_seqno);


/**
 * Acknowledge a FIN even if we've forgotten about the connection
 */
void
tcpcon_send_FIN(
    struct TCP_ConnectionTable *tcpcon,
    unsigned ip_me, unsigned ip_them,
    unsigned port_me, unsigned port_them,
    uint32_t seqno_them, uint32_t ackno_them);

#endif
