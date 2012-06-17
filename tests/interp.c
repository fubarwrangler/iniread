#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "iniread.h"
#include "interpolate.h"


int main(int argc, char *argv[])
{
	struct ini_file *ini;
	hash_table *ht;
	hash_iter iter;
	char *k;
	struct scoped_var *v;
	int tmp;


	if(argc < 2)
		return 1;

	setvbuf(stdout, NULL, _IONBF, 0);

	if(ini_read_file(argv[1], &ini) != INI_OK)
		return 2;



	ht = get_variables(ini);
	hash_iter_init(ht, &iter);
	while(hash_iterate(ht, &iter, &k, &v) != 0)	{
		while(v != NULL)	{
			printf("(@%p) (%s) %s - %s (next %p)\n", v, k, v->section_referenced, v->variable_referenced, v->next);
			v = v->next;
		}
	}

	topo_sort(ht);

	hash_destroy(ht);
	ini_free_data(ini);

	return 0;
}
