/*
 *Author: Shihang Zhang
 */


/*
 * tokenizer.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


/*
 * print the name of punctuations and operators.
 */
int printName(char *head, char* tail, unsigned long length) {
    char *name = NULL;
    char *tmp = head;
    while (tmp != tail) {
        switch (*tmp) {
            case '.': name = "period"; break;
            case '(': name = "leftparenthesis";break;
            case ')': name = "rightparenthesis";break;
            case '[': name = "leftbracket";break;
            case ']': name = "rightbracket";break;
            case '{': name = "leftbrace";break;
            case '}': name = "rightbrace";break;
            case '*': name = "asterisk";break;
            case '&': name = "ampersand";break;
            case '-': name = "hyphen";break;
            case '!': name = "exclamation mark";break;
            case '~': name = "tilde";break;
            case '/': name = "slash";break;
            case '%': name = "mod";break;
            case '+': name = "plus";break;
            case '<': name = "less";break;
            case '>': name = "greater";break;
            case '^': name = "circumflex";break;
            case '|': name = "vertical bar";break;
            case ',': name = "comma";break;
            case '=': name = "equals";break;
            case ';': name = "semicolon";break;
            case '#': name = "crosshatch";break;
            case '@': name = "at";break;
            case '$': name = "dollar";break;
            case '_': name = "underscore";break;
            case '?': name = "question mark";break;
            case '\'': name = "apostrophe";break;
            case '\\': name = "backslash";break;
            default:
                break;
        }
        printf("%s", name);
        tmp++;
    }
    printf(" ");
    return 0;
}


/*
 * test if the character is blank (0x20), tab (0x09), new-line (0x0a) or carriage return
 * (0x0d)
 */
int iswhiteSpace(int c) {
    if (isspace(c) && c != '\v' && c != '\f')   return 1;
    else return 0;
    
}


/*
 * Tokenizer type
 */
typedef struct TokenizerT_ {
    char *head, *tail;
}TokenizerT;


/*
 * TKCreate creates a new TokenizerT object for a given token stream
 */
TokenizerT *TKCreate( char * ts ) {
    TokenizerT * t = malloc(sizeof(TokenizerT));
    if (t != NULL) {
        t->head = ts;
        t->tail = t->head;
        return t;
    }
    else
        return NULL;
}

/*
 * TKDestroy destroys a TokenizerT object.
 */
void TKDestroy( TokenizerT * tk ) {
    free(tk);
}


char *TKGetNextToken( TokenizerT * tk ) {
    char * ret;
    //find the head of a token
    while (iswhiteSpace(*tk->head) || (*tk->head != '\0' && iscntrl(*tk->head))) {
        // if there is a special character, print its hex value
        if (iscntrl(*tk->head))
            printf("ERROR: [0x%02x]\n",*tk->head);
        tk->head++;
    }
    tk->tail = tk->head+1;
    
    //find the tail of a token
    while (*tk->tail != '\0' && !iswhiteSpace(*tk->tail) && !iscntrl(*tk->tail)) {
        tk->tail++;
    }
    
    unsigned long length = strlen(tk->head)-strlen(tk->tail);
    // if the token is a word
    if (isalpha(*tk->head)) printf("word ");
    
    else if (*tk->head == '0') {
        // if the token is a hex constant
        if (*(tk->head+1) == 'x' || *(tk->head+1) == 'X')
            printf("hex constant ");
        //if the token is an octal constant
        else
            printf("octal constant ");
    }
    
    else if (isdigit(*tk->head)) {
        char tmp[length];
        strncpy(tmp, tk->head, length);
        // if the token is a float
        if (strchr(tmp, '.'))
            printf("float ");
        // if the token is an integer
        else    printf("integer ");
    }
    
    //if the token is a punctuation or an operator
    else if (ispunct(*tk->head))
        printName(tk->head, tk->tail, length);
    
    //it is not a token
    else
        return NULL;
    
    ret = malloc(sizeof(char)*(length+1));
    strncpy(ret, tk->head, length);
    ret[length] = '\0';
    tk->head = tk->tail;
    return ret;
}


int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Input format: ./tokenizer <token string>\n");
        exit(0);
    }
    TokenizerT *tokenizer;
    tokenizer = TKCreate(argv[1]);
    char *token = TKGetNextToken(tokenizer);
    while (token != NULL) {
        printf("\"%s\"\n", token);
        free(token);
        token = TKGetNextToken(tokenizer);
    }
    TKDestroy(tokenizer);
    return 0;
}

