#ifndef ARP_H_
#define ARP_H_

#include "syshead.h"
#include "skbuff.h"
#include "eth.h"
#include "dl_list.h"
#include "net_dir.h"

// https://datatracker.ietf.org/doc/html/rfc826#autoid-1
// https://en.wikipedia.org/wiki/Address_Resolution_Protocol#Packet_structure
struct ipv4_over_eth_arp_pkt {
    uint16_t HTYPE;
    uint16_t PTYPE;
    uint8_t HLEN;
    uint8_t PLEN;
    uint16_t OPER; // aka opcode
    uint8_t SHA[6];
    uint32_t SPA;
    uint8_t THA[6];
    uint32_t TPA;
} __attribute__ ((packed));

// https://www.auvik.com/franklyit/blog/what-is-an-arp-table/
// only implementing the stuff under ARP entry
struct arp_table_entry {
    struct list_head list;
    uint16_t interface;
    uint8_t HARDWARE_ADDR[6];
    uint32_t PROTOCOL_ADDR;
    unsigned int valid;
    unsigned int expiry; // unix time stamp + x seconds - to be implemented later
};

void arp_receive(struct sk_buff *skb, struct eth_self_properties *dev);
void arp_reply(struct sk_buff *skb, struct eth_self_properties *dev, struct ipv4_over_eth_arp_pkt *arp_pkt);

#endif