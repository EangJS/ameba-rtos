#include "SEGGER_SYSVIEW_heaptrace.h"
#include "SEGGER_SYSVIEW.h"
#include "task.h"

#ifdef SRAM_HEAP0_START
    static void* gHeapSRAM0 = NULL;
    static size_t gHeapSRAM0Size = 0;
#endif

#ifdef SRAM_HEAP1_START
    static void* gHeapSRAM1 = NULL;
    static size_t gHeapSRAM1Size = 0;
#endif

#ifdef PSRAM_HEAP0_START
    static void* gHeapPSRAM0 = NULL;
    static size_t gHeapPSRAM0Size = 0;
#endif

#ifdef PSRAM_HEAP1_START
    static void* gHeapPSRAM1 = NULL;
    static size_t gHeapPSRAM1Size = 0;
#endif

void HeapTrace_Init(void) {

#ifdef SRAM_HEAP0_START
    gHeapSRAM0 = SRAM_HEAP0_START;
    gHeapSRAM0Size = ( size_t ) SRAM_HEAP0_SIZE;
    SEGGER_SYSVIEW_HeapDefine(gHeapSRAM0, gHeapSRAM0, gHeapSRAM0Size, 0);
#endif

#ifdef SRAM_HEAP1_START
    gHeapSRAM1 = SRAM_HEAP1_START;
    gHeapSRAM1Size = ( size_t ) SRAM_HEAP1_SIZE;
    SEGGER_SYSVIEW_HeapDefine(gHeapSRAM1, gHeapSRAM1, gHeapSRAM1Size, 0);
#endif

#ifdef PSRAM_HEAP0_START
    gHeapPSRAM0 = PSRAM_HEAP0_START;
    gHeapPSRAM0Size = ( size_t ) PSRAM_HEAP0_SIZE;
    SEGGER_SYSVIEW_HeapDefine(gHeapPSRAM0, gHeapPSRAM0, gHeapPSRAM0Size, 0);
#endif

#ifdef PSRAM_HEAP1_START
    gHeapPSRAM1 = PSRAM_HEAP1_START;
    gHeapPSRAM1Size = ( size_t ) PSRAM_HEAP1_SIZE;
    SEGGER_SYSVIEW_HeapDefine(gHeapPSRAM1, gHeapPSRAM1, gHeapPSRAM1Size, 0);
#endif

}

void* HeapTrace_Malloc(size_t size)
{
    void* ptr = pvPortMalloc(size);

    if (ptr != NULL)
    {
        if ((uint8_t*)ptr >= (uint8_t*)gHeapSRAM0 && (uint8_t*)ptr < ((uint8_t*)gHeapSRAM0 + gHeapSRAM0Size))
        {
            SEGGER_SYSVIEW_HeapAlloc(gHeapSRAM0, ptr, size);
        }
        else if ((uint8_t*)ptr >= (uint8_t*)gHeapPSRAM0 && (uint8_t*)ptr < ((uint8_t*)gHeapPSRAM0 + gHeapPSRAM0Size))
        {
            SEGGER_SYSVIEW_HeapAlloc(gHeapPSRAM0, ptr, size);
        }
    }

    return ptr;
}

void HeapTrace_Free(void* ptr)
{
    if (ptr != NULL)
    {
        if ((uint8_t*)ptr >= (uint8_t*)gHeapSRAM0 && (uint8_t*)ptr < ((uint8_t*)gHeapSRAM0 + gHeapSRAM0Size))
        {
            SEGGER_SYSVIEW_HeapFree(gHeapSRAM0, ptr);
        }
        else if ((uint8_t*)ptr >= (uint8_t*)gHeapPSRAM0 && (uint8_t*)ptr < ((uint8_t*)gHeapPSRAM0 + gHeapPSRAM0Size))
        {
            SEGGER_SYSVIEW_HeapFree(gHeapPSRAM0, ptr);
        }
        vPortFree(ptr);
    }
}
