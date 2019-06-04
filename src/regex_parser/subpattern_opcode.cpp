#include "subpattern_opcode.h"

int SubpatternOpcode::getGroup() const {
    return m_Group;
}

int SubpatternOpcode::getAddFlags() const {
    return m_AddFlags;
}

int SubpatternOpcode::getDelFlags() const {
    return m_DelFlags;
}

const std::shared_ptr<const Subpattern>& SubpatternOpcode::getSubpattern() const {
    return m_Subpattern;
}

int SubpatternOpcode::minNrCharacters() const {
    return m_Subpattern->minNrCharacters();
}

int SubpatternOpcode::maxNrCharacters() const {
    return m_Subpattern->maxNrCharacters();
}
