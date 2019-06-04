#include "graph_generator.h"

std::vector<int>
generateNodeDistributions(RandomDistribution *const a_Distribution, const int a_StartId, const int a_EndId) {
    std::vector<int> nodes;
    int amount = a_EndId - a_StartId + 1;
    assert(amount > 0);
    nodes.reserve(
            static_cast<size_t>(static_cast<double>(amount) *
                                a_Distribution->getMean())); // Maybe use confidence interval?

    for (int node_id = a_StartId; node_id <= a_EndId; ++node_id) {
        int nr_relations = a_Distribution->getRandomInteger();
        for (int i = 0; i < nr_relations; ++i) {
            nodes.push_back(node_id);
        }
    }
    return nodes;
}

void GraphGenerator::generateRandomEdges(const RelationDistribution &a_Relation,
                                         std::ostream &a_OutputStream) const {
    const auto &source_range = m_Config.getTypeRange(a_Relation.getSource());
    const auto &target_range = m_Config.getTypeRange(a_Relation.getTarget());
    const std::string &predicate = a_Relation.getPredicate();
    const bool loops_allowed = a_Relation.getLoopsAreAllowed();
    const bool parallel_edges_allowed = a_Relation.getParallelEdgesAreAllowed();

    std::vector<int> source_nodes = generateNodeDistributions(a_Relation.getOutDistribution(), source_range.first,
                                                              source_range.second);
    std::vector<int> target_nodes = generateNodeDistributions(a_Relation.getInDistribution(), target_range.first,
                                                              target_range.second);
    const size_t nr_edges = std::min(source_nodes.size(), target_nodes.size());
    if (nr_edges == 0) {
        return;
    }
    std::mt19937 generator{std::random_device{}()};
    const bool sources_are_shuffled = source_nodes.size() >= target_nodes.size();
    auto &shuffled_nodes = sources_are_shuffled ? source_nodes : target_nodes;
    const auto &fixed_nodes = sources_are_shuffled ? target_nodes : source_nodes;
    shuffle(shuffled_nodes.begin(), shuffled_nodes.end(), generator);
    const auto *fixed_nodes_distribution = sources_are_shuffled ? a_Relation.getInDistribution()
                                                                : a_Relation.getOutDistribution();
    const auto expected_fixed_nodes = static_cast<size_t>(std::ceil(fixed_nodes_distribution->getMean()));
    std::unordered_set<int> shuffled_nodes_seen(expected_fixed_nodes);
    int lastFixed = fixed_nodes[0];
    for (size_t i = 0; i < nr_edges; ++i) {
        const int fixed = fixed_nodes[i];
        const int shuffled = shuffled_nodes[i];
        if (!parallel_edges_allowed) {
            if (fixed != lastFixed) {
                lastFixed = fixed;
                shuffled_nodes_seen.clear();
            } else if (shuffled_nodes_seen.find(shuffled) != shuffled_nodes_seen.end()) {
                continue;
            }
            shuffled_nodes_seen.insert(shuffled);
        }
        if (loops_allowed || shuffled != fixed) {
            if (sources_are_shuffled) {
                writeEdge(shuffled, fixed, predicate, a_OutputStream);
            } else {
                writeEdge(fixed, shuffled, predicate, a_OutputStream);
            }
        }
    }
}

void GraphGenerator::generateGraph(std::ostream &a_OutputStream) {
    a_OutputStream << "### NODE RELATIONS ###" << "\n";
    for (const auto &relation : m_Config.getRelationDistributions()) {
        generateRandomEdges(relation, a_OutputStream);
    }
}

GraphGenerator::GraphGenerator(const Configuration &a_Config)
        : m_Config(a_Config) {}
