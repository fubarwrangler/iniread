#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

#include "iniread.h"

static bool to_bool(const char *str, int *err)
{
	bool rv = false;

	if(str == NULL)
		return false;

	/* If we match any 'true' values...*/
	if( !strcasecmp("1", str) || !strcasecmp("true", str) ||
		!strcasecmp("yes", str) || !strcasecmp("on", str)
	  )
		rv = true;
	/* By now, if we don't match any 'false' values, it is an error */
	else if(strcasecmp("0", str) && strcasecmp("false", str) &&
			strcasecmp("no", str) && strcasecmp("off", str)
		   )
		*err = INI_NOTBOOL;
	return rv;
}

static int to_int(const char *str, int *err)
{
	long int n;
	char *p;

	if(str == NULL)
		return 0;

	errno = 0;
	n = strtol(str, &p, 10);
	if(errno != 0 || *p != '\0' || n > INT_MAX)	{
		*err = INI_NOTINT;
		return 0;
	}

	return (int)n;
}

static float to_float(const char *str, int *err)
{
	float f;
	char *p;

	if(str == NULL)	{
		*err = INI_NOSECTION;
		return 0;
	}

	errno = 0;
	f = strtof(str, &p);
	if(errno != 0 || *p != '\0')	{
		*err = INI_NOTFLOAT;
		return 0;
	}

	return f;

}


bool ini_get_bool(struct ini_file *inf,
				  const char *section,
				  const char *key,
				  int *err)
{
	return to_bool(ini_get_value(inf, section, key, err), err);
}

bool ini_get_section_bool(struct ini_section *s, const char *key, int *err)
{
	char *p = ini_get_section_value(s, key);
	if(p == NULL)	{
		*err = INI_NOKEY;
		return false;
	}
	*err = INI_OK;
	return to_bool(p, err);
}

int ini_get_int(struct ini_file *inf,
				  const char *section,
				  const char *key,
				  int *err)
{
	return to_int(ini_get_value(inf, section, key, err), err);
}

int ini_get_section_int(struct ini_section *s, const char *key, int *err)
{
	char *p = ini_get_section_value(s, key);
	if(p == NULL)	{
		*err = INI_NOKEY;
		return 0;
	}
	*err = INI_OK;
	return to_int(p, err);
}


float ini_get_float(struct ini_file *inf,
				  const char *section,
				  const char *key,
				  int *err)
{
	return to_float(ini_get_value(inf, section, key, err), err);
}

float ini_get_section_float(struct ini_section *s, const char *key, int *err)
{
	char *p = ini_get_section_value(s, key);
	if(p == NULL)	{
		*err = INI_NOKEY;
		return 0;
	}
	*err = INI_OK;
	return to_float(p, err);
}
