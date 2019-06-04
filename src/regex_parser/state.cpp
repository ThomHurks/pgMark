#include "state.h"
#include "subpattern.h"
#include <sstream>

State::State(std::string a_String, const int a_Flags) : m_String(std::move(a_String)), m_Flags(a_Flags) {}

int State::groups() const {
    return static_cast<int>(m_GroupWidths.size());
}

int State::openGroup(const std::wstring &a_Name) {
    int gid = groups();
    // TODO(thom): maybe make the pair optional? In python they insert None here.
    m_GroupWidths.emplace_back(std::make_pair(-1, -1));
    if (groups() > MAXGROUPS) {
        throw std::length_error("Too many groups.");
    }
    if (!a_Name.empty()) {
        int ogid = 0;
        bool group_redefined = true;
        try {
            ogid = m_GroupDict.at(a_Name);
        } catch (std::out_of_range &) {
            group_redefined = false;
            m_GroupDict[a_Name] = gid;
        }
        if (group_redefined) {
            //std::stringstream ss;
            //ss << "Redefinition of group name " << a_Name << " as group " << gid << "; was group " << ogid;
            //throw std::invalid_argument(ss.str());
            throw std::invalid_argument("Redefinition of group name.");
        }
    }
    return gid;
}

void State::closeGroup(const int a_GID, Subpattern &a_Subpattern) {
    assert(a_GID >= 0);
    m_GroupWidths[static_cast<size_t>(a_GID)] = a_Subpattern.getwidth();
}

bool State::checkGroup(const int a_GID) {
    assert(a_GID >= 0);
    const auto index = static_cast<size_t>(a_GID);
    return index < m_GroupWidths.size() && (m_GroupWidths[index].first != -1 || m_GroupWidths[index].second != -1);
}

void State::checkLookBehindGroup(const int a_GID) {
    assert(a_GID >= 0);
    if (m_LookBehindGroups != -1) {
        if (!checkGroup(a_GID)) {
            throw std::invalid_argument("Cannot refer to an open group.");
        }
        if (a_GID >= m_LookBehindGroups) {
            throw std::invalid_argument("Cannot refer to group defined in the same lookbehind pattern.");
        }
    }
}

int State::getGroup(const std::wstring &a_Name) {
    return m_GroupDict.at(a_Name);
}

void State::fixFlags() {
    if (m_Flags & SRE_FLAG_LOCALE) {
        throw std::invalid_argument("Cannot use LOCALE flag with a string pattern");
    }
    if (!(m_Flags & SRE_FLAG_ASCII)) {
        m_Flags |= SRE_FLAG_UNICODE;
    } else if (m_Flags & SRE_FLAG_UNICODE) {
        throw std::invalid_argument("ASCII and UNICODE flags are incompatible");
    }
}

int State::getLookBehindGroups() const {
    return m_LookBehindGroups;
}

void State::setLookBehindGroups(const int a_LookBehindGroups) {
    m_LookBehindGroups = a_LookBehindGroups;
}

const std::pair<int, int> &State::getGroupWidth(int a_groupIndex) const {
    assert(a_groupIndex >= 0);
    auto groupIndex = static_cast<size_t>(a_groupIndex);
    assert(groupIndex < m_GroupWidths.size());
    return m_GroupWidths[groupIndex];
}
