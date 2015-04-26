#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "index.h"

int main(int argc, char **argv)
{
  /* if the calling is wrong, print the usage */
  if (argc == 3 || (argc == 4 && !strcmp(argv[3], "-n"))) {
    FLAG = (argc == 4 ? 0 : 1);
    createIndex(argv[2]);
    writeIndex(argv[1]);
  }
  else
    printf("Usage: ./index <inverted-index file name> <directory or file name> {<-n>}\n");
  return 0;
}
