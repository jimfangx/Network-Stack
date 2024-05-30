#include "syshead.h"
#include "tuntap_interface.h"
#include "net_dir.h"
#include "route.h"

int RUNNING = 1;

int main() {

  // 1. TUN TAP TEST: - run ping 10.0.3.50, see if program prints the write stuff - min byte 48 bytes - 4 bytes header = 44 bytes (i think)
  // tun_init();
  

  // char buf[1600];

  // while (1) {
  //   int read_bytes = tun_read(buf, sizeof(buf));
  //   printf("read bytes: %d\n", read_bytes);
  //   printf("read values: %x\n", buf);
  // }
  //   free_tun();

  // 2. ethernet frame parsing test: - run ping 10.0.3.50, see if program directs to the correct function (arp), check src & dest mac. dest mac should be ff:ff:ff:ff:ff:ff, src mac should tun0 (virtual network interface mac add) shown in ifconfig
  // tun_init();

  // net_dir_receive();



  // 3. ARP test arping -I <tun/tap interface name> 10.0.3.50 - should get reply. if applied mox in readme.md, tun/tap entry should be in arp table - check with: arp -a
  // tun_init();
  // struct eth_self_properties *dev = init_eth_self("1c:b6:bf:a2:55:67", "10.0.3.50", 1500); // random mac address since pc3 is simulated/does not really exist
  // net_dir_receive(dev);

  // free_tun();

  //4. ping 10.0.3.50 - the ping command should work now
  // tun_init();
  // struct eth_self_properties *dev = init_eth_self("1c:b6:bf:a2:55:67", "10.0.3.50", 1500);
  // rt_init("10.0.50.0", dev); // init our routing table + add tun/tap interface gateway entry

  // net_dir_receive(dev);
  
  // free_tun();

  // 5. nmap -Pn 10.0.3.50 -p <any port> - should show the port as open
  tun_init();
  struct eth_self_properties *dev = init_eth_self("1c:b6:bf:a2:55:67", "10.0.3.50", 1500);
  rt_init("10.0.50.0", dev); // init our routing table + add tun/tap interface gateway entry

  net_dir_receive(dev);
  
  free_tun();

  return 0;
}