#ifndef __CC_STRING_BUILDER_H
#define __CC_STRING_BUILDER_H

#include "cc_array_chain.h"

typedef struct cc_string_builder 
{
	cc_array_chain_t *chain;
} cc_string_builder_t;

int cc_string_builder_new(cc_string_builder_t **self);
int cc_string_builder_delete(cc_string_builder_t *self);

int cc_string_builder_to_string(cc_string_builder_t *self, char **result);

int cc_string_builder_append(cc_string_builder_t *self, char *s,
		size_t size);

int cc_string_builder_append_str(cc_string_builder_t *self, char *s);

int cc_string_concat(char **result, int n, ...);

#endif
