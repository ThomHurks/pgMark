#include "min_repeat.h"

int MinRepeat::getMin() const {
    return m_Min;
}

int MinRepeat::getMax() const {
    return m_Max;
}

const std::shared_ptr<const Subpattern>& MinRepeat::getSubpattern() const {
    return m_Subpattern;
}

int MinRepeat::minNrCharacters() const {
    return m_Min * m_Subpattern->minNrCharacters();
}

int MinRepeat::maxNrCharacters() const {
    return m_Max * m_Subpattern->maxNrCharacters();
}
