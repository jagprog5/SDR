#include "SparseDistributedRepresentation.h"
// #include <vector>
// #include <iostream>
// #include <type_traits>

int main()
{
    // vector<int> v;
    // v.resize(10);
    // cout << v[1] << '\n';
    // SDR<int> a{0, 4, 5, 7, 17};
    // SDR<int> b{0, 2, 3, 7, 17, 18};
    // a.andi(b);
    SDR<int> a(0.98, 1.0, 3, 100);
    // a.focus(b);
    cout << a << '\n';
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