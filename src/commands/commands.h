#pragma once


#include <memory>
#include <string>
#include "icommands.h"
#include "shape.h"


// forward declarations
class Document;
class IShapeFactory;


/// @brief Command: create shape
class CreateShapeCommand : public ICommand
{
public:
    /// @brief Construct create command
    /// @param doc       document reference
    /// @param factory   shape factory reference
    /// @param type      shape type
    /// @param id        shape id
    CreateShapeCommand(Document& doc, const IShapeFactory& factory, ShapeType type, const std::string& id);

    /// @brief Execute creation
    /// @return true on success
    bool Execute() override;

    /// @brief Undo creation (remove created shape)
    /// @return true on success
    bool Undo() override;

private:
    Document& m_doc;
    const IShapeFactory& m_factory;
    ShapeType m_type;
    std::string m_id;
    bool m_executed = false;
};

/// @brief Command: delete shape
class DeleteShapeCommand : public ICommand
{
public:
    /// @brief Construct delete command
    /// @param doc       document reference
    /// @param id        id of the shape to delete
    DeleteShapeCommand(Document& doc, const std::string& id);

    /// @brief Execute deletion (extract shape from document)
    /// @return true on success
    bool Execute() override;

    /// @brief Undo deletion (put shape back)
    /// @return true on success
    bool Undo() override;

private:
    Document& m_doc;
    std::string m_id;
    std::unique_ptr<class IShape> m_backup;
};

