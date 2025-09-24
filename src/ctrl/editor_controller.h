#pragma once

#include <memory>
#include <string>
#include <vector>

#include "commands.h"
#include "document.h"
#include "id_generator.h"
#include "serializer.h"
#include "shape.h"
#include "shape_factory.h"


/// @brief Editor controller (MVC Controller)
/// @details Encapsulates operations on the document model and I/O via the serializer
class EditorController 
{
public:
    /// @brief Create an editor controller
    /// @param factory      external shape factory (stored as a pointer)
    /// @param serializer   serializer implementation, transferred by ownership
    EditorController(const IShapeFactory& factory, std::unique_ptr<IDocumentSerializer> serializer);

    /// @brief Create a new document (clear current state)
    /// @return always true (for consistency)
    bool NewDocument();

    /// @brief Import a document from a file
    /// @param path      file path
    /// @return          true on success
    bool Import(const std::string& path);

    /// @brief Export the document to a file
    /// @param path      file path
    /// @return          true on success
    bool Export(const std::string& path) const;

    /// @brief Create a primitive and add it to the document
    /// @param type      primitive type
    /// @param id        identifier
    /// @return          true on success
    bool CreateShape(ShapeType type, const std::string& id);

    /// @brief Create a primitive with an auto-generated identifier
    /// @param type      primitive type
    /// @param prefix    prefix for the id generator (optional, defaults to the type)
    /// @return          generated identifier (empty string on error)
    std::string CreateShapeAutoId(ShapeType type, const std::string& prefix = "");

    /// @brief Delete a primitive by identifier
    /// @param id        identifier
    /// @return          true if deleted
    bool DeleteShape(const std::string& id);

    /// @brief Undo the last command
    /// @return          true on success
    bool Undo();

    /// @brief Redo the last undone command
    /// @return          true on success
    bool Redo();

    /// @brief Whether Undo/Redo is possible
    bool CanUndo() const;
    bool CanRedo() const;

    /// @brief Access to the document (const)
    /// @return          reference to the document model
    const Document& GetDocument() const;

private:
    Document m_doc;
    std::unique_ptr<IDocumentSerializer> m_serializer;
    const IShapeFactory* m_factory = nullptr;
    IdGenerator m_idGen;
    std::vector<std::unique_ptr<ICommand>> m_undoStack;
    std::vector<std::unique_ptr<ICommand>> m_redoStack;
};
