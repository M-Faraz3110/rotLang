#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk *chunk)
{
    chunk->capacity = 0;
    chunk->count = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    chunk->linesCount = 0;
    chunk->linesCapacity = 0;
    initValueArray(&chunk->constants);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line)
{
    if (chunk->count + 1 > chunk->capacity)
    {
        int oldCapacity = chunk->capacity;
        chunk->capacity = INCREASE_CAPACITY(oldCapacity);
        chunk->code = INCREASE_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        chunk->lines = INCREASE_ARRAY(Line, chunk->lines, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->count++;
    // chunk->lines[chunk->count] = line;
    if (chunk->linesCount == 0 || (chunk->lines[chunk->linesCount - 1].lineNumber == line))
    {
        if (chunk->linesCount + 1 > chunk->linesCapacity)
        {
            int oldCapacity = chunk->linesCapacity;
            chunk->linesCapacity = INCREASE_CAPACITY(oldCapacity);
            chunk->lines = INCREASE_ARRAY(Line, chunk->lines, oldCapacity, chunk->linesCapacity);
            Line newLine;
            newLine.lineNumber = line;
            newLine.runLength = 1;
            chunk->lines[chunk->linesCount++] = newLine;
        }
    }
    else
    {
        chunk->lines[chunk->linesCount - 1].runLength++;
    }
}

int addConstant(Chunk *chunk, Value value)
{
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}

void freeChunk(Chunk *chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}