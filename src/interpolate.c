#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "iniread.h"
#include "interpolate.h"

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

static char *make_key(char *section, char *key)
{
	char *buf = malloc(strlen(section) + strlen(key) + 3);
	if(buf != NULL)
		sprintf(buf, "%s::%s", section, key);
	return buf;
}



/* Take a key-value pair and  */
hash_table *get_variables(struct ini_file *ini)
{
	hash_table *ht = hash_init(NULL, 0);
	struct ini_section *section;
	struct scoped_var **sp, *head;
	hash_iter s_iter, v_iter;
	void *tk, *tv;
	char *sec_name, *key, *val;
	char buf[1024];



	hash_iter_init(ini->sections, &s_iter);
	while(hash_iterate(ini->sections, &s_iter, &sec_name, &section) != 0)	{
		hash_iter_init(section->items, &v_iter);
		while(hash_iterate(section->items, &v_iter, &key, &val) != 0)	{
			head = NULL;


			sp = &head;

			while(*val)	{
				if(*val == '$' && *(val + 1) == '{')	{
					char *sec, *var;
					int e;

					val = read_var(val + 2, &sec, &var);
					if(var != NULL)	{
						*sp = malloc(sizeof(struct scoped_var));

						(*sp)->section_referenced = (sec == NULL) ? strdup(sec_name) : sec;
						(*sp)->variable_referenced = var;
						(*sp)->next = NULL;

						sp = &(*sp)->next;
					}
				} /* s now at end of ${block} or at beginning if not valid */
				val++;
			}
			if(head != NULL)	{
				char *buf = make_key(sec_name, key);
				hash_insert(ht, buf, head);
				free(buf);
			}
		}
	}

	return ht;
}

static bool is_referenced(char *sec, char *var, hash_table *data)
{
	char buf = make_key(sec, var);
	bool found;
	found = (hash_get(data, buf) == NULL);
	free(buf);
	return found;
}


struct sorted_list *topo_sort(hash_table *ht)	{
	return NULL;
}





/*



static bool is_referenced(struct scoped_var *sv, struct scoped_var *v)
{
	while(sv)	{
		if(streq(sv->container_sec->name, v->section) && streq(sv->container_kvp->key, v->variable))
			return true;
		sv = sv->next;
	}
	return false;
}

struct sorted_list *topo_sort(struct scoped_var *list)	{
	struct scoped_var *p, *tmp;
	struct scoped_var *new_list;
	struct scoped_var **q, **r;
	struct sv_set *set;
	int n, m;
	size_t n_elm;

	for(p = list, n_elm = 0; p != NULL; p = p->next, n_elm++)
		pprint_node(p);

	for(p=list; p != NULL; p = p->next)	{
		pprint_node(p);


		printf("Node %s::%s ", p->section, p->variable);
		if(is_referenced(list, p))
			printf("is referenced by others\n");
		else
			printf("is NOT referenced by others\n");

		puts("\n");
	}


	q = &new_list;

	p = list;

	printf("---begin---\n");



	while(p) {

		if(is_referenced(list, p->next) == false)	{
			tmp = p->next;
			if(list == p)	{
				list = list->next;
			}

			*q = copy_sv(p);
			(*q)->next = NULL;
			pprint_node(*q);
			q = &(*q)->next;
		}
		p = p->next;
	}


	printf("---------------\n");

	for(p = list, n_elm = 0; p != NULL; p = p->next, n_elm++)
		pprint_node(p);

	printf("----new----\n");
	for(p = new_list, n_elm = 0; p != NULL; p = p->next, n_elm++)
		pprint_node(p);


	//free_set(set);





	return NULL;
}*/
