/*
 * Author: Shihang Zhang
 */

#include 	<stdio.h>
#include	<stdlib.h>
#include	"sorted-list.h"


int CompareFuncInt(void *p1, void *p2)
{
  int d1 = *(int*)p1;
  int d2 = *(int*)p2;
  return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

void DestructFuncInt(void * p)
{
  free(p);
}

int main() {
  int x, *tmp;
  SortedListPtr sl = SLCreate(CompareFuncInt, DestructFuncInt);
  SortedListIteratorPtr itr;
  // initiate a sorted list
  printf("Enter items(End with EOF): ");
  while(scanf("%d",&x) == 1) {
    tmp = malloc(sizeof(int));
    *tmp = x;
    SLInsert(sl, tmp);
  }
  printf("\n");
  
  // output all the items
  itr = SLCreateIterator(sl);
  tmp = SLGetItem(itr);
  while (tmp != NULL) {
    printf("%d\t",*tmp);
    tmp = SLNextItem(itr);
  }
  SLDestroyIterator(itr);
  printf("\n");
  
  // if access the next item of a removed item, print info, otherwise print the value
  itr = SLCreateIterator(sl);
  tmp = SLNextItem(itr);
  tmp = SLNextItem(itr);
  SLRemove(sl, tmp);
  tmp = SLNextItem(itr);
  if (tmp != NULL) {
    printf("%d\n",*tmp);
  }
  SLDestroyIterator(itr);
  
  // output all the items
  itr = SLCreateIterator(sl);
  tmp = SLGetItem(itr);
  while (tmp != NULL) {
    printf("%d\t",*tmp);
    tmp = SLNextItem(itr);
  }
  SLDestroyIterator(itr);
  printf("\n");
  
  SLDestroy(sl);
  return 0;
}
