#include "sorted-list.h"
#include <stdio.h>

SortedListPtr SLCreate(CompareFuncT cf, DestructFuncT df)
{
  SortedListPtr sl;
  sl = malloc(sizeof(struct SortedList));
  if (sl == NULL)
    return NULL;
  sl->CF = cf;
  sl->DF = df;
  //the initial state of a sorted list is that there is a base node
  sl->base = malloc(sizeof(node));
  sl->base->data = NULL;
  sl->base->next = sl->base;
  sl->base->prev = sl->base;
  return sl;
}


void SLDestroy(SortedListPtr list)
{
  node *p = list->base->next;
  // destroy every node in the list until reach the end of the list
  while (p != list->base) {
    p->prev->next = p->next;
    p->next->prev = p->prev;
    list->DF(p->data);
    free(p);
    p = list->base->next;
  }
  free(list->base);
  free(list);
}


int SLInsert(SortedListPtr list, void *newObj)
{
  node *p = list->base->next;
  // find the first item larger or equal to newObj
  while (p != list->base) {
    if (list->CF(newObj, p->data) != -1) {
      break;
    }
    p = p->next;
  }
  node *tmp = malloc(sizeof(node));
  if (tmp == NULL)
    return 0;
  // initialize a node
  tmp->refByItrCount = 0;
  tmp->data = newObj;
  // insert a node
  tmp->prev = p->prev;
  p->prev->next = tmp;
  tmp->next = p;
  p->prev = tmp;
  return 1;
}


int SLRemove(SortedListPtr list, void *newObj)
{
  node *p = list->base->next;
  // find the first item equal to newObj
  while (p != list->base) {
    if (list->CF(newObj, p->data) == 0) {
      break;
    }
    p = p->next;
  }
  // if not find an item equal to newObj
  if (p == list->base)
    return 0;
  p->next->prev = p->prev;
  p->prev->next = p->next;
  //if there is no iterator "pointing" to this item, free it.
  if (p->refByItrCount == 0) {
    list->DF(p->data);
    free(p);
  }
  p->next = NULL;
  p->prev = NULL;
  return 1;
}


SortedListIteratorPtr SLCreateIterator(SortedListPtr list)
{
  SortedListIteratorPtr ret;
  ret = malloc(sizeof(struct SortedListIterator));
  if (ret == NULL)
    return NULL;
  ret->list = list;
  // if the list is empty, the iterator is NULL, otherwise it points to the first element
  ret->itr = (list->base->next == list->base) ? NULL : list->base->next;
  // mark the node that is referenced by an iterator
  if (ret->itr != NULL)
    ret->itr->refByItrCount++;
  return ret;
}


void SLDestroyIterator(SortedListIteratorPtr iter)
{
  if (iter->itr->refByItrCount > 0)
    iter->itr->refByItrCount--;
  
  // if only one iterator points to the item, destroy this item
  if (iter->itr->refByItrCount == 0 && iter->itr->next == NULL) {
    iter->list->DF(iter->itr->data);
    free(iter->itr);
  }
  free(iter);
}


void * SLGetItem( SortedListIteratorPtr iter )
{
  if (iter->itr != iter->list->base)
    return iter->itr->data;
  else
    return NULL;
}


void * SLNextItem(SortedListIteratorPtr iter)
{
  // if the item pointed by iter is removed
  if (iter->itr->next == NULL) {
    printf("This iterator is invalid.\n");
    return NULL;
  }
  iter->itr->refByItrCount--;
  iter->itr = iter->itr->next;
  iter->itr->refByItrCount++;
  return SLGetItem(iter);
}

