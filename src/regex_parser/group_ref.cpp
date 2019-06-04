#include "group_ref.h"

int GroupRef::getGroupId() const {
    return m_GroupId;
}

int GroupRef::minNrCharacters() const {
    return m_State.getGroupWidth(m_GroupId).first;
}

int GroupRef::maxNrCharacters() const {
    return m_State.getGroupWidth(m_GroupId).second;
}
