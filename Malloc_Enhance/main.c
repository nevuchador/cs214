/*-------------------------------------------------------*/
/* main.c                                                */
/* Author: Shihang Zhang                                 */
/*-------------------------------------------------------*/

#include <stdio.h>
#include "myallocator.h"

/*-------------------------------------------------------*/

/* Test different error for myallocator.c. Return 0 */

int main(void)
{
  int x;
  char *p;

  /*-----------------------------------------------------*/
  /* test case 1: pointer never allocated.               */
  /*-----------------------------------------------------*/
  
  printf("\n");
  printf("test case 1: pointer never allocated\n");
  free(&x);

  /*-----------------------------------------------------*/
  /* test case 2: pointer not returned from malloc.      */
  /*-----------------------------------------------------*/

  printf("\n");
  printf("test case 2: pointer not returned from malloc\n");
  p = (char *)malloc(200);
  free(p + 10);
  free(p);

  /*-----------------------------------------------------*/
  /* test case 3: redundant free.                        */
  /*-----------------------------------------------------*/
  
  printf("\n");
  printf("test case 3: redundant free\n");
  p = (char *)malloc(400);
  free(p);
  free(p);

  /*-----------------------------------------------------*/
  /* test case 4: valid operation.                       */
  /*-----------------------------------------------------*/

  printf("\n");
  printf("test case 4: valid operation\n");
  p = (char *)malloc(100);
  free(p);
  p = (char *)malloc(100);
  free(p);

  return 0;
}

