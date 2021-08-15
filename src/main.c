#include <stdio.h>
#include "SparseDistributedRepresentation.h"

int main() {
    SDR* a = encodeLinearSDR(0.55, 15, 100);
    SDR* b = encodeLinearSDR(0.5, 15, 100);
    printSDR(a);
    printSDR(b);
    SDR* r = allocSDR();
    orSDR(&r, a, b, false, true);
    printSDR(r);
    // SDR* r = allocSDR();

    // int i = 2;
    // setAL((ArrayList*)a, 0, &i);
    // appendAL((ArrayList**)&a, &i);
    // appendAL((ArrayList**)&a, &i);
    // appendAL((ArrayList**)&a, &i);
    // printSDR(a);
    // freeSDR(a);
    // a = encodeLinearSDR(0, 3, 100);
    // printSDR(a);
    // freeSDR(a);
    // a = encodeLinearSDR(-1, 3, 100);
    // printSDR(a);
    // freeSDR(a);
    return 0;
}