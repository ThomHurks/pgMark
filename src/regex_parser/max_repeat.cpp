#include "max_repeat.h"

int MaxRepeat::getMin() const {
    return m_Min;
}

int MaxRepeat::getMax() const {
    return m_Max;
}

const std::shared_ptr<const Subpattern> MaxRepeat::getSubpattern() const {
    return m_Subpattern;
}

int MaxRepeat::minNrCharacters() const {
    return m_Min * m_Subpattern->minNrCharacters();
}

int MaxRepeat::maxNrCharacters() const {
    return m_Max * m_Subpattern->maxNrCharacters();
}
