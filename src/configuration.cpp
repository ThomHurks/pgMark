#include "configuration.h"

Configuration::Configuration(const std::string &a_Filename, const int a_GraphSize) {
    assert(a_GraphSize > 0);
    pugi::xml_document doc = openFile(a_Filename);
    pugi::xml_node root = doc.child("pgmark");
    if (!root) {
        throw std::invalid_argument("File does not have a pgmark element as root");
    }
    pugi::xml_node types_node = root.child("types");
    pugi::xml_node predicates_node = root.child("predicates");
    m_Schema = std::make_unique<Schema>(types_node, predicates_node, a_GraphSize);
    m_TypeRanges = computeTypeRanges();
}

pugi::xml_document Configuration::openFile(const std::string &a_Filename) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(a_Filename.c_str());
    if (result) {
        return doc;
    }
    std::cerr << "XML file '" << a_Filename << "' parsed with errors\n";
    std::cerr << "Error description: " << result.description() << "\n";
    std::cerr << "Error offset: " << result.offset << "\n";
    throw std::invalid_argument("Invalid XML input file");
}

const std::map<std::string, std::pair<int, int>> Configuration::computeTypeRanges() const {
    std::map<std::string, std::pair<int, int>> type_ranges;

    const auto &types = getTypeNames();
    int node_count = 0;
    for (const auto &type : types) {
        const auto nr_nodes = getConstraint(type);
        assert(nr_nodes > 0);
        type_ranges.emplace(type, std::make_pair(node_count, node_count + nr_nodes - 1));
        node_count += nr_nodes;
    }
    return type_ranges;
}
