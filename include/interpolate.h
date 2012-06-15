#ifndef _INTERPOLATE_H__
#define _INTERPOLATE_H__

#include "iniread.h"
#include "hash.h"

struct scoped_var {
	struct scoped_var *next;
	char *section_referenced;
	char *variable_referenced;
};

hash_table *get_variables(struct ini_file *ini);



#endif /* _INTERPOLATE_H__ */