#ifndef PGMARK_OPCODE_H
#define PGMARK_OPCODE_H

#include <string>
#include <vector>
#include <cassert>
#include "subpattern.h"
#include "category_types.h"

class Opcode {
private:
    const std::string m_Name;
public:
    explicit Opcode(const std::string &a_Name) : m_Name(a_Name) {}

    Opcode(const Opcode &a_Other) = delete;

    virtual int minNrCharacters() const = 0;

    virtual int maxNrCharacters() const = 0;

    const std::string &getName() const {
        return m_Name;
    }

    virtual ~Opcode() = 0;
};

class Literal : public Opcode {
private:
    const int m_Literal;
public:
    explicit Literal(const int a_CodePoint) : Opcode("LITERAL"), m_Literal(a_CodePoint) {}

    explicit Literal(const char &a_Char) : Opcode("LITERAL"), m_Literal(a_Char) {}

    explicit Literal(const std::wstring &a_String) : Opcode("LITERAL"), m_Literal(static_cast<int>(a_String[0])) {
        if (a_String.size() != 1) {
            throw std::invalid_argument("Literal must be a single character.");
        }
    }

    Literal(const Literal &a_Other);

    Literal &operator=(Literal a_Copy);

    bool operator==(const Literal &a_Other);

    bool operator!=(const Literal &a_Other);

    friend void swap(Literal &a_Self, Literal &a_Other);

    wchar_t getLiteral() const {
        return static_cast<wchar_t>(m_Literal);
    }

    int minNrCharacters() const override {
        return 1;
    }

    int maxNrCharacters() const override {
        return 1;
    }
};

class Negate : public Opcode {
public:
    Negate() : Opcode("NEGATE") {}

    Negate(const Negate &a_Other);

    Negate &operator=(Negate a_Copy);

    bool operator==(const Negate &a_Other);

    bool operator!=(const Negate &a_Other);

    friend void swap(Negate &a_Self, Negate &a_Other);

    int minNrCharacters() const override {
        return 0;
    }

    int maxNrCharacters() const override {
        return 0;
    }
};

class Range : public Opcode {
private:
    const int m_Low;
    const int m_High;
    const int m_Size;
public:
    Range(const int a_Low, const int a_High) : Opcode("RANGE"), m_Low(a_Low), m_High(a_High), m_Size(a_High - a_Low) {}

    Range(const char a_Low, const char a_High) : Opcode("RANGE"), m_Low(a_Low), m_High(a_High),
                                                 m_Size(a_High - a_Low) {}

    Range(const std::string &a_Low, const std::string &a_High) : Opcode("RANGE"), m_Low(a_Low[0]), m_High(a_High[0]),
                                                                 m_Size(a_High[0] - a_Low[0]) {
        if (a_Low.size() != 1 || a_High.size() != 1) {
            throw std::invalid_argument("Literal must be a single character.");
        }
    }

    Range(const Range &a_Other);

    Range &operator=(Range a_Copy);

    bool operator==(const Range &a_Other);

    bool operator!=(const Range &a_Other);

    friend void swap(Range &a_Self, Range &a_Other);

    int getLow() const {
        return m_Low;
    }

    int getHigh() const {
        return m_High;
    }

    int minNrCharacters() const override {
        return m_Size;
    }

    int maxNrCharacters() const override {
        return m_Size;
    }
};

class NotLiteral : public Opcode {
private:
    const int m_Literal;
public:
    explicit NotLiteral(const int a_Literal) : Opcode("NOT_LITERAL"), m_Literal(a_Literal) {}

    explicit NotLiteral(const char a_Literal) : Opcode("NOT_LITERAL"), m_Literal(a_Literal) {}

    explicit NotLiteral(const std::string &a_Literal) : Opcode("NOT_LITERAL"), m_Literal(a_Literal[0]) {
        if (a_Literal.size() != 1) {
            throw std::invalid_argument("Literal must be a single character.");
        }
    }

    NotLiteral(const NotLiteral &a_Other);

    NotLiteral &operator=(NotLiteral a_Copy);

    bool operator==(const NotLiteral &a_Other);

    bool operator!=(const NotLiteral &a_Other);

    friend void swap(NotLiteral &a_Self, NotLiteral &a_Other);

    wchar_t getLiteral() const {
        return static_cast<wchar_t>(m_Literal);
    }

    int minNrCharacters() const override {
        return 1;
    }

    int maxNrCharacters() const override {
        return 1;
    }
};

class Any : public Opcode {
public:
    Any() : Opcode("ANY") {}

    Any(const Any &a_Other);

    Any &operator=(Any a_Copy);

    bool operator==(const Any &a_Other);

    bool operator!=(const Any &a_Other);

    friend void swap(Any &a_Self, Any &a_Other);

    int minNrCharacters() const override {
        return 1;
    }

    int maxNrCharacters() const override {
        return 1;
    }
};

class Assert : public Opcode {
private:
    const int m_Direction;
    const Subpattern &m_Subpattern;
public:
    Assert(const int a_Direction, const Subpattern &a_Subpattern)
            : Opcode("ASSERT"),
              m_Direction(a_Direction),
              m_Subpattern(a_Subpattern) {}

    Assert(const Assert &a_Other);

    Assert &operator=(Assert a_Copy);

    bool operator==(const Assert &a_Other);

    bool operator!=(const Assert &a_Other);

    friend void swap(Assert &a_Self, Assert &a_Other);

    const Subpattern &getSubpattern() const {
        return m_Subpattern;
    }

    int minNrCharacters() const override {
        return 0;
    }

    int maxNrCharacters() const override {
        return 0;
    }
};

class AssertNot : public Opcode {
private:
    const int m_Direction;
    const Subpattern &m_Subpattern;
public:
    AssertNot(const int a_Direction, const Subpattern &a_Subpattern)
            : Opcode("ASSERT_NOT"),
              m_Direction(a_Direction),
              m_Subpattern(a_Subpattern) {}

    AssertNot(const AssertNot &a_Other);

    AssertNot &operator=(AssertNot a_Copy);

    bool operator==(const AssertNot &a_Other);

    bool operator!=(const AssertNot &a_Other);

    friend void swap(AssertNot &a_Self, AssertNot &a_Other);

    int minNrCharacters() const override {
        return 0;
    }

    int maxNrCharacters() const override {
        return 0;
    }
};

class At : public Opcode {
private:
    E_CATEGORY_TYPE m_Category;
public:
    explicit At(E_CATEGORY_TYPE a_Category) : Opcode("AT"), m_Category(a_Category) {}

    At(const At &a_Other);

    At &operator=(At a_Copy);

    bool operator==(const At &a_Other);

    bool operator!=(const At &a_Other);

    friend void swap(At &a_Self, At &a_Other);

    int minNrCharacters() const override {
        return 0;
    }

    int maxNrCharacters() const override {
        return 0;
    }
};

class Category : public Opcode {
private:
    E_CATEGORY_TYPE m_Category;
public:
    explicit Category(E_CATEGORY_TYPE a_Category) : Opcode("CATEGORY"), m_Category(a_Category) {}

    Category(const Category &a_Other);

    Category &operator=(Category a_Copy);

    bool operator==(const Category &a_Other);

    bool operator!=(const Category &a_Other);

    friend void swap(Category &a_Self, Category &a_Other);

    int minNrCharacters() const override {
        return 1; // TODO: get size of category.
    }

    int maxNrCharacters() const override {
        return 1;
    }

    E_CATEGORY_TYPE getCategory() const {
        return m_Category;
    }
};

#endif //PGMARK_OPCODE_H
