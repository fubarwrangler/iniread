#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "iniread.h"



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
			s = end;
		} else {
			*scope = NULL;
		}
		*name = calloc((s - str) + 1, 1);
		memmove(*name, str, (s - str));
	} else {
		end = str;
	}
	return end;
}

/* Scan list for matching section and value */
static bool is_present(struct scoped_var *list, char *sec, char *val)
{
	while(list)	{
		if(strcmp(list->section, sec) == 0 && strcmp(list->variable, val) == 0)	{
			return true;
		}
		list = list->next;
	}
	return false;
}

/* Take a key-value pair and  */
struct scoped_var *get_variables(struct ini_file *ini)
{
	struct scoped_var *slist = NULL;
	struct scoped_var **sp = &slist;
	struct ini_section *section = ini->first;
	struct kv_pair *kvp;
	char *str;

	while(section)	{
		kvp = section->items;
		while(kvp)	{
			int ctr = 0;
			str = kvp->value;

			while(*str)	{
				if(*str == '$' && *(str + 1) == '{')	{
					char *sec, *var;
					str = read_var(str + 2, &sec, &var);
					if(var != NULL)	{
						*sp = malloc(sizeof(struct scoped_var));
						(*sp)->section = (sec == NULL) ? strdup(section->name) : sec;
						(*sp)->variable = var;
						(*sp)->container_sec = section;
						(*sp)->container_kvp = kvp;
						(*sp)->index = ctr++;

						sp = &(*sp)->next;
					}
				} /* s now at end of ${block} or at begenning if not valid */
				str++;
			}
			kvp = kvp->next;
		}
		section = section->next;
	}
	return slist;
}

