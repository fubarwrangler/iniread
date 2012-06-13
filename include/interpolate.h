#ifndef _INTERPOLATE_H__
#define _INTERPOLATE_H__

#include "iniread.h"

struct scoped_var {
	char *section_referenced;
	char *variable_referenced;
	int index;
};

struct scoped_var *get_variables(struct ini_file *ini);


#endif /* _INTERPOLATE_H__ */