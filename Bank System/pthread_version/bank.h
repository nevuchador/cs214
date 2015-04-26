#ifndef __Bank_System__bank__
#define __Bank_System__bank__

#include <stdio.h>
#include <pthread.h>
#include "uthash.h"

/* macros */
#define ACCOUNT_MAX (20)
#define ACCOUNT_NAME_MAX (100)
#define ERROR_INFO_SIZE (50)

/* return values */
#define SUCCESS (1)
#define FAILURE (-1)

/* error info */
#define FULL (-2)
#define DUPLICATE (-3)
#define BUSY (-4)
#define IN_SERVICE (-5)
#define OUT_OF_SERVICE (-6)
#define NOT_FOUND (-7)
#define INSUFFICIENT (-8)
#define BAD_NAME (-9)
#define BAD_AMOUNT (-10)

/* account structure */
typedef struct bank_account {
  char name[ACCOUNT_NAME_MAX+1];
  double balance;
  int flag;
  pthread_mutex_t lock;
  UT_hash_handle hh;  /* makes this structure hashable */
} account_t;

/* bank structure */
typedef struct bank {
  account_t *accounts;
  int count;
  pthread_mutex_t mutex;
} bank_t;

bank_t *BANK;
extern char usage[100];

int init_bank();
int create(char*, account_t**);
int serve(char*, account_t**);
int deposit(char*, account_t**);
int withdraw(char*, account_t**);
double query(account_t**);
int end(account_t**);
int quit(account_t**);
int destroy_bank();
void print_bank_info();
char* get_error_info(int);

int __valid_name_(char*);
double __valid_amount_(char*);

#endif /* defined(__Bank_System__bank__) */
