#include "syshead.h"
#include "tuntap_interface.h"
#include "net_dir.h"

int RUNNING = 1;

int main() {

  // 1. TUN TAP TEST:
  // tun_init();
  

  // char buf[1600];

  // while (1) {
  //   int read_bytes = tun_read(buf, sizeof(buf));
  //   printf("read bytes: %d\n", read_bytes);
  //   printf("read values: %x\n", buf);
  // }
  //   free_tun();

  // 2. ethernet frame parsing test:
  // tun_init();

  // net_dir_receive();



  // 3. ARP test
  tun_init();
  struct eth_self_properties *dev = init_eth_self("1c:b6:bf:a2:55:67", "10.0.3.50", 1500); // random mac address since pc3 is simulated/does not really exist
  net_dir_receive(dev);

  free_tun();

  return 0;
}