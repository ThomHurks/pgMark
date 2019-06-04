#include "in.h"

const std::shared_ptr<const Opcode>& In::getItem(const int a_Index) const {
    assert(a_Index >= 0);
    return m_Args.at(static_cast<size_t>(a_Index));
}

const std::vector<std::shared_ptr<const Opcode>> &In::getArgs() const {
    return m_Args;
}

int In::length() const {
    return static_cast<int>(m_Args.size());
}

int In::minNrCharacters() const {
    int min = std::numeric_limits<int>::max();
    for (auto& opcode : m_Args) {
        min = std::min(min, opcode->minNrCharacters());
    }
    return min;
}

int In::maxNrCharacters() const {
    int max = std::numeric_limits<int>::min();
    for (auto& opcode : m_Args) {
        max = std::max(max, opcode->maxNrCharacters());
    }
    return max;
}
