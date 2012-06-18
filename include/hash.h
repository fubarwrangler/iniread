#ifndef HASH_H__
#define HASH_H__

#include <stddef.h>
#include <stdio.h>

#define INIT_HASH_TBL_SIZE 10

/* Hash function type -- maps strings to unsigned ints */
typedef unsigned int (*hash_fn_t)(const char *);
typedef void (*hash_callback_fn_t)(void *);

#define HASH_AUTOFREE_	0x01	/* Call free() on each value on destroy */
#define HASH_AUTOGROW_	0x02	/* Grow hash via rehash automatically */

typedef struct _bucket_data
{
	char *key;
	void *data;
	struct _bucket_data *next;

} bucket_data;

typedef struct _hash_table {
	bucket_data **buckets;
	hash_fn_t hash_fn;
	size_t size;	/* Number of buckets */
	size_t nelm;	/* Number of elements in hash table */
	float g_factor; /* How much larger to grow on an expansion (%) */
	float g_trigger;	/* nelm/size to trigger an expansion */
	unsigned char flags;	/* Holds various on/off flags */
} hash_table;

typedef struct _hash_iter {
	bucket_data *bucket;
	size_t idx;
} hash_iter;


/**
 * This is the hash to use if no user-supplied one is provided, it is an
 * implementation of Jenkins's hash -- a good general purpose hash function
 * for typical keys:
 *
 * SOURCE: Jenkins, Bob (September 1997). "Hash functions". Dr. Dobbs Journal.
 */
unsigned int default_hash_fn(const char *key);

/**
 * hash_set_autogrow() -- enable and set parameters for automatic re-hash on
 *                        insert if the table grows too large
 *	@h -- hash table to operate on
 *  @trigger -- nelm/nbuckets ratio to trigger a growth
 *  @factor -- scaling factor to grow, new_size = k * old_size, k > 1.0
 */
void hash_set_autogrow(hash_table *h, float trigger, float factor);
void hash_unset_autogrow(hash_table *h);
void hash_set_autofree(hash_table *h);

/**
 * hash_init() -- initalize a new hash table with hash function @hash_fn
 *
 * @hash_fn: the hash function to use, or NULL to use the default
 *
 * Returns: new empty hash table of INIT_HASH_TBL_SIZE size, NULL on failure
 */
hash_table *hash_init(hash_fn_t hash_fn, size_t initial_size);

/**
 * hash_destroy() -- free all memory associated with hash table
 *
 * @h: hash table to free
 */
void hash_destroy(hash_table *h);

/**
 * hash_destroy_callback() -- free a hash table, calling custom function @cb on
 *                            each data pointer
 *  @h -- hash table to free
 *  @cb -- callback function takes (void *) called with each ->data member
 *         WARNING: must free this pointer if desired, even if autofree is set.
 */
void hash_destroy_callback(hash_table *h, hash_callback_fn_t cb);

/**
 * hash_insert() -- insert a key/data pair into a hash table
 *  @h: hash table to use
 *  @key: the string to use as a key
 *  @data: pointer to data to store associated with key
 *
 * Returns: 0 on success, non-zero on failure (memory error)
 */
int hash_insert(hash_table* h, const char* key, void* data);

/**
 * hash_insert_string() -- like insert, but use a copy of char *val
 *	@@ -- same as hash_insert
 *  @val -- a string that will be dup'd before inserting
 *
 * WARNING: make sure to use freedata = 1 when destroying a hash table created
 *          by using this function.
 *
 * Returns: 0 on success, 1 on failure
 */
int hash_insert_string(hash_table *h, const char *key, char *val);

/**
 * hash_get() -- search for key in a hash table
 *  @h: hash table to look in
 *  @key: the string to search for
 *
 * Returns: the data corresponding to @key, or NULL if not present
 */
void *hash_get(hash_table *h, const char *key);

/**
 * hash_delete() -- remove key from hash table
 *  @h: hash table to delete from
 *  @key: key to remove
 *
 * Returns: 0 on successful delete, 1 on key-not-found
 */
int hash_delete(hash_table *h, const char *key);

/**
 * hash_resize() -- resize a hash table, rehashing all keys into new buckets
 *  @h: hash table
 *  @newsize: the new number of buckets
 *
 * Returns: 0 on success, 1 on error (memory)
 */
int hash_resize(hash_table *h, size_t newsize);

/**
 * hash_iter_init() -- initalize an iterator over all elements of @h
 *  @h -- hash to iterate over
 *  @state -- structure to hold context of iterator currently
 */
void hash_iter_init(hash_table *h, hash_iter *state);

/**
 * hash_iterate() -- walk over all hash elements in an undefined order
 *  @h -- table to walk over
 *  @state -- to store context of current state
 *  @key, @value -- pointers to pointers that will point at each element's
 *                  key and value if there are still elements left
 *
 * Returns: 1 if there are still elements, 0 if the current one is the last
 */
int hash_iterate(hash_table *h, hash_iter *state, void **key, void **val);


#endif /* HASH_H__ */
