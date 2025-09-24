#pragma once

#include <string>
#include <memory>

#include "shape.h"


/// @brief Line primitive with two endpoints (minimal stub)
class Line : public IShape 
{
public:
    /// @brief Construct a line with id
    /// @param id       unique shape identifier
    Line(std::string id);

    /// @brief Virtual destructor
    ~Line() override = default;

    /// @brief Clone this shape
    /// @return unique_ptr to the copy
    std::unique_ptr<IShape> clone() const override;

    /// @brief Shape id
    /// @return string id
    std::string id() const override;

    /// @brief Type of shape
    /// @return ShapeType::Line
    ShapeType type() const override;

    /// @brief Serialize shape to string
    /// @return textual representation
    std::string serialize() const override;

private:
    std::string m_id;
    // Minimal coordinates (stub)
    int m_x1 = 0;
    int m_y1 = 0;
    int m_x2 = 10;
    int m_y2 = 10;
};
