#ifndef ROUTE_H_
#define ROUTE_H_

#include "dl_list.h"
#include "net_dir.h"
#include "syshead.h"

#define RT_GATEWAY 0x02
#define RT_HOST 0x04

// https://en.wikipedia.org/wiki/Routing_table
// https://unix.stackexchange.com/questions/23383/show-gateway-ip-address-when-performing-ifconfig-command
struct routing_table_entry {
    struct list_head list;
    uint32_t dest;
    uint32_t gateway;
    uint32_t netmask;
    uint8_t flags;
    struct eth_self_properties *dev; // to store information for lower level network stack - upon correct routing, use info held here for sending
};

void rt_init(char *gateway_addr, struct eth_self_properties *dev);
struct routing_table_entry *rt_lookup(uint32_t dest_addr);

#endif