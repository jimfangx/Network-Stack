/*
TCP implementation

RFCs:
793
7414

*/

#include "syshead.h"
#include "skbuff.h"
#include "tcp.h"
#include "eth.h"
#include "ip.h"
#include "helpers.h"


void tcp_incoming(struct sk_buff *skb) {
  // extract ip header & tcp header from skb
  struct ip_hdr *incoming_ip = (struct ip_hdr *)(skb->head + sizeof(struct eth_hdr));
  struct tcp_hdr *incoming_tcp = (struct tcp_hdr *)(incoming_ip->data);

  // for tcp handshake only, we don't need to worry about sockets & ports

  // print every field of tcp header
  printf("TCP INCOMING: Src Port: %x\n", ntohs(incoming_tcp->src_port));
  printf("TCP INCOMING: Dest Port: %x\n", ntohs(incoming_tcp->dest_port));
  printf("TCP INCOMING: Seq Num: %u\n", ntohl(incoming_tcp->seq_num));
  printf("TCP INCOMING: Ack Num: %u\n", ntohl(incoming_tcp->ack_num));
  printf("TCP INCOMING: Data Offset: %d\n", incoming_tcp->data_offset);
  printf("TCP INCOMING: Control Bits: %x\n", incoming_tcp->ack);
  printf("TCP INCOMING: Window: %u\n", ntohs(incoming_tcp->window));
  printf("TCP INCOMING: Checksum: %x\n", ntohs(incoming_tcp->checksum));
  printf("TCP INCOMING: Urgent Ptr: %d\n", ntohs(incoming_tcp->urgent_ptr));

  // check checksum
  uint16_t checksum = tcp_checksum(incoming_ip, incoming_tcp);
  if (checksum != 0) {
    printf("TCP INCOMING: checksum failed!\n");
    // printf("TCP INCOMING: Checksum: %x\n", checksum);
  }

  // otherwise direct to output
  tcp_out(skb);


}

// bruh tcp checksum has to be a unicorn
int tcp_checksum(struct ip_hdr *iphdr, struct tcp_hdr *tcphdr) {
  struct tcp_pseudo_header tcpphdr;
  // int tcp_len = iphdr->len - (iphdr->IHL * 4);


  tcpphdr.src_addr = htonl(iphdr->src_addr);
  tcpphdr.dest_addr = htonl(iphdr->dest_addr);
  tcpphdr.zero = 0;
  tcpphdr.protocol = iphdr->protocol;
  // tcpphdr.tcp_len = htons(tcphdr->data_offset * 4);  
  tcpphdr.tcp_len = htons((tcphdr->data_offset) << 2);  
  // tcpphdr.tcp_len = 40;  
  
  printf("TCP CHECKSUM: TCP LEN: %d\n", tcphdr->data_offset);
  printf("TCP CHECKSUM SUDO HEADER SRC ADDR: %x\n", tcpphdr.src_addr);
  printf("TCP CHECKSUM SUDO HEADER DEST ADDR: %x\n", tcpphdr.dest_addr);
  printf("TCP CHECKSUM SUDO HEADER ZERO: %x\n", tcpphdr.zero);
  printf("TCP CHECKSUM SUDO HEADER PROTOCOL: %x\n", tcpphdr.protocol);
  printf("TCP CHECKSUM SUDO HEADER TCP LEN: %d\n", tcpphdr.tcp_len);

  // int starting_sum = sum_every_16(&tcpphdr, sizeof(struct tcp_pseudo_header));
  int starting_sum = sum_every_16(&tcpphdr, 12);

  printf("TCP CHECKSUM: Starting Sum: %x\n", starting_sum);

  int final_sum = inet_checksum(tcphdr, tcphdr->data_offset * 4, starting_sum);


  printf("TCP CHECKSUM: Final Sum: %x\n", final_sum);

  return final_sum;
}


void tcp_out(struct sk_buff *skb) {

  // seq = random #
  // ack = previous seq + 1
  // control bits SYN + ACK
  // calculate checksum

  uint16_t temp_port;

  struct tcp_sockets sock;
  memset(&sock, 0, sizeof(struct tcp_sockets));

  struct ip_hdr *outgoing_ip = (struct ip_hdr *)(skb->head + sizeof(struct eth_hdr));

  uint16_t tcp_len = outgoing_ip->len - (outgoing_ip->IHL * 4);

  skb_reserve(skb, sizeof(struct eth_hdr) + sizeof (struct ip_hdr) + tcp_len);
  skb_push(skb, tcp_len);

  struct tcp_hdr *outgoing_tcp = (struct tcp_hdr *)(skb->data);

  // set target ip for ip_transmit
  sock.dest_addr = outgoing_ip->src_addr;

  // swap the ports
  temp_port = outgoing_tcp->dest_port;

  outgoing_tcp->dest_port = outgoing_tcp->src_port;

  outgoing_tcp->src_port = temp_port;

  // ack = previous seq + 1
  outgoing_tcp->ack_num = htonl(ntohl(outgoing_tcp->seq_num) + 1);

  // control bits:
  outgoing_tcp->ack |= 1;

  // seq = random #
  outgoing_tcp->seq_num = htonl(2587902);

  // len
  // outgoing_tcp->data_offset = tcp_len;  // do not implement options for now

  // calc checksum
  outgoing_tcp->checksum = 0;
  outgoing_tcp->checksum = tcp_checksum(outgoing_ip, outgoing_tcp);

  // print checksum
  printf("------XXX--------\nTCP: Checksum: %x\n", outgoing_tcp->checksum);
  // print every field of tcp header
  printf("TCP: Src Port: %d\n", ntohs(outgoing_tcp->src_port));
  printf("TCP: Dest Port: %d\n", ntohs(outgoing_tcp->dest_port));
  printf("TCP: Seq Num: %d\n", ntohl(outgoing_tcp->seq_num));
  printf("TCP: Ack Num: %d\n", ntohl(outgoing_tcp->ack_num));
  printf("TCP: Data Offset: %d\n", outgoing_tcp->data_offset);
  printf("TCP: Control Bits: %x\n", outgoing_tcp->ack);
  printf("TCP: Window: %d\n", ntohs(outgoing_tcp->window));
  printf("TCP: Urgent Ptr: %d\n", ntohs(outgoing_tcp->urgent_ptr));

  // set skb for transmitting
  skb->protocol = TCP_PROTO_SIG;

  ip_transmit(&sock, skb);
  free_skb(skb);
}