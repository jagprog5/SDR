[![Build Status](https://travis-ci.com/jagprog5/sdr.svg?branch=master)](https://travis-ci.com/github/jagprog5/sdr)
[![codecov](https://codecov.io/gh/jagprog5/sdr/branch/master/graph/badge.svg)](https://codecov.io/gh/jagprog5/sdr)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
# Sparse Distributed Representation (SDR)

This is a header-only C++17 library for manipulating [SDRs](https://youtu.be/ZDgCdWTuIzc) and sparse vectors.

## Quick Example

```cpp
#include "SparseDistributedRepresentation/SDR.hpp"

using namespace sparse_distributed_representation;

int main() {
    SDR a{1, 2, 3, 4};
    SDR b{2, 3, 4, 5};
    std::cout << (a & b) << std::endl; // prints: [2,3,4]
    return 0;
}

```

## Building

### Tests and Fuzzing

```bash
# dependencies
apt-get install cmake g++ libboost-test-dev

# build
cd build && cmake .. -DBUILD_TESTING=true && cmake --build .

# test
ctest

# or run each test manually
./test_sdr # tests
./fuzz_sdr [<fuzz_amount>] # fuzzy tests
```

### Display Code Coverage

```bash
apt-get install lcov # and llvm if using clang
cd build && cmake .. -DBUILD_TESTING=true -DCODE_COVERAGE=true && make cov-show
```

### Display CPU Profile

```bash
apt-get install libgoogle-perftools-dev  
cd build && cmake .. -DBUILD_TESTING=true -DPERF=CPU && make perf-show
```

# Guide

## Operations

| Operation | Arg1   | Arg2   | Result    |
|-----------|--------|--------|-----------|
| AND       | [1, 2] | [2, 3] | [2]       |
| OR        | [1, 2] | [2, 3] | [1, 2, 3] |
| XOR       | [1, 2] | [2, 3] | [1, 3]    |
| RM        | [1, 2] | [2, 3] | [1]       |

Each operation has three variations:

- normal (*and* elements, "ande")
- size (*and* size, "ands")
- inplace (*and* inplace, "andi")

```cpp
SDR a{1, 2};
SDR b{2, 3};

// and elements
SDR r = a.ande(b);  // a new sdr is created with the result: [2]

// and size
int s = a.ands(b);  // returns 1; this is the size of the result if ande was called

// and inplace
a.andi(b)           // `a` is modified and contains the result [2]
```

## Library Structure

```mermaid
flowchart TB
    SDR["SDR\nHas a container_t of SDR_t elements"]
    SDR_t["SDR_t\nHas an id and (optionally) data"]
    container_t["container_t\nSDR is a container adaptor that can accept various containers\nIt is best suited for a std::vector or std::set"]
    ArrayAdaptor["ArrayAdaptor\nWraps std::array in a vector-like interface"]
    EmptyData["EmptyData\nDisables the data functionality"]
    UnitData["UnitData\nA float bounded from 0 to 1"]
    FloatData["FloatData\nA float"]
    id_t["id_t\nAn integral type which uniquely identifies an SDR_t in an SDR."]
    data_t["data_t\nA payload associated with the id."]

    SDR--template--->SDR_t
    SDR--template-->container_t
    container_t-->std::vector
    container_t-->std::set
    container_t-->std::forward_list
    container_t--->ArrayAdaptor

    SDR_t--template-->id_t
    SDR_t--template--->data_t

    data_t-->EmptyData
    data_t-->UnitData
    data_t-->FloatData


```

## SDR_t with Data

SDRs are composed of SDR_t elements. By default, an SDR_t has an `int` identifier and an `EmptyData` data.  
This means that each index in the dense representation is identified by an int,  
and that each index has no data (aka EmptyData) associated with it.

Data can define a "relevance". If the SDR_t's data is not relevant, then it is not placed in the result.  
This is helpful given the context in which sparse vector's are used. Only important values are worth mentioning.

```cpp
#include "SparseDistributedRepresentation/SDR.hpp"
#include "SparseDistributedRepresentation/DataTypes/UnitData.hpp"
using namespace sparse_distributed_representation;

int main() {
    // this SDR contains elements which are identified by an int
    // and each id has a UnitData associated with it
    using UnitSDR_t = SDR_t<int, UnitData>;
    using UnitSDR = SDR<UnitSDR_t>;

    auto r0 = UnitSDR{UnitSDR_t(0, 0.5)}.ande(UnitSDR{UnitSDR_t(0, 0.5)});
    // Both SDRs have an element in the same position [0],
    // and UnitData multiplies together in the and-op (0.5 * 0.5)
    std::cout << r0 << std::endl; // prints [0(.25)]

    // data types also define a "relevance".
    // for UnitData, if its value is < 0.1, then it is omitted from the result
    auto r1 = UnitSDR{UnitSDR_t(0, 0.25)}.ande(UnitSDR{UnitSDR_t(0, 0.25)});
    std::cout << r1 << std::endl; // empty
    int r1_size = UnitSDR{UnitSDR_t(0, 0.25)}.ands(UnitSDR{UnitSDR_t(0, 0.25)});
    std::cout << r1_size << std::endl;  // 0

    // data types are compatible with other data types
    // EmptyData converts to UnitData(1)
    auto r2 = UnitSDR{UnitSDR_t(0, 0.5)}.ande(SDR{SDR_t(0)});
    std::cout << r2 << std::endl;  // [0(.50)]
    return 0;
}
```

## Custom Operations

AND, OR, XOR, and RM are implemented via [visitors](https://en.wikipedia.org/wiki/Visitor_pattern). Visitors do something with selections of the data.  
For example, the `ands` operation implicitly does two separate steps:

1. Find ids that are shared between the SDRs (by using the function which applies an *and visitor*, "andv").
2. For each id pair, increment a value (which is implemented by that specific visitor).

`ands` can be reimplemented with the following:

```c++
SDR a{1, 2, 3};
SDR b{2, 3, 4};

int result = 0;
auto increment_visitor = [&result](typename decltype(a)::container_type::iterator,
                                   typename decltype(b)::container_type::iterator) {
    ++result;
};

a.andv(b, increment_visitor); // apply the visitor
std::cout << result << std::endl; // 2 elements in commmon
```

## Escaping the Walled Garden

If the SDR api is lacking in some niche way, then an SDR can be `reinterpret_cast`ed to its underlying container.

```cpp
SDR a{1, 2, 3};
// get the vector
auto brute_force_ptr = reinterpret_cast<std::vector<SDR_t<>>*>(&a);
// const_cast since id should normally not be set directly
const_cast<int&>((*brute_force_ptr)[1].id()) = 17;

// SDRs have ascending elements, with no duplicates
// this is not a valid SDR since it has [1,17,3]
// it will give strange values but not UB
std::cout << a;
```
