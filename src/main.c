#include "syshead.h"
#include "tuntap_interface.h"

int main() {
  tun_init();

  char buf[1600];

  while (1) {
    int read_bytes = tun_read(buf, sizeof(buf));
    printf("read bytes: %d\n", read_bytes);
    printf("read values: %x\n", buf);
  }

  //   free_tun();

  return 0;
}