#include <pthread.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include "bank.h"

int init_bank()
{
  if ((BANK = malloc(sizeof(bank_t))) == NULL) {
    perror("malloc");
    return FAILURE;
  }
  BANK->count = 0;
  BANK->accounts = NULL;
  pthread_mutex_init(&BANK->mutex, NULL);
  return SUCCESS;
}

int create(char *account_name, account_t **current)
{
  account_t *new;

  if (!__valid_name_(account_name)) {
    return BAD_NAME;
  }
  if (*current != NULL) {
    return IN_SERVICE;
  }
  pthread_mutex_lock(&BANK->mutex);
  if (BANK->count == ACCOUNT_MAX) {
    pthread_mutex_unlock(&BANK->mutex);
    return FULL;
  }
  HASH_FIND_STR(BANK->accounts, account_name, new);
  if (new != NULL) {  /* this account name alreadly exists */
    pthread_mutex_unlock(&BANK->mutex);
    return DUPLICATE;
  }
  if ((new = malloc(sizeof(account_t))) == NULL) {
    perror("malloc");
    exit(1);
  }
  bzero(new, sizeof(account_t));
  memcpy(new->name, account_name, strlen(account_name)+1);
  if (new->name[100] != '\0') {
    free(new);
    pthread_mutex_unlock(&BANK->mutex);
    return BAD_NAME;
  }
  new->balance = 0;
  new->flag = 0;
  pthread_mutex_init(&new->lock, NULL);
  HASH_ADD_STR(BANK->accounts, name, new);
  BANK->count++;
  pthread_mutex_unlock(&BANK->mutex);
  return SUCCESS;
}

int serve(char *account_name, account_t **current)
{
  if (*current != NULL) {
    return IN_SERVICE;
  }
  HASH_FIND_STR(BANK->accounts, account_name, *current);
  if (*current == NULL) {
    return NOT_FOUND;
  }
  if (pthread_mutex_trylock(&(*current)->lock) == 0) {
    (*current)->flag = 1;
    return SUCCESS;
  }
  else {
    *current = NULL;
    return BUSY;
  }
}

int deposit(char *amount, account_t **current)
{
  double a;
  
  if ((a = __valid_amount_(amount)) < 0) {
    return BAD_AMOUNT;
  }
  if (*current == NULL) {
    return OUT_OF_SERVICE;
  }
  (*current)->balance += a;
  return SUCCESS;
}

int withdraw(char *amount, account_t **current)
{
  double a;
  
  if ((a = __valid_amount_(amount)) < 0) {
    return BAD_AMOUNT;
  }
  if (*current == NULL) {
    return OUT_OF_SERVICE;
  }
  if (a > (*current)->balance) {
    return INSUFFICIENT;
  }
  (*current)->balance -= a;
  return SUCCESS;
}

double query(account_t **current)
{
  if (*current == NULL) {
    return OUT_OF_SERVICE;
  }
  return (*current)->balance;
}

int end(account_t **current)
{
  if (*current == NULL) {
    return OUT_OF_SERVICE;
  }
  (*current)->flag = 0;
  pthread_mutex_unlock(&(*current)->lock);
  (*current) = NULL;
  return SUCCESS;
}

int quit(account_t **current)
{
  if (*current != NULL) {
    end(current);
  }
  return SUCCESS;
}

void print_bank_info()
{
  account_t *itr;
  pthread_mutex_lock(&BANK->mutex);
  printf("Bank Info:\n");
  if (BANK->count == 0) {
    printf("No Account\n");
  }
  else {
    for(itr = BANK->accounts; itr != NULL; itr=itr->hh.next) {
      printf("%s\t\tbalance: %lf", itr->name, itr->balance);
      if (itr->flag) {
        printf("\t\tIN SERVICE\n");
      }
      else {
        printf("\n");
      }
    }
  }
  pthread_mutex_unlock(&BANK->mutex);
}

char* get_error_info(int error)
{
  char *ret;
  if ((ret = malloc(sizeof(char)*ERROR_INFO_SIZE)) == NULL) {
    fprintf(stderr, "server: failed to malloc\n");
    exit(1);
  }
  bzero(ret, ERROR_INFO_SIZE);
  switch (error) {
    case FULL: sprintf(ret, "this bank is full"); break;
    case DUPLICATE: sprintf(ret, "this account already exists"); break;
    case IN_SERVICE: sprintf(ret, "this request is invalid since this client is in service"); break;
    case OUT_OF_SERVICE: sprintf(ret, "serve this account first"); break;
    case NOT_FOUND: sprintf(ret, "no record for this account"); break;
    case INSUFFICIENT: sprintf(ret, "insufficient balance"); break;
    case BAD_NAME: sprintf(ret, "invalid account name"); break;
    case BAD_AMOUNT: sprintf(ret, "invalid amount"); break;
    default: break;
  }
  return ret;
}

int __valid_name_(char *name) {
  char *c;
  int ret;
  
  ret = 1;
  c = name;
  do {
    if (isalnum(*c) != 0) {
      c++;
    }
    else {
      ret = 0;
      break;
    }
  } while (*c != '\0');
  return ret;
}

double __valid_amount_(char *amount) {
  double ret;
  char *endPtr;
  
  if (*amount == '\0') {
    return -1;
  }
  ret = strtod(amount, &endPtr);
  if (*endPtr != '\0') {
    ret = -1;
  }
  return ret;
}


char usage[100] = "usage:\ncreate accountname\nserve accountname\ndeposit amount\nwithdraw amount\nquery\nend\nquit";
