#include "schema.h"
#include <algorithm>

Schema::Schema(const pugi::xml_node a_TypesNode, const pugi::xml_node a_PredicatesNode, const int a_GraphSize) {
    getTypes(m_Types, a_TypesNode);
    if (m_Types.empty()) {
        throw std::invalid_argument("The graph schema is required to specify node types");
    }
    std::set<std::string> type_names = getUniqueNodeNames(a_TypesNode, "type");
    m_Predicates = getUniqueNodeNames(a_PredicatesNode, "predicate");

    if (doSetsOverlap(type_names, m_Predicates)) {
        throw std::invalid_argument("The type and predicate names overlap");
    }
    if (!a_PredicatesNode) {
        throw std::invalid_argument("The graph schema is required to specify relation predicates");
    }
    m_Constraints = getConstraints(a_TypesNode, a_GraphSize);
    m_RelationDistributions = getDistributions(a_TypesNode, type_names, m_Predicates, m_Constraints);
}

std::map<std::string, int> Schema::getConstraints(const pugi::xml_node a_TypesNode, const int a_GraphSize) {
    std::map<std::string, int> constraints;
    for (pugi::xml_node type : a_TypesNode.children("type")) {
        std::string name = type.attribute("name").as_string();
        if (name.empty()) {
            throw std::invalid_argument("Type name cannot be empty");
        }
        pugi::xml_node count_element = type.child("count");
        if (!count_element) {
            throw std::invalid_argument("Type " + name + " does not have a count element");
        }
        int count = 0;
        pugi::xml_node fixed_element = count_element.child("fixed");
        if (fixed_element) {
            count = fixed_element.text().as_int();
        } else {
            pugi::xml_node proportion_element = count_element.child("proportion");
            if (proportion_element) {
                count = static_cast<int>(std::round(
                        proportion_element.text().as_double() * static_cast<double>(a_GraphSize)));
            } else {
                throw std::invalid_argument("Type " + name + " does not have a fixed or proportion constraint");
            }
        }
        if (count <= 0) {
            throw std::invalid_argument("Error: type " + name + " is constrained to 0 nodes");
        }
        constraints[name] = count;
    }
    return constraints;
}

bool Schema::doSetsOverlap(const std::set<std::string> &a_TypeNames, const std::set<std::string> &a_PredicateNames) {
    std::set<std::string> intersection;
    std::set_intersection(a_TypeNames.begin(), a_TypeNames.end(), a_PredicateNames.begin(), a_PredicateNames.end(),
                          std::inserter(intersection, intersection.begin()));
    return !intersection.empty();
}

void Schema::getTypes(std::map<std::string, std::vector<std::unique_ptr<Attribute>>> &a_Types,
                      const pugi::xml_node a_TypesNode) {
    for (pugi::xml_node type : a_TypesNode.children("type")) {
        std::string name = type.attribute("name").as_string();
        if (name.empty()) {
            throw std::invalid_argument("Type name cannot be empty.");
        }
        if (a_Types.find(name) != a_Types.end()) {
            throw std::invalid_argument("Duplicate type found: " + name);
        }
        a_Types.emplace(name, getAttributes(type.child("attributes")));
    }
}

std::vector<RelationDistribution> Schema::getDistributions(const pugi::xml_node a_TypesNode,
                                                           const std::set<std::string> &a_TypeNames,
                                                           const std::set<std::string> &a_PredicateNames,
                                                           const std::map<std::string, int> &a_Constraints) {
    std::vector<RelationDistribution> distributions;
    for (pugi::xml_node type : a_TypesNode.children("type")) {
        std::string source = type.attribute("name").as_string();
        if (source.empty()) {
            throw std::invalid_argument("Type name cannot be empty");
        }
        const pugi::xml_node relations_node = type.child("relations");
        if (!relations_node) {
            continue;
        }
        for (pugi::xml_node relation : relations_node.children("relation")) {
            std::string predicate = relation.attribute("predicate").as_string();
            if (predicate.empty()) {
                throw std::invalid_argument("Relation predicate cannot be empty");
            }
            if (a_PredicateNames.find(predicate) == a_PredicateNames.end()) {
                throw std::invalid_argument("Found relation with unspecified predicate " + predicate);
            }
            std::string target = relation.attribute("target").as_string();
            if (target.empty()) {
                throw std::invalid_argument("Relation target cannot be empty");
            }
            if (a_TypeNames.find(target) == a_TypeNames.end()) {
                throw std::invalid_argument("Found relation with unspecified target type " + target);
            }
            bool allow_loops = false;
            if (source != target) {
                allow_loops = true;
            } else {
                allow_loops = relation.attribute("allow_loops").as_bool(false);
            }
            bool allow_parallel_edges = relation.attribute("allow_parallel_edges").as_bool(false);
            std::map<std::string, Affinity> affinities = getAffinities(relation.child("affinities"));
            if (!affinities.empty() && source != target) {
                throw std::invalid_argument(
                        "Affinities can only be specified on relations between the same node types");
            }
            int nr_source_nodes = a_Constraints.at(source);
            int nr_target_nodes = a_Constraints.at(target);
            distributions.emplace_back(source, target, predicate, allow_loops, allow_parallel_edges,
                                       getDistribution(relation.child("inDistribution"), nr_target_nodes),
                                       getDistribution(relation.child("outDistribution"), nr_source_nodes), affinities);
        }
    }
    return distributions;
}

std::map<std::string, Affinity> Schema::getAffinities(const pugi::xml_node a_AffinitiesNode) {
    std::map<std::string, Affinity> affinities;
    if (a_AffinitiesNode) {
        for (pugi::xml_node affinity_node : a_AffinitiesNode.children("attributeAffinity")) {
            std::string name = affinity_node.attribute("name").as_string();
            if (name.empty()) {
                throw std::invalid_argument("Affinity name cannot be empty");
            }
            bool inverse = affinity_node.attribute("inverse").as_bool();
            double weight = affinity_node.attribute("weight").as_double();
            if (weight <= 0.0 || weight <= std::numeric_limits<double>::epsilon() || weight > 1.0) {
                throw std::invalid_argument("Affinity weight must be > 0 and <= 1");
            }
            if (affinities.find(name) != affinities.end()) {
                throw std::invalid_argument("Duplicate affinity declaration found: " + name);
            }
            affinities.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                               std::forward_as_tuple(name, inverse, weight));
        }
    }
    return affinities;
}

std::unique_ptr<DegreeDistribution> Schema::getDistribution(const pugi::xml_node a_DistributionNode,
                                                            const int a_NrOfNodes) {
    pugi::xml_node distribution = a_DistributionNode.child("uniformDistribution");
    if (distribution) {
        double min = distribution.attribute("min").as_double();
        double max = distribution.attribute("max").as_double();
        if (min > max) {
            throw std::invalid_argument("Invalid uniform distribution; min > max");
        }
        return std::make_unique<UniformDegreeDistribution>(min, max);
    }
    distribution = a_DistributionNode.child("gaussianDistribution");
    if (distribution) {
        double mean = distribution.attribute("mean").as_double();
        double stdev = distribution.attribute("stdev").as_double(-1.0);
        if (stdev <= 0.0 || stdev <= std::numeric_limits<double>::epsilon()) {
            throw std::invalid_argument("Standard deviation must be strictly larger than 0");
        }
        return std::make_unique<GaussianDegreeDistribution>(mean, stdev);
    }
    distribution = a_DistributionNode.child("zipfianDistribution");
    if (distribution) {
        double exponent = distribution.attribute("exponent").as_double(-1.0);
        if (exponent < 0.0) {
            throw std::invalid_argument("Exponent must be larger than or equal to 0");
        }
        return std::make_unique<ZipfianDegreeDistribution>(exponent, a_NrOfNodes);
    }
    // TODO(thom): check the Zeta parameters parsing.
    distribution = a_DistributionNode.child("zetaDistribution");
    if (distribution) {
        double alpha = distribution.attribute("alpha").as_double();
        if (alpha < 1.0 || (alpha - 1.0) <= std::numeric_limits<double>::epsilon()) {
            throw std::invalid_argument("Alpha must be strictly larger than 1");
        }
        return std::make_unique<ZetaDegreeDistribution>(alpha);
    }
    distribution = a_DistributionNode.child("exponentialDistribution");
    if (distribution) {
        double scale = distribution.attribute("scale").as_double(-1.0);
        if (scale <= 0.0 || scale <= std::numeric_limits<double>::epsilon()) {
            throw std::invalid_argument("Scale must be strictly larger than 0");
        }
        return std::make_unique<ExponentialDegreeDistribution>(scale);
    }
    distribution = a_DistributionNode.child("lognormalDistribution");
    if (distribution) {
        double mean = distribution.attribute("mean").as_double();
        double stdev = distribution.attribute("stdev").as_double(-1.0);
        if (stdev <= 0.0 || stdev <= std::numeric_limits<double>::epsilon()) {
            throw std::invalid_argument("Standard deviation must be strictly larger than 0");
        }
        return std::make_unique<LogNormalDegreeDistribution>(mean, stdev);
    }
    throw std::invalid_argument("Distribution must be uniform, gaussian, zipfian, zeta, exponential or lognormal");
}

std::vector<std::unique_ptr<Attribute>> Schema::getAttributes(const pugi::xml_node a_AttributesNode) {
    std::vector<std::unique_ptr<Attribute>> attributes;
    for (pugi::xml_node attribute : a_AttributesNode.children("attribute")) {
        std::string name = attribute.attribute("name").as_string();
        if (name.empty()) {
            throw std::invalid_argument("Attribute name cannot be empty");
        }
        bool required = attribute.attribute("required").as_bool();
        bool unique = attribute.attribute("unique").as_bool();
        pugi::xml_node numeric_kind = attribute.child("numeric");
        if (numeric_kind) {
            double min = attribute.attribute("min").as_double();
            double max = attribute.attribute("max").as_double();
            if (min > max) {
                throw std::invalid_argument("Invalid min and max attributes for numeric element");
            }
            // TODO(thom): Number argument for Zipfian should be configurable.
            int number = 1000;
            attributes.push_back(std::make_unique<NumericAttribute>(name, required, unique, min, max,
                                                                    getDistribution(numeric_kind, number)));
            continue;
        }
        pugi::xml_node categorical_kind = attribute.child("categorical");
        if (categorical_kind) {
            std::map<std::string, double> categories = getCategories(categorical_kind);
            attributes.push_back(std::make_unique<CategoricalAttribute>(name, required, unique, categories));
            continue;
        }
        pugi::xml_node regex_kind = attribute.child("regex");
        if (regex_kind) {
            std::string regex = regex_kind.text().as_string();
            if (regex.empty()) {
                throw std::invalid_argument("Attribute regex cannot be empty");
            }
            attributes.push_back(std::make_unique<RegexAttribute>(name, required, unique, regex));
            continue;
        }
        throw std::invalid_argument("Attribute must be numeric, categorical or regex");
    }
    return attributes;
}

std::map<std::string, double> Schema::getCategories(const pugi::xml_node a_categoriesNode) {
    std::map<std::string, double> categories;
    double total_probability = 0;
    size_t count = 0;
    for (pugi::xml_node category : a_categoriesNode.children("category")) {
        double probability = 0.0;
        std::string name = category.text().as_string();
        if (name.empty()) {
            throw std::invalid_argument("Category cannot be empty");
        }
        pugi::xml_attribute probability_attr = category.attribute("probability");
        if (probability_attr) {
            probability = probability_attr.as_double(-1.0);
            if (probability < 0.0 || probability > 1.0) {
                throw std::invalid_argument("Category probability must be in the range [0,1]");
            }
            total_probability += probability;
        } else if (total_probability > 0.0) {
            throw std::invalid_argument("Probability needs to be specified on all categories or none");
        }
        if (categories.find(name) != categories.end()) {
            throw std::invalid_argument("Category defined multiple times");
        }
        categories[name] = probability;
        ++count;
    }
    if (count == 0) {
        throw std::invalid_argument("No categories are defined in category element");
    }
    if (total_probability <= std::numeric_limits<double>::epsilon()) {
        double uniform_probability = 1.0 / static_cast<double>(count);
        for (auto &category : categories) {
            category.second = uniform_probability;
        }
    }
    if (std::abs(total_probability - 1.0) > (std::numeric_limits<double>::epsilon() * static_cast<double>(count))) {
        throw std::invalid_argument("The probabilities of categories need to sum to 1");
    }
    return categories;
}

std::set<std::string>
Schema::getUniqueNodeNames(const pugi::xml_node a_TypesNode, const std::string &a_ElementName) {
    std::set<std::string> names;
    for (pugi::xml_node type_node : a_TypesNode.children(a_ElementName.c_str())) {
        std::string name = type_node.attribute("name").as_string();
        if (name.empty()) {
            throw std::invalid_argument("Type name cannot be empty");
        }
        if (names.find(name) != names.end()) {
            throw std::invalid_argument("Duplicate entry for type " + name);
        }
        names.insert(name);
    }
    return names;
}
