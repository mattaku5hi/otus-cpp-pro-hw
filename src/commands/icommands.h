#pragma once


/// @brief Base command interface (Command pattern)
class ICommand
{
public:
    /// @brief Virtual destructor
    virtual ~ICommand() = default;

    /// @brief Execute command
    /// @return true on success
    virtual bool Execute() = 0;

    /// @brief Undo command
    /// @return true on success
    virtual bool Undo() = 0;
};