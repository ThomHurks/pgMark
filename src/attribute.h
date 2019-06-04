#ifndef GMARK_ATTRIBUTE_H
#define GMARK_ATTRIBUTE_H

#include <memory>
#include <map>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <codecvt>
#include "random_distribution.h"
#include "regex_parser/sre_parse.h"
#include "regex_parser/branch.h"
#include "regex_parser/subpattern_opcode.h"
#include "regex_parser/group_ref.h"
#include "regex_parser/min_repeat.h"
#include "regex_parser/max_repeat.h"
#include "random_string_generator.h"
#include <regex>

class Attribute {
protected:
    const std::string m_Name;
    bool m_Required;
    bool m_Unique;
    std::mt19937 m_Generator{std::random_device{}()};
public:
    Attribute(std::string a_Name, const bool a_Required, const bool a_Unique) :
            m_Name(std::move(a_Name)),
            m_Required(a_Required),
            m_Unique(a_Unique) {
        assert(!m_Name.empty());
    }

    virtual std::string getRandomAttribute() = 0;

    virtual ~Attribute() = 0;

    const std::string &getName() const {
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

    double getRandomNumber() {
        double random_value = m_Distribution->getRandomDouble();
        return std::clamp(random_value, m_Min, m_Max);
    }
public:
    NumericAttribute(const std::string &a_Name, bool a_Required, bool a_Unique, double a_Min, double a_Max, int a_Precision,
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
        m_Stream.str(std::string());
        m_Stream << getRandomNumber();
        return m_Stream.str();
    }
};

class DateAttribute : public NumericAttribute {
protected:
    char buffer[11] = {0};
public:
    DateAttribute(const std::string &a_Name, bool a_Required, bool a_Unique, double a_Min, double a_Max, int a_Precision,
                  std::unique_ptr<RandomDistribution> a_Distribution) :
              NumericAttribute(a_Name, a_Required, a_Unique, a_Min, a_Max, a_Precision, std::move(a_Distribution)) {}

    std::string getRandomAttribute() override {
        auto date = static_cast<std::time_t>(NumericAttribute::getRandomNumber());
        std::tm* date_tm = std::localtime(&date);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", date_tm);
        return std::string(buffer);
    }
};

class CategoricalAttribute : public Attribute {
protected:
    std::uniform_real_distribution<double> m_Distribution;
    std::map<double, std::string> m_Categories;
public:
    CategoricalAttribute(const std::string &a_Name, bool a_Required, bool a_Unique,
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
    RandomStringGenerator m_StringGenerator;
public:
    RegexAttribute(const std::string &a_Name, bool a_Required, bool a_Unique, const std::string &a_Regex)
            : Attribute(a_Name, a_Required, a_Unique),
              m_StringGenerator(a_Regex) {

    }

    std::string getRandomAttribute() override {
        return m_StringGenerator.getRandomString();
    }

};

class ChoiceAttribute : public Attribute {
protected:
    std::uniform_real_distribution<double> m_Distribution;
    std::map<double, std::unique_ptr<Attribute>> m_Choices;
public:
    ChoiceAttribute(const std::string &a_Name, bool a_Required, bool a_Unique,
                    std::map<double, std::unique_ptr<Attribute>> a_Choices, double a_CumulativeProbability)
            : Attribute(a_Name, a_Required, a_Unique),
              m_Distribution(std::uniform_real_distribution<double>(0.0, a_CumulativeProbability)),
              m_Choices(std::move(a_Choices)) {}

    std::string getRandomAttribute() override {
        double random_value = m_Distribution(m_Generator);
        return m_Choices.lower_bound(random_value)->second->getRandomAttribute();
    }
};

#endif //GMARK_ATTRIBUTE_H
