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

#ifndef _INIREAD_H
#define _INIREAD_H

#define INI_FOUND		0
#define INI_NOSECTION	1
#define INI_NOKEY		2
#define INI_IOERROR		3

/* ini_read_value() -- read a value given a key and section of an .ini file
 *	@fname:		filename to try to open
 *	@section:	a section heading to look for a match in
 *	@key:		the key whose value we'll fetch under @section
 *	@err:		int that will be set to the one of error code listed above
 *				if something goes wrong, see below for details.
 *
 * Returns: pointer to dynamically allocated value if found, else NULL
 *
 * Sets *err to an appropriate code in the following cases:
 *	INI_FOUND		Everything went OK and a value was returned
 * 	INI_NOSECTION	Section cannot be found
 * 	INI_NOKEY		Section found but key isn't in it.
 * 	INI_IOERROR		Error opening / reading from *fname
 */
char *ini_read_value(char *fname, char *section, char *key, int *err);

#endif
