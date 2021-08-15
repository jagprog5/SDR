#ifndef SDR_H
#define SDR_H

#include <limits.h>
// #include <assert.h>
#include "ArrayList.h"

#define SDR_t int
#define SDR_t_MAX INT_MAX

// Ascending Sorted ArrayList for the indices of the bits that are ON.
typedef struct SDR {
    // supports positive and negative indicies
    // can't have more than INT_MAX elements (unlike ArrayList)
    ArrayList indices;
} SDR;

#define allocSDR() ((SDR*)allocAL(SDR_t))

SDR* encodeLinearSDR(float input, uint numOnBits, SDR_t size);

SDR* encodePeriodicSDR(float input, float period, uint numOnBits, SDR_t size);

#define freeSDR(sdr)    static_assert(__builtin_types_compatible_p(typeof(sdr), SDR*));\
                        freeAL((ArrayList*)(sdr))

#define printSDR(sdr)   static_assert(__builtin_types_compatible_p(typeof(sdr), SDR*));\
                        printAL((ArrayList*)(sdr), 'd')

void setSDR(SDR** arr, SDR_t index, bool value);

bool getSDR(SDR* arr, SDR_t index);

void rmSDR(SDR** a, SDR* rm);

/*
 *  if lengthOnly is True:
 *      r is a uint* to uninitialized int
 *  else:
 *      r is a SDR**, with:
 *          - an uninitialized length
 *          - a valid or NULL data ptr with the correct corresponding capacity.
 */
void andSDR(void* r, SDR** a, SDR** b, bool length_only);

void orSDR(void* r, SDR* a, SDR* b, bool length_only, bool exclusive);

#endif