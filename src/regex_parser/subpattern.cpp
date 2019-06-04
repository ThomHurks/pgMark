#include "subpattern.h"
#include "branch.h"
#include "character_sets.h"
#include "group_ref.h"
#include "group_ref_exists.h"
#include "max_repeat.h"
#include "min_repeat.h"
#include "subpattern_opcode.h"

Subpattern::Subpattern(State &a_State) : m_State(a_State) {
    // TODO(thom): .
}

Subpattern::Subpattern(State &a_State, std::vector<std::shared_ptr<const Opcode>> &a_Data)
        : m_State(a_State), m_Data(a_Data) {
    // TODO(thom): .
}

Subpattern::Subpattern(State &a_State, const std::shared_ptr<const Opcode> &a_Opcode) : m_State(a_State), m_Data({a_Opcode}) {
    // TODO(thom): .
}

const std::pair<int, int> Subpattern::getwidth() const {
    // Determine the width (min, max) for this subpattern.
    int lo = 0;
    int hi = 0;
    for (const auto& opcode : m_Data) {
        const std::string &opcodeName = opcode->getName();
        if (opcodeName == "BRANCH") {
            int i = MAXREPEAT - 1;
            int j = 0;
            const auto branch = std::dynamic_pointer_cast<const Branch>(opcode);
            for (const auto& subpattern : branch->getArgs()) {
                i = std::min(i, subpattern->getwidth().first);
                j = std::max(j, subpattern->getwidth().second);
            }
            lo += i;
            hi += j;
        } else if (opcodeName == "SUBPATTTERN") {
            const auto subpattern = std::dynamic_pointer_cast<const SubpatternOpcode>(opcode);
            lo += subpattern->getSubpattern()->getwidth().first;
            hi += subpattern->getSubpattern()->getwidth().second;
        } else if (CharacterSets::REPEATCODES.find(opcodeName) != CharacterSets::REPEATCODES.end()) {
            if (opcodeName == "MIN_REPEAT") {
                const auto min_repeat = std::dynamic_pointer_cast<const MinRepeat>(opcode);
                int i = min_repeat->getSubpattern()->getwidth().first;
                int j = min_repeat->getSubpattern()->getwidth().second;
                lo += i * min_repeat->getMin();
                hi += j * min_repeat->getMax();
            } else if (opcodeName == "MAX_REPEAT") {
                const auto max_repeat = std::dynamic_pointer_cast<const MaxRepeat>(opcode);
                int i = max_repeat->getSubpattern()->getwidth().first;
                int j = max_repeat->getSubpattern()->getwidth().second;
                lo += i * max_repeat->getMin();
                hi += j * max_repeat->getMax();
            } else {
                throw std::invalid_argument("Invalid REPEAT opcode.");
            }
        } else if (CharacterSets::UNITCODES.find(opcodeName) != CharacterSets::UNITCODES.end()) {
            lo += 1;
            hi += 1;
        } else if (opcodeName == "GROUPREF") {
            const auto groupref = std::dynamic_pointer_cast<const GroupRef>(opcode);
            auto pair = m_State.getGroupWidth(groupref->getGroupId());
            lo += pair.first;
            hi += pair.second;
        } else if (opcodeName == "GROUPREF_EXISTS") {
            const auto groupref_exists = std::dynamic_pointer_cast<const GroupRefExists>(opcode);
            int i = groupref_exists->getItemYes()->getwidth().first;
            int j = groupref_exists->getItemYes()->getwidth().second;
            int l = groupref_exists->getItemNo()->getwidth().first;
            int h = groupref_exists->getItemNo()->getwidth().second;
            if (l != -1 && h != -1) {
                i = std::min(i, l);
                j = std::max(j, h);
            } else {
                i = 0;
            }
            lo += i;
            hi += j;
        }
    }
    return std::pair<int, int>(std::min(lo, MAXREPEAT - 1), std::min(hi, MAXREPEAT));
}

void Subpattern::append(std::shared_ptr<const Opcode> a_Prefix) {
    m_Data.emplace_back(std::move(a_Prefix));
}

std::shared_ptr<const Opcode> Subpattern::getItem(const int a_Index) const {
    // TODO(thom): what if a_Index is a slice?
    assert(a_Index >= 0);
    return m_Data.at(static_cast<size_t>(a_Index));
}

void Subpattern::setItem(const int a_Index, const std::shared_ptr<const Opcode> &a_Item) {
    assert(a_Index >= 0);
    const auto index = static_cast<size_t>(a_Index);
    assert(index < m_Data.size());
    m_Data[index] = a_Item;
}

void Subpattern::insertItem(const int a_Index, const std::shared_ptr<const Opcode> &a_Item) {
    assert(a_Index >= 0);
    assert(a_Index <= static_cast<int>(m_Data.size()));
    m_Data.insert(m_Data.begin() + a_Index, a_Item);
}

int Subpattern::length() const {
    return static_cast<int>(m_Data.size());
}

void Subpattern::deleteItem(const int a_Index) {
    assert(a_Index >= 0);
    m_Data.erase(m_Data.begin() + a_Index);
}

int Subpattern::minNrCharacters() const {
    int min = 0;
    for (const auto& opcode : m_Data) {
        min += opcode->minNrCharacters();
    }
    return min;
}

int Subpattern::maxNrCharacters() const {
    int max = 0;
    for (const auto& opcode : m_Data) {
        max += opcode->maxNrCharacters();
    }
    return max;
}
