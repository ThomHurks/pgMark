#ifndef PGMARK_GROUP_REF_H
#define PGMARK_GROUP_REF_H

#include "opcode.h"

class GroupRef : public Opcode {
private:
    const int m_GroupId;
    const State &m_State;
public:
    GroupRef(const int a_GroupId, State &a_State) : Opcode("GROUPREF"), m_GroupId(a_GroupId), m_State(a_State) {}

    GroupRef(const GroupRef &a_Other);

    GroupRef &operator=(GroupRef a_Copy);

    bool operator==(const GroupRef &a_Other);

    bool operator!=(const GroupRef &a_Other);

    friend void swap(GroupRef &a_Self, GroupRef &a_Other);

    int getGroupId() const;

    int minNrCharacters() const override;

    int maxNrCharacters() const override;
};

#endif //PGMARK_GROUP_REF_H
