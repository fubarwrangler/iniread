/* iniread.h: function to retrieve values from .ini-style config files
 *
 * INI File format:
 *		[section]
 *		key=value
 * e.g...
 *	|	# I am a comment, 'section1' below shouldn't have whitespace in name
 *	|	[ section1 ]
 *	|	name = value is afte first '=' sign
 *	|	[s2]
 *	|	key = = :=
 *	|	# the value for 'key' above is '= :='
 *	|	   key2	 =		  value
 *	|	# above, key2 = 'value', all whitespace before & after stripped
 */

#ifndef INIREAD_H__
#define INIREAD_H__

#define INI_OK			0
#define INI_NOSECTION	1
#define INI_NOKEY		2
#define INI_IOERROR		3
#define INI_NOMEM		4

#define INIREAD_LINEBUF	2048

struct ini_file {
	struct ini_section *first;
	int n_sec;
};

struct ini_section	{
	struct ini_section *next;
	struct kv_pair *items;
	char *name;
};

struct kv_pair {
	struct kv_pair *next;
	char *key;
	char *value;
};



/* read_ini() -- read an ini-formatted config file into an ini-file-structure
 * 	@fp:	the opened file stream to read from
 * 	@err:	error indicator, 0 == INI_OK indicates success
 *
 * Returns: ini-structure ready to read from, NULL on error (@err non-zero too)
 */
struct ini_file *read_inifile(FILE *fp, int *err);

/* free_inifile() -- destroy an ini-file, freeing all associated memory
 * 	@inf:	the ini-file-structure to destroy
 *
 * Always succeeds if passed a valid argument
 * WARNING -- any values returned by the get_ini_value/get_ini_section
 * 	are pointers within this structure and will be invalid if the parent
 * 	is freed.
 */
void free_inifile(struct ini_file *inf);

/* get_ini_value() -- parse an ini-file for the value of *key under *section
 * 	@inf:		ini-file-structure to look through
 * 	@section:	the section name to search for
 * 	@key:		the key whose value you want to find
 * 	@err:		integer error indicator -- 0 == INI_FOUND on success
 *
 * Returns: pointer to the value in @inf if found, else NULL and *err is set
 */
char *get_ini_value(struct ini_file *inf, char *section, char *key, int *err);

/* get_ini_section() -- get pointer to section element named *name
 * 	@inf:	ini-file object to read from
 * 	@name:	name of section to search for
 *
 * Returns: pointer to section if found, else NULL
 */
struct ini_section *get_ini_section(struct ini_file *ini, char *name);

/* get_section_value() -- look through a given section for the named key
 * 	@s:		section to look through (probably returned by get_ini_section())
 * 	@key:	key to search for
 *
 * Returns: value corresponding to @key, or NULL on failure
 */
char *get_section_value(struct ini_section *s, char *key);

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
