#ifndef PGMARK_SUBPATTERN_OPCODE_H
#define PGMARK_SUBPATTERN_OPCODE_H

#include "opcode.h"

class SubpatternOpcode : public Opcode {
private:
    int m_Group;
    int m_AddFlags;
    int m_DelFlags;
    const std::shared_ptr<const Subpattern> m_Subpattern;
public:
    SubpatternOpcode(int a_Group, int a_AddFlags, int a_DelFlags, const std::shared_ptr<const Subpattern> &a_Subpattern)
            : Opcode("SUBPATTERN"),
              m_Group(a_Group),
              m_AddFlags(a_AddFlags),
              m_DelFlags(a_DelFlags),
              m_Subpattern(a_Subpattern) {}

    SubpatternOpcode(const SubpatternOpcode &a_Other);

    SubpatternOpcode &operator=(SubpatternOpcode a_Copy);

    bool operator==(const SubpatternOpcode &a_Other);

    bool operator!=(const SubpatternOpcode &a_Other);

    friend void swap(SubpatternOpcode &a_Self, SubpatternOpcode &a_Other);

    int getGroup() const;

    int getAddFlags() const;

    int getDelFlags() const;

    const std::shared_ptr<const Subpattern>& getSubpattern() const;

    int minNrCharacters() const override;

    int maxNrCharacters() const override;
};

#endif //PGMARK_SUBPATTERN_OPCODE_H
