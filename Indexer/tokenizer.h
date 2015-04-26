#ifndef __TOKENIZER__H_
#define __TOKENIZER__H_

typedef struct TokenizerT_ {
  char *head, *tail;
}TokenizerT;

TokenizerT *TKCreate(char*);
void TKDestroy(TokenizerT *tk);
char *TKGetNextToken(TokenizerT *tk);

#endif
