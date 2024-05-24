#include "syshead.h"
#include "tuntap_interface.h"
#include "net_dir.h"

int RUNNING = 1;

int main() {
  tun_init();

  // char buf[1600];

  // while (1) {
  //   int read_bytes = tun_read(buf, sizeof(buf));
  //   printf("read bytes: %d\n", read_bytes);
  //   printf("read values: %x\n", buf);
  // }
  //   free_tun();

  net_dir_receive();

  return 0;
}