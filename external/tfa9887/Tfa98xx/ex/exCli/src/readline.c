#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "readline.h"

static void add_character(char** ppDest, char c, int* pn, int nMax)
{
	*pn = *pn + 1;
	if (*pn > nMax)
	{
		fprintf(stderr, "Line too long\n");
		abort();
	}
	else
	{
		**ppDest = c;
		*ppDest = *ppDest + 1;
	}
}

int readline(FILE* f, char* pDest, int nMax, int* linenumber)
{
	char	c;
	char*	p=pDest;
	int		done = 0;
	int		len = 0;
	int     chars_read;

	*linenumber = *linenumber + 1;
	*pDest = '\0';
	while ((!feof(f)) && (!done))
	{
		chars_read = (int) fread(&c, sizeof(char), 1, f);
		done = (c=='\n') || (chars_read==0);	/* UNIX style line termination */
		if (done)
		{
			*p = '\0';
		}
		else
		{
			if ((c >= ' ') && (c <= '~'))		/* printable character? */
			{
				add_character(&p, c, &len, nMax);
			}
			else if (c == '\t')					/* expand '\t' */
			{
				add_character(&p, ' ', &len, nMax);
				/* expand as alignment of TABSIZE */
				while ((len % TABSIZE) != 0)
				{
					add_character(&p, ' ', &len, nMax);
				}
			}
			/* else:                           ignore all other non-printable characters */
		}
	}
	if (feof(f))
	{
		*p = '\0';								/* in case that last line in file ends without line terminator */
	}
	return len;
}
