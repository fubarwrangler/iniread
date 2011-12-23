#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "iniread.h"

/* Get number of contigous characters at the end of string all in @accept */
int get_nend(char *str, char *accept)
{
	int n = 0;
	char *p = str;

	while(*p)	{
		if(strchr(accept, *p++) != NULL)
			n++;
		else
			n = 0;
	}
	return n;
}

/* Strip leading whitespace, return 0 for comments or blank, 1 otherwise */
int filter_line(char *raw, int *removed)
{
	size_t l_white = 0;
	size_t len = strlen(raw);

	if(len > 1)	{
		/* How many whitespace chars start the string? */
		l_white = strspn(raw, "\t ");

		/* Move the non-whitespace part to the begenning of the string */
		if(l_white > 0)
			memmove(raw, (raw + l_white), len - l_white);
	}
	*removed = (int)l_white;
	/* Skip comments and blank lines */
	return (*raw == '#' || *raw == ';' || len < l_white + 2) ? 0 : 1;
}

/* Strip off trailing whitespace in *raw and re-terminate the string */
char *strip_line(char *raw)
{
	size_t len = strlen(raw);

	/* Find out about and strip off any trailing whitespace */
	if(len >= 2)	{
		size_t r_white;
		r_white = get_nend(raw, " \t\n");
		*(raw + len - r_white) = '\0';
	}
	return raw;
}


/* If the line is a valid "[section]", modify *str by reterminating as
 * needed by stripping out any whitespace between the square brackets,
 * and return a pointer to the \0 terminated section name (in a now-
 * modified *str). If not return NULL and don't touch *str
 */
char *is_section(char *str)
{
	char *p = str;

	/* Must start w/ [, end w/ ], and have something between */
	if(*p == '[' && strlen(str) > 2 && *(p + strlen(str) - 1) == ']')	{
		int start, stop;
		p++;

		/* start is index(non-white), stop is index(last non-white) */
		start = strspn(p, " \t") + 1;
		stop = start + strcspn((p + start), " \t]");

		*(p + stop) = '\0';
		p += start - 1;

		/* If p is at ']' now, we didn't get any non-whitespace chars */
		return *p == ']' ? NULL : p;
	}
	return NULL;
}


/* Return a pointer into *str that contins just the value: from after
 * the first word and the first occurance of '=' or ':' till the end.
 */
char *get_key_value(char *str, char *key)
{
	char *p = NULL;
	size_t klen = strlen(key);

	if(strncmp(key, str, klen) == 0)	{

		p = str + klen;
		p += strspn(p, " \t");

		/* Next had better be the seperator, and we advance p */
		if(*p != '=' && *p != ':')
			return NULL;
        p++;
		p += strspn(p, " \t");
		return p;
	}
	return NULL;
}


char *ini_readline(FILE *fp, int *err)
{
	char line_buf[INIREAD_LINEBUF];
	char *real_line = NULL;
	size_t buflen = 0;
	int n_endslash = 0, adj;

	assert(fp != NULL);

	/* Read in first line */
	if(fgets(line_buf, INIREAD_LINEBUF, fp) == NULL)
		return NULL;

	/* Check for trailing slash */
	buflen = strlen(line_buf);
	n_endslash = get_nend(line_buf, "\\\n") - 1;
	
	if((real_line = malloc(buflen + 1)) == NULL)	{
		fputs("Error: malloc() failed\n", stderr);
		*err = INI_NOMEM;
		return NULL;
	}
	
	/* Reterminate line if trailing backslashes are present */
	line_buf[buflen - (n_endslash / 2) - 1] = '\n';
	line_buf[buflen - (n_endslash / 2)] = '\0';

	memmove(real_line, line_buf, buflen - (n_endslash / 2) + 1);

	/* Strip leading whitespace, and check for blanks/comments */
	if(filter_line(real_line, &adj) == 0)	{
		free(real_line);
		return ini_readline(fp, err); 
	}


	/* If number of trailing slashes is odd, it is a line continuation */
	if(n_endslash > 0 && n_endslash % 2 != 0)	{
		size_t cumlen = buflen - (n_endslash / 2) - 2 - adj;
		
		/* Read subsequent lines into an expanding buffer stopping when we
		 * encounter a line ending in 0 or an even number of backslashes
		 */
		while(fgets(line_buf, INIREAD_LINEBUF, fp) != NULL)	{
			buflen = strlen(line_buf);
	
			/* Count trailing slashes */
			n_endslash = get_nend(line_buf, "\\\n") - 1;

			errno = 0;
			real_line = realloc(real_line, cumlen + buflen + 2);
			if(errno == ENOMEM)	{
				fputs("Error: realloc() failed\n", stderr);
				*err = INI_NOMEM;
				free(real_line);
				return NULL;
			}
			
			memmove(real_line + cumlen, line_buf, buflen);

			/* Don't count backslash and newline */
			cumlen += buflen - (n_endslash / 2) - 2;

			/* If we end with \\, treat it as escaped */
			if(n_endslash % 2 == 0)	{
				real_line[cumlen + 1] = '\0';
				break;
			}
		}
	}

	return strip_line(real_line);
}



/* Public function: takes a filename, a section, and a key, and searches
 * for the value associated with that key in that section of the .ini-
 * formatted file.  See header for more.
 */
char *ini_read_value(char *fname, char *section, char *key, int *e)
{
	FILE *fp = NULL;
	char *line_buf = NULL, *value = NULL;
	char *p, *sec;

	*e = INI_IOERROR;

	if((fp = fopen(fname, "r")) != NULL)	{
		int in_section = 0;

		/* file opened, assume no section */
		*e = INI_NOSECTION;
		while(1)	{
			/* Second+ time around we free line_buf from prev time */
			if(line_buf != NULL)
				free(line_buf);

			/* Read a line (combining ones that end in back-slash) into
			 * heap storage -- MUST FREE
			 */
			if((line_buf = ini_readline(fp, e)) == NULL)
				break;
			p = line_buf;


			/* Did we find a [section]-defining line? */
			if((sec = is_section(p)) != NULL)	{
				/* If we already entered a section, we're now leaving it
				 * without finding a matching key, so we're done
				 */
				if(in_section)
					break;

				if(strcmp(section, sec) == 0)	{
					/* section found, assume no key */
					*e = INI_NOKEY;
					in_section = 1;
				}
			}
			if(in_section)	{
				if((p = get_key_value(p, key)) != NULL)	{
					/* found it */
					value = strdup(p);
					if(value == NULL)
						*e = INI_NOMEM;
					//else
						//puts(*value);
					*e = INI_FOUND;
					break;
				}
			}
		}
		fclose(fp);
		if(line_buf != NULL)
			free(line_buf);
	} else	{
		fprintf(stderr, "Error: cannot read config file: %s\n", fname);
	}
	return value;
}

