
#include <iostream>

#include "editor_controller.h"


EditorController::EditorController(const IShapeFactory& factory, std::unique_ptr<IDocumentSerializer> serializer)
    : m_serializer(std::move(serializer))
{
    std::cout << " Hello from " << __func__ << '\n';
    m_factory = &factory;
}

bool EditorController::NewDocument() 
{
    std::cout << " Hello from " << __func__ << '\n';
    return true;
}

bool EditorController::Import(const std::string& path) 
{
    std::cout << " Hello from " << __func__ << '\n';
    static_cast<void>(path);
    return true;
}

bool EditorController::Export(const std::string& path) const 
{
    std::cout << " Hello from " << __func__ << '\n';
    static_cast<void>(path);
    return true;
}

bool EditorController::CreateShape(ShapeType type, const std::string& id) 
{
    std::cout << " Hello from " << __func__ << '\n';
    static_cast<void>(type);
    static_cast<void>(id);
    return true;
}

std::string EditorController::CreateShapeAutoId(ShapeType type, const std::string& prefix)
{
    std::cout << " Hello from " << __func__ << '\n';
    static_cast<void>(type);
    static_cast<void>(prefix);
    return std::string{};
}

bool EditorController::DeleteShape(const std::string& id) 
{
    std::cout << " Hello from " << __func__ << '\n';
    static_cast<void>(id);
    return true;
}

bool EditorController::Undo()
{
    std::cout << " Hello from " << __func__ << '\n';
    return true;
}

bool EditorController::Redo()
{
    std::cout << " Hello from " << __func__ << '\n';
    return true;
}

bool EditorController::CanUndo() const
{
    std::cout << " Hello from " << __func__ << '\n';
    return false;
}

bool EditorController::CanRedo() const
{
    std::cout << " Hello from " << __func__ << '\n';
    return false;
}

const Document& EditorController::GetDocument() const 
{ 
    return m_doc; 
}

