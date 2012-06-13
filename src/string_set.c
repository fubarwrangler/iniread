#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "iniread.h"

typedef struct _bucket_data
{
	char *str;
	struct _bucket_data *next;

} bucket_data;

typedef struct _set_data {
	bucket_data **buckets;
	size_t size;	/* Number of buckets */
	size_t nelm;	/* Number of elements in set table */
} string_set;

typedef struct _hash_iter {
	bucket_data *bucket;
	size_t idx;
} set_iter;

/**
 * This is the hash to use if no user-supplied one is provided, it is an
 * implementation of Jenkins's hash -- a good general purpose hash function
 * for typical keys:
 *
 * SOURCE: Jenkins, Bob (September 1997). "Hash functions". Dr. Dobbs Journal.
 */
unsigned int hash_fn(const char *key)
{
    unsigned int hash = 0;

    while(*key)
    {
        hash += *key++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}


string_set *set_init(void)
{
	string_set *h = malloc(sizeof(string_set));

	if(h != NULL)	{
		h->size = 10;
		h->nelm = 0;
		h->buckets = calloc(h->size, sizeof(bucket_data *));
		if(h->buckets != NULL)	{
			size_t i;
			for(i = 0; i < h->size; i++)
				h->buckets[i] = NULL;
			return h;
		} else {
			free(h);
		}
	}
	return NULL;
}

void set_destroy(string_set *h)
{
	/* Walk each bucket's list freeing as we go */
	if(h!=NULL)	{
		if(h->buckets != NULL)	{
			bucket_data *b, *nb;
			int i;

			for(i = 0; i < h->size; i++)	{
				b = h->buckets[i];
				while(b != NULL)	{
					nb = b->next;
					free(b->str);
					free(b);
					b = nb;
				}
			}
			free(h->buckets);
		}
		free(h);
	}
}

bool set_member(string_set *h, const char *key)
{
	bucket_data *b = h->buckets[hash_fn(key) % h->size];

	while(b != NULL)	{
		if(strcmp(b->str, key) == 0)
			return true;
		b = b->next;
	}
	return false;
}

bool set_remove(string_set *h, const char *key)
{
	unsigned int idx = hash_fn(key) % h->size;
	bucket_data **b = &h->buckets[idx];

	while(*b != NULL)	{
		if(strcmp((*b)->str, key) == 0)	{
			bucket_data *bd = *b;	/* Mark node to be deleted */

			*b = (*b)->next;	/* Skip ahead to next node in list */

			free(bd->str);
			free(bd);

			h->nelm--;
			return true;
		}
		b = &(*b)->next;
	}
	return false;
}

int set_resize(string_set *h, size_t newsize)
{
	bucket_data **newbuckets;
	bucket_data *b, *target, *prev, *next;
	unsigned int i;

	/* New ptr array for buckets, hash each old val into this new array */
	if((newbuckets = calloc(newsize, sizeof(bucket_data *))) == NULL)
		return 1;
	for(i = 0; i < h->size; i++)	{
		b = h->buckets[i];
		/* For each old bucket item, insert into correct new bucket */
		while(b)	{
			unsigned int idx = hash_fn(b->str) % newsize;
			next = b->next;

			if(newbuckets[idx] == NULL)	{
				newbuckets[idx] = b;
				b->next = NULL;
			} else {
				target = newbuckets[idx];
				while(target) {
					prev = target;
					target = target->next;
				}
				prev->next = b;
				b->next = NULL;
			}

			b = next;
		}
	}
	/* Replace old with new */
	free(h->buckets);
	h->buckets = newbuckets;
	h->size = newsize;

	return 0;
}


int set_insert(string_set *h, const char *str)
{
	bucket_data **b = NULL;
	unsigned int idx = hash_fn(str) % h->size;
	float load = (float)h->nelm / (float)h->size;

	if(load > 0.8)	{
		if(set_resize(h, h->size * 2) != 0)
			return 1;
	}

	b = &h->buckets[idx];

	/* Walk the bucket list to see if it exists already, if so silently
	 * update, freeing if autofree set
	 */
	while(*b != NULL)	{
		if(strcmp((*b)->str, str) == 0)
			return 0;
		b = &(*b)->next;
	}

	/* If not in the list, append to the end of the list */
	if((*b = malloc(sizeof(bucket_data))) == NULL)
		return 1;

	if(((*b)->str = strdup(str)) == NULL)	{
		free(*b);
		return 1;
	}

	(*b)->next = NULL;
	h->nelm++;

	return 0;
}

void set_iter_init(string_set *h, set_iter *state)
{
	state->idx = 0;
	state->bucket = h->buckets[0];
}


int set_iterate(string_set *h, set_iter *state, char **item)
{
	if(state->bucket != NULL && state->bucket->next != NULL)	{
		state->bucket = state->bucket->next;
	} else {
		while(++(state->idx) < h->size)	{
			if(h->buckets[state->idx] != NULL)	{
				state->bucket = h->buckets[state->idx];
				goto found;
			}
		}
		return 0;
	}

	found:
	*item = state->bucket->str;
	return 1;
}


