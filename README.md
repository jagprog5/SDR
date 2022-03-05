# Sparse Distributed Representations

This provides a templated header-only C++ library for manipulating Sparse Distributed Representations.

Fuzzing / benchmarks and unit tests can be built with the cmake flag `-DBUILD_TESTING=true`.

## Example

```cpp
#include "SparseDistributedRepresentation.hpp"

int main() {
    SDR a{1, 2, 3};
    SDR b{2, 3, 4};
    std::cout << (a & b) << std::endl; // prints: [2,3]
    return 0;
}

```