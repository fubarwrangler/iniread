#include <stdio.h>
#include "iniread.h"

int main(int argc, char *argv[])
{
	struct ini_file *ini;

	if(argc < 4)	{
		printf("Usage: %s FILE SECTION KEY\n", argv[0]);
		return 1;
	}

	if(ini_read_file(argv[1], &ini) == INI_OK)	{
		int err;
		printf("[%s] %s = '%s'\n", argv[2], argv[3],
				ini_get_value(ini, argv[2], argv[3], &err)
		);
		puts(ini_error_string(err));
		ini_free_data(ini);
	}

	return 0;
}
