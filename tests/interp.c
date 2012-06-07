#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "iniread.h"
struct scoped_var *topo_sort(struct scoped_var *list);
void free_variables(struct scoped_var *head);



int main(int argc, char *argv[])
{
	struct ini_file *ini;
	struct scoped_var *sv;

	if(argc < 2)
		return 1;

	setvbuf(stdout, NULL, _IONBF, 0);

	if(ini_read_file(argv[1], &ini) != INI_OK)
		return 2;

	sv = get_variables(ini);


	/*while(sv)	{
		printf("Variable %s::%s [%d] referenced in section [%s]\n\tby key (%s): %s\n",
				sv->section, sv->variable, sv->index, sv->container_sec->name,
				sv->container_kvp->key, sv->container_kvp->value);
		sv = sv->next;
	}*/
	topo_sort(sv);

	free_variables(sv);
	ini_free_data(ini);

	return 0;
}
