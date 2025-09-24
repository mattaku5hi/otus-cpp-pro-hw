
#include <iostream>

#include "document.h"


void Document::Clear() 
{ 
    std::cout << " Hello from " << __func__ << '\n'; 
    return;
}

void Document::AddShape(std::unique_ptr<IShape> /*shape*/) 
{ 
    std::cout << " Hello from " << __func__ << '\n'; 
    return;
}

bool Document::RemoveShapeById(const std::string& /*id*/) 
{ 
    std::cout << " Hello from " << __func__ << '\n'; 
    return true; 
}

std::unique_ptr<IShape> Document::ExtractShapeById(const std::string& /*id*/) 
{ 
    std::cout << " Hello from " << __func__ << '\n'; 
    return nullptr; 
}

bool Document::HasShape(const std::string& /*id*/) const 
{ 
    std::cout << " Hello from " << __func__ << '\n'; 
    return false; 
}

const std::vector<std::unique_ptr<IShape>>& Document::Shapes() const 
{ 
    std::cout << " Hello from " << __func__ << '\n'; 
    return m_shapes; 
}

