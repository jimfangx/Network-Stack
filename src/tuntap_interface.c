/*
Tun Tap Interface to allow us to tap into L2 traffic from the kernel
https://docs.kernel.org/networking/tuntap.html

This will be used in our ethernet parsing file

*/

#include "tuntap_interface.h"
#include "syshead.h"

static char *dev;   // name of tun/tap device
static int tun_fd;  // our integer tun file descriptor

// from https://docs.kernel.org/networking/tuntap.html
static int tun_alloc(char *dev) {
  struct ifreq ifr;
  int fd, err;

  if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
    // failed
    perror(
        "Cannot open TUN/TAP dev\n"
        "Make sure one exists with "
        "'$ mknod /dev/net/tap c 10 200'");
    exit(1);
  }

  printf("opened %s\n", dev);

  memset(&ifr, 0, sizeof(ifr));

  /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
   *        IFF_TAP   - TAP device
   *
   *        IFF_NO_PI - Do not provide packet information
   */
  ifr.ifr_flags =
      IFF_TAP |
      IFF_NO_PI;  // https://www.saminiir.com/lets-code-tcp-ip-stack-1-ethernet-arp/
                  // https://stackoverflow.com/questions/43449664/why-the-leading-4bytes-data-missing-when-sending-raw-bytes-data-to-a-tap-device

  // using strncpy here: https://stackoverflow.com/questions/1003684/how-to-interface-with-the-linux-tun-driver
  if (*dev) strncpy(ifr.ifr_name, dev, IFNAMSIZ); 

  if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
    perror("ioctl err");
    close(fd);
    return err;
  }
  
  strcpy(dev, ifr.ifr_name);
  return fd;
}

int tun_read(char *buf, int nbyte) {
    printf("reading...\n");
  return read(tun_fd, buf, nbyte);  // tun_fd will get set by our init func
}

int tun_write(char *buf, int nbyte) {
  printf("writing...\n");
  return write(tun_fd, buf, nbyte);
}

void tun_init() {
  // init our tun device name (max size IFNAMSIZ) + fd
  dev = calloc(IFNAMSIZ, 1);
  strcpy(dev, "tun0");
  tun_fd = tun_alloc(dev);
}

void free_tun() { free(dev); }