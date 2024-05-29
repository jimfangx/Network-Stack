/*
ipv4 implementation including internet checksum algo

RFCs:
1180 - background info
791 - ip protocol

https://en.wikipedia.org/wiki/IPv4

for receiving (from net_dir, if we identify incoming pkt as ipv4):
    for this implementation we presume that our good old computer PC3 is LNI-2 (see fig 2, rfc 791) and this internet modeul is designed to forward the incoming packet to the right application program

    realistically that this entails: parse incoming packet, check for integrity, send it up to tcp or icmp
*/

#include "syshead.h"
#include "skbuff.h"
#include "eth.h"
#include "ip.h"
#include "helpers.h"
#include "route.h"
#include "arp.h"
#include "icmp.h"

void ip_rcv(struct sk_buff *skb) {
    /*
    Things we need to check:
    version is supported (aka ipv4)
    IHL header min val = 5
    total length is acceptable (ip frag), TO BE IMPLEMENTED LATER
    ttl is > 0; if not, drop packet
    checksum == 0, if not, drop pkt
    switch based on proto if above tests pass
    */

    //extract ip info from skb
    struct ip_hdr *recv_ip_hdr = (struct ip_hdr *)(skb->head + sizeof(struct eth_hdr));

    printf("IP version: %d\n", recv_ip_hdr->version);
    printf("IP IHL: %d\n", recv_ip_hdr->IHL);

    if (recv_ip_hdr->version != IPV4) {
        printf("IP: Not IPV4!\n");
        goto drop_pkt;
    }

    if (recv_ip_hdr->IHL < 5) {
        printf("IP: internet header length < 5\n");
        goto drop_pkt;
    }

    if (recv_ip_hdr->ttl <= 0) {
      // send icmp err
      // https://datatracker.ietf.org/doc/html/rfc792
      // send type 11 err
      printf("IP: TTL Reached\n");
      goto drop_pkt;
    }

    // do checksum
    // recv_ip_hdr->header_checksum = 0;
    // recv_ip_hdr->len = htons(0x54);

    // create dummy iphdr to test checksum
    int checksum = inet_checksum(recv_ip_hdr, recv_ip_hdr->IHL * 4, 0);
    printf("real checksum: %d\n", checksum);
    if (checksum != 0) {
        printf("CHECKSUM DEBUGGING:\n");
        printf("RECV IP HDR IHL: %d\n", recv_ip_hdr->IHL);
        printf("RECV IP HDR VERSION: %d\n", recv_ip_hdr->version);
        printf("RECV IP HDR TOS: %d\n", recv_ip_hdr->tos);
        printf("RECV IP HDR LEN: %d\n", ntohs(recv_ip_hdr->len));
        printf("RECV IP HDR ID: %d\n", ntohs(recv_ip_hdr->id));
        // flags
        // printf("RECV IP HDR FLAGS: %x\n", recv_ip_hdr->flags);
        printf("RECV IP HDR FRAG OFFSET: %x\n", recv_ip_hdr->frag_offset);
        printf("RECV IP HDR TTL: %d\n", recv_ip_hdr->ttl);
        printf("RECV IP HDR PROTOCOL: %d\n", recv_ip_hdr->protocol);
        printf("RECV IP HDR HEADER CHECKSUM: %d\n", recv_ip_hdr->header_checksum);
        printf("RECV IP HDR SRC ADDR: %x\n", recv_ip_hdr->src_addr);
        printf("RECV IP HDR DEST ADDR: %x\n", recv_ip_hdr->dest_addr);

        printf("IP: Invalid checksum, returned %d\n", checksum);
        goto drop_pkt;
    }

    // flip them addresses to small endian
    recv_ip_hdr->src_addr = ntohl(recv_ip_hdr->src_addr);
    recv_ip_hdr->dest_addr = ntohl(recv_ip_hdr->dest_addr);
    recv_ip_hdr->len = ntohs(recv_ip_hdr->len);
    recv_ip_hdr->id = ntohs(recv_ip_hdr->id);

    // send it to the right place according to protocol
    if (recv_ip_hdr->protocol == ICMP_PROTO_SIG) {

        printf("IP RECEIVE: Directing to ICMP\n");

        icmp_receive(skb);
        
        return;
    } else if (recv_ip_hdr->protocol == TCP_PROTO_SIG) {

        printf("IP RECEIVE: Directing to TCP\n");

        return;
    } else {
        printf("IP RECEIVE: Unrecognized protocol, dropping pkt\n");
        goto drop_pkt;
    }

    drop_pkt:
        free_skb(skb);
        return;
}

/*
transmit:
do rt lookup for dest address set in sock
set params in skb for ip header
set skb structs rt & dev

look up arp table to find target mac
    send arp request if needed, then drop pkt

tell net_dir to transmit with ether_type IP
*/
int ip_transmit(struct tcp_sockets *sock, struct sk_buff *skb) {

    // routing table lookup
    struct routing_table_entry *rt_entry = rt_lookup(sock->dest_addr);
    
    // cast skb to enter ip data
    struct ip_hdr *iphdr = (struct ip_hdr*)(skb->head + sizeof(struct eth_hdr));

    // check routing result
    if (!rt_entry) {
        perror("IP TRANSMIT: routing table entry lookup failed!\n");
        return -1;
    }

    // set skb structs rt & dev
    skb->dev = rt_entry->dev;
    skb->rt = rt_entry;

    // set outgoing ip headers
    // reserve space
    skb_push(skb, sizeof(struct ip_hdr));

    iphdr->version = 0x04;
    iphdr->IHL = 0x05;
    iphdr->tos = 0; // routine service
    iphdr->len = htons(skb->len);
    iphdr->frag_offset = htons(0x4000);
    // iphdr->frag_offset = htons(0);
    // iphdr->flags = 0b010;
    iphdr->ttl = 64; // https://www.imperva.com/learn/performance/time-to-live-ttl/#:~:text=The%20common%20default%20TTL%20values,255%20%E2%80%93%20Network%20devices%20like%20routers
    iphdr->protocol = skb->protocol; // skb.proto should be already set to whatever protocol packet came in with - whcih would be the correct protocol for returning pkt
    iphdr->src_addr = htonl(skb->dev->PROTOCOL_ADDR);
    iphdr->dest_addr = htonl(sock->dest_addr);

    iphdr->id = htons(iphdr->id);
    // iphdr->id = 0;

    printf("IP TRANSMIT: IP HEADER\n");
    printf("SRC IP: %x\n", skb->dev->PROTOCOL_ADDR);
    printf("DEST IP: %x\n", sock->dest_addr);

    iphdr->header_checksum = 0;
    iphdr->header_checksum = inet_checksum(iphdr, iphdr->IHL * 4, 0);
    // look up arp table to find target mac send arp request if needed,
    //     then drop pkt

    //         tell net_dir to transmit with ether_type IP

    uint32_t arp_table_lookup_add = sock->dest_addr;

    if (rt_entry->flags & RT_GATEWAY) {
        printf("IP TRANSMIT: RT ENTRY IS GATEWAY\n");
        arp_table_lookup_add = rt_entry->gateway;
    }

    uint8_t *hardware_addr = arp_table_lookup(arp_table_lookup_add);
    printf("IP TRANSMIT: HARDWARE ADDR: %02x:%02x:%02x:%02x:%02x:%02x\n", hardware_addr[0], hardware_addr[1], hardware_addr[2], hardware_addr[3], hardware_addr[4], hardware_addr[5]);

    if (!hardware_addr) {
        arp_request(sock->dest_addr, skb->dev);
        printf("IP TRANSMIT: ARP TABLE LOOKUP RECORD DOES NOT EXIST. SENDING ARP PING, DROPPING CURRENT PKT.\n");
        return -1;
    } else if (hardware_addr) {
        // print the entire skb
        printf("IP TRANSMIT: BEFORE NETDIR TRANSMIT\n");
        // print entire ip header line by line
        printf("IP TRANSMIT: IP HEADER\n");
        printf("VERSION: %d\n", iphdr->version);
        printf("IHL: %d\n", iphdr->IHL);
        printf("TOS: %d\n", iphdr->tos);
        printf("LEN: %d\n", ntohs(iphdr->len));
        printf("FRAG OFFSET: %x\n", ntohs(iphdr->frag_offset));
        printf("TTL: %d\n", iphdr->ttl);
        printf("PROTOCOL: %d\n", iphdr->protocol);
        printf("HEADER CHECKSUM: %d\n", ntohs(iphdr->header_checksum));
        printf("SRC ADDR: %x\n", ntohl(iphdr->src_addr));
        printf("DEST ADDR: %x\n", ntohl(iphdr->dest_addr));
        printf("ID: %d\n", ntohs(iphdr->id));
        return net_dir_transmit(skb, hardware_addr, ETH_P_IP);
    }
    

}