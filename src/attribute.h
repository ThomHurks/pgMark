#ifndef GMARK_ATTRIBUTE_H
#define GMARK_ATTRIBUTE_H

#include <memory>
#include <map>
#include <iomanip>
#include <sstream>
#include "random_distribution.h"

class Attribute {
protected:
    std::string m_Name;
    bool m_Required;
    bool m_Unique;
    std::mt19937 m_Generator{std::random_device{}()};
public:
    Attribute(std::string &a_Name, bool a_Required, bool a_Unique) :
            m_Name(a_Name),
            m_Required(a_Required),
            m_Unique(a_Unique) {}

    virtual std::string getRandomAttribute() = 0;

    virtual ~Attribute() = 0;

    const std::string& getName() const {
        return m_Name;
    }
};

class NumericAttribute : public Attribute {
protected:
    double m_Min;
    double m_Max;
    int m_Precision;
    std::unique_ptr<RandomDistribution> m_Distribution;
    std::stringstream m_Stream;
public:
    NumericAttribute(std::string &a_Name, bool a_Required, bool a_Unique, double a_Min, double a_Max, int a_Precision,
                     std::unique_ptr<RandomDistribution> a_Distribution) :
            Attribute(a_Name, a_Required, a_Unique),
            m_Min(a_Min),
            m_Max(a_Max),
            m_Precision(a_Precision),
            m_Distribution(std::move(a_Distribution)) {
        assert(m_Min <= m_Max);
        assert(m_Precision >= 0);
        m_Stream << std::fixed << std::setprecision(m_Precision);
    }

    std::string getRandomAttribute() override {
        double random_value = m_Distribution->getRandomDouble();
        random_value = std::clamp(random_value, m_Min, m_Max);
        m_Stream.str(std::string());
        m_Stream << random_value;
        return m_Stream.str();
    }
};

class CategoricalAttribute : public Attribute {
protected:
    std::uniform_real_distribution<double> m_Distribution;
    std::map<double, std::string> m_Categories;
public:
    CategoricalAttribute(std::string &a_Name, bool a_Required, bool a_Unique,
                         const std::map<std::string, double> &a_Categories)
            : Attribute(a_Name, a_Required, a_Unique) {
        double sum = 0.0;
        for (const auto &category : a_Categories) {
            assert(category.second > 0.0);
            sum += category.second;
            m_Categories[sum] = category.first;
        }
        m_Distribution = std::uniform_real_distribution<double>(0.0, sum);
    }

    std::string getRandomAttribute() override {
        double random_value = m_Distribution(m_Generator);
        return m_Categories.lower_bound(random_value)->second;
    }
};

class RegexAttribute : public Attribute {
protected:
    std::string m_Regex;
public:
    RegexAttribute(std::string &a_Name, bool a_Required, bool a_Unique, std::string &a_Regex)
            : Attribute(a_Name, a_Required, a_Unique),
              m_Regex(a_Regex) {}

    std::string getRandomAttribute() override {
        return "Regex attributes are not implemented yet.";
    }
};

#endif //GMARK_ATTRIBUTE_H
