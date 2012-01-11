#include <stdio.h>
#include <stdlib.h>

#include "iniread.h"


int main(int argc, char *argv[])
{
	char *v = NULL, *p;
	char *key, *val = NULL;
	int e;
	FILE *fp;

/*	if(argc <= 3)	{
		printf("Error, %s file section key\n", argv[0]);
		return 1;
	}
	v = ini_read_value(argv[1], argv[2], argv[3], &e);
	printf("%d: '%s'\n", e, v);
	free(v); */
	fp = fopen(argv[1], "r");
	while((v=ini_readline(fp, &e)) != NULL)	{
		if((p = is_section(v)) != NULL)
			printf("Section: %s\n", p);
		else {
			get_key_value(v, &key, &val);
			printf("Key: '%s', Value: '%s'\n", key, val);
			free(key);
			free(val);
		}
		//free(val);
		free(v);
	}
	fclose(fp);
	return 0;
}