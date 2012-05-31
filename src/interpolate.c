#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "iniread.h"

struct scoped_var {
	char *section;
	char *variable;
	struct scoped_var *next;
};

static char *read_var(char *str, char **scope, char **name)	{

	char *end = strchr(str, '}');

	*scope = NULL;
	*name = NULL;
	if(end != NULL)	{
		char *s = str;
		while(*s != '.' && s < end)	{
			s++;
		}
		if(*s == '.')	{
			*scope = calloc((s - str) + 1, 1);
			memmove(*scope, str, (s - str));
			str = s + 1;
			while(*s++ != '}')
				;
		} else {
			*scope = NULL;
		}
		*name = calloc((s - str), 1);
		memmove(*name, str, (s - str) - 1);
	} else {
		end = str;
	}
	return end;
}


static struct scoped_var *get_variables(struct kv_pair *kvp, char *current_sec)
{
	char *s = kvp->value;
	struct scoped_var *slist = NULL;
	struct scoped_var **sp = &slist;

	while(*s)	{
		if(*s == '$' && *(s + 1) == '{')	{
			char *sec, *var;
			s = read_var(s + 2, &sec, &var);
			if(var != NULL)	{
				*sp = malloc(sizeof(struct scoped_var));
				(*sp)->section = (sec == NULL) ? strdup(current_sec) : sec;
				(*sp)->variable = var;
				sp = &(*sp)->next;
			}
		} /* s now at end of ${block} or at begenning if not valid */
		s++;
	}
	return slist;
}

int ini_interpolate(struct ini_file *ini)
{
	return 0;
}

int main(int argc, char *argv[])
{
	char *s = NULL, *scope = NULL, *name = NULL;
	struct kv_pair k = {NULL, "key", "string that will ${be} interpolated ${sec.variable}${who}."};
	struct scoped_var *sv;

	sv = get_variables(&k, "http");
	while(sv)	{
		printf("Variable (%s)[%s]\n", sv->section, sv->variable);
		sv = sv->next;
	}

	return 0;
}