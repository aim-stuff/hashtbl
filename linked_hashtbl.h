#ifndef LINKED_HASHTBL_H
#define LINKED_HASHTBL_H

/* Copyright (c) 2009, 2010 <Andrew McDermott>
 *
 * Source can be cloned from:
 *
 *     git://github.com/aim-stuff/hashtbl.git
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * A hash table: efficiently map keys to values.
 *
 * SYNOPSIS
 *
 * 1. A hash table is created with l_hashtbl_create().
 * 2. To insert an entry use l_hashtbl_insert().
 * 3. To lookup a key use l_hashtbl_lookup().
 * 4. To remove a key use l_hashtbl_remove().
 * 5. To apply a function to all entries use l_hashtbl_apply().
 * 5. To clear all keys use l_hashtbl_clear().
 * 6. To delete a hash table instance use l_hashtbl_delete().
 * 7. To iterate over all entries use l_hashtbl_iter_init(), l_hashtbl_iter_next().
 *
 * Note: neither the keys or the values are copied so their lifetime
 * must match that of the hash table.  NULL keys are not permitted.
 * Inserting, removing or lookup up NULL keys is therefore undefined.
 */

#include <stddef.h>		/* size_t */

#ifdef	__cplusplus
# define __LINKED_HASHTBL_BEGIN_DECLS	extern "C" {
# define __LINKED_HASHTBL_END_DECLS	}
#else
# define __LINKED_HASHTBL_BEGIN_DECLS
# define __LINKED_HASHTBL_END_DECLS
#endif

__LINKED_HASHTBL_BEGIN_DECLS
/* Opaque types. */
struct l_hashtbl;
struct l_hashtbl_list_head;

/* Hash function. */
typedef unsigned int (*LINKED_HASHTBL_HASH_FN) (const void *k);

/* Key equality function. */
typedef int (*LINKED_HASHTBL_EQUALS_FN) (const void *a, const void *b);

/* Apply function. */
typedef int (*LINKED_HASHTBL_APPLY_FN) (const void *key,
					const void *val,
					const void *client_data);

/* Functions for deleting keys and values. */
typedef void (*LINKED_HASHTBL_KEY_FREE_FN) (void *k);
typedef void (*LINKED_HASHTBL_VAL_FREE_FN) (void *v);

/* Functions for allocating and freeing memory. */
typedef void *(*LINKED_HASHTBL_MALLOC_FN) (size_t n);
typedef void (*LINKED_HASHTBL_FREE_FN) (void *ptr);

/* Function for evicting oldest entries. */
typedef int (*LINKED_HASHTBL_EVICTOR_FN) (const struct l_hashtbl * h,
					  unsigned long count);

struct l_hashtbl_iter {
  void *key;
  void *val;
  /* The remaining fields are private: don't modify them. */
  const int direction;
  const struct l_hashtbl_list_head *const pos;
  const struct l_hashtbl_list_head *const end;
};

/*
 * [Default] Hash function.
 */
unsigned int l_hashtbl_direct_hash(const void *k);

/*
 * [Default] Key equals function.
 *
 * Returns 1 if pointer "a" equals key pointer "b".
 */
int l_hashtbl_direct_equals(const void *a, const void *b);

/* Hash functions for integer keys/values. */
unsigned int l_hashtbl_int_hash(const void *k);
int l_hashtbl_int_equals(const void *a, const void *b);

unsigned int l_hashtbl_int64_hash(const void *k);
int l_hashtbl_int64_equals(const void *a, const void *b);

/* Hash functions for nul-terminated string keys/values. */
unsigned int l_hashtbl_string_hash(const void *k);
int l_hashtbl_string_equals(const void *a, const void *b);

/*
 * Creates a new hash table.
 *
 * @param initial_capacity - initial size of the table
 * @param max_load_factor  - before resizing (0.0 uses a default value)
 * @param auto_resize	   - if true, table grows (pow2) as new keys are added
 * @param access_order	   - if true, iteration order is most recently accessed
 * @param hash_func	   - function that computes a hash value from a key
 * @param equals_func	   - function that checks keys for equality
 * @param key_free_func	   - function to delete keys
 * @param val_free_func	   - function to delete values
 * @param malloc_func	   - function to allocate memory (e.g., malloc)
 * @param free_func	   - function to free memory (e.g., free)
 * @param evictor_func	   - function to evict entries as new keys are added
 *
 * Returns non-null if the table was created successfully.
 */
struct l_hashtbl *l_hashtbl_create(int initial_capacity,
				   double max_load_factor,
				   int auto_resize,
				   int access_order,
				   LINKED_HASHTBL_HASH_FN hash_fun,
				   LINKED_HASHTBL_EQUALS_FN equals_fun,
				   LINKED_HASHTBL_KEY_FREE_FN key_free_func,
				   LINKED_HASHTBL_VAL_FREE_FN val_free_func,
				   LINKED_HASHTBL_MALLOC_FN malloc_func,
				   LINKED_HASHTBL_FREE_FN free_func,
				   LINKED_HASHTBL_EVICTOR_FN
				   evictor_func);

/*
 * Deletes the hash table instance.
 *
 * All the entries are removed via l_hashtbl_clear().
 *
 * @param h - hash table
 */
void l_hashtbl_delete(struct l_hashtbl *h);

/*
 * Removes a key and value from the table.
 *
 * @param h - hash table instance
 * @param k - key to remove
 *
 * Returns 0 if key was found, otherwise 1.
 */
int l_hashtbl_remove(struct l_hashtbl *h, const void *k);

/*
 * Clears all entries and reclaims memory used by each entry.
 */
void l_hashtbl_clear(struct l_hashtbl *h);

/*
 * Inserts a new key with associated value.
 *
 * @param h - hash table instance
 * @param k - key to insert
 * @param v - value associated with key
 *
 * Returns 0 on success, or 1 if a new entry cannot be created.
 */
int l_hashtbl_insert(struct l_hashtbl *h, void *k, void *v);

/*
 * Lookup an existing key.
 *
 * @param h - hash table instance
 * @param k - the search key
 *
 * Returns the value associated with key, or NULL if key is not present.
 */
void *l_hashtbl_lookup(struct l_hashtbl *h, const void *k);

/*
 * Returns the number of entries in the table.
 *
 * @param h - hash table instance
 */
unsigned long l_hashtbl_count(const struct l_hashtbl *h);

/*
 * Returns the table's capacity.
 *
 * @param h - hash table instance
 */
int l_hashtbl_capacity(const struct l_hashtbl *h);

/*
 * Apply a function to all entries in the table.
 *
 * The apply function should return 0 to terminate the enumeration
 * early.
 *
 * @param h - hash table instance
 * @param fn - function to apply to each table entry
 * @param client_data - arbitrary user data
 *
 * Returns the number of entries the function was applied to.
 */
unsigned long l_hashtbl_apply(const struct l_hashtbl *h,
			      LINKED_HASHTBL_APPLY_FN fn,
			      void *client_data);

/*
 * Returns the load factor of the hash table.
 *
 * @param h - hash table instance
 *
 * The load factor is a ratio and is calculated as:
 *
 *   l_hashtbl_count() / l_hashtbl_capacity()
 */
double l_hashtbl_load_factor(const struct l_hashtbl *h);

/*
 * Resize the hash table.
 *
 * Returns 0 on success, or 1 if no memory could be allocated.
 */
int l_hashtbl_resize(struct l_hashtbl *h, int new_capacity);

/*
 * Initialize an iterator.
 *
 * @param h - hash table instance
 * @param iter - iterator to initialize
 * @param direction - either 1 for FORWARD or -1 for REVERSE
 */
void l_hashtbl_iter_init(struct l_hashtbl *h,
			 struct l_hashtbl_iter *iter,
			 int direction);

/*
 * Advances the iterator.
 *
 * Returns 1 while there more entries, otherwise 0.  The key and value
 * for each entry can be accessed through the iterator structure.
 */
int l_hashtbl_iter_next(struct l_hashtbl_iter *iter);

__LINKED_HASHTBL_END_DECLS

#endif /* LINKED_HASHTBL_H */
