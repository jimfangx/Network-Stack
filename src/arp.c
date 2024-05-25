/*
ARP resolution

https://www.geeksforgeeks.org/how-address-resolution-protocol-arp-works/

https://en.wikipedia.org/wiki/Address_Resolution_Protocol#Operating_scope

ARP is a request-response protocol. meaning we need to implement the 2 following scenarios:

say this network stack is running on PC3. Our linux box that we initiate pings from is PC1

SCENARIO 1: PC1 sends a ping to PC3.
1. We need to receive that packet, update our translation table with the sender's hardware address (mac) & protocol address (IP)
2. compare target IP address in the packet (target hardware address will be ff:ff:ff:ff:ff:ff for a arp ping). 
3. if target IP equals our PC3's IP, respond with our hardware address (mac) & protocol address (IP) in the sender hardware address/protocol address respectively.
    aside: our PC3's hardware address, hardware address len, protocol address, protocl address len, protocol type, hardware type should be stored in a eth_self_properties struct and set in net_dir.c

SCENARIO 2: PC3 wants to ping the rest of the network
we need to implement a arp transmit func - to be implemented
we specify target ip, target mac ff:ff:ff:ff:ff:ff 

routing/translation table: uses a similar linked list structure as skbuff - this struct is defined in arp.h
    - internal functions not in header to implement: arp_table_add(), arp_table_edit()
*/

#include "syshead.h"
#include "skbuff.h"
#include "eth.h" // for ethernet header length
#include "arp.h"
#include "dl_list.h"

#define HTYPE_ETHERNET 1
// https://en.wikipedia.org/wiki/Address_Resolution_Protocol#Operating_scope - 1 for request, 2 for reply
#define ARP_REQUEST 1
#define ARP_REPLY 2

static LIST_HEAD(arp_cache_head);

static int arp_table_entry_insert(struct ipv4_over_eth_arp_pkt *arp_pkt) {
    // insert an entry into our arp table
    // we will call this function every time we have a new entry. the new entry will get tacked on to the last entry via the linked list in the arp_table_entry struct
    struct arp_table_entry *arp_entry = calloc(1, sizeof(struct arp_table_entry));
    list_init(&arp_entry->list);

    arp_entry->interface = arp_pkt->HTYPE;
    arp_entry->PROTOCOL_ADDR = arp_pkt->SPA; // packet sender's protocol addr
    arp_entry->valid = 1;
    arp_entry->expiry = ((unsigned long)time(NULL)) + 86400000; // 24 hours later
    memcpy(arp_entry->HARDWARE_ADDR, arp_pkt->SHA, sizeof(arp_entry->HARDWARE_ADDR));

    // add to linked list
    add_last(&arp_entry->list, &arp_cache_head);
    
    // ret success
    return 0;
}

// check if entry exists in ARP table, if it does, update entry & ret 1, else ret 0
static int arp_table_entry_exists_and_update(struct ipv4_over_eth_arp_pkt *arp_pkt){
    struct list_head *list_item;
    struct arp_table_entry *entry;

    list_for_each(list_item, &arp_cache_head) {
        entry = get_list_item(list_item, struct arp_table_entry, list);
        
        // check that every part of each entry other than the hardware address equals respective field in arp_pkt
        // if eq, update hardware add in arp_table_entry, ret 1
        // else ret 0
        if (entry->interface == arp_pkt->HTYPE && entry->PROTOCOL_ADDR == arp_pkt->SPA) {
            memcpy(entry->HARDWARE_ADDR, arp_pkt->SHA, sizeof(entry->HARDWARE_ADDR));
            entry->expiry = ((unsigned long)time(NULL)) + 86400000; // 24 hrs later
            entry->valid = 1;
            
            return 1;
        }
    }

    return 0;
}


void arp_receive(struct sk_buff *skb, struct eth_self_properties *dev) {
    // set skb so it skips the ethernet data & allocate our ipv4_over_eth_arp_pkt
    struct ipv4_over_eth_arp_pkt *arp_pkt = (struct ipv4_over_eth_arp_pkt *)(skb->data + sizeof(struct eth_hdr));

    // struct eth_self_properties *dev;  // bring our eth_self_proerperties over

    // turn the packet into little endian
    arp_pkt->HTYPE = ntohs(arp_pkt->HTYPE);
    arp_pkt->PTYPE = ntohs(arp_pkt->PTYPE);
    arp_pkt->OPER = ntohs(arp_pkt->OPER);
    arp_pkt->SPA = ntohl(arp_pkt->SPA);
    arp_pkt->TPA = ntohl(arp_pkt->TPA);


    // begin arp algo: see rfc 826

    // ?Do I have the hardware type in ar$hrd?
    if (arp_pkt->HTYPE != HTYPE_ETHERNET) {
        printf("arp: unsupported hardware type (not ethernet)\n");
        goto drop_packet;
    }
    // ?Do I speak the protocol in ar$pro?
    // note arp uses ipv4
    if (arp_pkt->PTYPE != dev->PTYPE) {
        printf("arp: unsupported protocol (not ipv4)\n");
        goto drop_packet;
    }
    
    int merge = 0;

    /*
    If the pair <protocol type, sender protocol address> is
        already in my translation table, update the sender
        hardware address field of the entry with the new
        information in the packet and set Merge_flag to true.
    */
   merge = arp_table_entry_exists_and_update(arp_pkt);

   // ?Am I the target protocol address?
   // Yes:
   // print protocol addr of dev
    printf("ARP: dev->PROTOCOL_ADDR: %x\n", dev->PROTOCOL_ADDR);
   if (arp_pkt->TPA == dev->PROTOCOL_ADDR) {
        printf("ARP: ARP is for us!\n");
        /*
        If Merge_flag is false, add the triplet <protocol type,
            sender protocol address, sender hardware address> to
            the translation table.
        */
        if (!merge && arp_table_entry_insert(arp_pkt) != 0) {
            perror("ARP: arp table insert errored\n");
            exit(1);
        }

        // ?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)
        // https://en.wikipedia.org/wiki/Address_Resolution_Protocol#Operating_scope - 1 for request, 2 for reply
        // bruh why 2
        if (arp_pkt->OPER == ARP_REQUEST) {
            // arp reply! pass the sk_buff packet we are taking in this func to reply - we can reuse the packet!
            arp_reply(skb, dev, arp_pkt);
            return;
        } else {
            printf("ARP: OPCODE not supported\n");
            goto drop_packet;
        }
   }

drop_packet:
    free_skb(skb);
    return;
}

void arp_reply(struct sk_buff *skb, struct eth_self_properties *dev,
               struct ipv4_over_eth_arp_pkt *arp_pkt) {
  /*
  rfc 826
  Swap hardware and protocol fields, putting the local
          hardware and protocol addresses in the sender fields.
      Set the ar$op field to ares_op$REPLY
      Send the packet to the (new) target hardware address on
          the same hardware on which the request was received.


  NOTE: since arp_pkt is casted from skb in arp_receive, we only need to modify arp_pkt and skb will get modified as well
  */

    // extend used data area of buffer at buffer start - if it exceeds buffer headroom, we kernel panic (err)
    skb_reserve(skb, sizeof(struct eth_hdr) + sizeof(struct ipv4_over_eth_arp_pkt));
    // increase headroom & reduce tailroom
    skb_push(skb, sizeof(struct ipv4_over_eth_arp_pkt));
    
    memcpy(arp_pkt->THA, arp_pkt->SHA, sizeof(arp_pkt->THA)); // target mac now = the original sender's mac
    arp_pkt->TPA = arp_pkt->SPA; // targeted ip is now the original sender's ip

    memcpy(arp_pkt->SHA, dev->HARDWARE_ADDR, sizeof(arp_pkt->SHA));
    arp_pkt->SPA = dev->PROTOCOL_ADDR;

    arp_pkt->OPER = ARP_REPLY;

    // flip all arp_pkt fields from small endian to big endian
    arp_pkt->HTYPE = htons(arp_pkt->HTYPE);
    arp_pkt->PTYPE = htons(arp_pkt->PTYPE);
    arp_pkt->OPER = htons(arp_pkt->OPER);
    arp_pkt->SPA = htonl(arp_pkt->SPA);
    arp_pkt->TPA = htonl(arp_pkt->TPA);

    // fill in dev in skb
    skb->dev = dev;

    // pass off to eth controller net_dir
    net_dir_transmit(skb, arp_pkt->THA, ETH_P_ARP);
    // cleanup
    free_skb(skb);
}