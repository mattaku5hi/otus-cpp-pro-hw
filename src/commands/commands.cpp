
#include "commands.h"
#include <iostream>

CreateShapeCommand::CreateShapeCommand(Document& doc, const IShapeFactory& factory, ShapeType type, const std::string& id)
    : m_doc(doc), m_factory(factory), m_type(type), m_id(id)
{
    std::cout << " Hello from " << __func__ << '\n';
}

bool CreateShapeCommand::Execute()
{
    std::cout << " Hello from " << __func__ << '\n';
    return true;
}

bool CreateShapeCommand::Undo()
{
    std::cout << " Hello from " << __func__ << '\n';
    return true;
}

DeleteShapeCommand::DeleteShapeCommand(Document& doc, const std::string& id)
    : m_doc(doc), m_id(id)
{
    std::cout << " Hello from " << __func__ << '\n';
}

bool DeleteShapeCommand::Execute()
{
    std::cout << " Hello from " << __func__ << '\n';
    return true;
}

bool DeleteShapeCommand::Undo()
{
    std::cout << " Hello from " << __func__ << '\n';
    return true;
}
