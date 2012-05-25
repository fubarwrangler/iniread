#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "iniread.h"
#include "readline.h"


/* Strip leading whitespace, return 0 for comments or blank, 1 otherwise */
static int filter_line(char *raw, size_t len, int *removed)
{
	size_t l_white = 0;

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


/* If the line is a valid "[section]", modify *str by reterminating as
 * needed by stripping out any whitespace between the square brackets,
 * and return a pointer to the \0 terminated section name (in a now-
 * modified *str). If not return NULL and don't touch *str
 */
static char *get_section(char *str)
{
	char *p = str;
	size_t len = strlen(str);

	/* Must start w/ [, end w/ ], and have something between */
	if(*p == '[' && len > 2 && *(p + len - 1) == ']')	{
		size_t start, stop;
		p++;

		/* start is index(non-white), stop is index(last non-white) */
		start = strspn(p, " \t") + 1;
		stop = start + strcspn((p + start), "]");

		*(p + stop) = '\0';
		p += start - 1;

		/* If p is at ']' now, we didn't get any non-whitespace chars */
		return p;
	}
	return NULL;
}

/* Populate *key / *value with the copies of the right sections from the
 * line given in *str.  Return 0 on success, 1 otherwise.
 */
static int get_key_value(char *str, char **key, char **value)
{
	char *p = str;
	size_t k_len, v_len;

	*key = *value = NULL;

	/* First word (up till whitespace or seperator) is the key */
	k_len = strcspn(p, "\t =:");
	if(k_len < 1)
		return 1;

	p += k_len + strspn(p + k_len, "\t ");	/* Possible white after key */


	if(*p != '=' && *p != ':')	/* The separator better be next */
		return 1;

	p += 1 + strspn(p + 1, "\t ");	/* Skip whitespace after seperator */

	v_len = strlen(p);

	if(v_len < 1)	/* Better be something for the key */
		return 1;

	/* From start to k_len -> key, from p to end -> value */
	if((*key = malloc(k_len + 1)) != NULL)	{
		memmove(*key, str, k_len);
		*(*key + k_len) = '\0';
		if((*value = malloc(v_len + 1)) != NULL)	{
			memmove(*value, p, v_len);
			*(*value + v_len) = '\0';
		} else {
			free(*key);
			*key = NULL;
			return 1;
		}
	} else {
		return 1;
	}

	return 0;
}


/* Return a pointer into *str that contins just the value: from after
 * the first word and the first occurance of '=' or ':' till the end.
 */
static char *get_val_from_string(char *str, char *key)
{
	char *p = NULL;
	size_t klen = strlen(key);

	if(strncmp(key, str, klen) == 0)	{

		p = str + klen;
		p += strspn(p, " \t");

		/* Next had better be the seperator, and we advance p past subsequent
		 * whitespace that may be before the value.
		 */
		if(*p == '=' || *p == ':')	{
			p += strspn(p + 1, " \t") + 1;
			return p;
		}
	}
	return NULL;
}

char *ini_readline(FILE *fp, int *err)	{
	size_t len;
	char *buf;

	buf = readline_continue_fp(fp, &len);

	switch(readline_error())	{
		case READLINE_OK:
			return buf;
		case READLINE_MEM_ERR:
			*err = INI_NOMEM;
			break;
		case READLINE_IO_ERR:
		case READLINE_FILE_ERR:
			*err = INI_IOERROR;
			break;
		default:
			abort();
	}

	return NULL;
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
		while((line_buf = ini_readline(fp, e)) == NULL)	{
			/* Read a line (combining ones that end in back-slash) into
			 * heap storage -- MUST FREE
			 */
			p = line_buf;

			/* Did we find a [section]-defining line? */
			if((sec = get_section(p)) != NULL)	{
				/* If we already entered a section, we're now leaving it
				 * without finding a matching key, so we're done
				 */
				if(in_section != 0)
					break;

				if(strcmp(section, sec) == 0)	{
					/* section found, assume no key */
					*e = INI_NOKEY;
					in_section = 1;
				}
			}
			if(in_section != 0)	{
				if((p = get_val_from_string(p, key)) != NULL)	{
					/* found it */
					value = strdup(p);
					if(value == NULL)
						*e = INI_NOMEM;
					//else
						//puts(*value);
					*e = INI_OK;
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

int ini_read_file(char *fname, struct ini_file **inf)
{
	int err;
	FILE *fp;

	*inf = NULL;

	if((fp = fopen(fname, "r")) == NULL)	{
		perror("ini_read_file");
		return INI_NOFILE;
	}

	*inf = ini_read_stream(fp, &err);

	fclose(fp);
	return err;
}


/* Read from stdio stream *fp ini-file data into a newly created ini-file
 * structure.  The sections are held in a linked list off the main struct
 * and the key/value pairs are in linked lists off each section.
 */
struct ini_file *ini_read_stream(FILE *fp, int *err)
{
	struct ini_file *inidata = NULL;
	struct ini_section *sec = NULL, *sp = NULL;
	char *line = NULL;
	int len;

	*err = INI_NOMEM;

	if((inidata = malloc(sizeof(struct ini_file))) == NULL)
		return NULL;

	if((inidata->sections = hash_init(NULL, 10)) == NULL)
		return NULL;

	while((line = ini_readline(fp, err)) != NULL)	{
		char *p;

		if((p = get_section(line)) != NULL)	{
			if((sp = malloc(sizeof(struct ini_section))) != NULL)	{

				if((sp->items = hash_init(NULL, 10)) == NULL)	{
					free(line);
					ini_free_data(inidata);
					fputs("Error allocating memory", stderr);
					return NULL;
				}
				hash_set_autogrow(sp->items, 1.1, 1.8);
				hash_set_autofree(sp->items);

				if(hash_insert(inidata->sections, p, sp) != 0)	{
					free(line);
					fputs("Error allocating memory", stderr);
					return NULL;
				}
			} else {
				fputs("Error allocating memory", stderr);
				free(line);
				ini_free_data(inidata);
				return NULL;
			}
		} else {
			char *key, *val;
			if(get_key_value(line, &key, &val) == 0)	{
				int inserted = hash_insert(sp->items, key, val);
				free(key);
				if(inserted != 0)	{
					free(val);
					fputs("Error allocating memory", stderr);
					ini_free_data(inidata);
					return NULL;
				}
			}
		}
	}
	*err = INI_OK;
	return inidata;
}

/* Destroy all sections, keys, and values in *inf structure */
void ini_free_data(struct ini_file *inf)
{
	struct ini_section *s;
	char *name;
	hash_iter ctx;

	hash_iter_init(inf->sections, &ctx);

	while(hash_iterate(inf->sections, &ctx, &name, &s))	{
		hash_destroy(s->items);
		free(s);
	}

	hash_destroy(inf->sections);
	free(inf);
}

/* In a given ini_file, search *section for *key and return the value */
char *ini_get_value(struct ini_file *inf, char *section, char *key, int *err)
{
	struct ini_section *s;;

	*err = INI_NOSECTION;

	s = (struct ini_section *)hash_get(inf->sections, section);
	if(s != NULL)	{
		char *val = (char *)hash_get(s->items, key);
		*err = (val == NULL) ? INI_NOKEY : INI_OK;
		return val;
	}
	return NULL;
}

/* Return the section from the *ini named *name */
struct ini_section *ini_find_section(struct ini_file *inf, char* name)
{
	if(inf != NULL)
		return hash_get(inf->sections, name);
	else
		return NULL;
}

/* Search through a section for a value */
inline char *ini_get_section_value(struct ini_section *s, char *key)
{
	if(s != NULL)
		return hash_get(s->items, key);
	else
		return NULL;
}
