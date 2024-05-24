/*
This file will handle ethernet frame receive parsing & transmit

we need to implement both incoming & outgoing. incoming is a bit easier to implement imo

for receiving/incoming:
- each incoming packet gets stored in a new allocated sk_buff - data will live in sk_buff as it gets passed up the pipeline
*/
#include "syshead.h"
#include "tuntap_interface.h"
#include "skbuff.h"
#include "eth.h"

// https://ethernethistory.typepad.com/papers/EthernetSpec.pdf - page 32
#define MAX_FRAME_SIZE 1600

extern int RUNNING;


void net_dir_receive() {
    while (RUNNING) {
        /* 
        allocate skbuff
        read from tap + check for error read
            if err, free skbuff, end
        otherwise: 
            cast to eth struct defined in eth.h (eth.h will take care of big endian -> little endian)
            look at ethertype, send to the correct place (ARP/IP)
        */

        struct sk_buff *skb = alloc_skb(MAX_FRAME_SIZE);

        if (tun_read((char *) skb->data, MAX_FRAME_SIZE) < 0) {
            perror("err reading from tun_read");
            free(skb);
            // return NULL;
            exit(1);
        }

        struct eth_hdr *header = unpack_eth_hdr(skb);

        if (header->ether_type == ETH_P_ARP) { // direct to ARP receive
            
            printf("DIRECTING TO ARP\n");
            // print the ARP packet
            printf("ARP PACKET\n");
            printf("SRC MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", header->ether_shost[0], header->ether_shost[1], header->ether_shost[2], header->ether_shost[3], header->ether_shost[4], header->ether_shost[5]);
            printf("DEST MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", header->ether_dhost[0], header->ether_dhost[1], header->ether_dhost[2], header->ether_dhost[3], header->ether_dhost[4], header->ether_dhost[5]);

        } else if (header->ether_type == ETH_P_IP) { // direct to IP receive

            printf("DIRECTING TO IP\n");

        }

    }

}