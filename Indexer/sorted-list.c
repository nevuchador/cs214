#include "sorted-list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

SortedListPtr SLCreate(CompareFuncT cf, DestructFuncT df)
{
  SortedListPtr list;
  if ((list = malloc(sizeof(sorted_list_t))) == NULL) {
    fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));

    exit(-1);
  }
  list->compareFunc = cf;
  list->destructFunc = df;
  list->base = malloc(sizeof(node_t));
  list->base->data = NULL;
  list->base->next = list->base;
  list->base->prev = list->base;
  return list;
}


void SLDestroy(SortedListPtr list)
{
  if (list != NULL) {
    node_t *p = list->base->next;
    while (p != list->base) {
      p->prev->next = p->next;
      p->next->prev = p->prev;
      list->destructFunc(p->data);
      free(p);
      p = list->base->next;
    }
    free(list->base);
    free(list);
  }
}


int SLInsert(SortedListPtr list, void *Obj)
{
  node_t *p = list->base->next;
  while (p != list->base) {
    if (list->compareFunc(Obj, p->data) > 0) {
      break;
    }
    p = p->next;
  }
  node_t *newNode;
  if ((newNode = malloc(sizeof(node_t))) == NULL) {
    fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
    exit(-1);
  }
  newNode->refByItrCount = 0;
  newNode->data = Obj;
  newNode->prev = p->prev;
  p->prev->next = newNode;
  newNode->next = p;
  p->prev = newNode;
  return 1;
}


int SLRemove(SortedListPtr list, void *Obj)
{
  node_t *p = list->base->next;
  while (p != list->base) {
    if (list->compareFunc(Obj, p->data) == 0) {
      break;
    }
    p = p->next;
  }
  if (p == list->base) {
    printf("this item doesn't exist.\n");
    return 0;
  }
  p->next->prev = p->prev;
  p->prev->next = p->next;
  if (p->refByItrCount == 0) {
    list->destructFunc(p->data);
    free(p);
  }
  p->next = NULL;
  p->prev = NULL;
  return 1;
}


void * SLFind(SortedListPtr list, void *Obj, CompareFuncT findCF)
{
  node_t *p = list->base->next;
  while (p != list->base) {
    if (findCF(Obj, p->data) == 0) {
      break;
    }
    p = p->next;
  }
  return p->data;
}

int SLEmpty(SortedListPtr list)
{
  return ((list->base->next == list->base) ? 1 : 0);
}

SortedListIteratorPtr SLCreateIterator(SortedListPtr list)
{
  SortedListIteratorPtr ret;
  if ((ret = malloc(sizeof(sorted_list_iterator_t))) == NULL) {
    fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
    exit(-1);
  }
  ret->list = list;
  ret->itr = SLEmpty(list) ? NULL : list->base->next;
  if (ret->itr != NULL)
    ret->itr->refByItrCount++;
  else
    printf("cannot create iterator since the list is empty.\n");
  return ret;
}


void SLDestroyIterator(SortedListIteratorPtr iter)
{
  if (iter->itr->refByItrCount > 0)
    iter->itr->refByItrCount--;
  if (iter->itr->refByItrCount == 0 && iter->itr->next == NULL) {
    iter->list->destructFunc(iter->itr->data);
    free(iter->itr);
  }
  free(iter);
}


void * SLGetItem( SortedListIteratorPtr iter )
{
  if (iter->itr->next == NULL) {
    printf("the item pointed by this iterator is deleted. please destroy it.\n");
    exit(1);
  }
  return iter->itr->data;

}


void * SLNextItem(SortedListIteratorPtr iter)
{
  // if the item pointed by iter is removed
  if (iter->itr->next == NULL) {
    printf("the item pointed by this iterator is deleted. please destroy it.\n");
    exit(1);
  }
  if (iter->itr->next != iter->list->base) {
    iter->itr->refByItrCount--;
    iter->itr = iter->itr->next;
    iter->itr->refByItrCount++;
  }
  else
    return NULL;
  return SLGetItem(iter);
}

int SLIsLastItem(SortedListIteratorPtr iter)
{
  if (iter->itr->next == iter->list->base)
    return 1;
  else
    return 0;
}
