#pragma once

#include <memory>
#include <string>
#include <vector>

#include "shape.h"


/// @brief Document that owns graphical primitives
/// @details Owns shapes via std::unique_ptr for explicit ownership
class Document 
{
public:
    /// @brief Clear document, remove all shapes
    void Clear();

    /// @brief Add shape to the document (takes ownership)
    /// @param shape     unique_ptr to shape
    void AddShape(std::unique_ptr<IShape> shape);

    /// @brief Remove shape by id
    /// @param id        string id
    /// @return          true if found and removed
    bool RemoveShapeById(const std::string& id);

    /// @brief Extract (detach) shape by id
    /// @param id        string id
    /// @return          unique_ptr to shape or nullptr if not found
    std::unique_ptr<IShape> ExtractShapeById(const std::string& id);

    /// @brief Check if shape exists by id
    /// @param id        string id
    /// @return          true if exists
    bool HasShape(const std::string& id) const;

    /// @brief Access shapes (const)
    /// @return          const reference to shapes container
    const std::vector<std::unique_ptr<IShape>>& Shapes() const;

private:
    std::vector<std::unique_ptr<IShape>> m_shapes;
};
