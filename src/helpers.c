/*

file containing some helper functions

*/

#include "syshead.h"
#include "helpers.h"

#define ODDBYTE(v) (v)

// calculated in big endian
uint16_t inet_checksum(void *addr, int count, int start_num) {
  /* Compute Internet Checksum for "count" bytes
   *         beginning at location "addr".
   * Taken from
   * https://github.com/iputils/iputils/blob/bacf1b7bb8555c407d065e97015319abef2a742d/ping/ping.c#L1497 with consideration of https://tools.ietf.org/html/rfc1071
   */

  register int nleft = count;
  const unsigned short *w = addr;
  register unsigned short answer;
  register int sum = start_num;

  /*
   *  Our algorithm is simple, using a 32 bit accumulator (sum),
   *  we add sequential 16 bit words to it, and at the end, fold
   *  back all the carry bits from the top 16 bits into the lower
   *  16 bits.
   */
  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }

  /* mop up an odd byte, if necessary */
  if (nleft == 1)
    sum += ODDBYTE(
        *(unsigned char *)w); /* le16toh() may be unavailable on old systems */

  /*
   * add back carry outs from top 16 bits to low 16 bits
   */
  sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
  sum += (sum >> 16);                 /* add carry */
  answer = ~sum;                      /* truncate to 16 bits */
  return (answer);
}