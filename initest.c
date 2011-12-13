#include <stdio.h>
#include <stdlib.h>

#include "iniread.h"


int main(int argc, char *argv[])
{
	char *v = NULL;
	int e;

	if(argc <= 3)	{
		printf("Error, %s file section key\n", argv[0]);
		return 1;
	}
	e = ini_read_value(v, argv[1], argv[2], argv[3]);
	printf("%d: '%s'\n", e, v);
	free(v);
	return 0;
}
