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

#define INI_FOUND		0
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


/* ini_read_value() -- read a value given a key and section of an .ini file
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
