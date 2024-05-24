/*
what we need in our skbuff is documented here:
https://dev.to/amrelhusseiny/linux-networking-part-1-kernel-net-stack-180l
https://lxr.linux.no/linux+v2.6.20/include/linux/skbuff.h#L184

*/
#ifndef SK_BUFF_H
#define SK_BUFF_H

#include "dl_list.h"
#include "route.h"
#include "net_dir.h"
#include <pthread.h>
#include <stdint.h>

struct sk_buff {
    struct list_head list; // implements *next *prev
    // struct routing_table_entry *rt;
    // struct outgoing_eth_pack_meta *dev;
    uint32_t interface;
    uint16_t protocol; // 2 byte identifier
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

#endif