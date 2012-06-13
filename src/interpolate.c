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
hash_table *get_variables(struct ini_file *ini)
{
	hash_table *ht = hash_init(NULL, 0);
	struct ini_section *section;
	hash_iter s_iter, v_iter;
	void *tk, *tv;
	char buf[1024];
	char *str;



	hash_iter_init(&v_iter);
	while(hash_iterate(ini->sections, &s_iter, &tk, &tv) != 0)	{
		section = (struct ini_section *)tv;
		hash_iter_init(section->items, &v_iter);
		while(hash_iterate(section->items, &v_iter, &tk, &tv)	{

		}
	}




}


















struct sv_set {
	struct scoped_var **ptrs;
	size_t num;
};

/* See if @sv is in @set */
static bool is_present(struct sv_set *set, struct scoped_var *sv)
{
	for(int count = 0; count < set->num; count++)	{
		if(	streq(set->ptrs[count]->section, sv->section) &&
			streq(set->ptrs[count]->variable, sv->variable) )	{
			return true;
		}
	}
	return false;
}

static bool is_referenced(struct scoped_var *sv, struct scoped_var *v)
{
	while(sv)	{
		if(streq(sv->container_sec->name, v->section) && streq(sv->container_kvp->key, v->variable))
			return true;
		sv = sv->next;
	}
	return false;
}

static void pprint_node(struct scoped_var *sv)
{
	printf("(%p) Variable %s::%s references %s::%s (next -> %p)\n",
		sv, sv->section, sv->variable, sv->container_sec->name,
		sv->container_kvp->key, sv->next);
}


struct sv_set *get_head_node_set(struct scoped_var *list)
{
	struct sv_set *set = malloc(sizeof(struct sv_set));
	struct scoped_var *p;
	int i;

	if(set == NULL)
		return NULL;

	set->ptrs = calloc(23, sizeof(struct scoped_var *));
	set->num = 0;
	i = 0;
	p = list;

	while(p)	{
		//printf("Node: %s::%s\n", p->section, p->variable);
		//printf("ref: %d, present: %d\n", is_referenced(list, p), is_present(set, p));
		if(!is_referenced(list, p)) { // && !is_present(set, p))	{
			set->ptrs[i] = p;
			set->num++;
			i++;
		}
		p = p->next;
	}
	return set;
}

struct scoped_var *copy_sv(struct scoped_var *sv)	{
	struct scoped_var *nv = malloc(sizeof(struct scoped_var));
	if(nv != NULL)	{
		memmove(nv, sv, sizeof(struct scoped_var));
		nv->section = strdup(sv->section);
		nv->variable = strdup(sv->variable);
	}
	return nv;
}

void print_set(struct sv_set *set)
{
	printf("Set has %lu:\n", set->num);
	for(int i = 0; i < set->num; i++)	{
		printf("%d: %s::%s (%p)\n", i, set->ptrs[i]->section, set->ptrs[i]->variable, set->ptrs[i]);
	}
}

void free_set(struct sv_set *set)
{
	free(set->ptrs);
	free(set);
}

void free_variables(struct scoped_var *head)
{
	struct scoped_var *p = head;

	while(head)	{
		p = head->next;
		free(head->section);
		free(head->variable);
		free(head);
		head = p;
	}
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

	/* List of nodes not referenced by anyone else */
	/*for(p=list; p != NULL; p = p->next)	{
		pprint_node(p);


		printf("Node %s::%s ", p->section, p->variable);
		if(is_referenced(list, p))
			printf("is referenced by others\n");
		else
			printf("is NOT referenced by others\n");

		puts("\n");
	} */


	q = &new_list;

	p = list;

	printf("---begin---\n");


	/*set = get_head_node_set(list);
	n_elm -= set->num;
	n = set->num;
	print_set(set);*/

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
}
