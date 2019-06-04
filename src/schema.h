#ifndef GMARK_SCHEMA_H
#define GMARK_SCHEMA_H

#include "pugixml.hpp"
#include "relation_distribution.h"
#include "random_distribution.h"
#include "attribute.h"
#include "affinity.h"
#include <set>

class Schema {
private:
    std::set<std::string> m_Predicates;
    std::map<std::string, std::vector<std::unique_ptr<Attribute>>> m_Types;
    std::map<std::string, int> m_Constraints;
    std::vector<RelationDistribution> m_RelationDistributions;
    static const std::regex m_DateRegex;
    static const std::regex m_CSVParseRegex;
    static const double m_LenientCategoryProbabilityEpsilon;

    static void getTypes(std::map<std::string, std::vector<std::unique_ptr<Attribute>>> &a_Types,
                         const pugi::xml_node a_TypesNode);

    static std::unique_ptr<NumericAttribute> getNumericAttribute(const pugi::xml_node a_AttributeNode,
                                                                 const bool a_IsDate, const std::string& a_Name,
                                                                 const bool a_Required, const bool a_Unique);

    static std::vector<std::unique_ptr<Attribute>> getAttributes(const pugi::xml_node a_AttributesNode);

    static std::unique_ptr<Attribute> getChoiceAttribute(const pugi::xml_node a_ChoiceNode, const std::string &a_Name, bool a_Required, bool a_Unique);

    static std::unique_ptr<Attribute> parseAttributeNode(const pugi::xml_node a_AttributeNode);

    static std::unique_ptr<Attribute> getAttributeKind(const pugi::xml_node a_AttributeKindNode,
                                                       const std::string &a_Name, bool a_Required, bool a_Unique);

    static std::set<std::string>
    getUniqueNodeNames(const pugi::xml_node a_TypesNode, const std::string &a_ElementName);

    static bool doSetsOverlap(const std::set<std::string> &a_TypeNames, const std::set<std::string> &a_PredicateNames);

    static std::map<std::string, int> getConstraints(const pugi::xml_node a_TypesNode, const int a_GraphSize);

    static std::map<std::string, double> getCategories(const pugi::xml_node a_categoriesNode);

    static std::pair<std::map<std::string, double>, double> getCategoriesFromFile(const std::string& a_FileName);

    static std::pair<std::map<std::string, double>, double> getCategoriesFromSchema(pugi::xml_object_range<pugi::xml_named_node_iterator> a_CategoriesIterator);

    static std::vector<RelationDistribution> getDistributions(const pugi::xml_node a_TypesNode,
                                                              const std::set<std::string> &a_TypeNames,
                                                              const std::set<std::string> &a_PredicateNames,
                                                              const std::map<std::string, int> &a_Constraints);

    static std::unique_ptr<RandomDistribution> getDistribution(const pugi::xml_node a_DistributionNode,
                                                               const int a_NrOfNodes,
                                                               const bool a_IntegerPrecision,
                                                               const bool a_IsDate,
                                                               const bool a_MustBeUnique);

    static std::map<std::string, Affinity> getAffinities(const pugi::xml_node a_AffinitiesNode);

    static long dateStringToEpoch(const std::string &a_DateString);
    static double attributeAsDouble(pugi::xml_attribute a_Attribute, const bool a_IsDate, const double a_Default = 0.0);
    static int attributeAsInteger(pugi::xml_attribute a_Attribute, const bool a_IsDate);

public:
    Schema(const pugi::xml_node a_TypesNode, const pugi::xml_node a_PredicatesNode, const int a_GraphSize);

    Schema(const Schema &) = delete; // No copying.
    Schema &operator=(const Schema &) = delete; // No copying.

    const std::vector<RelationDistribution> &getRelationDistributions() const {
        return m_RelationDistributions;
    }

    const std::map<std::string, int> &getConstraints() const {
        return m_Constraints;
    }

    const std::set<std::string> &getPredicates() const {
        return m_Predicates;
    }

    const std::map<std::string, std::vector<std::unique_ptr<Attribute>>> &getTypes() const {
        return m_Types;
    }
};

#endif //GMARK_SCHEMA_H
