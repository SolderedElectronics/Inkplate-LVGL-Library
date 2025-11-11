#include "custom_allocation_algorithm.h"
#include "esp_heap_caps.h"
void lv_mem_init(void)
{
}
void *lv_malloc_core(size_t size)
{
	return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void *lv_realloc_core(void *p, size_t new_size)
{
	return heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM);
}

void lv_free_core(void *p)
{
	heap_caps_free(p);
}