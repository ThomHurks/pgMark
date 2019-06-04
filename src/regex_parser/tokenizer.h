#ifndef REGEX_TOKENIZER_H
#define REGEX_TOKENIZER_H

#include <string>
#include <unordered_set>
#include <codecvt>
#include <locale>

class Tokenizer {
private:
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> m_Converter;
    const std::string &m_String;
    std::wstring m_DecodedString;
    int m_Index = 0;
    std::wstring m_Next = L"";

    void next();

public:
    explicit Tokenizer(const std::string &a_String);

    bool match(const std::wstring &a_Char);

    std::wstring get();

    const std::wstring getWhile(int a_N, const std::unordered_set<wchar_t> &a_Charset);

    const std::wstring getUntil(const std::wstring &a_Terminator, const std::string &a_Name);

    int pos();

    int tell();

    void seek(const int a_Index);

    std::wstring readNext() const;
};

#endif //REGEX_TOKENIZER_H
