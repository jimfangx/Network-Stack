/*
This file will handle ethernet frame receive parsing & transmit

we need to implement both incoming & outgoing. incoming is a bit easier to implement imo

for receiving/incoming:
- each incoming packet gets stored in a new allocated sk_buff - data will live in sk_buff as it gets passed up the pipeline
*/
#include "net_dir.h"
#include "arp.h"
#include "eth.h"
#include "ip.h"
#include "skbuff.h"
#include "syshead.h"
#include "tuntap_interface.h"

// https://ethernethistory.typepad.com/papers/EthernetSpec.pdf - page 32
#define MAX_FRAME_SIZE 1600

extern int RUNNING;

struct eth_self_properties *dev; // dev will be alive throughout our program - on the heap

// initialize the eth_self_properties struct - fields will be called from main.c (since it may vary depending our linux box)
// self_hardware_mac is in the form of ff:ff:ff:ff:ff:ff
// self_protocol_ip is in the form of 255.255.255.255
// originally ret void
struct eth_self_properties *init_eth_self(char *self_hardware_mac, char *self_protocol_ip, int self_mtu)
{

    dev = malloc(sizeof(struct eth_self_properties));

    // add supported protocols
    // https://www.iana.org/assignments/arp-parameters/arp-parameters.xhtml#:~:text=these%20numbers%20share%20the%20ethertype%20space.%20see%20%5Bhttps%3A%2F%2Fwww.iana.org%2Fassignments%2Fethernet-numbers%5D.%20
    dev->PTYPE = ETH_P_IP; // 0x0800

    // resolve our protocol (ip) addr - man inet_pton
    if (inet_pton(AF_INET, self_protocol_ip, &dev->PROTOCOL_ADDR) != 1) {
        perror("ETHERNET: protocol addr parsing failed");
        exit(1);
    }

    dev->PROTOCOL_ADDR = ntohl(dev->PROTOCOL_ADDR); // big endian -> little endian

    // resolve hardware address
    sscanf(self_hardware_mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &dev->HARDWARE_ADDR[0], &dev->HARDWARE_ADDR[1],
           &dev->HARDWARE_ADDR[2], &dev->HARDWARE_ADDR[3],
           &dev->HARDWARE_ADDR[4], &dev->HARDWARE_ADDR[5]);

    // set mtu
    dev->mtu = self_mtu;

    // print out the values
    printf("~~~TAP DEVICE PROPERTIES~~~\n");
    printf("PTYPE: %x\n", dev->PTYPE);
    printf("PROTOCOL ADDR: %x\n", dev->PROTOCOL_ADDR);
    printf("HARDWARE ADDR: %02x:%02x:%02x:%02x:%02x:%02x\n", dev->HARDWARE_ADDR[0], dev->HARDWARE_ADDR[1], dev->HARDWARE_ADDR[2], dev->HARDWARE_ADDR[3], dev->HARDWARE_ADDR[4], dev->HARDWARE_ADDR[5]);
    printf("MTU: %d\n", dev->mtu);

    return dev;
}

int net_dir_transmit(struct sk_buff *skb, uint8_t *ether_dhost, uint16_t ether_type)
{
    // increase headroom & reduce tailroom
    skb_push(skb, sizeof(struct eth_hdr));

    struct eth_self_properties *dev = skb->dev;
    // cant use unpack_eth_hdr since we dont need to convert to small endian
    struct eth_hdr *outgoing_eth_hdr = (struct eth_hdr *)(skb->data);

    memcpy(outgoing_eth_hdr->ether_dhost, ether_dhost, ETH_ALEN);
    // memcpy(outgoing_eth_hdr->ether_dhost, ether_dhost, sizeof(outgoing_eth_hdr->ether_dhost));
    memcpy(outgoing_eth_hdr->ether_shost, dev->HARDWARE_ADDR, ETH_ALEN);
    // memcpy(outgoing_eth_hdr->ether_shost, dev->HARDWARE_ADDR, sizeof(outgoing_eth_hdr->ether_shost));

    printf("~~~INCOMING ETHERNET PKT HEADER~~~\n");
    printf("SRC MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", outgoing_eth_hdr->ether_shost[0], outgoing_eth_hdr->ether_shost[1], outgoing_eth_hdr->ether_shost[2], outgoing_eth_hdr->ether_shost[3], outgoing_eth_hdr->ether_shost[4], outgoing_eth_hdr->ether_shost[5]);
    printf("DEST MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", outgoing_eth_hdr->ether_dhost[0], outgoing_eth_hdr->ether_dhost[1], outgoing_eth_hdr->ether_dhost[2], outgoing_eth_hdr->ether_dhost[3], outgoing_eth_hdr->ether_dhost[4], outgoing_eth_hdr->ether_dhost[5]);
    printf("ETHER TYPE: %x\n", ether_type);
    printf("SKB LEN: %d\n", skb->len);

    // remember that ether_type will be something like ETH_P_ARP... it is in small
    // endian and needs to be converted to big endian
    outgoing_eth_hdr->ether_type = htons(ether_type);

    printf("ABOUT TO WRITE PACKET\n");

    return tun_write((char *)skb->data, skb->len);
}

void net_dir_receive(struct eth_self_properties *dev)
{
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

        if (tun_read((char *)skb->data, MAX_FRAME_SIZE) < 0) {
            perror("err reading from tun_read");
            free(skb);
            // return NULL;
            exit(1);
        }

        struct eth_hdr *header = unpack_eth_hdr(skb);
        // note that eth_hdr ethertype has been converted to little endian (host byte order) BUT it is still in big endian (network byte order) in skb
        // - this means: skb storage SHOULD BE BIG ENDIAN (NETWORK BYTE ORDER)

        if (header->ether_type == ETH_P_ARP) { // direct to ARP receive; eth_p_arp is little endian format (host byte order)

            printf("~~~ETHERNET: DIRECTING TO ARP~~~\n");
            // print the ARP packet
            printf("PASSED ARP PACKET:\n");
            printf("SRC MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", header->ether_shost[0], header->ether_shost[1], header->ether_shost[2], header->ether_shost[3], header->ether_shost[4], header->ether_shost[5]);
            printf("DEST MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", header->ether_dhost[0], header->ether_dhost[1], header->ether_dhost[2], header->ether_dhost[3], header->ether_dhost[4], header->ether_dhost[5]);

            arp_receive(skb, dev);
        }
        else if (header->ether_type == ETH_P_IP) { // direct to IP receive

            printf("~~~ETHERNET: DIRECTING TO IP~~~\n");

            ip_rcv(skb);
        }
    }
}

void free_eth_self()
{
    free(dev);
}