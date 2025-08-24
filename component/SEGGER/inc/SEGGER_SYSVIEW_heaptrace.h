#ifndef HEAP_TRACE_H
#define HEAP_TRACE_H
#include "ameba.h"
#include <stddef.h>  // for size_t

/**
 * @brief Initialize heap tracing with SEGGER SYSTEMVIEW
 * by defining the various heap regions.
 * 
 */
void HeapTrace_Init(void);

/**
 * @brief Allocate dynamic memory and trace the allocation with SEGGER SYSTEMVIEW
 * 
 * @param size Number of bytes to allocate
 * @return void* Pointer to the allocated memory
 */
void HeapTrace_Malloc(void* ptr, size_t size);

/**
 * @brief Free dynamically allocated memory and trace the deallocation with SEGGER SYSTEMVIEW
 * 
 * @param ptr Pointer to the memory to free
 */
void HeapTrace_Free(void* ptr);

#endif // HEAP_TRACE_H
