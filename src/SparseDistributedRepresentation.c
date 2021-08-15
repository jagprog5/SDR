#include <math.h>
#include "SparseDistributedRepresentation.h"

SDR* encodeLinearSDR(float input, uint numOnBits, SDR_t size) {
    assert(numOnBits <= size
        && numOnBits > 0 && numOnBits <= INT_MAX
        && size > 0);
    ArrayList* r = malloc(sizeof(*r) + sizeof(SDR_t) * numOnBits);
    r->element_size = sizeof(SDR_t);
    r->length = numOnBits;
    r->capacity = numOnBits;
    SDR_t startOffset = roundf((size - numOnBits) * input);
    SDR_t elem;
    for (SDR_t i = 0; i < numOnBits; ++i) {
        elem = startOffset + i;
        setAL(r, i, &elem);
    }
    return (SDR*)r;
}

SDR* encodePeriodicSDR(float input, float period, uint numOnBits, SDR_t size) {
    assert(period != 0
        && numOnBits <= size
        && numOnBits > 0 && numOnBits <= INT_MAX
        && size > 0);
    ArrayList* r = malloc(sizeof(*r) + sizeof(SDR_t) * numOnBits);
    r->element_size = sizeof(SDR_t);
    r->length = numOnBits;
    r->capacity = numOnBits;
    float progress = input / period;
    progress -= (int)progress;
    assert(progress >= 0);
    SDR_t startOffset = roundf(progress * size);
    SDR_t indexWrap;
    SDR_t indexLeading;
    if (startOffset + numOnBits > size) {
        indexLeading = indexWrap = startOffset + numOnBits - size;
        while (indexWrap > 0) {
            --indexWrap;
            setAL(r, indexWrap, &indexWrap);
        }
        while (indexLeading < numOnBits) {
            SDR_t elem = size - (numOnBits - indexLeading);
            setAL(r, indexLeading++, &elem);
        }
    } else {
        for (int i = 0; i < numOnBits; ++i) {
            SDR_t elem = startOffset + i;
            setAL(r, i, &elem);
        }
    }
    return (SDR*)r;
}

void onSDR(SDR** a, SDR_t index) {
    int left = 0;
    int right = (*a)->indices.length - 1;
    int middle = 0;
    SDR_t midVal = SDR_t_MAX;
    while (left <= right) {
        middle = (right + left) / 2;
        midVal = *(SDR_t*)getAL((ArrayList*)(*a), (uint)middle);
        if (midVal < index) {
            left = middle + 1;
        } else if (midVal > index) {
            right = middle - 1;
        } else {
            return;
        }
    }
    if (index > midVal) {
        ++middle;
    }
    insertAL((ArrayList**)a, (uint)middle, &index);
}

void offSDR(SDR** a, SDR_t index) {
    int left = 0;
    int right = (*a)->indices.length - 1;
    int middle;
    SDR_t midVal;
    while (left <= right) {
        middle = (right + left) / 2;
        midVal = *(SDR_t*)getAL((ArrayList*)*a, (uint)middle);
        if (midVal < index) {
            left = middle + 1;
        } else if (midVal > index) {
            right = middle - 1;
        } else {
            removeAL((ArrayList**)a, (uint)middle);
        }
    }
}

void setSDR(SDR** a, SDR_t index, bool value) {
    if (value) {
        onSDR(a, index);
    } else {
        offSDR(a, index);
    }
}

bool getSDR(SDR* a, SDR_t index) {
    int left = 0;
    int right = a->indices.length - 1;
    int middle;
    SDR_t midVal;
    while (left <= right) {
        middle = (right + left) / 2;
        midVal = *(SDR_t*)getAL((ArrayList*)a, middle);
        if (midVal < index) {
            left = middle + 1;
        } else if (midVal > index) {
            right = middle - 1;
        } else {
            return true;
        }
    }
    return false;
}

void rmSDR(SDR** a, SDR* rm) {
    ArrayList* aArr = (ArrayList*)*a;
    int aFrom = 0;
    int aTo = 0;
    SDR_t aVal;
    ArrayList* rmArr = (ArrayList*)rm;
    int rmOffset = 0;
    SDR_t rmVal;
    while (aFrom < aArr->length) {
        if (rmOffset < rmArr->length) {
            aVal = ((SDR_t*)aArr->data)[aFrom];
            rmVal = ((SDR_t*)rmArr->data)[rmOffset];
            if (rmVal < aVal) {
                ++rmOffset;
                continue;
            } else if (rmVal == aVal) {
                ++rmOffset;
                ++aFrom;
                continue;
            } 
        }
        ((SDR_t*)aArr->data)[aTo++] = ((SDR_t*)aArr->data)[aFrom++];
    }
    if (aTo < aArr->capacity / 2) {
        aArr = (ArrayList*)(*a = realloc(aArr, aArr->element_size * aTo));
    }
    aArr->length = aTo;
    aArr->capacity = aTo;
}

void getOneSDR(ArrayList* a, uint* offset, SDR_t* value, bool* notEmpty) {
    assert(*offset <= a->length);
    if (*offset == a->length) {
        *notEmpty = false;
    } else {
        *value = *(SDR_t*)getAL(a, (*offset)++);
    }
}

void andSDR(void* r, SDR** a, SDR** b, bool lengthOnly) {
    ArrayList* rArr;
    if (!lengthOnly) {
        rArr = *(ArrayList**)r;
    }
    ArrayList* aArr = (ArrayList*)*a;
    bool aNotEmpty = true;
    uint aOffset = 0;
    SDR_t aVal;
    ArrayList* bArr = (ArrayList*)*b;
    bool bNotEmpty = true; 
    uint bOffset = 0;
    SDR_t bVal;
    uint rLen = 0;
    getOneSDR(aArr, &aOffset, &aVal, &aNotEmpty);
    getOneSDR(bArr, &bOffset, &bVal, &bNotEmpty);
    while (aNotEmpty && bNotEmpty) {
        if (aVal < bVal) getOneSDR(aArr, &aOffset, &aVal, &aNotEmpty);
        else if (aVal > bVal) getOneSDR(bArr, &bOffset, &bVal, &bNotEmpty);
        else {
            if (!lengthOnly) {
                // append to output, reallocing to accommodate half the max remaining possible elements
                assert(rLen <= rArr->capacity);
                if (rLen == rArr->capacity) {
                    uint aLeft = aArr->length - aOffset;
                    uint bLeft = bArr->length - bOffset;
                    uint maxRemaining = aLeft < bLeft ? aLeft : bLeft;
                    uint capIncrement = (1 + maxRemaining) / 2 + 1;
                    rArr->capacity += capIncrement;
                    rArr = *(ArrayList**)r = realloc(rArr, rArr->element_size * rArr->capacity);
                    aArr = (ArrayList*)*a; // if r = a or r = b
                    bArr = (ArrayList*)*b;
                }
                setAL(rArr, rLen, &aVal);
            }
            ++rLen;
            getOneSDR(aArr, &aOffset, &aVal, &aNotEmpty);
            getOneSDR(bArr, &bOffset, &bVal, &bNotEmpty);
        }
    }
    if (lengthOnly) {
        *(uint*)r = rLen;
    } else {
        rArr->length = rLen;
    }
}

#define addToOutputORSDR(val) \
if (!lengthOnly) {\
    assert(rLen <= rArr->capacity);\
    if (rLen == rArr->capacity) {\
        uint aLeft = aArr->length - aOffset;\
        uint bLeft = bArr->length - bOffset;\
        uint maxRemaining = aLeft + bLeft;\
        uint capIncrement = (1 + maxRemaining) / 2 + 1;\
        rArr->capacity += capIncrement;\
        rArr = *(ArrayList**)r = realloc(rArr, rArr->element_size * rArr->capacity);\
    }\
    setAL(rArr, rLen, &val);\
}\
++rLen;

void orSDR(void* r, SDR* a, SDR* b, bool lengthOnly, bool exclusive) {
    ArrayList* aArr = (ArrayList*)a;
    ArrayList* bArr = (ArrayList*)b;
    ArrayList* rArr;
    if (!lengthOnly) {
        rArr = *(ArrayList**)r;
        assert(rArr != aArr && rArr != bArr);
    }
    bool aNotEmpty = true;
    uint aOffset = 0;
    SDR_t aVal;
    bool bNotEmpty = true; 
    uint bOffset = 0;
    SDR_t bVal;
    uint rLen = 0;
    getOneSDR(aArr, &aOffset, &aVal, &aNotEmpty);
    getOneSDR(bArr, &bOffset, &bVal, &bNotEmpty);
    while (aNotEmpty || bNotEmpty) {
        if ((aNotEmpty && !bNotEmpty) || (aNotEmpty && bNotEmpty && aVal < bVal)) {
            addToOutputORSDR(aVal)
            getOneSDR(aArr, &aOffset, &aVal, &aNotEmpty);
        } else if ((!aNotEmpty && bNotEmpty) || (aNotEmpty && bNotEmpty && aVal > bVal)) {
            addToOutputORSDR(bVal)
            getOneSDR(bArr, &bOffset, &bVal, &bNotEmpty);
        } else {
            if (!exclusive) { addToOutputORSDR(aVal) }
            getOneSDR(aArr, &aOffset, &aVal, &aNotEmpty);
            getOneSDR(bArr, &bOffset, &bVal, &bNotEmpty);
        }
    }
    if (lengthOnly) {
        *(uint*)r = rLen;
    } else {
        rArr->length = rLen;
    }
}