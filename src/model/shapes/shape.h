#pragma once


#include <memory>
#include <string>


/// @brief Supported primitive types
enum class ShapeType 
{
    Line,
    Rectangle,
};

/// @brief Base interface for graphical primitives
/// @details Provides polymorphic API for storing and serializing shapes
class IShape 
{
public:
    /// @brief Virtual destructor
    virtual ~IShape() = default;

    /// @brief Clone object
    /// @return unique_ptr to a new object of the same concrete type
    virtual std::unique_ptr<IShape> clone() const = 0;

    /// @brief Shape identifier
    /// @return textual id
    virtual std::string id() const = 0;

    /// @brief Shape type
    /// @return ShapeType enum value
    virtual ShapeType type() const = 0;

    /// @brief Serialize shape into a simple text line
    /// @return string textual representation
    virtual std::string serialize() const = 0;
};
