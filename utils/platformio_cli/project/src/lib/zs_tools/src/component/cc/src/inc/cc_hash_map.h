#ifndef __CC_HASH_MAP_H
#define __CC_HASH_MAP_H

#include "cc_array.h"
#include "cc_common.h"
#include "cc_iter.h"
#include "cc_list_map.h"
#include "cc_map.h"
#include <stddef.h>

typedef struct cc_hash_map 
{
	cc_map_i_t *interface;
	/*
	 * `data` is an array whose elements are of type `cc_list_map_t *`
	 */
	cc_array_t *data;
	size_t bucket_size;
	cc_cmp_fn_t cmp;
	cc_hash_fn_t calc_hash;
} cc_hash_map_t;

int cc_hash_map_new(cc_hash_map_t **self, size_t bucket_size,
		cc_cmp_fn_t cmp, cc_hash_fn_t calc_hash);

int cc_hash_map_delete(cc_hash_map_t *self);

int cc_hash_map_get(cc_hash_map_t *self, void *key, void **result);

int cc_hash_map_set_new(cc_hash_map_t *self, void *key, void *value);

int cc_hash_map_set(cc_hash_map_t *self, void *key, void *value,
		void **old_value);

int cc_hash_map_del(cc_hash_map_t *self, void *key,
		cc_map_item_t **result);

int cc_hash_map_print_slot(cc_list_map_t *slot, void (*callback)(cc_map_item_t *), int index);
int cc_hash_map_print(cc_hash_map_t *self, void (*callback)(cc_map_item_t *), char *end_string);

typedef struct cc_hash_map_iter 
{
	cc_iter_i_t *iterator;
	cc_array_iter_t inner_array_iter;
	cc_list_map_iter_t inner_list_map_iter;
	size_t count;
	unsigned char is_empty;
} cc_hash_map_iter_t;

int cc_hash_map_iter_init(cc_hash_map_iter_t *self,
		cc_hash_map_t *map);

int cc_hash_map_iter_next(cc_hash_map_iter_t *self, void **item,
		size_t *index);

#endif
