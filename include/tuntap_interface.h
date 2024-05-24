#ifndef TUNTAP_INTERFACE_H
#define TUNTAP_INTERFACE_H
void tun_init();
int tun_read(char *buf, int nbyte); // implements read() signature
int tun_write(char *buf, int nbyte);
void free_tun();
#endif