#include <iostream>
#include <memory>

#include "editor_controller.h"
#include "serializer.h"
#include "shape.h"
#include "shape_factory.h"


/// @brief Event handler: create a new document
/// @param ctrl     editor controller
void handleCreateNewDocument(EditorController &ctrl)
{
    bool status = ctrl.NewDocument();
    if(status == false)
    {
        std::cerr << "NewDocument failed" << '\n';
    }
}

/// @brief Event handler: import document from file
/// @param ctrl     editor controller
/// @param filename file path
void handleImportDocument(EditorController &ctrl, const std::string &filename)
{
    bool status = ctrl.Import(filename);
    if(status == false)
    {
        std::cerr << "Import failed: " << filename << '\n';
    }
}

/// @brief Event handler: export document to file
/// @param ctrl     editor controller
/// @param filename file path
void handleExportDocument(EditorController &ctrl, const std::string &filename)
{
    bool status = ctrl.Export(filename);
    if(status == false)
    {
        std::cerr << "Export failed: " << filename << '\n';
    }
}

/// @brief Event handler: create shape by textual type and id
/// @param ctrl     editor controller
/// @param type     textual type ("line", "rect"/"rectangle")
/// @param id       shape id
void handleCreateShape(EditorController &ctrl, const std::string &type, const std::string &id)
{
    ShapeType st = ShapeType::Line;
    if(type == "line")
    {
        st = ShapeType::Line;
    }
    else if(type == "rect" || type == "rectangle")
    {
        st = ShapeType::Rectangle;
    }
    else
    {
        std::cerr << "Unknown shape type: " << type << '\n';
        return;
    }

    bool status = ctrl.CreateShape(st, id);
    if(status == false)
    {
        std::cerr << "CreateShape failed: type=" << type << ", id=" << id << '\n';
    }
}

/// @brief Event handler: delete shape by id
/// @param ctrl     editor controller
/// @param id       shape id
void handleDeleteShape(EditorController &ctrl, const std::string &id)
{
    bool status = ctrl.DeleteShape(id);
    if(status == false)
    {
        std::cerr << "DeleteShape failed: id=" << id << '\n';
    }
}

/// @brief Event handler: render document (console dump)
/// @param ctrl     editor controller (const)
void handleRenderDocument(const EditorController &ctrl)
{
    const auto& shapes = ctrl.GetDocument().Shapes();
    for(const auto& s : shapes)
    {
        if(s != nullptr)
        {
            std::cout << s->serialize() << '\n';
        }
    }
}

/// @brief          Entry point
/// @return         0 on success
int main(int, char **)
{
    DefaultShapeFactory factory;
    auto serializer = std::make_unique<SimpleTextSerializer>(factory);
    EditorController controller(factory, std::move(serializer));

    handleCreateNewDocument(controller);
    handleCreateShape(controller, "line", "line-1");
    handleCreateShape(controller, "rectangle", "rect-1");
    handleRenderDocument(controller);

    const std::string path = "demo_document.txt";
    handleExportDocument(controller, path);

    // Импорт обратно для проверки простого потока
    handleCreateNewDocument(controller);
    handleImportDocument(controller, path);
    handleRenderDocument(controller);

    std::cout << "Editor skeleton is running. Document imported from: " << path << '\n';
    return 0;
}
