#include "SDRGraph.hpp"

int main() {
  SDR<int> a{1, 2, 3, 5, 6, 7};
  SDR<int> b{};
  separate(a, b);
  cout << a << endl << b << endl;
  // SDRGraph g;
  // SDRGraph::Vertex v(r);
  // cout << v << endl;
  // Tester t0{1};
  // Tester t1{t0};
  // funct(t1);
  // adjacency_list<> a;
  // cout << num_vertices(a);
}