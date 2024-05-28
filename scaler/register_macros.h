#ifndef REGISTER_MACROS_H
#define REGISTER_MACROS_H

#include "fifo_macros.h"
#include <stdbool.h>

// 定义寄存器
#define DEFINE_REGISTER(TYPE, NAME, CAPACITY) \
    DEFINE_FIFO(TYPE, NAME, CAPACITY) 

#define DECLARE_REGISTER(TYPE, BASE_NAME) \
    TYPE BASE_NAME##SrcR; \
    TYPE BASE_NAME##DestR

#define REGISTER_init(NAME, BASE_NAME) \
do { \
    NAME##_init(&BASE_NAME##SrcR); \
    NAME##_init(&BASE_NAME##DestR); \
} while (0)

#define REGISTER_free(NAME, BASE_NAME) \
do { \
    NAME##_free(&BASE_NAME##SrcR); \
    NAME##_free(&BASE_NAME##DestR); \
} while (0)

#define REGISTER_enqueue(NAME, BASE_NAME, ITEM) \
    NAME##_enqueue(&BASE_NAME##SrcR, ITEM)

#define REGISTER_dequeue(NAME, BASE_NAME, ITEM) \
    NAME##_dequeue(&(BASE_NAME##DestR), ITEM)

#define NEED_REGISTER(NAME, BASE_NAME) \
    DECLARE_REGISTER(NAME, BASE_NAME); \
    REGISTER_init(NAME, BASE_NAME)

#define REFRESH_REGISTER(NAME, BASE_NAME) \
    NAME##_appendS(&BASE_NAME##DestR, &BASE_NAME##SrcR)

#define DEFINE_FIELD_MATCH_BY(TYPE, FIELD, FIELD_TYPE) \
bool FIELD##MATCH_BY(void *item, void *arg) { \
    TYPE *typedItem = (TYPE *)item; \
    FIELD_TYPE *fieldArg = (FIELD_TYPE *)arg; \
    return typedItem->FIELD == *fieldArg; \
}

#define REGISTER_PURGE_WITH_CONDITION(NAME, BASE_NAME, condition_func, arg) \
do { \
    NAME##_purge(&BASE_NAME##DestR, condition_func, arg); \
} while (0)

#define REGISTER_S_R_F(NAME, BASE_NAME, condition_func, arg) \
    (NAME##_SELECTIVE_READ_FIRST(&BASE_NAME##DestR, condition_func, arg))


// 函数声明



#endif
