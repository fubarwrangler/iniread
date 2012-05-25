#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readline.h"

int _readl_init_buf = 80;
int _readl_shrink_thres = 90;
int _readl_skip_shrink = 4;
char _readl_error = READLINE_OK;
char _readl_strip = 0;
char _readl_comment_skip = 0;
char _readl_comment_char = '#';

char *_readl_err_map[] = {
	"readline: No error",
	"readline: File open error",
	"readline: File I/O error",
	"readline: Memory allocation error",
	"readline: BUG! Invalid error num",
};

// #define _READLINE_DEBUG

/* resize() -- realloc a string to a new size safely
 *	@buf, @new -- buffer to realloc / new size
 *
 * Sets error flag and frees buffer if an error occurs
 *
 * Returns: pointer to new resized buffer or NULL on error
 */
static char *resize(char *buf, size_t new)
{
	char *tmp = realloc(buf, new);
#ifdef _READLINE_DEBUG
	fprintf(stderr, "---Resize to %d\n", (int)new);
#endif
	if(tmp == NULL)	{
		_readl_error = READLINE_MEM_ERR;
		free(buf);
	}
	return tmp;
}

char *readline_fp(FILE *fp, size_t *slen)
{
	size_t len = 0;
	static char *buf = NULL;
	static int bufsize = 0;
	static int offset = 0;
	static int n_nogrow = 0;

	if(buf == NULL)	{
		buf = malloc(_readl_init_buf);
		if(buf == NULL)	{
			_readl_error = READLINE_MEM_ERR;
			return NULL;
		}
		bufsize = _readl_init_buf;
	}

	while(fgets(buf + offset, bufsize - offset - 1, fp) != NULL)	{
		len = strlen(buf);

		/* Grow if we need to */
		if(buf[len - 1] != '\n' && !feof(fp))	{
			bufsize *= 2;
			if((buf = resize(buf, bufsize)) == NULL)
				return NULL;
			offset = len;
			n_nogrow = 0;
			continue;
		} else {
			offset = 0;
			/* Shrink if we should and we fit */
			if( (bufsize / 2 > _readl_shrink_thres) &&
			    (n_nogrow > _readl_skip_shrink) &&
			    (len < bufsize / 2)
			  )	{

				bufsize /= 2;
				if((buf = resize(buf, bufsize)) == NULL)
					return NULL;
				n_nogrow = 0;
			}

			n_nogrow++;
			if(_readl_strip)	{
				buf[len - 1] = '\0';
				*slen = len - 1;
			} else {
				*slen = len;
			}
			return buf;
		}
	}
	if(!feof(fp))
		_readl_error = READLINE_IO_ERR;
	free(buf);
	return NULL;
}

static char *readline_skel_fopen(const char *fname, size_t *slen,
								 char *(read_fn)(FILE *, size_t *))
{
	static FILE *fp = NULL;
	char *storage;

	if(fp == NULL)	{
		if((fp = fopen(fname, "r")) == NULL)	{
			_readl_error = READLINE_FILE_ERR;
			return NULL;
		}
	}
	storage = read_fn(fp, slen);
	if(storage == NULL)
		fclose(fp);
	return storage;
}


/* get_nend() -- count the occurrences of character @c at the end of @str
 *	@str / @c - string to search through / character to search for
 * Returns: count how many c's are at end of str
 */
static size_t get_nend(char *str, char c)
{
	size_t n = 0;

	while(*str)	{
		if(*str++ == c)
			n++;
		else
			n = 0;
	}
	return n;
}


char *readline_continue_fp(FILE *fp, size_t *slen)
{
	char *buf = NULL;
	static size_t buf_s = 0;
	static char *new_storage = NULL;
	size_t len = 0, old_len = 0;

	_readl_strip = 1;

	while((buf = readline_fp(fp, &len)) != NULL)	{
		int n_slash = get_nend(buf, '\\');

		if((new_storage = resize(new_storage, len + old_len + 1)) == NULL)
			return NULL;

		memmove(new_storage + old_len, buf, len);

		if(_readl_comment_skip != 0)	{
			if(strchr(buf, _readl_comment_char) != NULL)
				n_slash = 0;
		}

		if(n_slash == 0) { /* Zero */
			new_storage[len + old_len] = '\0';
			*slen = len + old_len;
			return new_storage;
		} else if (n_slash % 2 == 0) { /* Even */
			new_storage[len + old_len - (n_slash / 2)] = '\0';
			len -= n_slash / 2;
			*slen = len + old_len;
			return new_storage;
		} else { /* Odd */
			new_storage[len + old_len - ((n_slash + 1) / 2)] = '\0';
			len -= ((n_slash + 1) / 2) - 1;
		}
		old_len += len - 1;
		*slen = len + old_len;
	}
	free(new_storage);
	return NULL;
}


char *readline(const char *fname, size_t *slen)
{
	return readline_skel_fopen(fname, slen, readline_fp);
}

char *readline_continue(const char *fname, size_t *slen)
{
	return readline_skel_fopen(fname, slen, readline_continue_fp);
}
