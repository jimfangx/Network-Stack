#ifndef NET_DIR_H_
#define NET_DIR_H_

#include "syshead.h"
#include "skbuff.h"

// our device's properties
struct eth_self_properties {
  // fields here are selected from RFC 826's packet reception algo
  // https://datatracker.ietf.org/doc/html/rfc826#autoid-1 
  // Naming convention from:
  // https://en.wikipedia.org/wiki/Address_Resolution_Protocol#Operating_scope
  // stored in little endian
  // sidenote: avoid u_int16_t to prevent suicide by coworker
                   // https://stackoverflow.com/a/23291438/9108905
  uint16_t PTYPE; // so far we support ipv4 only
  uint32_t PROTOCOL_ADDR; // we've put 2 x 2 bytes together
  uint8_t HARDWARE_ADDR[6]; // xx:xx:xx:xx:xx:xx
  uint32_t mtu;
};

struct eth_self_properties* init_eth_self(char* self_hardware_mac, char* self_protocol_ip,
                   int self_mtu);
// TODO: FIX: 
// for some stupid reason uncommenting this leads to a conflicting types err, however not including it in header works
int net_dir_transmit(struct sk_buff *skb, uint8_t *ether_dhost, uint16_t ether_type);
void net_dir_receive(struct eth_self_properties *dev);
void free_eth_self();

#endif