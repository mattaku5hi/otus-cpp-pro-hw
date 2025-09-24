#pragma once


#include <memory>
#include <string>


// forward declarations
class Document;
class IShapeFactory;


/// @brief Document serializer interface
class IDocumentSerializer 
{
public:
    /// @brief Virtual destructor
    virtual ~IDocumentSerializer() = default;

    /// @brief Save document to file
    /// @param doc       document to save
    /// @param path      file path
    /// @return          true on success
    virtual bool saveToFile(const Document& doc, const std::string& path) = 0;

    /// @brief Load document from file
    /// @param doc       destination document
    /// @param path      file path
    /// @return          true on success
    virtual bool loadFromFile(Document& doc, const std::string& path) = 0;
};

/// @brief Simple text serializer: one line per primitive
class SimpleTextSerializer : public IDocumentSerializer 
{
public:
    /// @brief Construct serializer with a shape factory
    /// @param factory   external factory used to create shapes when loading
    explicit SimpleTextSerializer(const IShapeFactory& factory);

    /// @brief Save document to file
    /// @param doc       document to save
    /// @param path      file path
    /// @return          true on success
    bool saveToFile(const Document& doc, const std::string& path) override;

    /// @brief Load document from file
    /// @param doc       destination document
    /// @param path      file path
    /// @return          true on success
    bool loadFromFile(Document& doc, const std::string& path) override;

private:
    const IShapeFactory* m_factory = nullptr;
};
