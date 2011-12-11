#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iniread.h"

/* Take a line from a config file, strip leading/trailing whitespace, skip
 * comments, and return the modified line or NULL for blank/comment lines.
 */
char *prepare_line(char *raw)
{
	int l_white = 0, r_white = 0;
	size_t len;
	char *p;

	/* How many whitespace chars start the string? */
	l_white = strspn(raw, "\t ");
	len = strlen(raw);

	/* Find out about and strip off any trailing whitespace */
	if(len >= 2 && l_white < len - 1)	{
		p = raw + len - 2;
		while(p != raw)	{
			if(*p == '\t' || *p == ' ')
				r_white++;
			else
				break;
			p--;
		}
		*(raw + len - (1 + r_white)) = '\0';
	}

	/* Move the non-whitespace part to the begenning of the string */
	if(l_white || r_white)
		memmove(raw, (raw + l_white), len - (l_white + r_white));

	/* Skip comments and blank lines */
	return (*raw == '#' || *raw == ';' || len < l_white + 2) ? NULL : raw;
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
char *get_value(char *str, char *key)
{
	char *p = NULL;
	size_t klen = strlen(key);

	if(!strncmp(key, str, klen))	{

		p = str + klen;
		p += strspn(p, " \t");

		/* Next had better be the seperator, and we advance p */
		if(*p != '=' && *p++ != ':')
			return NULL;

		p += strspn(p, " \t");
		return p;
	}
	return NULL;
}


/* Public function: takes a filename, a section, and a key, and searches
 * for the value associated with that key in that section of the .ini-
 * formatted file.  See header for more.
 */
char *ini_read_value(char *fname, char *section, char *key, int *err)
{
	FILE *fp = NULL;
	char *p, *sec, *val = NULL;

	/* Assume the worst */
	*err = INI_IOERROR;

	if((fp = fopen(fname, "r")) != NULL)	{
		int in_section = 0;
		char line_buf[1024];

		/* file opened, assume no section */
		*err = INI_NOSECTION;
		while(fgets(line_buf, 1024, fp) != NULL)	{
			if(*(line_buf + strlen(line_buf) - 1) != '\n')	{
				fprintf(stderr, "Warning: Extremely long line found in %s\n", fname);
				continue;
			}
			if((p = prepare_line(line_buf)) == NULL)
				continue;

			/* Did we find a [section]-defining line? */
			if((sec = is_section(p)) != NULL)	{
				/* If we already entered a section, we're now leaving it
				 * without finding a matching key, so we're done
				 */
				if(in_section)
					break;

				if(!strcmp(section, sec))	{
					/* section found, assume no key */
					*err = INI_NOKEY;
					in_section = 1;
				}
			}
			if(in_section)	{
				if((p = get_value(p, key)) != NULL)	{
					/* found it */
					*err = INI_FOUND;
					val = strdup(p);
					break;
				}
			}
		}
		fclose(fp);
	} else	{
		fprintf(stderr, "Error: cannot read config file: %s\n", fname);
	}
	return val;
}