#ifndef REGEX_SUBPATTERN_H
#define REGEX_SUBPATTERN_H

#include "state.h"
#include "opcode.h"
#include <tuple>
#include <vector>
#include <unordered_set>
#include <memory>

class Opcode;

class Subpattern {
private:
    const int MAXREPEAT = 999; // TODO: find value and deduplicate.
    const int MAXGROUPS = 999; // TODO: find value and deduplicate.
    State &m_State;
    std::vector<std::shared_ptr<const Opcode>> m_Data;
public:
    explicit Subpattern(State &a_State);

    Subpattern(State &a_State, const std::shared_ptr<const Opcode> &a_Opcode);

    Subpattern(State &a_State, std::vector<std::shared_ptr<const Opcode>> &a_Data);

    Subpattern(const Subpattern &a_Other);

    Subpattern &operator=(Subpattern a_Copy);

    bool operator==(const Subpattern &a_Other);

    bool operator!=(const Subpattern &a_Other);

    friend void swap(Subpattern &a_Self, Subpattern &a_Other) {
        std::swap(a_Self.m_State, a_Other.m_State);
        std::swap(a_Self.m_Data, a_Other.m_Data);
    }

    const std::pair<int, int> getwidth() const;

    void append(std::shared_ptr<const Opcode> a_Prefix);

    int length() const;

    std::shared_ptr<const Opcode> getItem(int a_Index) const;

    void setItem(int a_Index, const std::shared_ptr<const Opcode> &a_Item);

    void deleteItem(int a_Index);

    void insertItem(int a_Index, const std::shared_ptr<const Opcode> &a_Item);

    int minNrCharacters() const;

    int maxNrCharacters() const;
};

#endif //REGEX_SUBPATTERN_H
