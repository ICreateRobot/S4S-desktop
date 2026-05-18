#include "../inc/cc_mem.h"
#include "cc_conf.h"

void * cc_alloc(size_t size) 
{
    return CC_MEM_ALLOC(size);
}


void cc_free(void * ptr)
{
    CC_MEM_FREE(ptr);
}

