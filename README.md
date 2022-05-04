[![Build Status](https://travis-ci.com/jagprog5/sdr.svg?branch=master)](https://travis-ci.com/jagprog5/sdr)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
# Sparse Distributed Representations

This provides a header-only C++17 library for manipulating Sparse Distributed Representations.

Fuzzing / benchmarks and unit tests can be built with the cmake flag `-DBUILD_TESTING=true`.

## Example

```cpp
#include "SparseDistributedRepresentation.hpp"

using namespace SparseDistributedRepresentation;

int main() {
    SDR<> a{1, 2, 3};
    SDR<> b{2, 3, 4};
    std::cout << (a & b) << std::endl; // prints: [2,3]
    return 0;
}

```