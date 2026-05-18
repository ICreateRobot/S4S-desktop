#ifndef __MM_TLSF_H__
#define __MM_TLSF_H__

#include "zst_conf.h"
#if (ZST_USE_ALLOC==1 && ZST_MEM_CUSTOM==0)

#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h>

void mm_init(void *memory_pool, size_t pool_size);
void *mm_alloc(size_t size);
void *mm_calloc(size_t num, size_t size);
void *mm_realloc(void *ptr, size_t size);
void *mm_memalign(size_t align, size_t size);
void mm_free(void *ptr);

size_t mm_get_free_memory(void);
size_t mm_get_used_memory(void);



#ifdef __cplusplus
}
#endif

#endif /* __MM_TLSF_H__ */

#endif /* ZST_MEM_CUSTOM */
