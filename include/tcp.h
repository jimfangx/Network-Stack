#include "skbuff.h"
#include "syshead.h"

// see rfc 793
struct tcp_hdr {
    uint16_t src_port;
    uint16_t dest_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t reserved : 4;
    uint8_t data_offset : 4;
    uint8_t fin : 1,
        syn : 1,
        rst : 1,
        psh : 1,
        ack : 1,
        urg : 1,
        rsv1 : 1, // no congestion implementation - https://www.johnpfernandes.com/2018/12/17/tcp-flags-what-they-mean-and-how-they-help/
        rsv0 : 1;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent_ptr;
    uint8_t data[];
} __attribute__((packed));

// for tcp checksum
struct tcp_pseudo_header {
    uint32_t src_addr;
    uint32_t dest_addr;
    uint8_t zero;
    uint8_t protocol;
    uint16_t tcp_len;
} __attribute__((packed));

void tcp_incoming(struct sk_buff *skb);