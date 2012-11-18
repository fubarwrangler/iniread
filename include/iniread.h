/* iniread.h: functions to retrieve values from .ini-style config files
 *
 * INI File format:
 *		[section]
 *		key=value
 * e.g...
 *	|	# I am a comment, 'section1' below shouldn't have whitespace in name
 *	|	[ section1 ]
 *	|	name = value is after first '=' sign
 *	|	[s2]
 *	|	key = = :=
 *	|	# the value for 'key' above is '= :='
 *	|	   key2	 =		  value
 *	|	# above, key2 = 'value', all whitespace before & after stripped
 *
 *
 * This also does backslash-interpolation.  If a key/value line ends with a '\'
 * the next line is joined to the current one and the '\' and newline are
 * removed.  Backslashes can be escaped with another backslash, so as to not
 * continue the line.  The rule is: even # of '\''s => n/2 real backslashes,
 * odd number of backslashes, (n - 1) / 2 backslashes with continuation.
 *
 * TODO: handle duplicate sections -- currently returns first in order of read
 *       1. Check each one -- currently O(n^2) (sum_1..n ~ n^2)
 *       2. Store sections in hash-table to avoid duplicates cheaply
 */

#ifndef INIREAD_H__
#define INIREAD_H__

#define INI_OK			0
#define INI_NOSECTION	1
#define INI_NOKEY		2
#define INI_NOFILE		3
#define INI_IOERROR		4
#define INI_NOMEM		5
#define INI_NOTBOOL		6
#define INI_NOTINT		7
#define INI_NOTFLOAT	9
#define INI_PARSEERROR	10

extern char *ini_errors[];

#define INIREAD_LINEBUF	2048

#define ini_error_string(code) \
				(code <= 9) ? ini_errors[code] : ini_errors[10]

struct ini_file {
	struct ini_section *first;
	int n_sec;
};

struct ini_section	{
	struct ini_section *next;
	struct ini_kv_pair *items;
	char *name;
};

struct ini_kv_pair {
	struct ini_kv_pair *next;
	char *key;
	char *value;
};

#include <stdbool.h>


/* ini_read_file() -- read an ini-formatted config file into an ini-file-structure
 * 	@fname:	filename to open and read
 * 	@inf:	Pointer to pointer to ini_file structure to store result in
 *
 * Returns: INI_OK (0) on success -- also *inf is non-null
 */
int ini_read_file(const char *fname, struct ini_file **inf);

/* ini_read_stream() -- read an ini-formatted config file into an ini-file-structure
 * 	@fp:	the opened file stream to read from
 * 	@err:	error indicator, 0 == INI_OK indicates success
 *
 * Returns: ini-structure ready to read from, NULL on error (@err non-zero too)
 */
struct ini_file *ini_read_stream(FILE *fp, int *err);

/* free_inifile() -- destroy an ini-file, freeing all associated memory
 * 	@inf:	the ini-file-structure to destroy
 *
 * Always succeeds if passed a valid argument
 * WARNING -- any values returned by the get_ini_value/get_ini_section
 * 	are pointers within this structure and will be invalid if the parent
 * 	is freed.
 */
void ini_free_data(struct ini_file *inf);

/* ini_get_value() -- parse an ini-file for the value of *key under *section
 * 	@inf:		ini-file-structure to look through
 * 	@section:	the section name to search for
 * 	@key:		the key whose value you want to find
 * 	@err:		integer error indicator -- 0 == INI_FOUND on success
 *
 * Returns: pointer to the value in @inf if found, else NULL and *err is set
 */
char *ini_get_value(struct ini_file* inf, const char *section, const char *key, int *err);

/* ini_get_bool() -- like ini_get_value but interpret strings as boolean
 *	@args same as ini_get_value
 *
 * The following are interpreted: yes/no, true/false, 1/0, on/off
 *
 * Returns: true or false, sets *err to INI_NOTBOOL if the string can't be
 * can't be interpreted as a boolean value
 */
bool  ini_get_bool(struct ini_file *inf, const char *section, const char *key, int *err);
int   ini_get_int(struct ini_file *inf, const char *section, const char *key, int *err);
float ini_get_float(struct ini_file *inf, const char *section, const char *key, int *err);

/* get_ini_section() -- get pointer to section element named *name
 * 	@inf:	ini-file object to read from
 * 	@name:	name of section to search for
 *
 * Returns: pointer to section if found, else NULL
 */
struct ini_section *ini_find_section(struct ini_file *inf, const char *name);

/* get_section_value() -- look through a given section for the named key
 * 	@s:		section to look through (probably returned by get_ini_section())
 * 	@key:	key to search for
 *
 * Returns: value corresponding to @key, or NULL on failure
 */
char *ini_get_section_value(struct ini_section *s, const char *key);

/* ini_get_section_<type>() -- like ini_find_section() but converts the type
 * See: ini_get_bool() and ini_find_section()
 */
bool  ini_get_section_bool(struct ini_section *s, const char *key, int *err);
int   ini_get_section_int(struct ini_section *s, const char *key, int *err);
float ini_get_section_float(struct ini_section *s, const char *key, int *err);

/* ini_read_value() -- read a value given a key and section of an .ini file,
 * 		skip creating the ini-file object and whatnot.
 *	@value:		storage that will be allocated for and hold the value
 *	@fname:		filename to try to open
 *	@section:	a section heading to look for a match in
 *	@key:		the key whose value we'll fetch under @section
 *
 *
 * Returns: error indicator, set to INI_FOUND if success (also value != NULL)
 *
 * Returns the appropriate code in the following cases:
 *	INI_FOUND		Everything went OK and a value was stored
 * 	INI_NOSECTION	Section cannot be found
 * 	INI_NOKEY		Section found but key isn't in it.
 * 	INI_IOERROR		Error opening / reading from *fname
 *	INI_NOMEM		Memory for the found buffer not found
 */
char *ini_read_value(char *fname, char *section, char *key, int *e);

#endif
