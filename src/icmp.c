/*
Internet Controm Message Protocol ICMP implementation

RFCs:
792 - https://datatracker.ietf.org/doc/html/rfc792

functions to implement:
icmp receive
icmp reply

*/

#include "icmp.h"
#include "eth.h"
#include "helpers.h"
#include "ip.h"
#include "syshead.h"
#include "tcp_sockets.h"

void icmp_receive(struct sk_buff *skb)
{

    // move skb to the right place - base hdr first
    struct ip_hdr *temp_ip_hdr = (struct ip_hdr *)(skb->head + sizeof(struct eth_hdr));
    // since we have a data array in our ip_hdr, we can just call that and cast to icmp base hdr struct
    struct icmp_base_hdr *base_hdr = (struct icmp_base_hdr *)(temp_ip_hdr->data);

    // check checksum
    // sizeof(struct icmp_base_hdr)
    uint16_t icmp_len = temp_ip_hdr->len - (temp_ip_hdr->IHL * 4);
    int checksum = inet_checksum(base_hdr, icmp_len, 0);
    if (checksum != 0) {
        printf("ICMP: Invalid checksum\n");
        printf("ICMP: Checksum: %d\n", checksum);
        goto drop_pkt;
    }

    // sort according to type
    if (base_hdr->type == ICMP_TYPE_ECHO) {
        printf("ICMP Requsting echo reply! Forwading to ICMP Echo Reply Func.\n");

        // extract icmp_echo_addtions
        struct icmp_echo_additions *echo_additions = (struct icmp_echo_additions *)(base_hdr->data);
        printf("ICMP: ID: %d\n", ntohs(echo_additions->id));
        printf("ICMP: Seq: %d\n", ntohs(echo_additions->seq_num));

        icmp_reply(skb);
        return;
    }
    else if (base_hdr->type == ICMP_TYPE_DEST_UNREACHABLE) {
        printf("ICMP: Received destination unreachable (likely returns from a request this program has sent). code: %02u\n", base_hdr->code);
        goto drop_pkt;
    }
    else if (base_hdr->type == ICMP_TYPE_TIME_EXCEED) {
        printf(
            "ICMP: Received time exceeded (likely returns from a "
            "request this program has sent). code: %02u\n",
            base_hdr->code);
        goto drop_pkt;
    }
    else if (base_hdr->type == ICMP_TYPE_ECHO_REPLY) {
        printf(
            "ICMP: Received successful echo reply (likely returns from a "
            "request this program has sent).\n");
        goto drop_pkt;
    }

drop_pkt:
    free_skb(skb);
    return;
}

void icmp_reply(struct sk_buff *skb)
{

    /*
    set reply params
    pass to ip_transmit
    */

    // setup sock
    struct tcp_sockets sock;
    memset(&sock, 0, sizeof(struct tcp_sockets));

    // extract ip_header from skb
    struct ip_hdr *reply_ip_hdr = (struct ip_hdr *)(skb->head + sizeof(struct eth_hdr));

    uint16_t icmp_len = reply_ip_hdr->len - (reply_ip_hdr->IHL * 4);

    // reserve & push skb
    skb_reserve(skb, sizeof(struct eth_hdr) + sizeof(struct ip_hdr) + icmp_len);
    skb_push(skb, icmp_len);

    // cast icmp header from ipheader data
    struct icmp_base_hdr *reply_icmp = (struct icmp_base_hdr *)(skb->data);

    reply_icmp->type = ICMP_TYPE_ECHO_REPLY;

    reply_icmp->code = 0x00; // defined in rfc 792

    // extract icmp_echo_addtions
    struct icmp_echo_additions *echo_additions = (struct icmp_echo_additions *)(reply_icmp->data);
    // print id & seq

    // echo_additions->id = echo_additions->id;
    // echo_additions->seq_num = echo_additions->seq_num;

    reply_icmp->checksum = 0;
    reply_icmp->checksum = inet_checksum(reply_icmp, icmp_len, 0);
    printf("ICMP: checksum: icmp len: %d\n", icmp_len);
    printf("ICMP: Checksum: %d\n", reply_icmp->checksum);

    // print every field of icmp echo addtions and icmp base hddr
    printf("ICMP: ID: %d\n", ntohs(echo_additions->id));
    printf("ICMP: Seq: %d\n", ntohs(echo_additions->seq_num));
    printf("ICMP: Type: %d\n", reply_icmp->type);
    printf("ICMP: Code: %d\n", reply_icmp->code);

    skb->protocol = ICMP_PROTO_SIG;

    sock.dest_addr = reply_ip_hdr->src_addr; // we will sort out the rest of the ip_hdr fields in the ip_transmit func

    ip_transmit(&sock, skb);
    free_skb(skb);
}