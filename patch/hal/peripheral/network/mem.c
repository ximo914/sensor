#include "port.h"

#pragma arm section zidata = "DYNAMICCACHEABLEZI_NC"
static __align(32) kal_uint8 hb_heap[MEM_SIZE] = {0};
#pragma arm section zidata

static KAL_ADM_ID hb_heap_adm_id = NULL;

void mem_init()
{
	if (hb_heap_adm_id != NULL) {
		return;
	}
	hb_heap_adm_id = kal_adm_create2(hb_heap, MEM_SIZE, NULL, KAL_FALSE, 0);
	if (hb_heap_adm_id == NULL) {
		xi_trace("Init heap mem faild!\n");
	}
}

void *xi_malloc(uint32_t size)
{
    size += 3;
    size >>= 2;
    size <<= 2;
	return kal_adm_alloc(hb_heap_adm_id, size);
}

void xi_free(void *mem)
{
	kal_adm_free(hb_heap_adm_id, mem);
}


