#include "index.h"
#include "tokenizer.h"
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

void createIndex(char* rootFile)
{
  Index = SLCreate(compareTokens, destroyToken);
  RelPaths = SLCreate(compareStrings, destroyString);
  AbsPaths = SLCreate(compareStrings, destroyString);
  struct stat s;
  char *absPath = __copy_file_name_(rootFile);
  char *relPath = __copy_file_name_(basename(rootFile));
  if (absPath == NULL || relPath == NULL) {
    fprintf(stderr, "errno %d\n", errno);
    fprintf(stderr, "mesg: %s\n", strerror(errno));
    exit(-1);
  }
  if (lstat(rootFile,&s) == 0) {
    if(S_ISDIR(s.st_mode))
      readDir(absPath, relPath, 0);
    else if(S_ISREG(s.st_mode))
      readFile(absPath, relPath, 0);
    else if(FLAG && S_ISLNK(s.st_mode)) {
      readSymbolicLink(absPath, relPath);
    }
    else
    {
      if (FLAG == 1)
        fprintf(stderr, "%s cannot be read.\n", absPath);
      exit(-1);
    }
  }
  else {
    fprintf(stderr, "%s\n", relPath);
    fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
    exit(-1);
  }
}

void writeIndex(char* rootFile)
{
  if (!SLEmpty(Index)) {
    if (access(rootFile, 0) == 0) {
      printf("The inverted-index file alrealy exists.\n");
      printf("Rewrite it?(y/n)\n");
      char choice;
      scanf("%c", &choice);
      if (choice == 'y') {
        writeFile(rootFile);
      }
      else {
        exit(0);
      }
    }
    else {
      writeFile(rootFile);
    }
  }
  else {
    printf("No files to read or no tokens in files.\n");
  }
  SLDestroy(Index);
  SLDestroy(RelPaths);
  SLDestroy(AbsPaths);
}

void writeFile(char *filename)
{
  FILE *file;
  if ((file = fopen(filename, "w+")) != NULL) {
    fprintf(file, "{\"list\" : [\n");
    SortedListIteratorPtr itr1 = SLCreateIterator(Index);
    token_t *tmp = SLGetItem(itr1);
    while (tmp != NULL) {
      fprintf(file, "\t  {\"%s\" : [ \n", tmp->token);
      SortedListIteratorPtr itr2 = SLCreateIterator(tmp->records);
      record_t *tmp2 = SLGetItem(itr2);
      while (tmp2 != NULL) {
        fprintf(file, "\t\t  {\"%s\" : %d}", tmp2->filename, tmp2->count);
        if (!SLIsLastItem(itr2))
          fprintf(file, ",");
        fprintf(file, "\n");
        tmp2 = SLNextItem(itr2);
      }
      fprintf(file, "\t  ]}");
      if (!SLIsLastItem(itr1))
        fprintf(file, ",");
      fprintf(file, "\n");
      SLDestroyIterator(itr2);
      tmp = SLNextItem(itr1);
    }
    fprintf(file, "]}\n");
    SLDestroyIterator(itr1);
    fclose(file);
  }
  else {
    fprintf(stderr, "%s\n", filename);
    fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
  }
}

void readDir(char* absPath, char* relPath, int flag)
{
  DIR *dir;
  struct dirent *ent;
  if ((dir=opendir(absPath)) != NULL) {
    while((ent=readdir(dir)) != NULL) {
      if (ent->d_name[0] == '.')
        continue;
      char *child_absPath = __construct_path_name_(absPath, ent->d_name, 0);
      char *child_relPath = __construct_path_name_(relPath, ent->d_name, 0);
      if (child_absPath == NULL || child_relPath == NULL)
        continue;
      struct stat s;
      if (lstat(child_absPath,&s) == 0) {
        if(S_ISDIR(s.st_mode))
          readDir(child_absPath, child_relPath, flag);
        else if(S_ISREG(s.st_mode))
          readFile(child_absPath, child_relPath, flag);
        else if(FLAG && S_ISLNK(s.st_mode)) {
          readSymbolicLink(child_absPath, child_relPath);
        }
        else {
          if (FLAG == 1)
            fprintf(stderr, "%s cannot be read.\n", child_relPath);
          free(child_absPath);
          free(child_relPath);
        }
      }
      else {
        fprintf(stderr, "%s\n", child_relPath);
        fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
      }
    }
    closedir(dir);
  }
  else {
    fprintf(stderr, "%s\n", relPath);
    fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
  }
  free(absPath);
  free(relPath);
}

void readFile(char* absPath, char* relPath, int flag)
{
  if (SLFind(AbsPaths, absPath, compareStrings) != NULL) {
    fprintf(stderr, "%s has been read.\n", relPath);
    fprintf(stderr, "check your files before start again.\n");
    exit(-1);
  }
  FILE *file;
  TokenizerT *tokenizer;
  char *token, *buffer, *symPath;
  if (flag) {
    symPath = __construct_path_name_(relPath, absPath, flag);
    free(relPath);
    relPath = symPath;
  }
  if ((file=fopen(absPath, "r+")) != NULL) {
    fseek(file, 0, SEEK_END);
    size_t buf_size;
    buf_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = malloc((buf_size+1)*sizeof(char));
    if (buffer != NULL) {
      fread(buffer, buf_size, sizeof(char), file);
      buffer[buf_size] = '\0';
      if (__is_ascii_file_(buffer, buf_size)) {
        tokenizer = TKCreate(buffer);
        while ((token = TKGetNextToken(tokenizer))) {
          readToken(token, relPath);
        }
        SLInsert(RelPaths, relPath);
        SLInsert(AbsPaths, absPath);
        TKDestroy(tokenizer);
      }
      else {
        fprintf(stderr, "%s is not a ascii file.\n", relPath);
        free(absPath);
        free(relPath);
      }
      free(buffer);
    }
    else {
      fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
      free(absPath);
      free(relPath);
    }
    fclose(file);
  }
  else {
    fprintf(stderr, "%s\n", relPath);
    fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
    free(absPath);
    free(relPath);
  }
}
void readSymbolicLink(char *absPath, char *relPath)
{
  printf("%s is a symbolic link.\n", absPath);
  char *truPath;
  struct stat s;
  if ((truPath = realpath(absPath, NULL)) != NULL && (lstat(truPath,&s) == 0)) {
    if(S_ISDIR(s.st_mode)) {
      readDir(truPath, relPath, 1);
      free(absPath);
    }
    else if(S_ISREG(s.st_mode)) {
      readFile(truPath, relPath, 1);
      free(absPath);
    }
    else {
      fprintf(stderr, "%s cannot be read.\n", truPath);
      free(absPath);
      free(truPath);
      free(relPath);
    }
  }
  else {
    fprintf(stderr, "the file pointed by %s\n", relPath);
    fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
    free(absPath);
    free(truPath);
    free(relPath);
  }
}

void readToken(char* token, char* filename)
{
  token_t *newToken = malloc(sizeof(token_t));
  record_t *newRecord = malloc(sizeof(record_t));
  token_t *foundToken;
  record_t *foundRecord;
  if (newRecord == NULL || newToken == NULL) {
    fprintf(stderr, "errno %d\n", errno);
    fprintf(stderr, "mesg: %s\n", strerror(errno));
    exit(-1);
  }
  newToken->token = token;
  newToken->records = NULL;
  newRecord->filename = filename;
  newRecord->count = 1;
  if ((foundToken = SLFind(Index, newToken, compareFilenames)) != NULL) {
    if ((foundRecord = SLFind(foundToken->records, newRecord, compareFilenames)) != NULL) {
      foundRecord->count++;
      destroyRecord(newRecord);
    }
    else {
      SLInsert(foundToken->records, newRecord);
    }
    destroyToken(newToken);
  }
  else {
    newToken->records = SLCreate(compareRecords, destroyRecord);
    SLInsert(newToken->records, newRecord);
    SLInsert(Index, newToken);
  }
}

int compareRecords(void *p1, void *p2)
{
  record_t * rec1 = p1;
  record_t * rec2 = p2;
  return (rec1->count-rec2->count>0) ? 1 : ((rec1->count-rec2->count==0) ? 0 : -1);
}

int compareTokens(void *p1, void *p2)
{
  token_t * tok1 = p1;
  token_t * tok2 = p2;
  return (-strcmp(tok1->token, tok2->token));
}

int compareFilenames(void *p1, void *p2)
{
  record_t * rec1 = p1;
  record_t * rec2 = p2;
  return strcmp(rec1->filename, rec2->filename);
}

int compareStrings(void *p1, void *p2)
{
  char *s1 = p1;
  char *s2 = p2;
  return strcmp(s1, s2);
}

void destroyRecord(void *p)
{
  if (p != NULL) {
    record_t *rec = p;
    free(rec);
  }
}

void destroyToken(void *p)
{
  token_t *tok = p;
  free(tok->token);
  SLDestroy(tok->records);
  free(tok);
}

void destroyString(void *p)
{
  char *s = p;
  free(s);
}


/* implementation of internal variable and functions */

char text[256] = {
  /*                  BEL BS HT LF    FF CR    */
  F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  /* 0x0X */
  /*                              ESC          */
  F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
  T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
  T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */
  T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x4X */
  T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x5X */
  T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x6X */
  T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  /* 0x7X */
  /*            NEL                            */
  X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  /* 0x8X */
  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  /* 0x9X */
  I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xaX */
  I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xbX */
  I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xcX */
  I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xdX */
  I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xeX */
  I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   /* 0xfX */
};

int __is_ascii_file_(char* buffer, size_t buf_size)
{
  int i;
  for (i = 0; i < buf_size; i++) {
    int t = text[(int)buffer[i]];
    if (t != T) {
      return 0;
    }
  }
  return 1;
}

char* __construct_path_name_(char* first, char* second, int flag)
{
  char *ret;
  char s[5];
  flag ? sprintf(s," -> ") : sprintf(s,"/");
  size_t length = strlen(first) + strlen(s) + strlen(second) + 1;
  if (length > PATH_MAX) {
    fprintf(stderr, "the length of path of this file %s exceeds the maximum.\n", second);
    exit(-1);
  }
  if ((ret = malloc(length*sizeof(char))) == NULL) {
    fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
    return NULL;
  }
  else {
    sprintf(ret,"%s%s%s",first, s, second);
    return ret;
  }
}

char* __copy_file_name_(char* filename)
{
  char *ret;
  size_t length = strlen(filename) + 1;
  if (length > PATH_MAX) {
    fprintf(stderr, "the length of path of this file %s exceeds the maximum.\n", filename);
    exit(-1);
  }
  if ((ret = malloc(length*sizeof(char))) == NULL) {
    fprintf(stderr, "errno %d: %s\n",errno, strerror(errno));
    return NULL;
  }
  else {
    sprintf(ret,"%s",filename);
    return ret;
  }
}
