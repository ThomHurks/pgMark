#ifndef PGMARK_GROUP_REF_EXISTS_H
#define PGMARK_GROUP_REF_EXISTS_H

#include "opcode.h"

class GroupRefExists : public Opcode {
private:
    int m_CondGroup;
    const std::shared_ptr<const Subpattern> m_ItemYes;
    const std::shared_ptr<const Subpattern> m_ItemNo;
public:
    GroupRefExists(int a_CondGroup, const std::shared_ptr<const Subpattern>& a_ItemYes, const std::shared_ptr<const Subpattern>& a_ItemNo)
            : Opcode("GROUPREFEXISTS"),
              m_CondGroup(a_CondGroup),
              m_ItemYes(a_ItemYes),
              m_ItemNo(a_ItemNo) {
        assert(m_ItemYes != nullptr);
        assert(m_ItemNo != nullptr);
    }

    GroupRefExists(const GroupRefExists &a_Other);

    GroupRefExists &operator=(GroupRefExists a_Copy);

    bool operator==(const GroupRefExists &a_Other);

    bool operator!=(const GroupRefExists &a_Other);

    friend void swap(GroupRefExists &a_Self, GroupRefExists &a_Other);

    const std::shared_ptr<const Subpattern> getItemYes() const;

    const std::shared_ptr<const Subpattern> getItemNo() const;

    int minNrCharacters() const override;

    int maxNrCharacters() const override;
};


#endif //PGMARK_GROUP_REF_EXISTS_H
