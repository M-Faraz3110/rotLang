#ifndef crotlang_memory_h
#define crotlang_memory_h

#include "common.h"

#define INCREASE_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

#define INCREASE_ARRAY(type, pointer, oldCount, newCount)  \
    (type *)reallocate(pointer, sizeof(type) * (oldCount), \
                       sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

void *reallocate(void *pointer, size_t oldSize, size_t newSize);

#endif