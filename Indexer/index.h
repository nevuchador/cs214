#ifndef __INDEX__H_
#define __INDEX__H_

#include "sorted-list.h"
#include <stdlib.h>

#define F 0   /* character never appears in text */
#define T 1   /* character appears in plain ASCII text */
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

typedef struct Record {
  char *filename;
  int count;
} record_t;

typedef struct Token {
  char *token;
  SortedListPtr records;
} token_t;

void createIndex(char*); /* this function initiates this program using the input file argument from main function.
                            it includes: malloc space for three global sorted-lists; call diffrent functions
                            according to the type of the filename.
                          */
void writeIndex(char*); /*  this function will first test if the file writing to exists, then ask user to decide
                            the next operations.
                         */
void writeFile(char *); /*  this function writes the inverted index to file following JSON format by iterate the
                            index list.
                         */
void readDir(char*, char*, int flag); /*  this function reads directory*/
void readFile(char*, char*, int flag);  /* this function reads file */
void readSymbolicLink(char*, char*);  /* this function reads symbolic link */
void readToken(char*, char*); /* this function will be called by readFile to tokenize the file */
int compareRecords(void*, void*); /* this function compares records by count */
int compareTokens(void*, void*);  /* this function compares rokens by ascii order */
int compareFilenames(void*, void*); /* this function compares filenames, which will be used to retrive the 
                                     record by being used as a parameter to SLFind function
                                     */
int compareStrings(void *p1, void *p2); /* this functin compares strings*/
void destroyRecord(void*);  /* this fucntion destroy record by free the malloced space */
void destroyToken(void*); /* this fucntion destroy token by free the malloced space */
void destroyString(void*);  /* this fucntion destroy string by free the malloced space */

int FLAG;
SortedListPtr Index;
SortedListPtr RelPaths;
SortedListPtr AbsPaths;

/* internal variable and functions */
char text[256];
int __is_ascii_file_(char*, size_t);  /* this fucntion test a file if it is a ascii file */
char* __construct_path_name_(char*, char*, int);  /* this fucntion malloce space for child file path and test its 
                                                   length if exceeds the maximum
                                                   */
char* __copy_file_name_(char*); /* this fucntion malloce space for file path and test its
                                 length if exceeds the maximum
                                 */

#endif
