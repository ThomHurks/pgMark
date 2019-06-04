#include "tokenizer.h"

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> Tokenizer::m_Converter;

Tokenizer::Tokenizer(const std::string &a_String) : m_String(a_String), m_DecodedString(m_Converter.from_bytes(a_String)) {
    next();
}

void Tokenizer::next() {
    int index = m_Index;
    std::wstring c;
    try {
        c = m_DecodedString.at(static_cast<size_t>(index));
    } catch (std::out_of_range &) {
        m_Next = L"";
        return;
    }
    if (c == L"\\") {
        ++index;
        try {
            c += m_DecodedString.at(static_cast<size_t>(index));
        } catch (std::out_of_range &) {
            throw std::invalid_argument("Bad escape (end of pattern).");
        }
    }
    m_Index = index + 1;
    m_Next = c;
}

bool Tokenizer::match(const std::wstring &a_Char) {
    if (a_Char == m_Next) {
        next();
        return true;
    }
    return false;
}

std::wstring Tokenizer::get() {
    std::wstring that = m_Next;
    next();
    return that;
}

const std::wstring Tokenizer::getWhile(int a_N, const std::unordered_set<wchar_t> &a_Charset) {
    std::wstring result;
    for (int i = 0; i < a_N; ++i) {
        if (a_Charset.find(m_Next.at(0)) == a_Charset.end()) {
            break;
        }
        result += m_Next;
        next();
    }
    return result;
}

const std::wstring Tokenizer::getUntil(const std::wstring &a_Terminator, const std::string &a_Name) {
    std::wstring result;
    while (true) {
        const std::wstring c = m_Next;
        next();
        if (c.empty()) {
            if (result.empty()) {
                throw std::invalid_argument("Missing " + a_Name);
            }
            throw std::invalid_argument("Missing terminator, unterminated name");
        }
        if (c == a_Terminator) {
            if (result.empty()) {
                throw std::invalid_argument("Missing " + a_Name);
            }
            break;
        }
        result += c;
    }
    return result;
}

int Tokenizer::pos() {
    return m_Index - static_cast<int>(m_Next.length());
}

int Tokenizer::tell() {
    return m_Index - static_cast<int>(m_Next.length());
}

void Tokenizer::seek(const int a_Index) {
    m_Index = a_Index;
    next();
}

std::wstring Tokenizer::readNext() const {
    return m_Next;
}
