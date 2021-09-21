#include <boost/graph/adjacency_list.hpp>
#include <iomanip>
#include "SparseDistributedRepresentation.hpp"

using namespace boost;

class Vertex {
    public:
        using AbstractionType = SDR<>::IndexType;
        Vertex() : abstraction{SDR<>::get_random_indice()}, activity(0) {}
        Vertex(float activity) : abstraction{SDR<>::get_random_indice()}, activity(activity) {}
        Vertex(AbstractionType abstraction) : abstraction{SDR<>::get_random_indice()}, activity(activity) {}
        Vertex(AbstractionType abstraction, float activity) : abstraction{abstraction}, activity(activity) {}
        AbstractionType get_abstraction() const { return abstraction; }
        float get_activity() const { return activity; }
    private:
        AbstractionType abstraction; // serves as a index in the state, but also as a lazy uuid
        float activity;
};

class Edge {
    public:
        Edge(SDR<> sdr) { attention = std::move(sdr); }
        Edge() : Edge(SDR<>()) {}
        SDR<> get_attention() const { return attention; }
    private:
        SDR<> attention;
};

class SDRGraph: public adjacency_list<vecS, vecS, directedS, Vertex, Edge> {
    public:
        using AdjacencyIterator = graph_traits<SDRGraph>::adjacency_iterator;
        using OutEdgeIterator = graph_traits<SDRGraph>::out_edge_iterator;
        using InEdgeIterator = graph_traits<SDRGraph>::in_edge_iterator;
        using VertexIterator = graph_traits<SDRGraph>::vertex_iterator;
        using EdgeIterator = graph_traits<SDRGraph>::edge_iterator;
        using VertexDescriptor = graph_traits<SDRGraph>::vertex_descriptor;
        using EdgeDescriptor = graph_traits<SDRGraph>::edge_descriptor;

        friend ostream& operator<<(ostream& os, const SDRGraph& graph);

        SDRGraph() {
            VertexDescriptor v1 = add_vertex(Vertex(), *this);
            VertexDescriptor v2 = add_vertex(Vertex(), *this);
            add_edge(v1, v2, Edge(SDR<>{2, 3, 4}), *this);
            add_edge(v1, v2, Edge(SDR<>{2, 3, 4, 50000}), *this);
            cout << *this << endl;
            cout << "test" << endl;
        }

    private:
        unsigned int outVertexOverlap(VertexDescriptor a, VertexDescriptor b, unsigned int depth);
        
        SDR<> state;
};