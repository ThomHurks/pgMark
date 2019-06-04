#include "character_sets.h"

const std::unordered_set<std::wstring> CharacterSets::SPECIAL_CHARS = {L".", L"\\", L"[", L"{", L"(", L")", L"*", L"+",
                                                                       L"?", L"^", L"$", L"|"};
const std::unordered_set <std::wstring> CharacterSets::REPEAT_CHARS = {L"*", L"+", L"?", L"{"};
const std::unordered_set<wchar_t> CharacterSets::DIGITS = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
const std::unordered_set<wchar_t> CharacterSets::OCTDIGITS = {'0', '1', '2', '3', '4', '5', '6', '7'};
const std::unordered_set<wchar_t> CharacterSets::HEXDIGITS = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a',
                                                               'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F'};
const std::unordered_set<wchar_t> CharacterSets::ASCIILETTERS = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
                                                                 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                                                 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
                                                                 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                                                                 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
const std::unordered_set<char> CharacterSets::WHITESPACE = {' ', '\t', '\n', '\r', '\v', '\f'};
const std::unordered_set <std::string> CharacterSets::REPEATCODES = {"MIN_REPEAT", "MAX_REPEAT"};
const std::unordered_set <std::string> CharacterSets::UNITCODES = {"ANY", "RANGE", "IN", "LITERAL", "NOT_LITERAL",
                                                                "CATEGORY"};
const std::unordered_map<std::wstring, std::shared_ptr<Opcode>> CharacterSets::ESCAPES = {
        {L"\\a",  std::make_shared<Literal>(L"\a")},
        {L"\\b",  std::make_shared<Literal>(L"\b")},
        {L"\\f",  std::make_shared<Literal>(L"\f")},
        {L"\\n",  std::make_shared<Literal>(L"\n")},
        {L"\\r",  std::make_shared<Literal>(L"\r")},
        {L"\\t",  std::make_shared<Literal>(L"\t")},
        {L"\\v",  std::make_shared<Literal>(L"\v")},
        {L"\\\\", std::make_shared<Literal>(L"\\")}
};
const std::unordered_map<std::wstring, std::shared_ptr<Opcode>> CharacterSets::CATEGORIES = {
        {L"\\A", std::make_shared<At>(E_CATEGORY_TYPE::AT_BEGINNING_STRING)}, // Start of string.
        {L"\\b", std::make_shared<At>(E_CATEGORY_TYPE::AT_BOUNDARY)},
        {L"\\B", std::make_shared<At>(E_CATEGORY_TYPE::AT_NON_BOUNDARY)},
        {L"\\d", std::make_shared<In>(std::make_shared<const Category>(E_CATEGORY_TYPE::CATEGORY_DIGIT))},
        {L"\\D", std::make_shared<In>(std::make_shared<const Category>(E_CATEGORY_TYPE::CATEGORY_NOT_DIGIT))},
        {L"\\s", std::make_shared<In>(std::make_shared<const Category>(E_CATEGORY_TYPE::CATEGORY_SPACE))},
        {L"\\S", std::make_shared<In>(std::make_shared<const Category>(E_CATEGORY_TYPE::CATEGORY_NOT_SPACE))},
        {L"\\w", std::make_shared<In>(std::make_shared<const Category>(E_CATEGORY_TYPE::CATEGORY_WORD))},
        {L"\\W", std::make_shared<In>(std::make_shared<const Category>(E_CATEGORY_TYPE::CATEGORY_NOT_WORD))},
        {L"\\Z", std::make_shared<At>(E_CATEGORY_TYPE::AT_END_STRING)} // End of string.
};
const std::unordered_set <std::wstring> CharacterSets::FLAGS = {L"i", L"L", L"m", L"s", L"x", L"a", L"t", L"u"}; // TODO(thom): make a map.
