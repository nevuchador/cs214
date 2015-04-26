#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <semaphore.h>
#include "bank.h"

#define PORT "50875"
#define BACKLOG (20)
#define MESSAGE_SIZE (1024)

static sem_t printCycleSemaphore;
static pthread_attr_t attr;

void periodic_print_handler(int signo, siginfo_t *info, void *ucontext)
{
  if (signo == SIGALRM) {
    sem_post(&printCycleSemaphore);
  }
}

void* periodic_print_cycle_thread( void * arg)
{
  struct sigaction	sa;
  struct itimerval	itimer;
  
  pthread_detach(pthread_self());
  sa.sa_flags = SA_SIGINFO | SA_RESTART;
  sa.sa_sigaction = periodic_print_handler;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGALRM, &sa, NULL) == -1) {
    perror("server: failed to sigaction");
    exit(1);
  }
  itimer.it_value.tv_sec = 20;
  itimer.it_value.tv_usec = 0;
  itimer.it_interval.tv_sec = 20;
  itimer.it_interval.tv_usec = 0;
  if(setitimer(ITIMER_REAL, &itimer, NULL) < 0) {
    perror("server: failed to set timer");
    exit(1);
  }
  while (1) {
    sem_wait( &printCycleSemaphore );
    print_bank_info();
    sched_yield();
  }
  return NULL;
}

void* client_service_thread(void *fd)
{
  int connection_fd;
  char request[MESSAGE_SIZE];
  char response[MESSAGE_SIZE];
  int status;
  char *error_info;
  account_t *current_account;
  
  connection_fd = *((int *)fd);
  free(fd);
  current_account = NULL;
  pthread_detach(pthread_self());
  printf("server: connected to a client\n");
  while (recv(connection_fd, request, MESSAGE_SIZE, 0) > 0) {
    if (strncmp(request, "create ", 7) == 0) {
      if ((status = create(&request[7], &current_account)) < 0) {
        error_info = get_error_info(status);
        send(connection_fd, error_info, strlen(error_info)+1, 0);
        free(error_info);
      }
      else {
        sprintf(response, "create: success");
        send(connection_fd, response, strlen(response)+1, 0);
      }
    }
    else if (strncmp(request, "serve ", 6) == 0) {
      while ((status = serve(&request[6], &current_account)) == BUSY) {
        sprintf(response, "serve: waiting...");
        send(connection_fd, response, strlen(response)+1, 0);
        sleep(2);
      }
      if (status != SUCCESS) {
        error_info = get_error_info(status);
        send(connection_fd, error_info, strlen(error_info)+1, 0);
        free(error_info);
      }
      else {
        sprintf(response, "serve: success");
        send(connection_fd, response, strlen(response)+1, 0);
      }
   }
    else if (strncmp(request, "deposit ", 8) == 0) {
      if ((status = deposit(&request[8],&current_account)) < 0) {
        error_info = get_error_info(status);
        send(connection_fd, error_info, strlen(error_info)+1, 0);
        free(error_info);
      }
      else {
        sprintf(response, "deposit: success");
        send(connection_fd, response, strlen(response)+1, 0);
      }
    }
    else if (strncmp(request, "withdraw ", 9) == 0) {
      if ((status = withdraw(&request[9],&current_account)) < 0) {
        error_info = get_error_info(status);
        send(connection_fd, error_info, strlen(error_info)+1, 0);
        free(error_info);
      }
      else {
        sprintf(response, "withdraw: success");
        send(connection_fd, response, strlen(response)+1, 0);
      }
    }
    else if (strcmp(request, "query") == 0) {
      double amount;
      if ((amount = query(&current_account)) < 0) {
        status = amount;
        error_info = get_error_info(status);
        send(connection_fd, error_info, strlen(error_info)+1, 0);
        free(error_info);
      }
      else {
        sprintf(response, "query: success\nbalance: %lf", amount);
        send(connection_fd, response, strlen(response)+1, 0);
      }
    }
    else if (strcmp(request, "end") == 0) {
      if ((status = end(&current_account)) < 0) {
        error_info = get_error_info(status);
        send(connection_fd, error_info, strlen(error_info)+1, 0);
        free(error_info);
      }
      else {
        sprintf(response, "end: success");
        send(connection_fd, response, strlen(response)+1, 0);
      }
    }
    else if (strcmp(request, "quit") == 0) {
      quit(&current_account);
      sprintf(response, "quit");
      send(connection_fd, response, strlen(response)+1, 0);
      printf("server: disconnected to a client\n");
      close(connection_fd);
      pthread_exit(NULL);
    }
    else if (strcmp(request, "help") == 0) {
      sprintf(response, "%s", usage);
      send(connection_fd, response, strlen(response)+1, 0);
    }
    else {
      sprintf(response, "invalid command, enter again('help' for usage)");
      send(connection_fd, response, strlen(response)+1, 0);
    }
    bzero(request, MESSAGE_SIZE);
    bzero(response, MESSAGE_SIZE);
  }
  close(connection_fd);
  return NULL;
}


void* session_acceptor_thread(void *arg)
{
  int listen_fd;
  int connection_fd;
  int *connection_fd_ptr;
  int sockoptval = 1;
  struct addrinfo hints, *res, *p;
  int status;
  struct sockaddr_storage client_addr;
  socklen_t addr_len;
  pthread_t thread;
  
  /* get socket address structure */
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  hints.ai_addrlen = 0;
  hints.ai_addr = NULL;
  hints.ai_canonname = NULL;
  hints.ai_next = NULL;
  if ((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit(1);
  }
  
  for (p = res; p != NULL; p = p->ai_next) {
    if ((listen_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int)) == -1) {
      freeaddrinfo(res);
      exit(1);
    }
    if (bind(listen_fd, p->ai_addr, p->ai_addrlen) == -1) {
      close(listen_fd);
      continue;
    }
    break;
  }
  freeaddrinfo(res);
  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }
  if (listen(listen_fd, BACKLOG) == -1) {
    perror("server: failed to listen");
    exit(1);
  }
  
  addr_len = sizeof(client_addr);
  while (1) {
    if ((connection_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len)) == -1) {
      perror("server: failed to accept conenction");
      break;
    }
    connection_fd_ptr = (int *)malloc(sizeof(int));
    *connection_fd_ptr = connection_fd;
    if (pthread_create(&thread, &attr, client_service_thread, connection_fd_ptr) != 0) {
      perror("server: failed to create thread");
      return NULL;
    }
  }
  close(listen_fd);
  return NULL;
}

int main(int argc, char* argv[])
{
  pthread_t thread;
 
  if (argc != 1) {
    fprintf(stderr, "usage: ./server\n");
    exit(1);
  }
  if (init_bank() == FAILURE) {
    fprintf(stderr, "failed to initialize bank\n");
  }
  if (pthread_attr_init(&attr) != 0) {
    perror("server: failed to initiate thread attr");
    exit(1);
  }
  if (sem_init(&printCycleSemaphore, 0, 0) != 0) {
    perror("server: failed to initiate semaphore");
    exit(1);
  }
  if (pthread_create(&thread, &attr, session_acceptor_thread, 0) != 0) {
    perror("server: failed to create thread");
    exit(1);
  }
  if (pthread_create(&thread, &attr, periodic_print_cycle_thread, 0) != 0 )
  {
    perror("server: failed to create thread");
    exit(1);
  }
  printf("server: accepting connections\n");
  pthread_exit( 0 );
}
