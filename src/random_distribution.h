#ifndef GMARK_RANDOM_DISTRIBUTION_H
#define GMARK_RANDOM_DISTRIBUTION_H

#include <random>
#include <cassert>

class RandomDistribution {
    // TODO (thom): enforce min, max, unique.
protected:
    std::mt19937 m_Generator{std::random_device{}()};
    const std::string m_Name;

    explicit RandomDistribution(const std::string &a_Name) : m_Name(a_Name) {
        assert(!m_Name.empty());
    }

public:
    const std::string getName() const {
        return m_Name;
    }

    virtual int getRandomInteger() = 0;

    virtual double getRandomDouble() = 0;

    virtual double getMean() const = 0;

    virtual ~RandomDistribution() = 0;
};

class UniformIntegerDistribution : public RandomDistribution {
private:
    const int m_Min;
    const int m_Max;
    const double m_Mean;
    std::uniform_int_distribution<int> m_Distribution;
public:
    UniformIntegerDistribution(int a_Min, int a_Max) :
            RandomDistribution("uniform"),
            m_Min(a_Min),
            m_Max(a_Max),
            m_Mean(static_cast<double>(m_Min + m_Max) / 2.0),
            m_Distribution(m_Min, m_Max) {
        assert(m_Min <= m_Max);
    }

    double getMean() const override {
        return m_Mean;
    }

    int getRandomInteger() override {
        return m_Distribution(m_Generator);
    }

    double getRandomDouble() override {
        return static_cast<double>(getRandomInteger());
    }
};

class UniformIntegerUniqueDistribution : public RandomDistribution {
private:
    int m_Counter;
    const int m_Min;
    const double m_Mean;
public:
    explicit UniformIntegerUniqueDistribution(int a_Min) :
            RandomDistribution("counter"),
            m_Counter(a_Min - 1),
            m_Min(a_Min),
            m_Mean(static_cast<double>(std::numeric_limits<int>::max() -  m_Min) / 2.0) {}

    double getMean() const override {
        return m_Mean;
    }

    int getRandomInteger() override {
        return ++m_Counter;
    }

    double getRandomDouble() override {
        return static_cast<double>(getRandomInteger());
    }
};

class UniformDoubleDistribution : public RandomDistribution {
private:
    const double m_Min;
    const double m_Max;
    const double m_Mean;
    std::uniform_real_distribution<double> m_Distribution;
public:
    UniformDoubleDistribution(double a_Min, double a_Max) :
            RandomDistribution("uniform"),
            m_Min(a_Min),
            m_Max(a_Max),
            m_Mean((m_Min + m_Max) / 2.0),
            m_Distribution(m_Min, m_Max) {
        assert(m_Min <= m_Max);
    }

    double getMean() const override {
        return m_Mean;
    }

    int getRandomInteger() override {
        return static_cast<int>(getRandomDouble());
    }

    double getRandomDouble() override {
        return m_Distribution(m_Generator);
    }
};

class GaussianDistribution : public RandomDistribution {
private:
    const double m_Mean;
    const double m_StandardDeviation;
    std::normal_distribution<double> m_Distribution;
public:
    GaussianDistribution(double a_Mean, double a_StandardDeviation)
            : RandomDistribution("gaussian"),
              m_Mean(a_Mean),
              m_StandardDeviation(a_StandardDeviation),
              m_Distribution(a_Mean, a_StandardDeviation) {
        assert(!std::isnan(m_Mean));
        assert(!std::isnan(m_StandardDeviation));
        assert(m_StandardDeviation > 0.0);
    }

    double getMean() const override {
        return m_Mean;
    }

    int getRandomInteger() override {
        return static_cast<int>(std::round(getRandomDouble()));
    }

    double getRandomDouble() override {
        return m_Distribution(m_Generator);
    }
};

class ZipfianDistribution : public RandomDistribution {
private:
    const double m_NthHarmonicNumber;
    const double m_NumericMean;
    std::vector<double> m_CDF;
    std::uniform_real_distribution<double> m_Distribution;

    static double generalizedHarmonic(const int a_N, const double a_M) {
        double nth_harmonic = 0.0;
        for (int n = a_N; n > 0; --n) {
            nth_harmonic += 1.0 / std::pow(static_cast<double>(n), a_M);
        }
        return nth_harmonic;
    }

public:
    ZipfianDistribution(double a_Exponent, int a_Number)
            : RandomDistribution("zipfian"),
              m_NthHarmonicNumber(generalizedHarmonic(a_Number, a_Exponent)),
              m_NumericMean(generalizedHarmonic(a_Number, a_Exponent - 1.0) / m_NthHarmonicNumber),
              m_Distribution(0.0, 1.0) {
        assert(a_Number > 0);
        assert(!std::isnan(a_Exponent));
        assert(a_Exponent >= 0.0);
        m_CDF.reserve(static_cast<size_t>(a_Number));
        double rank_harmonic = 0.0;
        for (int n = 1; n <= a_Number; n++) {
            rank_harmonic += 1.0 / std::pow(static_cast<double>(n), a_Exponent);
            m_CDF.push_back(rank_harmonic / m_NthHarmonicNumber);
        }
    }

    double getMean() const override {
        return m_NumericMean;
    }

    int getRandomInteger() override {
        double z = m_Distribution(m_Generator);
        auto p = std::lower_bound(m_CDF.begin(), m_CDF.end(), z);
        return static_cast<int>(std::distance(m_CDF.begin(), p)) + 1;
    }

    double getRandomDouble() override {
        return static_cast<double>(getRandomInteger());
    }
};

class ZetaDistribution : public RandomDistribution {
private:
    const double m_Alpha;
public:
    explicit ZetaDistribution(double a_Alpha)
            : RandomDistribution("zeta"),
              m_Alpha(a_Alpha) {
        assert(!std::isnan(m_Alpha));
        assert(m_Alpha > 1.0);
    }

    double getMean() const override {
        //TODO: Riemann-Zeta mean.
        //Riemann-Zeta function evaluated at (s - 1)
        // Divided by the Riemann-Zeta function evaluated at (s)
        // For s > 2
        // For other s, maybe just return the mode = 1
        return 0.0;
    }

    int getRandomInteger() override {
        // TODO: implement this. (Zeta distribution)
        return 0;
    }

    double getRandomDouble() override {
        // TODO: implement this. (Zeta distribution)
        return 0.0;
    }
};

class ExponentialDistribution : public RandomDistribution {
private:
    const double m_Rate;
    const double m_Scale;
    std::exponential_distribution<double> m_Distribution;
public:
    explicit ExponentialDistribution(double a_Scale)
            : RandomDistribution("exponential"),
              m_Rate(1.0 / a_Scale),
              m_Scale(a_Scale),
              m_Distribution(1.0 / a_Scale) {
        assert(!std::isnan(m_Rate));
        assert(m_Rate > 0.0);
        assert(!std::isnan(m_Scale));
        assert(m_Scale > 0.0);
    }

    double getMean() const override {
        return m_Scale;
    }

    int getRandomInteger() override {
        return static_cast<int>(std::round(getRandomDouble()));
    }

    double getRandomDouble() override {
        return m_Distribution(m_Generator);
    }
};

class LogNormalDistribution : public RandomDistribution {
private:
    const double m_Mean;
    const double m_StandardDeviation;
    std::lognormal_distribution<double> m_Distribution;
public:
    LogNormalDistribution(double a_Mean, double a_StandardDeviation)
            : RandomDistribution("lognormal"),
              m_Mean(std::exp(a_Mean + ((a_StandardDeviation * a_StandardDeviation) / 2.0))),
              m_StandardDeviation(a_StandardDeviation),
              m_Distribution(a_Mean, a_StandardDeviation) {
        assert(!std::isnan(m_Mean));
        assert(!std::isnan(m_StandardDeviation));
        assert(m_StandardDeviation > 0.0);
    }

    double getMean() const override {
        return m_Mean;
    }

    int getRandomInteger() override {
        return static_cast<int>(std::round(getRandomDouble()));
    }

    double getRandomDouble() override {
        return m_Distribution(m_Generator);
    }
};

#endif //GMARK_RANDOM_DISTRIBUTION_H
