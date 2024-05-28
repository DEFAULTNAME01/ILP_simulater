// fifo_macros.h
#ifndef FIFO_MACROS_H
#define FIFO_MACROS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define DEFINE_FIFO(TYPE, NAME, CAPACITY) \
typedef struct { \
    TYPE **buffer; \
    int head; \
    int tail; \
    int capacity; \
    int size; \
} NAME; \
\
void NAME##_init(NAME *fifo) { \
    fifo->buffer = (TYPE**)malloc( CAPACITY * sizeof(TYPE*)); \
    fifo->capacity =  CAPACITY; \
    fifo->size = 0; \
    fifo->head = 0; \
    fifo->tail = 0; \
    if (!fifo->buffer) { \
        fprintf(stderr, "Failed to allocate memory\n"); \
        exit(EXIT_FAILURE); \
    } \
} \
\
bool NAME##_IS_FULL(NAME *fifo){ \
return (fifo)->size == (fifo)->capacity;\
}\
\
int NAME##_enqueue(NAME *fifo, TYPE *item) { \
    if (fifo->size == fifo->capacity) { \
        return -1; /* FIFO is full */ \
    } \
    fifo->buffer[fifo->tail] = item; \
    fifo->tail = (fifo->tail + 1) % fifo->capacity; \
    fifo->size++; \
    return 0; \
} \
\
int NAME##_dequeue(NAME *fifo, TYPE **item) { \
    if (fifo->size == 0) { \
        return -1; /* FIFO is empty */ \
    } \
    *item = fifo->buffer[fifo->head]; \
    fifo->head = (fifo->head + 1) % fifo->capacity; \
    fifo->size--; \
    return 0; \
} \
\
void NAME##_free(NAME *fifo) { \
    for (int i = 0; i < fifo->size; i++) { \
        free(fifo->buffer[(fifo->head + i) % fifo->capacity]); /* 确保传进来的值是通过 malloc(), calloc(), 或 realloc() 分配的指针*/\
    } \
    free(fifo->buffer); \
}\
\
void NAME##_append(NAME *dest, NAME *src) { \
    for (int i = 0; i < src->size; i++) { \
        int index = (src->head + i) % src->capacity; \
        if (NAME##_enqueue(dest, src->buffer[index]) == -1) { \
            fprintf(stderr, "Failed to append all items: destination FIFO is full\n"); \
            break; \
        } \
    } \
}\
\
void NAME##_appendS(NAME *dest, NAME *src) {\
    TYPE *item = malloc(sizeof(NAME)); \
    while (src->size > 0) { \
        if (NAME##_dequeue(src, &item) != 0) { \
            fprintf(stderr, "Failed to dequeue from source FIFO\n"); \
            break; \
        } \
        if (NAME##_enqueue(dest, item) != 0) { \
            fprintf(stderr, "Failed to enqueue to destination FIFO: FIFO is full\n"); \
            break; \
        } \
    }\
}\
\
typedef bool (*MatchCondition)(void *item, void *arg); /*定义一个函数指针类型，用于判断元素是否匹配*/ \
\
void NAME##_purge(NAME *fifo, MatchCondition condition, void *arg){ \
    int writeIndex = fifo->head; \
    for (int i = 0; i < fifo->size; i++) { \
        int currentIndex = (fifo->head + i) % fifo->capacity; \
        TYPE *item = fifo->buffer[currentIndex]; \
        if (!condition(item, arg)) { \
            if (writeIndex != currentIndex) { \
                fifo->buffer[writeIndex] = fifo->buffer[currentIndex]; \
            } \
            writeIndex = (writeIndex + 1) % fifo->capacity; \
        } else { \
            free(item); /* Assuming TYPE* is dynamically allocated and needs to be freed */ \
        } \
    } \
    fifo->size = (writeIndex - fifo->head + fifo->capacity) % fifo->capacity; \
    fifo->tail = writeIndex; \
}\
\

#define DEFINE_SELECTIVE_READ_FIRST(TYPE, NAME) \
TYPE *NAME##_SELECTIVE_READ_FIRST(NAME *fifo, MatchCondition condition, void *arg) { \
    for (int i = 0; i < fifo->size; i++) { \
        int index = (fifo->head + i) % fifo->capacity; \
        TYPE *item = fifo->buffer[index]; \
        if (condition(item, arg)) { \
            return item; \
        } \
    } \
    return NULL; /* No item matched the condition */ \
}\

#endif