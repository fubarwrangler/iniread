#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "iniread.h"

#define streq(a, b) (strcmp(a, b) == 0)

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
						(*sp)->next = NULL;

						sp = &(*sp)->next;
					}
				} /* s now at end of ${block} or at beginning if not valid */
				str++;
			}
			kvp = kvp->next;
		}
		section = section->next;
	}
	return slist;
}

/* Scan list for matching section and value */
static int count_present(struct scoped_var *list, char *sec, char *var)
{
	int count = 0;

	while(list)	{
		if(streq(list->section, sec) && streq(list->variable, var))	{
			count++;
		}
		list = list->next;
	}
	return count;
}

static bool is_referenced(struct scoped_var *sv, char *sec, char *var)
{
	while(sv)	{
		if(streq(sv->section, sec) && streq(sv->variable, var))
			return true;
		sv = sv->next;
	}
	return false;
}

static void pprint_node(struct scoped_var *sv)
{
	printf("Variable %s::%s references %s::%s\n",
		sv->container_sec->name, sv->container_kvp->key,
		sv->section, sv->variable);
}


struct scoped_var *topo_sort(struct scoped_var *list)	{
	struct scoped_var *p, *q;
	char *sec_name, *key_name;
	int x;

	/* List of nodes not referenced by anyone else */
	for(p=list; p != NULL; p = p->next)	{
		//x = count_present(list, p->container_sec->name, p->container_kvp->key);
		pprint_node(p);


		printf("Node %s::%s ", p->container_sec->name, p->container_kvp->key);
		if(is_referenced(list, p->container_sec->name, p->container_kvp->key))
			printf("is referenced by others\n");
		else
			printf("is NOT referenced by others\n");

		puts("\n");
	}


	return NULL;
}
