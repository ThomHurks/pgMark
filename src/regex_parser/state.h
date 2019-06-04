#ifndef REGEX_STATE_H
#define REGEX_STATE_H

#include <vector>
#include <unordered_map>
#include <string>

class Subpattern;

class State {
private:
    const int SRE_FLAG_LOCALE = 2;
    const int SRE_FLAG_ASCII = 4;
    const int SRE_FLAG_UNICODE = 8;
    std::string m_String;
    int MAXGROUPS = 1000;
    int m_Flags = 0;
    std::unordered_map<std::wstring, int> m_GroupDict = {};
    std::vector<std::pair<int, int>> m_GroupWidths = {}; // [None] = group 0.
    int m_LookBehindGroups = -1; // None
public:
    State(std::string a_String, const int a_Flags);

    State(const State &a_Other) = default;

    State &operator=(State a_Copy) {
        std::swap(*this, a_Copy);
        return *this;
    }

    friend void swap(State &a_Self, State &a_Other) {
        std::swap(a_Self.m_String, a_Other.m_String);
        std::swap(a_Self.MAXGROUPS, a_Other.MAXGROUPS);
        std::swap(a_Self.m_Flags, a_Other.m_Flags);
        std::swap(a_Self.m_GroupDict, a_Other.m_GroupDict);
        std::swap(a_Self.m_GroupWidths, a_Other.m_GroupWidths);
        std::swap(a_Self.m_LookBehindGroups, a_Other.m_LookBehindGroups);
    }

    int groups() const;

    int openGroup(const std::wstring &a_Name = L"");

    void closeGroup(const int a_GID, Subpattern &a_Subpattern);

    bool checkGroup(const int a_GID);

    int getGroup(const std::wstring &a_Name);

    void checkLookBehindGroup(const int a_GID);

    void fixFlags();

    int getLookBehindGroups() const;

    void setLookBehindGroups(const int a_LookBehindGroups);

    const std::pair<int, int> &getGroupWidth(int a_groupIndex) const;
};

#endif //REGEX_STATE_H
