#ifndef GMARK_NODE_ATTRIBUTE_GENERATOR_H
#define GMARK_NODE_ATTRIBUTE_GENERATOR_H

#include "configuration.h"

class NodeAttributeGenerator {
protected:
    const Configuration &m_Config;
public:
    NodeAttributeGenerator(const Configuration &a_Config) : m_Config(a_Config) {}
    void generateAttributes(std::ostream &a_OutputStream);
};

#endif //GMARK_NODE_ATTRIBUTE_GENERATOR_H
