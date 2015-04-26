#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "tokenizer.h"


TokenizerT *TKCreate(char *ts) {
  TokenizerT * t = calloc(1, sizeof(TokenizerT));
  if (t == NULL) {
    fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
    exit(-1);
  }
  t->head = ts;
  t->tail = t->head;
  return t;
}

void TKDestroy(TokenizerT *tk) {
  free(tk);
}

char *TKGetNextToken(TokenizerT *tk) {
  char * ret;
  while (!isalpha(*tk->head) && *tk->head != '\0') tk->head++;
  tk->tail = tk->head;
  while (isalnum(*tk->tail) && *tk->tail != '\0')  {*tk->tail = tolower(*tk->tail); tk->tail++;}
  if (*tk->head != '\0') {
    unsigned long length = strlen(tk->head)-strlen(tk->tail);
    ret = malloc((length+1)*sizeof(char));
    if (ret == NULL) {
      fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
      exit(-1);
    }
    memcpy(ret, tk->head, length);
    ret[length] = '\0';
    tk->head = tk->tail;
    return ret;
  }
  else
    return 0;
}
