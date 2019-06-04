#ifndef PGMARK_CHARACTER_SETS_H
#define PGMARK_CHARACTER_SETS_H

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include "opcode.h"
#include "in.h"

struct CharacterSets {
    static const std::unordered_set<std::wstring> SPECIAL_CHARS;
    static const std::unordered_set <std::wstring> REPEAT_CHARS;
    static const std::unordered_set<wchar_t> DIGITS;
    static const std::unordered_set<wchar_t> OCTDIGITS;
    static const std::unordered_set<wchar_t> HEXDIGITS;
    static const std::unordered_set<wchar_t> ASCIILETTERS;
    static const std::unordered_set<char> WHITESPACE;
    static const std::unordered_set <std::string> REPEATCODES;
    static const std::unordered_set <std::string> UNITCODES;
    static const std::unordered_map<std::wstring, std::shared_ptr<Opcode>> ESCAPES;
    static const std::unordered_map<std::wstring, std::shared_ptr<Opcode>> CATEGORIES;
    static const std::unordered_set <std::wstring> FLAGS; // TODO: make a map.
};

#endif //PGMARK_CHARACTER_SETS_H
