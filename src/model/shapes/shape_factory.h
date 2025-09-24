#pragma once


#include <memory>
#include <string>

#include "shape.h"


/// @brief Shape factory interface
class IShapeFactory
{
public:
    /// @brief Virtual destructor
    virtual ~IShapeFactory() = default;

    /// @brief Create shape by type and id
    /// @param type     shape type
    /// @param id       textual identifier
    /// @return         unique_ptr to created shape
    virtual std::unique_ptr<IShape> Create(ShapeType type, const std::string& id) const = 0;
};

/// @brief Default shape factory implementation
class DefaultShapeFactory : public IShapeFactory
{
public:
    /// @brief Create shape by type and id
    /// @param type     shape type
    /// @param id       textual identifier
    /// @return         unique_ptr to created shape
    std::unique_ptr<IShape> Create(ShapeType type, const std::string& id) const override;
};

