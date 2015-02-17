#ifndef	READLINE_H
#define READLINE_H


#include <stdio.h>


/*
f must be opened in binary mode
return value = number of characters read, excluding \n and \r (stripped), including TAB expansion into spaces
*/

#define TABSIZE	4


extern int readline(FILE* f, char* pDest, int nMax, int* linenumber);


#endif
