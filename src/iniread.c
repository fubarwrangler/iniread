#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "iniread.h"
#include "readline.h"

char *ini_errors[] = {	"Everything OK",
						"Section not found",
						"Key not found in section",
						"Unable to open file",
						"I/O error occured",
						"Error allocating memory",
						"Interpolation parse error",
						"BUG: invalid error code"
					 };


/* Strip leading whitespace + newline, return false for comments or blank */
static int filter_line(char *raw)
{
	size_t l_white = 0;
	size_t len = strlen(raw);

    if(len > 1) {
		/* How many whitespace chars start the string? */
		l_white = strspn(raw, "\t ");

        /* Move the non-whitespace part to the begenning of the string */
		if(l_white > 0)
			memmove(raw, (raw + l_white), len - l_white);

		/* Strip off newline */
		raw[len - l_white] = '\0';
    }
	/* Skip comments and blank lines */
	return !(*raw == '#' || *raw == ';' || len < l_white + 2);
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
		start = strspn(p, " \t");
		stop = start + strcspn((p + start), "]") - 1;
        while(*(p + stop) == ' ' || *(p + stop) == '\t')
            stop--;

		*(p + stop + 1) = '\0';
		p += start;

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

static char *ini_readline(FILE *fp, int *err)	{
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

int ini_read_file(char *fname, struct ini_file **inf)
{
	FILE *fp = NULL;
	int err;

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
	hash_set_autogrow(inidata->sections, 0.8, 1.6);

	while((line = ini_readline(fp, err)) != NULL)	{
		char *p;

		if(filter_line(line) == 0)
			continue;

		if((p = get_section(line)) != NULL)	{
			if((sp = malloc(sizeof(struct ini_section))) != NULL)	{

				if((sp->items = hash_init(NULL, 10)) == NULL)	{
					free(line);
					ini_free_data(inidata);
					fputs("Error allocating memory", stderr);
					return NULL;
				}
				hash_set_autogrow(sp->items, 0.8, 1.6);
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
		} else if (sp != NULL) {
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
	struct ini_section *s;

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
char *ini_get_section_value(struct ini_section *s, char *key)
{
	if(s != NULL)
		return hash_get(s->items, key);
	else
		return NULL;
}
