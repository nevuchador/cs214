/*-------------------------------------------------------*/
/* myallocator.h                                         */
/* Author: Shihang Zhang                                 */
/*-------------------------------------------------------*/

#ifndef MYALLOCATOR_H
#define MYALLOCATOR_H

#define malloc( x ) mymalloc( x, __FILE__, __LINE__ )
#define free( x ) myfree( x, __FILE__, __LINE__ )

/* Definition of header */

typedef union header {
  struct {
    union header *next;
    unsigned size;
  }s;
  long align;
}header_t;

void *mymalloc(size_t size, const char* file, const long line);
void myfree(void *ap, const char* file, const long line);

#endif
