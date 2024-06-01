/*
This is our routing table for mapping an ip to a gateway and netmask for easier
routing

https://www.baeldung.com/cs/routing-table-entry
https://en.wikipedia.org/wiki/Routing_table

implementation wise:
this will be a dll linked list implementation with new entries to the table tacked on to the old one

*/

#include "route.h"
#include "syshead.h"

static LIST_HEAD(rt_head);

void route_add(uint32_t dest, uint32_t gateway, uint32_t netmask, uint8_t flags, struct eth_self_properties *dev)
{

    struct routing_table_entry *rt_entry = calloc(1, sizeof(struct routing_table_entry));
    list_init(&rt_entry->list);

    rt_entry->dest = dest;
    rt_entry->gateway = gateway;
    rt_entry->netmask = netmask;
    rt_entry->flags = flags;
    rt_entry->dev = dev;

    add_last(&rt_entry->list, &rt_head);
}

void rt_init(char *gateway_addr, struct eth_self_properties *dev)
{

    // add dev
    route_add(dev->PROTOCOL_ADDR, 0, 0xffffff00, RT_HOST, dev);
    // add gateway
    // parse our ip address first
    uint32_t gw_ip_dest = 0;
    if (inet_pton(AF_INET, gateway_addr, &gw_ip_dest) != 1) {
        perror("ROUTE: gateway IP parsing (str -> int) failed.\n");
    }
    route_add(0, ntohl(gw_ip_dest), 0, RT_GATEWAY, dev);
}

struct routing_table_entry *rt_lookup(uint32_t dest_addr)
{

    struct list_head *list_item;
    struct routing_table_entry *rt_entry;

    // set rt_entry as record matching dest_addr & same net mask - otherwise, set to gateway
    list_for_each(list_item, &rt_head)
    {
        rt_entry = get_list_item(list_item, struct routing_table_entry, list);
        if ((dest_addr & rt_entry->netmask) == (rt_entry->dest & rt_entry->netmask))
            break;
    }

    return rt_entry;
}
