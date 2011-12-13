#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "iniread.h"

/* Take a line from a config file, strip leading/trailing whitespace, skip
 * comments, and return the modified line or NULL for blank/comment lines.
 */
char *prepare_line(char *raw)
{
	size_t l_white = 0, r_white = 0;
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
	if(l_white + r_white > 0)
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

char *is_value(char *str)
{
	return NULL;
}

/* Return a pointer into *str that contins just the value: from after
 * the first word and the first occurance of '=' or ':' till the end.
 */
char *get_value(char *str, char *key)
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
	size_t buflen = 0, real_len = 0;
	char *real_line = NULL;
	char eol, linecont = 0;


	assert(fp != NULL);

	while(fgets(line_buf, INIREAD_LINEBUF, fp) != NULL)	{
		buflen = strlen(line_buf);


		if(*(line_buf + buflen - 1) != '\n')	{
			fputs("Error: Line too long for iniread found\n", stderr);
			*err = INI_IOERROR;
			break;
		}
		eol = (buflen < 2) ? 0 : *(line_buf + buflen - 2);

		if(eol == '\\' || linecont)	{
			linecont = 1;
			line_buf[buflen - 2] = '\n';
			line_buf[buflen - 1] = '\0';
			if(buflen >= 3)	{
				if(eol == '\\' && line_buf[buflen - 3] == '\\')	{
					goto single_line;
				}
			}
			real_line = realloc(real_line, buflen + real_len);
			if(real_line == NULL)	{
				fputs("Error: realloc() failed\n", stderr);
				*err = INI_NOMEM;
				break;
			}
			strncat(real_line, line_buf, buflen);
			if(eol != '\\')
				break;
		} else {
			single_line:
			if((real_line = malloc(buflen + 1)) == NULL)	{
				fputs("Error: malloc() failed\n", stderr);
				*err = INI_NOMEM;
			} else {
				memmove(real_line, line_buf, buflen + 1);
			}
			break;
		}
		real_len = linecont ? real_len + buflen : buflen;

	}
	return real_line;
}

/* Public function: takes a filename, a section, and a key, and searches
 * for the value associated with that key in that section of the .ini-
 * formatted file.  See header for more.
 */
char *ini_read_value(char *fname, char *section, char *key, int *e)
{
	FILE *fp = NULL;
	char *p, *sec;
	char *line_buf = NULL, *value = NULL;
	int err;

	*e = INI_IOERROR;

	if((fp = fopen(fname, "r")) != NULL)	{
		int in_section = 0;

		/* file opened, assume no section */
		*e = INI_NOSECTION;
		while(1)	{

			if(line_buf != NULL)
				free(line_buf);
			if((line_buf = ini_readline(fp, &err)) == NULL)
				break;

			if((p = prepare_line(line_buf)) == NULL)
				continue;

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
				if((p = get_value(p, key)) != NULL)	{
					/* found it */
					value = strdup(p);
					if(value == NULL)
						*e = INI_NOMEM;
					//else
						//puts(*value);
					err = INI_FOUND;
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
