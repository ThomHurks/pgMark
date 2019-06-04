#ifndef GMARK_GRAPH_GENERATOR_H
#define GMARK_GRAPH_GENERATOR_H

#include "configuration.h"

std::vector<int>
generateNodeDistributions(RandomDistribution *const a_Distribution, const int a_StartId, const int a_EndId);

class GraphGenerator {
protected:
    const Configuration &m_Config;

    void generateRandomEdges(const RelationDistribution &a_Relation,
                             std::ostream &a_OutputStream) const;

    void writeEdge(const int a_Source, const int a_Target, const std::string &a_Predicate,
                   std::ostream &a_OutputStream) const {
        // TODO: get unique ID of predicate instead.
        a_OutputStream << a_Source << ',' << a_Predicate << ',' << a_Target << "\n";
    }

public:
    GraphGenerator(const GraphGenerator &) = delete; // no copy operations.
    GraphGenerator &operator=(const GraphGenerator &) = delete; // no copy operations.
    GraphGenerator(GraphGenerator &&) = delete; // no move operations.
    GraphGenerator &operator=(GraphGenerator &&) = delete; // no move operations.
    explicit GraphGenerator(const Configuration &a_Config);

    void generateGraph(std::ostream &a_OutputStream);
};

#endif // GMARK_GRAPH_GENERATOR_H
