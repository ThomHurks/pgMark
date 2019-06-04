#include "branch.h"

const std::vector<std::shared_ptr<Subpattern>> &Branch::getArgs() const {
    return m_Args;
}

const std::shared_ptr<Subpattern>& Branch::getItem(const int a_Index) const {
    assert(a_Index >= 0);
    return m_Args.at(static_cast<size_t>(a_Index));
}

int Branch::length() const {
    return static_cast<int>(m_Args.size());
}

int Branch::minNrCharacters() const {
    int min = std::numeric_limits<int>::max();
    for (const auto &subpattern : m_Args) {
        min = std::min(min, subpattern->minNrCharacters());
    }
    return min;
}

int Branch::maxNrCharacters() const {
    int max = std::numeric_limits<int>::min();
    for (const auto &subpattern : m_Args) {
        max = std::max(max, subpattern->maxNrCharacters());
    }
    return max;
}
