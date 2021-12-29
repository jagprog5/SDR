#include <boost/graph/adjacency_list.hpp>
#include <queue>
#include "SparseDistributedRepresentation.hpp"

class Vertex {
    public:
        using AbstractionType = SDR<>::index_type;
        Vertex() : abstraction{SDR<>::get_random_number()}, activity(0) {}
        Vertex(AbstractionType abstraction) : abstraction{abstraction}, activity(0) {}
        Vertex(AbstractionType abstraction, float activity) : abstraction{abstraction}, activity(activity) {}
    private:
        AbstractionType abstraction; // serves as an index in the state, but also as a lazy uuid
        float activity; // from 0 to 1; higher activity means a vertex has higher priority to be updated
        bool queued = false;
    public:
        bool operator<(Vertex& other) { return activity < other.activity; }
        AbstractionType get_abstraction() const { return abstraction; }
        float get_activity() const { return activity; }
        void set_activity(float a) { activity = a; }
        void increase_activity(float other_activity) { activity += (1 - activity) * other_activity; }
        void decrease_activity(float other_activity) { activity *= other_activity; }
        bool get_queued() const { return queued; }
        void set_queued(bool q) { queued = q; }
};

class Edge {
    public:
        Edge(SDR<> sdr) { attention = std::move(sdr); }
        operator SDR<>() { return attention; }
    private:
        SDR<> attention;
};

class SDRGraph {
    public:
        using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Vertex, Edge>;
        using AdjacencyIterator = boost::graph_traits<Graph>::adjacency_iterator;
        using OutEdgeIterator = boost::graph_traits<Graph>::out_edge_iterator;
        using InEdgeIterator = boost::graph_traits<Graph>::in_edge_iterator;
        using VertexIterator = boost::graph_traits<Graph>::vertex_iterator;
        using EdgeIterator = boost::graph_traits<Graph>::edge_iterator;
        using VertexDescriptor = boost::graph_traits<Graph>::vertex_descriptor;
        using EdgeDescriptor = boost::graph_traits<Graph>::edge_descriptor;

        SDRGraph();
        void update(SDR<> inputs);
        friend std::ostream& operator<<(std::ostream& os, const SDRGraph& graph);

    private:
        // the activity that must be exceed such that a vertex's index appears in the state
        static constexpr float STATE_THRESHOLD = 0.2;
        // the activity that must be exceed such that a vertex is added to the process queue
        // todo this should increase with process queue size
        static constexpr float ACTIVE_THRESHOLD = 0.4;
        // upon failure to propogate a signal, at what rate does a vertex's activity decay
        static constexpr float ACTIVITY_DECAY = 0.1;

        void update_vertex(VertexDescriptor vd);

        Graph graph;
        SDR<> state;
        // It would be impossible to update every single vertices each timestep.
        // This queue hold the most active vertices. This is iterated through instead.
        std::priority_queue<VertexDescriptor> vertices_to_process;
};