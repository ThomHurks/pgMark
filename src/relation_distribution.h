#ifndef GMARK_RELATION_DISTRIBUTION_H
#define GMARK_RELATION_DISTRIBUTION_H

#include <map>
#include <memory>
#include "random_distribution.h"
#include "affinity.h"

class RelationDistribution {
private:
    std::string m_Source;
    std::string m_Target;
    std::string m_Predicate;
    bool m_AllowLoops;
    bool m_AllowParallelEdges;
    std::unique_ptr<RandomDistribution> m_InDistribution;
    std::unique_ptr<RandomDistribution> m_OutDistribution;
    std::map<std::string, Affinity> m_Affinities;
public:
    RelationDistribution(std::string a_Source, std::string a_Target, std::string a_Predicate, bool a_AllowLoops,
                         bool a_AllowParallelEdges,
                         std::unique_ptr<RandomDistribution> a_InDistribution,
                         std::unique_ptr<RandomDistribution> a_OutDistribution,
                         std::map<std::string, Affinity> a_Affinities)
            : m_Source(std::move(a_Source)),
              m_Target(std::move(a_Target)),
              m_Predicate(std::move(a_Predicate)),
              m_AllowLoops(a_AllowLoops),
              m_AllowParallelEdges(a_AllowParallelEdges),
              m_InDistribution(std::move(a_InDistribution)),
              m_OutDistribution(std::move(a_OutDistribution)),
              m_Affinities(std::move(a_Affinities)) {}

    // Move constructor.
    RelationDistribution(RelationDistribution &&a_Other)
            : m_Source(std::move(a_Other.m_Source)),
              m_Target(std::move(a_Other.m_Target)),
              m_Predicate(std::move(a_Other.m_Predicate)),
              m_AllowLoops(std::move(a_Other.m_AllowLoops)),
              m_AllowParallelEdges(std::move(a_Other.m_AllowParallelEdges)),
              m_InDistribution(std::move(a_Other.m_InDistribution)),
              m_OutDistribution(std::move(a_Other.m_OutDistribution)),
              m_Affinities(std::move(a_Other.m_Affinities)) {}

    RelationDistribution(const RelationDistribution &) = delete; // No copying.
    RelationDistribution &operator=(const RelationDistribution &) = delete; // No copying.
    const std::string &getSource() const {
        return m_Source;
    }

    const std::string &getTarget() const {
        return m_Target;
    }

    const std::string &getPredicate() const {
        return m_Predicate;
    }

    bool getLoopsAreAllowed() const {
        return m_AllowLoops;
    }

    bool getParallelEdgesAreAllowed() const {
        return m_AllowParallelEdges;
    }

    RandomDistribution *getInDistribution() const {
        return m_InDistribution.get();
    }

    RandomDistribution *getOutDistribution() const {
        return m_OutDistribution.get();
    }
};

#endif //GMARK_RELATION_DISTRIBUTION_H
