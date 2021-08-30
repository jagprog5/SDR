#include "SparseDistributedRepresentation.h"
#include <random>
// #include <vector>
// #include <iostream>
// #include <type_traits>

int main()
{
    // list<int> v {0, 1, 2};
    // *v.begin() = 3;
    // for (auto e : v) {
        // cout << e << endl;
    // }
    // vector<int> b;
    // sample(v.cbegin(), v.cend(), back_inserter(v), 3, std::mt19937{std::random_device{}()});
    // for (auto& elem : v) {
        // cout << elem << '\n';
    // }
    // v.resize(10);
    // cout << v[1] << '\n';
    // SDR<int> a{0, 4, 5, 7, 17};
    // SDR<int> b{0, 2, 3, 7, 17, 18};
    // a.andi(b);
    SDR<unsigned int,set<unsigned int>> a{1, 2, 4, 5, 6, 8 ,9};
    SDR<unsigned int,set<unsigned int>> b{1, 2, 3, 7, 9};
    // a.andi(b);
    // a >>= 3;
    // a.focus(b);
    cout << (a + 10) << '\n';
    // a.focus(b);
    // cout << a << '\n';
    // cout << a.get(b);
    // // r.set(6, true);
    // auto section = r.get(3, 6);
    // auto q(section);
    // // int r = ({
    //     // 2;
    // // });
    // cout << q << '\n';
    // // Test t;
	return 0;
}

/*
#include <iostream>
#include "SparseDistributedRepresentation.h"
using namespace std;

auto funct() {
    int i = 3;
    return i;
}

int main() {
    cout << funct() << '\n';
    /*
    SDR<> a;
    a.length = 2;
    // cout << *a.crbegin() << " " << *a.crend() << endl;
    // cou
    for (auto it = a.cbegin(), end = a.cend(); it != end; ++it) {
        auto i = *it;
        i = 3;
        std::cout << i << "\n";
    }
    for (auto it = a.cbegin(), end = a.cend(); it != end; ++it) {
        auto i = *it;
        std::cout << i << "\n";
    }
}
*/