#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iniread.h"

typedef char *(*hash_printer)(void *);

void print_hash(hash_table *h, hash_printer hp)
{
	int i;

	printf("%d hashed into %d buckets, hash_fn = %p, buckets at %p\n",
	h->nelm, h->size, h->hash_fn, h->buckets);
	for(i=0; i<h->size; i++)	{
		printf("Bucket %d: %p (%p)", i, h->buckets[i], &h->buckets[i]);
		if(h->buckets[i])	{
			int j = 0;
			bucket_data *b = h->buckets[i];
			while(b)	{
				j++;
				if(j < 4 || b->next == NULL)
					printf("\n  %s => (%p) (%s) (next %p)", b->key, b->data, (*hp)(b->data), b->next);
				else if(j == 4 && b->next != NULL)
					printf("\n  ...");
				b = b->next;
			}
			if(j > 4)
				printf("\n%d skipped", j - 4);
		}
		putchar('\n');
	}
}

void print_hash1(hash_table *h, const char *title, hash_printer hp)
{
	hash_iter ctx;
	char *k;
	void *v;
	
	printf("%s\n", title);
	for(int i=0; i<strlen(title); i++)
		putchar('=');
	putchar('\n');
	hash_iter_init(h, &ctx);
	for(int i = 0; hash_iterate(&ctx, &k, &v); i++)	{
		printf("%3d) %s: %s\n", i, k, (*hp)(v));
	}
}

struct dual_value {
	char *v1;
	char *v2;
};

char *hp_simple(void *d)	{
	return (char *)d;
}

char *hp_val(void *d)	{
	static char cb[4096];
	struct dual_value *dv = d;
	snprintf(cb, 4095, "\n\t%s\n\t%s", dv->v1, dv->v2);
	return cb;
}


int ini_compare(struct ini_section *s1, struct ini_section *s2)
{
	struct dual_value *dv;
	char *key, *v1, *v2;
	hash_iter ctx;
	
	hash_table *first, *second, *same, *diff;
	
	first = hash_init(NULL, s1->items->size);
	second = hash_init(NULL, s2->items->size);
	same = hash_init(NULL, (s1->items->size + s2->items->size) / 2);
	diff = hash_init(NULL, (s1->items->size + s2->items->size) / 2);

	if(!first || !second || !same || !diff)
		exit(1);

	hash_iter_init(s1->items, &ctx);

	while(hash_iterate(&ctx, (void **)&key, (void **)&v1))	{

		if((v2 = hash_get(s2->items, key)) != NULL)	{
			if(strcmp(v1, v2))	{ /* Not equal */
				dv = malloc(sizeof(struct dual_value));
				dv->v1 = v1;
				dv->v2 = v2;
				hash_insert(diff, key, dv);
			} else {
				hash_insert(same, key, v1);
			}
		} else {
			hash_insert(first, key, v1);
		}
	}
	
	hash_iter_init(s2->items, &ctx);
	while(hash_iterate(&ctx, (void **)&key, (void **)&v2))
		if(!hash_get(same, key) && !hash_get(diff, key))
			hash_insert(second, key, v2);
		
		
	
	print_hash1(first, "First Only", hp_simple);
	print_hash1(second, "Second Only", hp_simple);
	print_hash1(same, "Common Fields", hp_simple);
	print_hash1(diff, "Different", hp_val);

	
	hash_destroy(first);
	hash_destroy(second);
	hash_destroy(same);
	hash_destroy(diff);
	
	return 0;
}

int main(int argc, char *argv[])
{
	struct ini_file *ini1, *ini2;
	struct ini_section *s1, *s2;
	int err;

	if(argc < 5)	{
		printf("Usage: %s FILE SECTION FILE2 SECTION2\n", argv[0]);
		return 1;
	}

	if((err = ini_read_file(argv[1], &ini1)) != INI_OK)	{
		fputs(ini_error_string(err), stderr);
		return 2;
	}

	if((err = ini_read_file(argv[3], &ini2)) != INI_OK)	{
		fputs(ini_error_string(err), stderr);
		return 2;
	}
	
	if((s1 = ini_find_section(ini1, argv[2])) == NULL)	{
		fprintf(stderr, "Error finding section '%s' in %s", argv[2], argv[1]);
		return 3;
	}
	if((s2 = ini_find_section(ini2, argv[4])) == NULL)	{
		fprintf(stderr, "Error finding section '%s' in %s", argv[2], argv[1]);
		return 3;
	}

	ini_compare(s1, s2);

	ini_free_data(ini1);
	ini_free_data(ini2);
	
	return 0;
}