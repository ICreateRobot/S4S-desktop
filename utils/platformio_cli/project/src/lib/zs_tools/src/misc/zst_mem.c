#include "zst_mem.h"

#include "string.h"

#if ZST_USE_ALLOC==1
    #if ZST_MEM_CUSTOM==1
        #include ZST_MEM_CUSTOM_INCLUDE
    #elif ZST_MEM_CUSTOM==0
        #include "./tlsf/mm_tlsf.h"
    #endif
#endif




void zst_memset(void * dst, uint8_t v, size_t len)
{
    memset(dst, v, len);
}


void zst_memset_00(void * dst, size_t len)
{
    memset(dst, 0, len);
}


void * zst_mem_alloc(size_t size)
{
    #if ZST_USE_ALLOC
        #if ZST_MEM_CUSTOM==1
            return ZST_MEM_CUSTOM_ALLOC(size);
        #elif ZST_MEM_CUSTOM==0
            return mm_alloc(size);
        #endif
    #else
        return NULL;
    #endif
}
void * zst_mem_calloc(size_t nmemb, size_t size)
{
    #if ZST_USE_ALLOC
        #if ZST_MEM_CUSTOM==1
            void * ptr = zst_mem_alloc(nmemb * size);
            if (ptr)
            zst_memset_00(ptr, nmemb * size);
        #elif ZST_MEM_CUSTOM==0
            void * ptr = mm_calloc(nmemb, size);
        #endif
        return ptr;
    #else
        return NULL;
    #endif
}

void zst_mem_free(void * ptr)
{
    #if ZST_USE_ALLOC
        #if ZST_MEM_CUSTOM==1
            ZST_MEM_CUSTOM_FREE(ptr);
        #elif ZST_MEM_CUSTOM==0
            mm_free(ptr);
        #endif
    #endif
}

void * zst_mem_realloc(void * ptr, size_t size)
{
    #if ZST_USE_ALLOC
        #if ZST_MEM_CUSTOM==1
            return ZST_MEM_CUSTOM_REALLOC(ptr, size);
        #elif ZST_MEM_CUSTOM==0
            return mm_realloc(ptr, size);
        #endif
    #else
        return NULL;
    #endif
}

size_t zst_mem_free_size(void)
{
    #if ZST_USE_ALLOC
        #if ZST_MEM_CUSTOM==1
            return 0;
        #elif ZST_MEM_CUSTOM==0
            return mm_get_free_memory();
        #endif
    #else
        return 0;
    #endif
}

size_t zst_mem_used_size(void)
{
    #if ZST_USE_ALLOC
        #if ZST_MEM_CUSTOM==1
            return 0;
        #elif ZST_MEM_CUSTOM==0
            return mm_get_used_memory();
        #endif
    #else
        return 0;
    #endif
}



