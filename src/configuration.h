#ifndef GMARK_CONFIGURATION_H
#define GMARK_CONFIGURATION_H

#include "schema.h"

class Configuration {
private:
    std::unique_ptr<Schema> m_Schema;
    std::map<std::string, std::pair<int, int>> m_TypeRanges;

    static pugi::xml_document openFile(const std::string &a_Filename);

    const std::map<std::string, std::pair<int, int>> computeTypeRanges() const;

public:
    Configuration(const std::string &a_Filename, const int a_GraphSize);

    Configuration(const Configuration &) = delete; // No copying.
    Configuration &operator=(const Configuration &) = delete; // No copying.

    size_t getNrOfPredicates() const {
        return m_Schema->getPredicates().size();
    }

    size_t getNrOfTypes() const {
        return m_Schema->getTypes().size();
    }

    const std::vector<RelationDistribution> &getRelationDistributions() const {
        return m_Schema->getRelationDistributions();
    }

    const std::set<std::string> &getPredicates() const {
        return m_Schema->getPredicates();
    }

    const std::vector<std::string> getTypeNames() const {
        std::vector<std::string> types;
        types.reserve(m_Schema->getTypes().size());
        for (const auto &type_attribute_pair : m_Schema->getTypes()) {
            types.push_back(type_attribute_pair.first);
        }
        return types;
    }

    const std::vector<std::unique_ptr<Attribute>> &getTypeAttributes(const std::string &a_TypeName) const {
        return m_Schema->getTypes().at(a_TypeName);
    }

    const std::map<std::string, int> &getConstraints() const {
        return m_Schema->getConstraints();
    }

    int getConstraint(const std::string &a_TypeName) const {
        return m_Schema->getConstraints().at(a_TypeName);
    }

    const std::pair<int, int> &getTypeRange(const std::string &a_Type) const {
        return m_TypeRanges.at(a_Type);
    }
};

#endif //GMARK_CONFIGURATION_H
