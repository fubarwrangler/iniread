#include <stdio.h>
#include "iniread.h"

void dump_ini(struct ini_file *ini)
{
	struct ini_section *s = ini->first;
	while(s)	{
		struct ini_kv_pair *kv = s->items;
		printf("Section '%s':\n", s->name);
		while(kv)	{
			printf("'%s': '%s'\n", kv->key, kv->value);
			kv = kv->next;
		}
		printf("\n");
		s = s->next;
	}
}


int main(int argc, char *argv[])
{
	struct ini_file *ini;

	if(argc < 4)	{
		printf("Usage: %s FILE SECTION KEY\n", argv[0]);
		return 1;
	}

	if(ini_read_file(argv[1], &ini) == INI_OK)	{
		int err = INI_OK;
		dump_ini(ini);
		printf("[%s] %s = '%d'\n", argv[2], argv[3],
			   ini_get_bool(ini, argv[2], argv[3], &err)
			  );
		puts(ini_error_string(err));
		ini_free_data(ini);
	}

	return 0;
}
