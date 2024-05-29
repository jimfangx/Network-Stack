#ifndef ICMP_H_
#define ICMP_H_

#include "syshead.h"
#include "skbuff.h"

#define ICMP_TYPE_DEST_UNREACHABLE 0x03
#define ICMP_TYPE_ECHO 0x08
#define ICMP_TYPE_ECHO_REPLY 0x00
#define ICMP_TYPE_TIME_EXCEED 0x0B

struct icmp_base_hdr {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint8_t data[];
} __attribute__ ((packed));

// additional data for echos
struct icmp_echo_additions {
    uint16_t id;
    uint16_t seq_num;
    uint8_t data[];
} __attribute__ ((packed));

void icmp_receive(struct sk_buff *skb);
void icmp_reply(struct sk_buff *skb);

#endif