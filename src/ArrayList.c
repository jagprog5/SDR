#include <stdio.h>
#include "ArrayList.h"

ArrayList* _allocAL_do_not_call_this(uint element_size) {
    assert(element_size > 0);
    ArrayList* arr = malloc(sizeof(*arr));
    arr->element_size = element_size;
    arr->length = 0;
    arr->capacity = 0;
}

void printAL(ArrayList* arr, char format_specifier) {
    static char elemStr[] = {'%', 'd', '\0'};
    elemStr[1] = format_specifier;
    putchar(format_specifier);
    putchar('[');
    for (int i = 0; i < arr->length; ++i) {
        printf(elemStr, *(char*)getAL(arr, i));
        if (i != arr->length - 1) {
            printf(", ");
        }
    }
    printf("]/%u\n", arr->capacity);
}

void* getAL(ArrayList* arr, uint index) {
    assert(arr->length <= arr->capacity && index < arr->capacity);
    return (void*)(arr->data + arr->element_size * index);
}

void _setAL_do_not_call_this(ArrayList* arr, uint index, char* element) {
    char* ptr = getAL(arr, index);
    for (uint i = 0; i < arr->element_size; ++i) {
        ptr[i] = element[i];
    }
}

void _insertAL_do_not_call_this(ArrayList** a, uint index, char* element) {
    ArrayList* arr = *a;
    assert(arr->length <= arr->capacity && index < arr->length);
    if (arr->length == arr->capacity) {
        arr->capacity += (arr->capacity / 2) + 1;
        arr = *a = realloc(arr, sizeof(*arr) + arr->element_size * arr->capacity);
    }
    char* ptr = arr->data + arr->element_size * index;
    for (uint i = (arr->length - index) * arr->element_size; i-- > 0;) {
        ptr[i + arr->element_size] = ptr[i];
    }
    ++arr->length;
    _setAL_do_not_call_this(arr, index, element);
}

void removeAL(ArrayList** a, uint index) {
    ArrayList* arr = *a;
    assert(arr->length <= arr->capacity && index < arr->length);
    --arr->length;
    char* ptr = arr->data + arr->element_size * index;
    for (uint i = 0; i < (arr->length - index) * arr->element_size; ++i) {
        ptr[i] = ptr[i + arr->element_size];
    }
    if (arr->length < arr->capacity / 2) {
        arr->capacity = arr->length;
        arr = *a = realloc(arr, arr->element_size * arr->capacity);
    }
}