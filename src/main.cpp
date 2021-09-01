#include "SparseDistributedRepresentation.h"
int main() {
    cout << "================ list ================" << endl;
    SDR<int,list<int>>::do_benchark();
    cout << "================ vector ================" << endl;
    SDR<int,vector<int>>::do_benchark();
    cout << "================ set ================" << endl;
    SDR<int,set<int>>::do_benchark();

    /*
    ================ list ================
    init: 49818
    sizes: 6293 6321
    6318 6334
    6315 6300
    6343 6386
    6288 6343
    andb: 1655
    ands: 1638
    orb: 12
    ors: 6
    xorb: 8
    xors: 5
    andi: 1631
    rm: 2194
    ================ vector ================
    init: 85
    sizes: 6335 6328
    6355 6308
    6341 6332
    6299 6262
    6315 6293
    andb: 7
    ands: 6
    orb: 3
    ors: 1
    xorb: 2
    xors: 1
    andi: 7
    rm: 14
    ================ set ================
    init: 53642
    sizes: 6335 6328
    6355 6308
    6341 6332
    6299 6262
    6315 6293
    andb: 2893
    ands: 2907
    orb: 18
    ors: 8
    xorb: 13
    xors: 7
    andi: 2461
    rm: 1434
    */
}
