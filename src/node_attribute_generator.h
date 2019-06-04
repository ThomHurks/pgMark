#ifndef GMARK_NODE_ATTRIBUTE_GENERATOR_H
#define GMARK_NODE_ATTRIBUTE_GENERATOR_H

#include "configuration.h"

class NodeAttributeGenerator {
protected:
    const Configuration &m_Config;

    void generateRandomAttributes(const std::string &a_TypeName, std::ostream &a_OutputStream) const;

    void generateNodeAttributes(const std::unique_ptr<Attribute> &a_Attribute, const int a_StartId,
                                const int a_EndId, std::ostream &a_OutputStream) const;

public:
    explicit NodeAttributeGenerator(const Configuration &a_Config) : m_Config(a_Config) {}

    void generateAttributes(std::ostream &a_OutputStream);
};

#endif //GMARK_NODE_ATTRIBUTE_GENERATOR_H
