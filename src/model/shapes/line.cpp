
#include <iostream>

#include "line.h"


Line::Line(std::string id)
    : m_id(std::move(id))
{
    std::cout << " Hello from " << __func__ << '\n';
}

std::unique_ptr<IShape> Line::clone() const 
{
    std::cout << " Hello from " << __func__ << '\n';
    return nullptr;
}

std::string Line::id() const 
{
    std::cout << " Hello from " << __func__ << '\n';
    return m_id;
}

ShapeType Line::type() const 
{
    std::cout << " Hello from " << __func__ << '\n';
    return ShapeType::Line;
}

std::string Line::serialize() const 
{
    std::cout << " Hello from " << __func__ << '\n';
    return std::string{"LINE "} + m_id;
}
