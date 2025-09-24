
#include <iostream>

#include "serializer.h"


SimpleTextSerializer::SimpleTextSerializer(const IShapeFactory& factory)
{
    std::cout << " Hello from " << __func__ << '\n';
    m_factory = &factory;
}

bool SimpleTextSerializer::saveToFile(const Document& /*doc*/, const std::string& /*path*/) {
    std::cout << " Hello from " << __func__ << '\n';
    return true;
}

bool SimpleTextSerializer::loadFromFile(Document& /*doc*/, const std::string& /*path*/) {
    std::cout << " Hello from " << __func__ << '\n';
    return true;
}
