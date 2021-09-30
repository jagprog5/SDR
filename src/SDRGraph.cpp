#include "SDRGraph.hpp"
#include <iomanip>
#include <limits>

static inline std::string colorcode(std::string code) {
    #ifdef _WIN32
        return "";
    #else
        return code;
    #endif
}

std::ostream& operator<<(std::ostream& os, const SDRGraph& graph) {
    // SDRGraph::VertexIterator vpos, vend;
    // for (tie(vpos, vend) = vertices(graph); vpos != vend; ++vpos) {
    //     Vertex v = graph[*vpos];
    //     os << colorcode("\033[32m") << uppercase << setw(4) << setfill('0') << hex << (std::hash<Vertex::AbstractionType>{}(v.get_abstraction()) & 0xFFFF)
    //         << dec << colorcode("\033[37m") << "*." << setw(2) << (int)(v.get_activity() * 100) << ":";
    //     SDRGraph::OutEdgeIterator epos, eend;
    //     tie(epos, eend) = out_edges(*vpos, graph);
    //     for (; epos != eend; ++epos) {
    //         Edge e = graph[*epos];
    //         Vertex av = graph[target(*epos, graph)];
    //         os << colorcode("\033[32m") << uppercase << setw(4) << hex << (std::hash<Vertex::AbstractionType>{}(av.get_abstraction()) & 0xFFFF) << dec << colorcode("\033[37m");
    //         os << e.get_attention();
    //         if (boost::next(epos) != eend) os << ",";
    //     }
    //     if (boost::next(vpos) != vend) os << endl;
    // }
    return os;
}

SDRGraph::SDRGraph() {
    // The first vertex is special.
    // It is always active. Propagation of activity starts from it.
    boost::add_vertex(Vertex(std::numeric_limits<Vertex::AbstractionType>::min(), 1), graph);
}

void SDRGraph::update_vertex(VertexDescriptor vd) {
    /*
    Check the overlap between the state and each out edge, and
    increase the activity of each out vertice based on the overlap score.
    If a out vertex is sufficiently active, then its id gets added to the state.
    If a out vertex is sufficiently very active, then it is added to the update queue.
    */
    OutEdgeIterator epos, eend;
    tie(epos, eend) = out_edges(vd, graph);
    bool handled = false;
    for (; epos != eend; ++epos) {
        const SDR<>& edge = graph[*epos];
        float score = (float)state.ands(edge) / edge.size();
        VertexDescriptor target_vd = boost::target(*epos, graph);
        Vertex& target = graph[target_vd];
        target.increase_activity(score);
        if (target.get_activity() > STATE_THRESHOLD) {
            handled = true;
            state.set(target.get_abstraction(), true);
        }
        if (!target.get_queued()) {
            if (target.get_activity() > ACTIVE_THRESHOLD) {
                target.set_queued(true);
                vertices_to_process.push(target_vd);
            }
        }
    }
    if (handled) { // The vertex propagated it's activity to something else
        graph[vd].set_activity(0);
    } else {
        
    }
    // TODO decrease activity  HERE
    
    /*
    If this vertex can't handle the current state, meaning:
        - No out edges have sufficient overlap with the state
    and the graph can't handle the state either, meaning:
        - No vertices 
    */
}

void SDRGraph::update(SDR<> inputs) {
    state.set(inputs, true);
    VertexDescriptor vd = *graph.vertex_set().begin();
    graph[vd].set_activity(1);
    while (true) {
        update_vertex(vd);
        if (vertices_to_process.empty()) {
            break;
        } else {
            vd = vertices_to_process.top();
            vertices_to_process.pop();
        }
    }
    state.set(inputs, false);
}

// void SDRGraph::update(SDR<> inputs) {
//     state.set(inputs, true);
//     unsigned int prop_count = 0;
//     VertexDescriptor vd = *graph.vertex_set().begin();
//     while (prop_count++ < prop_count_max) {
//         OutEdgeIterator epos, eend;
//         tie(epos, eend) = out_edges(vd, graph);
//         for (; epos != eend; ++epos) {
//             const SDR<>& edge = graph[*epos];
//             float score = (float)state.ands(edge) / edge.size();
//             Vertex& target = graph[boost::target(*epos, graph)];
//             target.increase_activity(score);
//         }
//         if (!active_vertices.empty()) {
//             vd = active_vertices.top();
//             active_vertices.pop();
//         } else {

//         }
//     }

//     // create a new vertex, connected to the current state
    


//     // propagate through the graph
//     SDR<> unresolved_states;
//     std::vector<VertexDescriptor> unresolved_vertices;
    


//     // the unresolved states are connected through a single edge to a new vertex
//     VertexDescriptor new_vertex = boost::add_vertex(Vertex(), graph);
//     for (auto& elem : unresolved_vertices) {
//         boost::add_edge(elem, new_vertex, Edge(unresolved_states), graph);
//     }
//     state.set(inputs, false);
// }