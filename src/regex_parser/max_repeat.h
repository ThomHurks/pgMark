#ifndef PGMARK_MAX_REPEAT_H
#define PGMARK_MAX_REPEAT_H

#include "opcode.h"

class MaxRepeat : public Opcode {
private:
    const int m_Min;
    const int m_Max;
    const std::shared_ptr<const Subpattern> m_Subpattern;
public:
    MaxRepeat(const int a_Min, const int a_Max, const std::shared_ptr<const Subpattern>& a_Subpattern)
            : Opcode("MAX_REPEAT"),
              m_Min(a_Min),
              m_Max(a_Max),
              m_Subpattern(a_Subpattern) {
        assert(m_Min <= m_Max);
        assert(m_Subpattern != nullptr);
    }

    MaxRepeat(const MaxRepeat &a_Other);

    MaxRepeat &operator=(MaxRepeat a_Copy);

    bool operator==(const MaxRepeat &a_Other);

    bool operator!=(const MaxRepeat &a_Other);

    friend void swap(MaxRepeat &a_Self, MaxRepeat &a_Other);

    int getMin() const;

    int getMax() const;

    const std::shared_ptr<const Subpattern> getSubpattern() const;

    int minNrCharacters() const override;

    int maxNrCharacters() const override;
};

#endif //PGMARK_MAX_REPEAT_H
