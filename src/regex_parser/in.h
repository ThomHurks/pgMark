#ifndef PGMARK_IN_H
#define PGMARK_IN_H

#include "opcode.h"
#include <memory>

class In : public Opcode {
private:
    const std::vector<std::shared_ptr<const Opcode>> m_Args;
public:
    In() : Opcode("IN") {}

    explicit In(const std::vector<std::shared_ptr<const Opcode>> &a_Args) : Opcode("IN"), m_Args(a_Args) {}

    explicit In(std::shared_ptr<const Opcode> a_Opcode) : Opcode("IN"), m_Args({std::move(a_Opcode)}) {}

    In(const In &a_Other);

    In &operator=(In a_Copy);

    bool operator==(const In &a_Other);

    bool operator!=(const In &a_Other);

    friend void swap(In &a_Self, In &a_Other);

    const std::shared_ptr<const Opcode>& getItem(int a_Index) const;

    const std::vector<std::shared_ptr<const Opcode>> &getArgs() const;

    int length() const;

    int minNrCharacters() const override;

    int maxNrCharacters() const override;
};

#endif //PGMARK_IN_H
