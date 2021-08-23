#include "SparseDistributedRepresentation.h"
// #include <vector>
// #include <iostream>
 
int main()
{
    SDR<int> a;
    for (int i = 0; i < 10; ++i) {
        a.set(i, true);
    }
    SDR<int> b;
    for (int i = 5; i < 15; ++i) {
        b.set(i, true);
    }
    b.focus(a);
    cout << b;
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