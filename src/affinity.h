#ifndef GMARK_AFFINITY_H
#define GMARK_AFFINITY_H

#include <string>

class Affinity {
private:
    std::string m_Name;
    bool m_Inverse;
    double m_Weight;
public:
    Affinity(std::string a_Name, bool a_Inverse, double a_Weight)
            : m_Name(std::move(a_Name)),
              m_Inverse(a_Inverse),
              m_Weight(a_Weight) {}
};

#endif //GMARK_AFFINITY_H
