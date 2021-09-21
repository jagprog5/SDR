#include "SDRGraph.hpp"

static inline string colorcode(string code) {
    #ifdef _WIN32
        return "";
    #else
        return code;
    #endif
}

ostream& operator<<(ostream& os, const SDRGraph& graph) {
    SDRGraph::VertexIterator vpos, vend;
    for (tie(vpos, vend) = vertices(graph); vpos != vend; ++vpos) {
        Vertex v = graph[*vpos];
        os << colorcode("\033[32m") << uppercase << setw(4) << setfill('0') << hex << (std::hash<Vertex::AbstractionType>{}(v.get_abstraction()) & 0xFFFF)
            << dec << colorcode("\033[37m") << "*." << setw(2) << (int)(v.get_activity() * 100) << ":";
        SDRGraph::OutEdgeIterator epos, eend;
        tie(epos, eend) = out_edges(*vpos, graph);
        for (; epos != eend; ++epos) {
            Edge e = graph[*epos];
            Vertex av = graph[target(*epos, graph)];
            os << colorcode("\033[32m") << uppercase << setw(4) << hex << (std::hash<Vertex::AbstractionType>{}(av.get_abstraction()) & 0xFFFF) << dec << colorcode("\033[37m");
            os << e.get_attention();
            if (boost::next(epos) != eend) os << ",";
        }
        if (boost::next(vpos) != vend) os << endl;
    }
    return os;
}
