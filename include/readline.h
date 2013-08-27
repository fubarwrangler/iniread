#ifndef _READLINE_H
#define _READLINE_H


extern int _readl_init_buf;
extern int _readl_shrink_thres;
extern int _readl_skip_shrink;
extern char _readl_error;
extern char _readl_strip;
extern char _readl_comment_skip;
extern char _readl_comment_char;
extern char *_readl_err_map[];

#define READLINE_OK         0
#define READLINE_FILE_ERR   1
#define READLINE_IO_ERR     2
#define READLINE_MEM_ERR    3

/* Sets the parameters described below in readline()'s comment */
#define readline_set_init_size(n)		_readl_init_buf = n
#define readline_set_shrink_thresh(n)	_readl_shrink_thres = n
#define readline_set_n_skip_shrink(n)	_readl_skip_shrink = n
#define readline_set_comment_char(n)	_readl_comment_char = n

/* Set character that starts comment and if we check for that:
 * Used by readline_continue to not continue if we are in a comment
 */
#define readline_set_comment()		_readl_comment_skip = 1;
#define readline_unset_comment()	_readl_comment_skip = 0;

/* Determine if and which errors occured */
#define readline_is_error()	(_readl_error != 0)
#define readline_error()	(_readl_error)
#define readline_errstr()	_readl_err_map[_readl_error]

/* Control if we strip off the trailing '\n' from each line */
#define readline_set_strip()	_readl_strip = 1
#define readline_unset_strip()	_readl_strip = 0


/** readline() -- read a file a line at a time, safe for very long lines, will
 *                only stop when it runs out of memory
 *
 *  @filename: the file to open and iterate through
 *  @slen: address to write the length of the current line to (it's computed
 *         by this function anyway so why waste it?)
 *
 * Function iterates over lines, expanding and contracting a heap-allocated
 * buffer according to a simple algorithm to store an entire line at once.
 * If strip is set (see above) the '\n' is removed and the size is reduced.
 * It works by doubling the buffer size upon reading a too-long line and
 * halving it after reading a configurable number of short-enough lines.
 * The function will keep reading until it hits a '\n', so be careful if you
 * are worried about memory. Uses repeated calls to fgets() under the hood.
 *
 * Returns: temporary buffer holding the current line, NULL on EOF or error
 *          On error, readline_error() returns true and readline_errstr()
 *          returns a pointer to an appropriate error message
 *
 * Globals: parameters listed above affect how this function manages memory
 *    init_size: how large is the initial buffer -- defaults to 81 bytes
 *    shrink_thresh: will never shrink below this size, will bottom out at
 *                   at most twice this -- defaults to 120
 *    n_skip_shrink: how many short-enough lines (lines half as long as the
 *                   current buffer size or smaller) in a row to read before
 *                   re-allocing a buffer half as large -- default is 4
 */
char *readline(const char *fname, size_t *slen);

/** readline_fp() -- same as readline but iterates over an already-opened file
 */
char *readline_fp(FILE *fp, size_t *slen);


/** readline_continue() -- read a line from a file but take a backslash at the
 *                         end as meaning to append the next line to the end
 *
 *	@args -- same as for readline()
 *
 * An odd number of trailing backslashes signals this function to read the
 * next line and append it to the end of the current one.  Multiple
 * backslashes are treated as escaped so an even number will be turned into
 * n / 2 real ones and not continue the line.  Odd numbers are turned into
 * (n - 1) / 2 real backslashes and do continue the line.
 *
 * Returns: pointer to statically allocated buffer holding finished line
 *
 */
char *readline_continue(const char *fname, size_t *slen);

char *readline_continue_fp(FILE *fp, size_t *slen);


#endif
