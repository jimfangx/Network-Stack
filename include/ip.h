#ifndef IP_H_
#define IP_H_

#include "syshead.h"
#include "skbuff.h"
#include "tcp_sockets.h"

#define IPV4 0x04 // for ip version in ip_hdr

// https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
#define TCP_PROTO_SIG 0x06
#define ICMP_PROTO_SIG 0x01

struct ip_hdr {
    // https://stackoverflow.com/questions/11197931/what-is-meaning-of-in-struct-c
    uint8_t IHL : 4; // internet header length
    uint8_t version : 4; // "but how can i get a 4-bit variable?" -- "Build your own microprocessor." - but u can pack bits (see above), though upper bit comes first so IHL & version is reversed
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    // uint8_t flags : 3;
    // uint16_t frag_offset : 13;
    uint16_t frag_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint32_t src_addr;
    uint32_t dest_addr;
    uint8_t data[];
} __attribute__ ((packed));

void ip_rcv(struct sk_buff *skb);
int ip_transmit(struct tcp_sockets *sock, struct sk_buff *skb);

#endif