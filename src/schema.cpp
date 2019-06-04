#include "schema.h"
#include <fstream>
#include <string>

const std::regex Schema::m_DateRegex(R"(^(\d{4})-(\d{2})-(\d{2})$)");
const std::regex Schema::m_CSVParseRegex(R"'(^(?:"(.+)"|(.+)),(1|\d\.\d+)$)'");
const double Schema::m_LenientCategoryProbabilityEpsilon = 0.1;

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

long Schema::dateStringToEpoch(const std::string &a_DateString) {
    int year = 0;
    int month = 0;
    int day = 0;
    std::smatch match;
    bool success = false;
    if (std::regex_match(a_DateString, match, m_DateRegex)) {
        if (match.size() == 4) {
            try {
                year = stoi(match[1].str());
                month = stoi(match[2].str());
                day = stoi(match[3].str());
                success = true;
            } catch (std::logic_error&) {
                // Do nothing.
            }
        }
    }
    if (!success) {
        throw std::invalid_argument("Date does not follow the pattern 1988-11-23.");
    }
    struct tm t = {0,0,0,0,0,0,0,0,0,0, nullptr};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    time_t timeSinceEpoch = mktime(&t);
    if (timeSinceEpoch == -1) {
        throw std::invalid_argument("Date is not a valid date.");
    }
    return timeSinceEpoch;
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
                                       getDistribution(relation.child("inDistribution"), nr_target_nodes, true, false, false),
                                       getDistribution(relation.child("outDistribution"), nr_source_nodes, true, false, false),
                                       affinities);
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

double Schema::attributeAsDouble(pugi::xml_attribute a_Attribute, const bool a_IsDate, const double a_Default) {
    if (a_IsDate) {
        const std::string &attrAsString = a_Attribute.as_string();
        if (attrAsString.empty()) {
            return a_Default;
        }
        return dateStringToEpoch(attrAsString);
    }
    return a_Attribute.as_double(a_Default);
}

int Schema::attributeAsInteger(pugi::xml_attribute a_Attribute, const bool a_IsDate) {
    if (a_IsDate) {
        // TODO(thom): this cast causes the 2038 problem. Fix it.
        return static_cast<int>(dateStringToEpoch(a_Attribute.as_string()));
    }
    return a_Attribute.as_int();
}

std::unique_ptr<RandomDistribution> Schema::getDistribution(const pugi::xml_node a_DistributionNode,
                                                            const int a_NrOfNodes,
                                                            const bool a_IntegerPrecision,
                                                            const bool a_IsDate,
                                                            const bool a_MustBeUnique) {
    pugi::xml_node distribution = a_DistributionNode.child("uniformDistribution");
    if (distribution) {
        // When generating node degrees with the uniform distribution, it only makes sense that the min and max
        // parameters are integers. In that specific case we can use a uniform distribution that generates integers.
        // We cannot use that trick for other distributions like the Gaussian distribution because even for node
        // degrees the mean may be a floating point number.
        if (a_IntegerPrecision) {
            int min = attributeAsInteger(distribution.attribute("min"), a_IsDate);
            pugi::xml_attribute max_attribute = distribution.attribute("max");
            // If the user specified that the generated values must be unique and no max parameter is specified
            // for the uniform integer distribution, then we can simply use a counter starting from the minimum
            // value as an optimization.
            if (!max_attribute && a_MustBeUnique) {
                return std::make_unique<UniformIntegerUniqueDistribution>(min);
            }
            int max = attributeAsInteger(max_attribute, a_IsDate);
            if (min > max) {
                throw std::invalid_argument("Invalid uniform distribution; min > max");
            }
            return std::make_unique<UniformIntegerDistribution>(min, max);
        }

        double min = attributeAsDouble(distribution.attribute("min"), a_IsDate);
        double max = attributeAsDouble(distribution.attribute("max"), a_IsDate);
        if (min > max) {
            throw std::invalid_argument("Invalid uniform distribution; min > max");
        }
        return std::make_unique<UniformDoubleDistribution>(min, max);
    }
    distribution = a_DistributionNode.child("gaussianDistribution");
    if (distribution) {
        double mean = attributeAsDouble(distribution.attribute("mean"), a_IsDate);
        double stdev = attributeAsDouble(distribution.attribute("stdev"), a_IsDate, -1.0);
        if (stdev <= 0.0 || stdev <= std::numeric_limits<double>::epsilon()) {
            throw std::invalid_argument("Standard deviation must be strictly larger than 0");
        }
        return std::make_unique<GaussianDistribution>(mean, stdev);
    }
    distribution = a_DistributionNode.child("zipfianDistribution");
    if (distribution) {
        double exponent = attributeAsDouble(distribution.attribute("exponent"), a_IsDate, -1.0);
        if (exponent < 0.0) {
            throw std::invalid_argument("Exponent must be larger than or equal to 0");
        }
        return std::make_unique<ZipfianDistribution>(exponent, a_NrOfNodes);
    }
    // TODO(thom): check the Zeta parameters parsing.
    distribution = a_DistributionNode.child("zetaDistribution");
    if (distribution) {
        double alpha = attributeAsDouble(distribution.attribute("alpha"), a_IsDate);
        if (alpha < 1.0 || (alpha - 1.0) <= std::numeric_limits<double>::epsilon()) {
            throw std::invalid_argument("Alpha must be strictly larger than 1");
        }
        return std::make_unique<ZetaDistribution>(alpha);
    }
    distribution = a_DistributionNode.child("exponentialDistribution");
    if (distribution) {
        double scale = attributeAsDouble(distribution.attribute("scale"), a_IsDate, -1.0);
        if (scale <= 0.0 || scale <= std::numeric_limits<double>::epsilon()) {
            throw std::invalid_argument("Scale must be strictly larger than 0");
        }
        return std::make_unique<ExponentialDistribution>(scale);
    }
    distribution = a_DistributionNode.child("lognormalDistribution");
    if (distribution) {
        double mean = attributeAsDouble(distribution.attribute("mean"), a_IsDate);
        double stdev = attributeAsDouble(distribution.attribute("stdev"), a_IsDate, -1.0);
        if (stdev <= 0.0 || stdev <= std::numeric_limits<double>::epsilon()) {
            throw std::invalid_argument("Standard deviation must be strictly larger than 0");
        }
        return std::make_unique<LogNormalDistribution>(mean, stdev);
    }
    throw std::invalid_argument("Distribution must be uniform, gaussian, zipfian, zeta, exponential or lognormal");
}

std::unique_ptr<NumericAttribute> Schema::getNumericAttribute(const pugi::xml_node a_AttributeNode, const bool a_IsDate,
                                             const std::string& a_Name, const bool a_Required, const bool a_Unique) {
    double min = std::numeric_limits<double>::lowest();
    double max = std::numeric_limits<double>::max();
    pugi::xml_attribute min_attr = a_AttributeNode.attribute("min");
    pugi::xml_attribute max_attr = a_AttributeNode.attribute("max");
    if (min_attr) {
        min = attributeAsDouble(min_attr, a_IsDate);
    }
    if (max_attr) {
        max = attributeAsDouble(max_attr, a_IsDate);
    }
    if (min > max) {
        throw std::invalid_argument("Invalid min and max attributes for numeric or date element");
    }
    int precision = a_AttributeNode.attribute("precision").as_int(0);
    if (precision < 0) {
        throw std::invalid_argument("Invalid precision attribute for numeric element");
    }
    // TODO(thom): Number argument for Zipfian should be configurable.
    int number = 1000;
    if (a_IsDate) {
        return std::make_unique<DateAttribute>(a_Name, a_Required, a_Unique, min, max, precision,
                                               getDistribution(a_AttributeNode, number, 0 == precision, true, a_Unique));
    }
    return std::make_unique<NumericAttribute>(a_Name, a_Required, a_Unique, min, max, precision,
                                              getDistribution(a_AttributeNode, number, 0 == precision, false, a_Unique));
}

std::vector<std::unique_ptr<Attribute>> Schema::getAttributes(const pugi::xml_node a_AttributesNode) {
    std::vector<std::unique_ptr<Attribute>> attributes;
    for (pugi::xml_node attribute : a_AttributesNode.children()) {
        const std::string name = attribute.name();
        if (name == "attribute") {
            attributes.emplace_back(parseAttributeNode(attribute));
        } else {
            throw std::invalid_argument("Children of the attributes node must be attribute elements.");
        }
    }
    return attributes;
}

std::unique_ptr<Attribute> Schema::getChoiceAttribute(const pugi::xml_node a_ChoiceNode, const std::string &a_Name,
                                                      bool a_Required, bool a_Unique) {
    std::vector<std::unique_ptr<Attribute>> attributes;
    std::vector<double> probabilities;
    double total_probability = 0;
    for (pugi::xml_node attribute_node : a_ChoiceNode.children()) {
        attributes.emplace_back(getAttributeKind(attribute_node, a_Name, a_Required, a_Unique));
        pugi::xml_attribute probability_attr = attribute_node.attribute("probability");
        if (probability_attr) {
            double probability = probability_attr.as_double(-1.0);
            if (probability < 0.0 || probability > 1.0) {
                throw std::invalid_argument("Attribute choice probability must be in the range [0,1]");
            }
            total_probability += probability;
            probabilities.emplace_back(probability);
        } else if (!probabilities.empty()) {
            throw std::invalid_argument("Probability needs to be specified on all attribute choices or none");
        }
    }

    auto count = attributes.size();
    if (count <= 1) {
        throw std::invalid_argument("Too few choices are defined in the choice element.");
    }
    if (probabilities.empty() || total_probability <= std::numeric_limits<double>::epsilon()) {
        double uniform_probability = 1.0 / static_cast<double>(count);
        probabilities.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            probabilities[i] = uniform_probability;
        }
        total_probability = 1.0;
    }
    const double epsilon = std::numeric_limits<double>::epsilon() * static_cast<double>(count);
    const double deviation = std::abs(total_probability - 1.0);
    if (deviation > m_LenientCategoryProbabilityEpsilon) {
        throw std::invalid_argument("The probabilities of attribute choices need to sum (approximately) to 1");
    }
    if (deviation > epsilon) {
        // Normalize the probabilities.
        for (auto& probability : probabilities) {
            probability /= total_probability;
        }
        total_probability = 1.0;
    }

    std::map<double, std::unique_ptr<Attribute>> probabilitiesAttributes;
    double cumulativeProbability = 0.0;
    for (size_t i = 0; i < count; ++i) {
        cumulativeProbability += probabilities[i];
        probabilitiesAttributes[cumulativeProbability] = std::move(attributes[i]);
    }

    return std::make_unique<ChoiceAttribute>(a_Name, a_Required, a_Unique, std::move(probabilitiesAttributes), total_probability);
}

std::unique_ptr<Attribute> Schema::parseAttributeNode(const pugi::xml_node a_AttributeNode) {
    std::string name = a_AttributeNode.attribute("name").as_string();
    if (name.empty()) {
        throw std::invalid_argument("Attribute name cannot be empty");
    }
    bool required = a_AttributeNode.attribute("required").as_bool();
    bool unique = a_AttributeNode.attribute("unique").as_bool();
    pugi::xml_node kind = a_AttributeNode.first_child();
    if (kind && !kind.next_sibling()) {
        return getAttributeKind(kind, name, required, unique);
    }
    throw std::invalid_argument("The attribute node must have a single child node.");
}

std::unique_ptr<Attribute> Schema::getAttributeKind(const pugi::xml_node a_AttributeKindNode, const std::string &a_Name,
                                                    bool a_Required, bool a_Unique) {
    const std::string kind = a_AttributeKindNode.name();
    if (kind.empty()) {
        throw std::invalid_argument("Attribute kind node name cannot be empty");
    }
    if (kind == "date") {
        return getNumericAttribute(a_AttributeKindNode, true, a_Name, a_Required, a_Unique);
    }
    if (kind == "numeric") {
        return getNumericAttribute(a_AttributeKindNode, false, a_Name, a_Required, a_Unique);
    }
    if (kind == "categorical") {
        std::map<std::string, double> categories = getCategories(a_AttributeKindNode);
        return std::make_unique<CategoricalAttribute>(a_Name, a_Required, a_Unique, categories);
    }
    if (kind == "regex") {
        std::string regex = a_AttributeKindNode.text().as_string();
        if (regex.empty()) {
            throw std::invalid_argument("Attribute regex cannot be empty");
        }
        return std::make_unique<RegexAttribute>(a_Name, a_Required, a_Unique, regex);
    }
    if (kind == "choice") {
        return getChoiceAttribute(a_AttributeKindNode, a_Name, a_Required, a_Unique);
    }
    throw std::invalid_argument("Attribute must be numeric, categorical, regex, date or choice");
}

std::pair<std::map<std::string, double>, double> Schema::getCategoriesFromFile(const std::string& a_FileName) {
    std::map<std::string, double> categories;
    double total_probability = 0;
    std::ifstream file(a_FileName);
    if (!file.is_open()) {
        throw std::invalid_argument("Cannot open category file.");
    }
    std::string line;
    double probability = 0.0;
    std::string name;
    std::smatch match;
    bool hasMatch;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            hasMatch = false;
            if (std::regex_match(line, match, m_CSVParseRegex)) {
                if (match.size() == 4) {
                    if (match[1].matched) {
                        name = match[1].str();
                    } else {
                        name = match[2].str();
                    }
                    try {
                        probability = std::stod(match[3].str());
                    } catch (std::logic_error&) {
                        throw std::invalid_argument("Category probability must be a valid number.");
                    }
                    if (probability < 0.0 || probability > 1.0) {
                        throw std::invalid_argument("Category probability must be in the range [0,1]");
                    }
                    total_probability += probability;
                    hasMatch = true;
                }
            }
            if (!hasMatch) {
                if (total_probability > 0.0) {
                    throw std::invalid_argument("Probability needs to be specified on all categories or none");
                }
                name = line;
                probability = 0.0;
            }
            if (categories.find(name) != categories.end()) {
                throw std::invalid_argument("Category defined multiple times");
            }
            categories[name] = probability;
        }
    }
    file.close();
    return make_pair(categories, total_probability);
}

std::pair<std::map<std::string, double>, double> Schema::getCategoriesFromSchema(const pugi::xml_object_range<pugi::xml_named_node_iterator> a_CategoriesIterator) {
    std::map<std::string, double> categories;
    double total_probability = 0;
    for (pugi::xml_node category : a_CategoriesIterator) {
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
    }
    return make_pair(categories, total_probability);
}

std::map<std::string, double> Schema::getCategories(const pugi::xml_node a_categoriesNode) {
    std::map<std::string, double> categories;
    double total_probability = 0;
    pugi::xml_attribute file_attr = a_categoriesNode.attribute("file");
    const auto& categoriesIterator = a_categoriesNode.children("category");
    if (file_attr && categoriesIterator.begin() != categoriesIterator.end()) {
        throw std::invalid_argument("You cannot specify categories in-schema and also point to a file.");
    }
    if (file_attr) {
        const std::string& fileName = file_attr.as_string();
        if (fileName.empty()) {
            throw std::invalid_argument("Category file name cannot be empty.");
        }
        auto category_pair = getCategoriesFromFile(fileName);
        categories = category_pair.first;
        total_probability = category_pair.second;
    } else {
        auto category_pair = getCategoriesFromSchema(categoriesIterator);
        categories = category_pair.first;
        total_probability = category_pair.second;
    }
    auto count = categories.size();
    if (count == 0) {
        throw std::invalid_argument("No categories are defined in the category element or file.");
    }
    if (total_probability <= std::numeric_limits<double>::epsilon()) {
        double uniform_probability = 1.0 / static_cast<double>(count);
        for (auto &category : categories) {
            category.second = uniform_probability;
        }
        total_probability = 1.0;
    }
    const double epsilon = std::numeric_limits<double>::epsilon() * static_cast<double>(count);
    const double deviation = std::abs(total_probability - 1.0);
    if (deviation > m_LenientCategoryProbabilityEpsilon) {
        throw std::invalid_argument("The probabilities of categories need to sum (approximately) to 1");
    }
    if (deviation > epsilon) {
        // Normalize the probabilities.
        for (auto &[key, value] : categories) {
            value /= total_probability;
        }
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
