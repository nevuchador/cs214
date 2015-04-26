#ifndef __SORTED_LIST__H_
#define __SORTED_LIST__H_

typedef struct SortedList_Node
{
  void * data;
  struct SortedList_Node *next;
  struct SortedList_Node *prev;
  int refByItrCount;
}node_t;

typedef struct SortedList
{
  node_t* base;
  int (*compareFunc)( void *, void * );
  void (*destructFunc)( void * );
} *SortedListPtr, sorted_list_t;

typedef struct SortedListIterator
{
  SortedListPtr list;
  node_t *itr;
} *SortedListIteratorPtr, sorted_list_iterator_t;

typedef int (*CompareFuncT)( void *, void * );
typedef void (*DestructFuncT)( void * );


SortedListPtr SLCreate(CompareFuncT cf, DestructFuncT df);

void SLDestroy(SortedListPtr list);

int SLInsert(SortedListPtr list, void *Obj);

int SLRemove(SortedListPtr list, void *Obj);

void * SLFind(SortedListPtr list, void *Obj, CompareFuncT);

int SLEmpty(SortedListPtr list);

SortedListIteratorPtr SLCreateIterator(SortedListPtr list);

void SLDestroyIterator(SortedListIteratorPtr iter);

void * SLGetItem( SortedListIteratorPtr iter );

void * SLNextItem(SortedListIteratorPtr iter);

int SLIsLastItem(SortedListIteratorPtr iter);

#endif
