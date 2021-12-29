#include "SDRMemory.hpp"

// struct InputConnection {
//     SDR_t input;
//     float* permanence;
//     operator SDR_t() const { return input; }
//     bool operator <(const SDR_t o) const { return input < o; }
//     friend std::ostream& operator<<(std::ostream& os, const InputConnection& ic) {
//         os << ic.input << "/" << *ic.permanence;
//         return os;
//     }
// };

int main() {
  SDRMemory<int, int> mem;
  mem.write(SDR<int>(0.5, 3, 100), 5);
  // std::cerr << (a < b) << '\n';
  // SDRMemory
  // SDRGraph g;
  // std::cerr << "cake yep" << std::endl;
  // SDR<> a{0, 1, 2, 3, 4, 5, 8};
  // SDR<> b{1, 3, 6, 7, 8, 9};
  // std::cerr << a.andp(b) << std::endl;
  // SDR<InputConnection> a;
  // a.push_back(InputConnection{0,0.5});
  // a.push_back(InputConnection{1,0.1});

  // SDR<int> b{0, 2};
  // std::cerr << a.andb(b) << std::endl;
  // separate(a, b);
  // cout << a << endl << b << endl;
  // SDRGraph g;
  // SDRGraph::Vertex v(r);
  // cout << v << endl;
  // Tester t0{1};
  // Tester t1{t0};
  // funct(t1);
  // adjacency_list<> a;
  // cout << num_vertices(a);
}