/*
Reimplementation of Linux's skbuff data structure.
https://docs.kernel.org/networking/skbuff.html
struct defs: https://docs.kernel.org/networking/kapi.html#c.sk_buff

why do we need skbuff? can't we just pass our raw data into some raw thing?
https://wiki.linuxfoundation.org/networking/sk_buff#:~:text=However%2C%20sk_buff%20provides%20an%20additional,need%20to%20be%20copied%20around.

essentially:

we are reimplementing a doubly-linked list + the additional functions that skbuff implements (ex: skb_put(), skb_put(), etc)
    - for the doubly linked list implementation, see dl_list.c/h

skbuff will be the main carrier that carries our network data through the networking layers.

*/

#include "skbuff.h"
#include "dl_list.h"

struct sk_buff  *alloc_skb(unsigned int size) {
    struct sk_buff *skb = calloc(1, sizeof(struct sk_buff));
    skb->head = malloc(size);
    skb->data = skb->head;
    skb->tail = skb->head;
    skb->end = skb->head + size;

    list_init(&skb->list);

    return skb;
}

void *skb_reserve(struct sk_buff *skb, unsigned int len) {
  skb->data += len;
  return skb->data;
}

uint8_t *skb_push(struct sk_buff *skb, unsigned int len) {
  skb->data -= len;
  skb->len += len;
  return skb->data;
}

void free_skb(struct sk_buff *skb) {
    free(skb->head);
    free(skb);
}