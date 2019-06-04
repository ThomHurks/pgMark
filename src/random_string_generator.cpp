#include "random_string_generator.h"
#include "regex_parser/branch.h"
#include "regex_parser/group_ref.h"
#include "regex_parser/max_repeat.h"
#include "regex_parser/min_repeat.h"
#include "regex_parser/sre_parse.h"
#include "regex_parser/subpattern_opcode.h"

RandomStringGenerator::RandomStringGenerator(const std::string &a_Regex, const int a_RepeatLimit)
        : m_Regex(a_Regex),
          m_RepeatLimit(a_RepeatLimit) {
    m_PrintableDistribution = std::uniform_int_distribution<size_t>(0, m_Printable.size() - 1);
    sre_parse parser;
    m_Subpattern = parser.parse(a_Regex);
}

std::string RandomStringGenerator::getRandomString() {
    std::wstring new_string;
    for (int i = 0; i < m_Subpattern->length(); ++i) {
        new_string += handleOpcode(m_Subpattern->getItem(i));
    }
    return m_Converter.to_bytes(new_string);
}

const std::wstring RandomStringGenerator::handleOpcode(const std::shared_ptr<const Opcode> &a_Opcode) {
    const std::string &opcode_name = a_Opcode->getName();
    if (opcode_name == "LITERAL") {
        const auto literal = std::dynamic_pointer_cast<const Literal>(a_Opcode);
        return std::wstring(1, literal->getLiteral());
    }
    if (opcode_name == "NOT_LITERAL") {
        const auto not_literal = std::dynamic_pointer_cast<const NotLiteral>(a_Opcode);
        const auto not_this = not_literal->getLiteral();
        return getRandomPrintableCharacter(not_this);
    }
    if (opcode_name == "AT") {
        return L""; // TODO(thom): How to handle AT?
    }
    if (opcode_name == "IN") {
        const auto in = std::dynamic_pointer_cast<const In>(a_Opcode);
        return handleIn(in);
    }
    if (opcode_name == "ANY") {
        // TODO(thom): Also do not generate other whitespace?
        return getRandomPrintableCharacter('\n');
    }
    if (opcode_name == "RANGE") {
        const auto range = std::dynamic_pointer_cast<const Range>(a_Opcode);
        std::uniform_int_distribution<int> distribution(range->getLow(), range->getHigh());
        int code_point = distribution(m_Generator);
        return std::wstring(1, static_cast<wchar_t>(code_point));
    }
    if (opcode_name == "CATEGORY") {
        const auto category = std::dynamic_pointer_cast<const Category>(a_Opcode);
        auto name = category->getCategory();
        if (name == E_CATEGORY_TYPE::CATEGORY_DIGIT) {
            return getRandomCharacter(m_Digits);
        }
        if (name == E_CATEGORY_TYPE::CATEGORY_NOT_DIGIT) {
            return getRandomCharacter(m_NonDigits);
        }
        if (name == E_CATEGORY_TYPE::CATEGORY_SPACE) {
            return getRandomCharacter(m_Whitespace);
        }
        if (name == E_CATEGORY_TYPE::CATEGORY_NOT_SPACE) {
            return getRandomCharacter(m_NonWhitespace);
        }
        if (name == E_CATEGORY_TYPE::CATEGORY_WORD) {
            return getRandomCharacter(m_Word);
        }
        if (name == E_CATEGORY_TYPE::CATEGORY_NOT_WORD) {
            return getRandomCharacter(m_NonWord);
        }
        throw std::invalid_argument("This category is not supported yet.");
    }
    if (opcode_name == "BRANCH") {
        const auto branch = std::dynamic_pointer_cast<const Branch>(a_Opcode);
        std::uniform_int_distribution<int> distribution(0, branch->length() - 1);
        return handleSubpattern(branch->getItem(distribution(m_Generator)));
    }
    if (opcode_name == "SUBPATTERN") {
        const auto subpattern = std::dynamic_pointer_cast<const SubpatternOpcode>(a_Opcode);
        return handleGroup(subpattern->getSubpattern(), subpattern->getGroup());
    }
    if (opcode_name == "ASSERT") {
        std::wstring result;
        const auto assert = std::dynamic_pointer_cast<const Assert>(a_Opcode);
        const auto &subpattern = assert->getSubpattern();
        for (int i = 0; i < subpattern.length(); ++i) {
            result += handleOpcode(subpattern.getItem(i));
        }
        return result;
    }
    if (opcode_name == "ASSERT_NOT") {
        // TODO(thom): How do we handle this case? Assert that a piece of text does *not* exist.
        return L"";
    } if (opcode_name == "GROUPREF") {
        const auto groupRef = std::dynamic_pointer_cast<const GroupRef>(a_Opcode);
        return m_RealizedGroups.at(groupRef->getGroupId());
    } if (opcode_name == "MIN_REPEAT") {
        const auto min_repeat = std::dynamic_pointer_cast<const MinRepeat>(a_Opcode);
        return handleRepeat(min_repeat->getMin(), min_repeat->getMax(), min_repeat->getSubpattern());
    } if (opcode_name == "MAX_REPEAT") {
        const auto max_repeat = std::dynamic_pointer_cast<const MaxRepeat>(a_Opcode);
        return handleRepeat(max_repeat->getMin(), max_repeat->getMax(), max_repeat->getSubpattern());
    }
    throw std::invalid_argument("Unexpected opcode! " + opcode_name);
}

const std::wstring RandomStringGenerator::handleIn(const std::shared_ptr<const In> &a_InOpcode) {
    assert(a_InOpcode->length() > 0);
    bool negate = a_InOpcode->getItem(0)->getName() == "NEGATE";
    if (negate) {
        throw std::invalid_argument("Negative character classes not supported yet.");
    }
    return chooseFromCharacterClass(a_InOpcode);
}

const std::wstring RandomStringGenerator::chooseFromCharacterClass(const std::shared_ptr<const In> &a_InOpcode) {
    // In consists of ranges, categories and literals.
    int nr_choices = 0;
    std::vector<int> lookup(static_cast<size_t>(a_InOpcode->length()));
    for (int i = 0; i < a_InOpcode->length(); ++i) {
        nr_choices += a_InOpcode->getItem(i)->minNrCharacters();
        lookup[static_cast<size_t>(i)] = nr_choices;
    }
    assert(nr_choices > 0);
    std::uniform_int_distribution<int> distribution(0, nr_choices - 1);
    int character_choice = distribution(m_Generator);
    auto lower_bound_iterator = std::lower_bound(lookup.begin(), lookup.end(), character_choice);
    auto choice_index = std::distance(lookup.begin(), lower_bound_iterator);
    return handleOpcode(a_InOpcode->getItem(static_cast<int>(choice_index)));
}

const std::wstring RandomStringGenerator::handleSubpattern(const std::shared_ptr<const Subpattern> &a_Subpattern) {
    std::wstring result;
    for (int i = 0; i < a_Subpattern->length(); ++i) {
        result += handleOpcode(a_Subpattern->getItem(i));
    }
    return result;
}

const std::wstring RandomStringGenerator::handleGroup(const std::shared_ptr<const Subpattern> &a_Subpattern, int a_Group) {
    std::wstring result = handleSubpattern(a_Subpattern);
    m_RealizedGroups[a_Group] = result;
    return result;
}

const std::wstring RandomStringGenerator::handleRepeat(int a_Min, int a_Max, const std::shared_ptr<const Subpattern> &a_Subpattern) {
    std::wstring result;
    a_Max = std::max(a_Min, std::min(a_Max, m_RepeatLimit));
    std::uniform_int_distribution<int> distribution(a_Min, a_Max);
    int times = distribution(m_Generator);
    for (int i = times; i > 0; --i) {
        for (int k = 0; k < a_Subpattern->length(); ++k) {
            result += handleOpcode(a_Subpattern->getItem(k));
        }
    }
    return result;
}

const std::wstring RandomStringGenerator::getRandomPrintableCharacter(const wchar_t a_NotThisCharacter) {
    while (true) {
        size_t random_index = m_PrintableDistribution(m_Generator);
        auto iterator = m_Printable.begin();
        std::advance(iterator, random_index);
        wchar_t character = *iterator;
        if (character != a_NotThisCharacter) {
            return std::wstring(1, character);
        }
    }
}

const std::wstring RandomStringGenerator::getRandomCharacter(const std::unordered_set<wchar_t> &a_CharacterSet) {
    assert(!a_CharacterSet.empty());
    auto distribution = std::uniform_int_distribution<size_t>(0, a_CharacterSet.size() - 1);
    size_t random_index = distribution(m_Generator);
    auto iterator = a_CharacterSet.begin();
    std::advance(iterator, random_index);
    wchar_t character = *iterator;
    return std::wstring(1, character);
}
