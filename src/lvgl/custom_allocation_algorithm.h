#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
void lv_mem_init(void);
void *lv_malloc_core(size_t size);
void *lv_realloc_core(void *p, size_t new_size);
void lv_free_core(void *p);
#ifdef __cplusplus
}
#endif