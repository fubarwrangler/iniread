#include <stdio.h>
#include "iniread.h"

void dump_ini(struct ini_file *ini)
{
	hash_iter ictx, sctx;
	struct ini_section *sec;
	char *secname, *key, *val;

	hash_iter_init(ini->sections, &ictx);
	while(hash_iterate(&ictx, (void **)&secname, (void **)&sec))	{
		printf("Section '%s':\n", secname);
		hash_iter_init(sec->items, &sctx);
		while(hash_iterate(&sctx, (void **)&key, (void **)&val))	{
			printf("%s: %s\n", key, val);
		}
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
			   ini_get_int(ini, argv[2], argv[3], &err)
			  );
		puts(ini_error_string(err));
		ini_free_data(ini);
	}

	return 0;
}
