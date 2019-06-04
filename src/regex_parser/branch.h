#ifndef PGMARK_BRANCH_H
#define PGMARK_BRANCH_H

#include "opcode.h"

class Branch : public Opcode {
private:
    const std::vector<std::shared_ptr<Subpattern>> m_Args;
public:
    explicit Branch(const std::vector<std::shared_ptr<Subpattern>> &a_Items) : Opcode("BRANCH"), m_Args(a_Items) {}

    Branch(const Branch &a_Other);

    Branch &operator=(Branch a_Copy);

    bool operator==(const Branch &a_Other);

    bool operator!=(const Branch &a_Other);

    friend void swap(Branch &a_Self, Branch &a_Other);

    const std::vector<std::shared_ptr<Subpattern>> &getArgs() const;

    const std::shared_ptr<Subpattern>& getItem(const int a_Index) const;

    int length() const;

    int minNrCharacters() const override;

    int maxNrCharacters() const override;
};

#endif //PGMARK_BRANCH_H
