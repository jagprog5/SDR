#include <boost/graph/adjacency_list.hpp>
#include <iomanip>
#include <functional>
#include "SparseDistributedRepresentation.h"

using namespace boost;

class SDRGraph {
    private:
        class Vertex; // forward declare
        class Edge;
    public:
        using Graph = adjacency_list<vecS, vecS, directedS, Vertex, Edge>; // edges, vertices stored in vectors, edges are directed
        using AdjacencyIterator = graph_traits<Graph>::adjacency_iterator;
        using OutEdgeIterator = graph_traits<Graph>::out_edge_iterator;
        using InEdgeIterator = graph_traits<Graph>::in_edge_iterator;
        using VertexIterator = graph_traits<Graph>::vertex_iterator;
        using EdgeIterator = graph_traits<Graph>::edge_iterator;
        using VertexDescriptor = graph_traits<Graph>::vertex_descriptor;
        using EdgeDescriptor = graph_traits<Graph>::edge_iterator;

        SDRGraph() {
            // VertexDescriptor v1 = add_vertex(Vertex(), graph);
            // VertexDescriptor v2 = add_vertex(Vertex(), graph);
            // add_edge(v1, v2, Edge(SDR<>{2, 3, 4}), graph);
            // add_edge(v1, v2, Edge(SDR<>{2, 3, 4, 50000}), graph);
            // cout << *this << endl;
            // cout << 100000 << endl;
        }

        friend ostream& operator<<(ostream& os, const SDRGraph& sdrgraph) {
            Graph graph = sdrgraph.graph;
            VertexIterator vpos, vend;
            for (tie(vpos, vend) = vertices(graph); vpos != vend; ++vpos) {
                Vertex v = graph[*vpos];
                os << "\033[32m" << uppercase << setw(4) << setfill('0') << hex << (std::hash<decltype(v.abstraction)>{}(v.abstraction) & 0xFFFF) 
                    << dec << "\033[37m" << "*." << setw(2) << (int)(v.activity * 100) << ":";
                OutEdgeIterator epos, eend;
                tie(epos, eend) = out_edges(*vpos, graph);
                const bool flag = epos == eend;
                for (; epos != eend; epos++) {
                    Edge e = graph[*epos];
                    Vertex av = graph[target(*epos, graph)];
                    os << "\033[32m" << uppercase << setw(4) << hex << (std::hash<decltype(av.abstraction)>{}(av.abstraction) & 0xFFFF) << dec << "\033[37m";
                    os << e.attention;
                    if (epos + 1 != eend) os << ",";
                }
                if (!flag) os << endl;
            }
            return os;
        }

    private:
        struct Vertex {
            Vertex() : abstraction{SDR<>::get_random_indice()}, activity(0) {}
            decltype(SDR<>::get_random_indice()) abstraction; // serves as a index in the state, but also as a lazy uuid
            float activity;
        };

        struct Edge {
            Edge(SDR<> sdr) { attention = std::move(sdr); }
            Edge() : Edge(SDR<>()) {}
            SDR<> attention;
        };

        SDR<> state;
        Graph graph;
};