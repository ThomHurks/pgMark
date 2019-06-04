#include "group_ref_exists.h"

const std::shared_ptr<const Subpattern> GroupRefExists::getItemYes() const {
    return m_ItemYes;
}

const std::shared_ptr<const Subpattern> GroupRefExists::getItemNo() const {
    return m_ItemNo;
}

int GroupRefExists::minNrCharacters() const {
    return std::min(m_ItemYes->minNrCharacters(), m_ItemNo->minNrCharacters());
}

int GroupRefExists::maxNrCharacters() const {
    return std::max(m_ItemYes->maxNrCharacters(), m_ItemNo->maxNrCharacters());
}
