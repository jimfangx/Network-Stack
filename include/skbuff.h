/*
what we need in our skbuff is documented here:
https://dev.to/amrelhusseiny/linux-networking-part-1-kernel-net-stack-180l
https://lxr.linux.no/linux+v2.6.20/include/linux/skbuff.h#L184

*/
#ifndef SK_BUFF_H_
#define SK_BUFF_H_

#include "dl_list.h"
#include "route.h"
#include "net_dir.h"
#include <pthread.h>
#include <stdint.h>

struct sk_buff {
    struct list_head list; // implements *next *prev
    // struct routing_table_entry *rt;
    struct eth_self_properties *dev;
    uint32_t interface;
    uint16_t protocol; // 2 byte identifier
    uint32_t len; // length of packet
    uint8_t *head;
    uint8_t *data;
    uint8_t *tail;
    uint8_t *end;
};

struct sk_buff_head {
    struct list_head head;
    int len;
};

struct sk_buff *alloc_skb(unsigned int size);

// see linux skb_reserve man page for why we need this
void *skb_reserve(struct sk_buff *skb, unsigned int len);

// see skb_push man page for motivation
uint8_t *skb_push(struct sk_buff *skb, unsigned int len);

void free_skb(struct sk_buff *skb);

#endif