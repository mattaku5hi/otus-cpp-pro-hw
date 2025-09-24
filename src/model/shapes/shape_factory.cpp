#include <iostream>

#include "shape_factory.h"


std::unique_ptr<IShape> DefaultShapeFactory::Create(ShapeType /*type*/, const std::string &/*id*/) const
{
    std::cout << " Hello from " << __func__ << '\n';
    return nullptr;
}

