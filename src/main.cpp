#include "SDRGraph.hpp"
#include "SpatialPooler.hpp"

struct InputConnection {
    int input;
    float permanence;
    operator int() const { return input; }
    bool operator <(const int o) const { return input < o; }
    friend std::ostream& operator<<(std::ostream& os, const InputConnection& ic);
};

std::ostream& operator<<(std::ostream& os, const InputConnection& ic)
{
    os << ic.input << "/" << ic.permanence;
    return os;
}


int main() {
  SDR<InputConnection> a;
  a.push_back(InputConnection{0,0.5});
  a.push_back(InputConnection{1,0.1});

  SDR<int> b{0, 2};
  std::cerr << a.andb(b) << std::endl;
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