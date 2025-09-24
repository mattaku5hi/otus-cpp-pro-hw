#pragma once

#include <memory>
#include <string>

#include "shape.h"


/// @brief Rectangle primitive (minimal stub)
class Rectangle : public IShape 
{
public:
    /// @brief Construct a rectangle with id
    /// @param id       unique shape identifier
    Rectangle(std::string id);

    /// @brief Virtual destructor
    ~Rectangle() override = default;

    /// @brief Clone this shape
    /// @return unique_ptr to the copy
    std::unique_ptr<IShape> clone() const override;

    /// @brief Shape id
    /// @return string id
    std::string id() const override;

    /// @brief Type of shape
    /// @return ShapeType::Rectangle
    ShapeType type() const override;

    /// @brief Serialize shape to string
    /// @return textual representation
    std::string serialize() const override;

private:
    std::string m_id;
    // Minimal parameters (stub)
    int m_x = 0; 
    int m_y = 0;
    int m_w = 10;
    int m_h = 10;
};
