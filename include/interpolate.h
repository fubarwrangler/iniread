#ifndef _INTERPOLATE_H__
#define _INTERPOLATE_H__

#include "iniread.h"
#include "hash.h"

struct scoped_var {
	struct scoped_var *next;
	char *section_referenced;
	char *variable_referenced;
};

struct sorted_list {
	char *section;
	char *variable;
	struct sorted_list *next;
};

hash_table *get_variables(struct ini_file *ini);
struct sorted_list *topo_sort(hash_table *ht);



#endif /* _INTERPOLATE_H__ */