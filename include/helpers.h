#ifndef HELPERS_H_
#define HELPERS_H_

#include "syshead.h"

uint16_t inet_checksum(void *addr, int count, int start_num);
uint32_t sum_every_16(void *addr, int count);

#endif