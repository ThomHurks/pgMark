#include "node_attribute_generator.h"

void NodeAttributeGenerator::generateAttributes(std::ostream &a_OutputStream) {
    for (const auto &type : m_Config.getTypeNames()) {
        generateRandomAttributes(type, a_OutputStream);
    }
}

void NodeAttributeGenerator::generateRandomAttributes(const std::string &a_TypeName,
                                                      std::ostream &a_OutputStream) const {
    const auto &attributes = m_Config.getTypeAttributes(a_TypeName);
    if (!attributes.empty()) {
        const auto &type_range = m_Config.getTypeRange(a_TypeName);
        a_OutputStream << "### NODE ATTRIBUTES ###" << "\n";
        for (const auto &attribute : attributes) {
            generateNodeAttributes(attribute, type_range.first, type_range.second, a_OutputStream);
        }
    }
}

void NodeAttributeGenerator::generateNodeAttributes(const std::unique_ptr<Attribute> &a_Attribute, const int a_StartId,
                                                    const int a_EndId, std::ostream &a_OutputStream) const {
    int amount = a_EndId - a_StartId + 1;
    auto &attribute_name = a_Attribute->getName();
    assert(amount > 0);
    assert(!attribute_name.empty());
    for (int node_id = a_StartId; node_id <= a_EndId; ++node_id) {
        std::string random_attribute = a_Attribute->getRandomAttribute();
        a_OutputStream << node_id << ',' << attribute_name << ',' << random_attribute << "\n";
    }
}
