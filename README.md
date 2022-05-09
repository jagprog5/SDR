[![Build Status](https://travis-ci.com/jagprog5/sdr.svg?branch=master)](https://travis-ci.com/jagprog5/sdr)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
# Sparse Distributed Representation (SDR)

This is a header-only C++17 library for manipulating [SDRs](https://youtu.be/ZDgCdWTuIzc).

## Build Tests and Fuzzing

```bash
cmake . -DBUILD_TESTING=true ; make
./test_sdr # tests
./fuzz_sdr [<fuzz_amount>] # fuzzy tests
```

## Simple Example

```cpp
#include "SparseDistributedRepresentation/SDR.hpp"

using namespace SparseDistributedRepresentation;

int main() {
    SDR a{1, 2, 3, 4};
    SDR b{2, 3, 4, 5};
    std::cout << (a & b) << std::endl; // prints: [2,3,4]
    return 0;
}

```

# Guide

## Operations

**AND**(`[1, 2]`, `[2, 3]`) = `[2]`  
**OR**&nbsp;&nbsp;&nbsp;(`[1, 2]`, `[2, 3]`) = `[1, 2, 3]`  
**XOR**(`[1, 2]`, `[2, 3]`) = `[1, 3]`  
**RM**&nbsp;&nbsp;&nbsp;(`[1, 2]`, `[2, 3]`) = `[1]` 

Each op has three variations:

```cpp
SDR a{1, 2};
SDR b{2, 3};

// and elements
SDR r = a.ande(b);  // a new sdr is created with the content: [2]

// and size
int s = a.ands(b);  // returns 1; this is the size of the result if ande was called

// and in-place
a.andi(b)           // `a` is modified and now contains [2]
```

## SDR_t

`SDRs` contain `SDR_t` elements. An `SDR_t` consists of an `id` and (optionally) `data`. 

The id is an integral type which is used to compare equality between elements.

The data is a payload associated with the id. Data is combined with other data when ops are computed on SDRs. Three data types are defined:

`EmptyData`: disables the data functionality

`UnitData`: a float which is bounded from 0 to 1 inclusively

`FloatData`: normal float

The default SDR_t has id: int, and data: EmptyData.

```cpp
#include "SparseDistributedRepresentation/SDR.hpp"
#include "SparseDistributedRepresentation/DataTypes/UnitData.hpp"

int main() {
    using UnitSDR_t = SDR_t<int, UnitData>;
    using UnitSDR = SDR<UnitSDR_t>;

    // prints [0(.25)]
    // since both SDRs have an element in the same position (0),
    // and because UnitData multiplies together in the and-op (0.5 * 0.5)
    auto r0 = UnitSDR{UnitSDR_t(0, 0.5)}.ande(UnitSDR{UnitSDR_t(0, 0.5)});
    std::cout << r0 << std::endl;

    // data types also define a "relevance".
    // for UnitData, if its value is < 0.1, then it is omitted from the result
    auto r1 = UnitSDR{UnitSDR_t(0, 0.25)}.ande(UnitSDR{UnitSDR_t(0, 0.25)});
    std::cout << r1 << std::endl; // []
    int r1_size = UnitSDR{UnitSDR_t(0, 0.25)}.ands(UnitSDR{UnitSDR_t(0, 0.25)});
    std::cout << r1_size << std::endl;  // 0

    // data types are compatible with other data types
    // EmptyData converts to UnitData(1)
    auto r2 = UnitSDR{UnitSDR_t(0, 0.5)}.ande(SDR{SDR_t(0)});
    std::cout << r2 << std::endl;  // [0(.50)]
    return 0;
}
```

## Containers

The SDR class is a container adapter which uses a vector by default. An stl vector or set is best suited to be the underlying container, but it can also use a forward_list, list, multiset, or any non stl container types that have appropriate interfaces.

## Escaping the Walled Garden

If the SDR api is lacking in some niche way, then an SDR can be `reinterpret_cast`ed to its underlying container.

```cpp
SDR a{1, 2, 3};
auto brute_force_ptr = reinterpret_cast<std::vector<SDR_t<>>*>(&a);
(*brute_force_ptr)[1].id = 17;

// SDRs have ascending elements with no duplicates
// this is not a valid SDR since it has [1,17,3]
// it will give strange values but not UB
std::cout << a;
```
