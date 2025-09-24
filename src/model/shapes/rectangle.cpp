
#include <iostream>

#include "rectangle.h"


Rectangle::Rectangle(std::string id)
    : m_id(std::move(id))
{
    std::cout << " Hello from " << __func__ << '\n';
}

std::unique_ptr<IShape> Rectangle::clone() const 
{
    std::cout << " Hello from " << __func__ << '\n';
    return nullptr;
}

std::string Rectangle::id() const 
{
    std::cout << " Hello from " << __func__ << '\n';
    return m_id;
}

ShapeType Rectangle::type() const 
{
    std::cout << " Hello from " << __func__ << '\n';
    return ShapeType::Rectangle;
}

std::string Rectangle::serialize() const 
{
    std::cout << " Hello from " << __func__ << '\n';
    return std::string{"RECT "} + m_id;
}

