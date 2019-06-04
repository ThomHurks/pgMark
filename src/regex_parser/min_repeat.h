#ifndef PGMARK_MIN_REPEAT_H
#define PGMARK_MIN_REPEAT_H

#include "opcode.h"

class MinRepeat : public Opcode {
private:
    const int m_Min;
    const int m_Max;
    const std::shared_ptr<const Subpattern> m_Subpattern;
public:
    MinRepeat(const int a_Min, const int a_Max, const std::shared_ptr<const Subpattern>& a_Subpattern)
            : Opcode("MIN_REPEAT"),
              m_Min(a_Min),
              m_Max(a_Max),
              m_Subpattern(a_Subpattern) {
        assert(m_Min <= m_Max);
        assert(m_Subpattern != nullptr);
    }

    MinRepeat(const MinRepeat &a_Other);

    MinRepeat &operator=(MinRepeat a_Copy);

    bool operator==(const MinRepeat &a_Other);

    bool operator!=(const MinRepeat &a_Other);

    friend void swap(MinRepeat &a_Self, MinRepeat &a_Other);

    int getMin() const;

    int getMax() const;

    const std::shared_ptr<const Subpattern> &getSubpattern() const;

    int minNrCharacters() const override;

    int maxNrCharacters() const override;
};

#endif //PGMARK_MIN_REPEAT_H
