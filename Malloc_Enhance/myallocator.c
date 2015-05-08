/*-------------------------------------------------------*/
/* myallocator.c                                         */
/* Author: Shihang Zhang                                 */
/*-------------------------------------------------------*/

#include <stddef.h>
#include <stdio.h>
#include "myallocator.h"

#define BLOCKSIZE 8192 /* 8K */

static char block[BLOCKSIZE];
static header_t *freelist_large = NULL;
static header_t freelist_large_base;
static header_t *freelist_small = NULL;
static header_t freelist_small_base;
static header_t malloclist_base;

/*-------------------------------------------------------*/

/* Malloc a block of size bytes. Return the pointer to the first byte of the block */

void *mymalloc(size_t size, const char* file, const long line)
{
  header_t *prevp, *p, *prevp1, *p1;
  unsigned nu;

  nu = (size+sizeof(header_t)-1) / sizeof(header_t) + 1; /* round up */
  if (nu <= 7) {
    if ((prevp = freelist_small) == NULL) {
      freelist_small = prevp = &freelist_small_base;
      freelist_small_base.s.next = (header_t *)block;
      prevp->s.next->s.next = prevp;
      prevp->s.next->s.size = BLOCKSIZE / 2 / sizeof(header_t); /*the first 4K is reserved for small blocks */
      malloclist_base.s.next = NULL;
    }
  } else {
    if ((prevp = freelist_large) == NULL) {
      freelist_large = prevp = &freelist_large_base;
      freelist_large_base.s.next = (header_t *)&block[BLOCKSIZE/2];
      prevp->s.next->s.next = prevp;
      prevp->s.next->s.size = BLOCKSIZE / 2 / sizeof(header_t);  /*the last 4K is reserved for large blocks */
      malloclist_base.s.next = NULL;
    }
  }
  for (p=prevp->s.next; ; prevp=p, p=p->s.next) {
    if (p->s.size >= nu) {
      if (p->s.size == nu) {
        prevp->s.next = p->s.next;
      }
      else {
        p->s.size -= nu;
        p += p->s.size;
        p->s.size = nu;
      }
      /* find proper position for the new malloc block in the malloc list */
      for (prevp1=&malloclist_base, p1=prevp1->s.next; p1 != NULL; prevp1 = prevp1->s.next, p1=p1->s.next) {
        if ((prevp1 == &malloclist_base ? 1 : (p > prevp1)) && p < p1) {
          break;
        }
      }
      prevp1->s.next = p;
      p->s.next = p1;
    }
    return p+1;
  }
  if (p == freelist_small || p == freelist_large) {
    fprintf(stderr, "there isn't  enough space for the requested size. at line %ld in %s\n", line, file);
    return NULL;
  }
}

/* Free the block allocated by mymalloc. */

void myfree(void *ap, const char* file, const long line)
{
  header_t *bp, *prevp, *p, *freelist;

  if ((char *)ap < block || (char *)ap > block+BLOCKSIZE) {
    fprintf(stderr, "this block was never allocated by malloc. at line %ld in %s\n", line, file);
    return;
  }
  bp = (header_t *)ap - 1;
  for (prevp= &malloclist_base, p=prevp->s.next; p != NULL; prevp=prevp->s.next, p=p->s.next) {
    if (p == bp) {
      break;
    }
  }
  if (p == NULL) {
    fprintf(stderr, "this pointer is invalid. at line %ld in %s\n", line, file);
    return;
  }
  prevp->s.next = p->s.next;
  freelist = p->s.size <= 7 ? freelist_small : freelist_large;
  for (p = freelist; p->s.next != freelist; p = p->s.next) {
    if (bp < p->s.next) {
      break;
    }
  }
  if (bp + bp->s.size == p->s.next) {
    bp->s.size += p->s.next->s.size;
    bp->s.next = p->s.next->s.next;
  } else {
    bp->s.next = p->s.next;
  }
  if (p + p->s.size == bp) {
    p->s.size += bp->s.size;
    p->s.next = bp->s.next;
  } else {
    p->s.next = bp;
  }
}
