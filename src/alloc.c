/*
alloc.c

Copyright (c) 25 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "alloc.h"

#if 1

#include <stdlib.h>
#include <stdio.h>

#define INITIAL_MAX_CHUNKS 0x1000

static void** allocated_chunks;
static int chunk_count;
static int chunk_capacity;
static int allocated_memory;

static void expand_array(size_t new_size)
{
    allocated_chunks = (void**)realloc(allocated_chunks, new_size * sizeof(void*));
    chunk_capacity = new_size;
}

void *danpa_alloc(size_t bytes)
{
    if (allocated_chunks == NULL)
    {
        expand_array(INITIAL_MAX_CHUNKS);
    }

    void* ptr = malloc(bytes);
    if (!ptr)
    {
        fprintf(stderr, "memory allocation error\n");
        abort();
    }

    allocated_chunks[chunk_count] = ptr;
    ++chunk_count;
    if (chunk_count >= chunk_capacity)
        expand_array(chunk_capacity + chunk_capacity/2); // *1.5

    allocated_memory += bytes;

    return ptr;
}

void *danpa_realloc(void* ptr, size_t new_size)
{
    if (ptr == 0)
        return danpa_alloc(new_size);

    for (int i = 0; i < chunk_count; ++i)
    {
        if (ptr == allocated_chunks[i])
        {
            allocated_chunks[i] = realloc(ptr, new_size);
            return allocated_chunks[i];
        }
    }

    fprintf(stderr, "memory reallocation error\n");
    abort();
}

void cleanup_memory()
{
    printf("allocated %d + %d bytes, %d/%d slots\n", allocated_memory, chunk_capacity*sizeof(void*), chunk_count, chunk_capacity);

    for (int i = 0; i < chunk_count; ++i)
    {
        free(allocated_chunks[i]);
    }

    free(allocated_chunks);
}

#else

#include <stdint.h>

#define CHUNK_SIZE 0x10000


typedef struct chunk_t
{
    unsigned used_size;
    uint8_t data[];
} chunk_t;

static chunk_t** chunk_list;
static unsigned chunk_count;
static unsigned chunk_capacity;


static void expand_array(size_t new_size)
{
    chunk_list = (chunk_t**)realloc(chunk_list, new_size * sizeof(chunk_t*));
    chunk_capacity = new_size;
}


void *danpa_alloc(size_t bytes)
{
    for (int i = 0; i < chunk_count; ++i)
    {
        if ((CHUNK_SIZE - chunk_list[i]->used_size) >= bytes)
        {
            void* ret = &chunk_list[i]->data[chunk_list[i]->used_size];
            chunk_list[i]->used_size += bytes;

            return ret;
        }
    }

    ++chunk_count;
    if (chunk_count > chunk_capacity)
    {
        if (chunk_capacity == 0)
            chunk_capacity = 2;
        expand_array(chunk_capacity + chunk_capacity/2);
    }
    // allocate another chunk
    chunk_list[chunk_count-1] = (chunk_t*)malloc(CHUNK_SIZE + sizeof(chunk_t));
    chunk_list[chunk_count-1]->used_size = bytes;
    return &chunk_list[chunk_count-1]->data;
}

void cleanup_memory()
{
    for (int i = 0; i < chunk_count; ++i)
    {
        free(chunk_list[i]);
    }
    free(chunk_list);
}

#endif
