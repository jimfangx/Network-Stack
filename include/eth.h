/*
defs for ethernet frame struct & code

The following is helpful for interpreting our internet frame.
This data should come from our TAP driver

https://medium.com/kernel-space/unpacking-a-raw-packet-ethernet-frame-part-1-e91033e745a4
    - Heres a chart! Note that preamble is "omitted"

We will be implementing 802.3 frame structure

Here is the original standard:
https://ethernethistory.typepad.com/papers/EthernetSpec.pdf (page 27)


Linux provides a struct for the ethernet frame header as a guide also:
https://github.com/torvalds/linux/blob/master/include/uapi/linux/if_ether.h#L174

https://stackoverflow.com/questions/35306080/parsing-ethernet-frames-and-data-types

*/

#ifndef ETH_H_
#define ETH_H_

#include "skbuff.h"
#include "syshead.h"

#define ETH_ALEN 6

struct eth_hdr {
    uint8_t ether_dhost[ETH_ALEN]; // destination mac
    uint8_t ether_shost[ETH_ALEN]; // source mac
    uint16_t ether_type;
    // uint8_t payload[]; // not in official implementation
} __attribute__((packed));

static inline struct eth_hdr *unpack_eth_hdr(struct sk_buff *skb)
{
    struct eth_hdr *eth_hdr = (struct eth_hdr *)(skb->head);
    // convert ether_type from big endian -> small endian
    eth_hdr->ether_type = ntohs(eth_hdr->ether_type);
    return eth_hdr;
};

#endif