#ifndef ArrayList_H
#define ArrayList_H

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#define uint unsigned int

typedef struct ArrayList {
    uint element_size, length, capacity;
    char data[];
} ArrayList;

ArrayList* _allocAL_do_not_call_this(uint element_size);
#define allocAL(type) _allocAL_do_not_call_this(sizeof(type))

#define freeAL(arr) assert(__builtin_types_compatible_p(typeof(arr), ArrayList*));\
                    free(arr)

void printAL(ArrayList* arr, char format_specifier);

#define clearAL(arr)    assert(__builtin_types_compatible_p(typeof(arr), ArrayList*));\
                        arr->length = 0

void* getAL(ArrayList* arr, uint index);

void _setAL_do_not_call_this(ArrayList* arr, uint index, char* element);
#define setAL(arr, index, element)  assert((arr)->element_size == sizeof(*(element)));\
                                    _setAL_do_not_call_this(arr, index, (char*)(element))

void _insertAL_do_not_call_this(ArrayList** arr, uint index, char* element);
#define insertAL(arr, index, element)   assert((*(arr))->element_size == sizeof(*(element)));\
                                        _insertAL_do_not_call_this(arr, index, (char*)(element))

#define appendAL(arr, element) insertAL(arr, (*(arr))->length, element)

void removeAL(ArrayList** arr, uint index);

#endif