#ifndef PGMARK_RANDOM_STRING_GENERATOR_H
#define PGMARK_RANDOM_STRING_GENERATOR_H

#include <string>
#include <random>
#include <locale>
#include <unordered_map>
#include <unordered_set>
#include <codecvt>
#include "regex_parser/subpattern.h"
#include "regex_parser/in.h"

class RandomStringGenerator {
protected:
    const std::string m_Regex;
    const int m_RepeatLimit;
    std::mt19937 m_Generator{std::random_device{}()};
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> m_Converter;
    std::shared_ptr<Subpattern> m_Subpattern;
    std::unordered_map<int, std::wstring> m_RealizedGroups;
    const std::unordered_set<wchar_t> m_Printable = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c',
                                                     'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
                                                     'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C',
                                                     'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '\"', '#',
                                                     '$', '%', '&', '\\', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
                                                     ':', ';', '<', '=', '>', '?', '@', '[', ']', '^', '_', '`', '{',
                                                     '|', '}', '~', ' ', '\t', '\n', '\r', '\x0b', '\x0c'};
    std::uniform_int_distribution<size_t> m_PrintableDistribution;
    const std::unordered_set<wchar_t> m_Digits = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    const std::unordered_set<wchar_t> m_Whitespace = {' ', '\t', '\n', '\r', '\x0b', '\x0c'};
    const std::unordered_set<wchar_t> m_NonDigits = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                                     'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                                     'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                                                     'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                                                     '!', '\"', '#', '$', '%', '&', '\\', '\'', '(', ')', '*', '+',
                                                     ',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[', ']',
                                                     '^', '_', '`', '{', '|', '}', '~'};
    const std::unordered_set<wchar_t> m_NonWhitespace = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b',
                                                         'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                                         'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                                         'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
                                                         'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                                         'Y', 'Z', '!', '\"', '#', '$', '%', '&', '\\', '\'', '(', ')',
                                                         '*', '+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '?',
                                                         '@', '[', ']', '^', '_', '`', '{', '|', '}', '~'};
    const std::unordered_set<wchar_t> m_Word = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd',
                                                'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                                                's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F',
                                                'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                                                'U', 'V', 'W', 'X', 'Y', 'Z', '_'};
    const std::unordered_set<wchar_t> m_NonWord = {'!', '\"', '#', '$', '%', '&', '\\', '\'', '(', ')', '*', '+', ',',
                                                   '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[', ']', '^',
                                                   '_', '`', '{', '|', '}', '~', ' ', '\t', '\n', '\r', '\x0b', '\x0c'};

    const std::wstring handleOpcode(const std::shared_ptr<const Opcode> &a_Opcode);

    const std::wstring handleIn(const std::shared_ptr<const In> &a_InOpcode);

    const std::wstring chooseFromCharacterClass(const std::shared_ptr<const In> &a_InOpcode);

    const std::wstring handleSubpattern(const std::shared_ptr<const Subpattern> &a_Subpattern);

    const std::wstring handleGroup(const std::shared_ptr<const Subpattern> &a_Subpattern, int a_Group);

    const std::wstring handleRepeat(int a_Min, int a_Max, const std::shared_ptr<const Subpattern> &a_Subpattern);

    const std::wstring getRandomPrintableCharacter(const wchar_t a_NotThisCharacter);

    const std::wstring getRandomCharacter(const std::unordered_set<wchar_t> &a_CharacterSet);

public:
    explicit RandomStringGenerator(const std::string &a_Regex, const int a_RepeatLimit = 999);

    std::string getRandomString();
};


#endif //PGMARK_RANDOM_STRING_GENERATOR_H
