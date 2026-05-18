#include "mm_tlsf.h"

#if (ZST_USE_ALLOC==1 && ZST_MEM_CUSTOM==0)

/* mm_tlsf.c - TLSF Memory Manager Wrapper */
#include "mm_tlsf.h"
#include "./tlsf/tlsf.h"
#include <string.h>



static tlsf_t tlsf_handle = NULL;
static pool_t tlsf_pool = NULL;

void mm_init(void *memory_pool, size_t pool_size)
{
    tlsf_handle = tlsf_create_with_pool(memory_pool, pool_size);
    tlsf_pool = tlsf_get_pool(tlsf_handle);
}

void *mm_alloc(size_t size)
{
    return tlsf_malloc(tlsf_handle, size);
}

void *mm_calloc(size_t num, size_t size)
{
    void *ptr = tlsf_malloc(tlsf_handle, num * size);
    if (ptr)
    {
        memset(ptr, 0, num * size);
    }
    return ptr;
}

void *mm_realloc(void *ptr, size_t size)
{
    return tlsf_realloc(tlsf_handle, ptr, size);
}

void *mm_memalign(size_t align, size_t size)
{
    return tlsf_memalign(tlsf_handle, align, size);
}

void mm_free(void *ptr)
{
    tlsf_free(tlsf_handle, ptr);
}

// Memory usage stats

typedef struct
{
    size_t free_bytes;
    size_t used_bytes;
} mm_stat_t;

static void memory_walker(void *ptr, size_t size, int used, void *user)
{
    mm_stat_t *stat = (mm_stat_t *)user;
    if (used)
        stat->used_bytes += size;
    else
        stat->free_bytes += size;
}

size_t mm_get_free_memory(void)
{
    mm_stat_t stat = {0};
    tlsf_walk_pool(tlsf_pool, memory_walker, &stat);
    return stat.free_bytes;
}

size_t mm_get_used_memory(void)
{
    mm_stat_t stat = {0};
    tlsf_walk_pool(tlsf_pool, memory_walker, &stat);
    return stat.used_bytes;
}

#endif /* ZST_USE_ALLOC==1 && ZST_MEM_CUSTOM==0 */
