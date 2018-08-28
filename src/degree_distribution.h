#ifndef GMARK_DISTRIBUTION_H
#define GMARK_DISTRIBUTION_H

#include <random>
#include <cassert>

class DegreeDistribution {
protected:
    std::mt19937 m_Generator{std::random_device{}()};
    const std::string m_Name;

    DegreeDistribution(const std::string &a_Name) : m_Name(a_Name) {
        assert(!m_Name.empty());
    }

public:
    const std::string getName() const {
        return m_Name;
    }

    virtual int getNextRandom() = 0;

    virtual double getMean() const = 0;

    virtual ~DegreeDistribution() = 0;
};

class UniformDegreeDistribution : public DegreeDistribution {
private:
    const int m_Min;
    const int m_Max;
    const double m_Mean;
    std::uniform_int_distribution<int> m_Distribution;
public:
    UniformDegreeDistribution(int a_Min, int a_Max) :
            DegreeDistribution("uniform"),
            m_Min(a_Min),
            m_Max(a_Max),
            m_Mean(static_cast<double>(m_Min + m_Max) / 2.0),
            m_Distribution(m_Min, m_Max) {
        assert(m_Min <= m_Max);
    }

    double getMean() const override {
        return m_Mean;
    }

    int getNextRandom() override {
        return m_Distribution(m_Generator);
    }
};

class GaussianDegreeDistribution : public DegreeDistribution {
private:
    const double m_Mean;
    const double m_StandardDeviation;
    std::normal_distribution<double> m_Distribution;
public:
    GaussianDegreeDistribution(double a_Mean, double a_StandardDeviation)
            : DegreeDistribution("gaussian"),
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

    int getNextRandom() override {
        return static_cast<int>(std::round(m_Distribution(m_Generator)));
    }
};

class ZipfianDegreeDistribution : public DegreeDistribution {
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
    ZipfianDegreeDistribution(double a_Exponent, int a_Number)
            : DegreeDistribution("zipfian"),
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

    int getNextRandom() override {
        double z = m_Distribution(m_Generator);
        auto p = std::lower_bound(m_CDF.begin(), m_CDF.end(), z);
        return static_cast<int>(std::distance(m_CDF.begin(), p)) + 1;
    }
};

class ZetaDegreeDistribution : public DegreeDistribution {
private:
    const double m_Alpha;
public:
    ZetaDegreeDistribution(double a_Alpha)
            : DegreeDistribution("zeta"),
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

    int getNextRandom() override {
        // TODO: implement this. (Zeta distribution)
        return 0;
    }
};

class ExponentialDegreeDistribution : public DegreeDistribution {
private:
    const double m_Rate;
    const double m_Scale;
    std::exponential_distribution<double> m_Distribution;
public:
    ExponentialDegreeDistribution(double a_Rate)
            : DegreeDistribution("exponential"),
              m_Rate(a_Rate),
              m_Scale(1.0 / a_Rate),
              m_Distribution(a_Rate) {
        assert(!std::isnan(m_Rate));
        assert(m_Rate > 0.0);
        assert(!std::isnan(m_Scale));
        assert(m_Scale > 0.0);
    }

    double getMean() const override {
        return m_Scale;
    }

    int getNextRandom() override {
        return static_cast<int>(std::round(m_Distribution(m_Generator)));
    }
};

class LogNormalDegreeDistribution : public DegreeDistribution {
private:
    const double m_Mean;
    const double m_StandardDeviation;
    std::lognormal_distribution<double> m_Distribution;
public:
    LogNormalDegreeDistribution(double a_Mean, double a_StandardDeviation)
            : DegreeDistribution("lognormal"),
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

    int getNextRandom() override {
        return static_cast<int>(std::round(m_Distribution(m_Generator)));
    }
};

#endif //GMARK_DISTRIBUTION_H
