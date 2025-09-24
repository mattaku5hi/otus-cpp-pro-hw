#include <gtest/gtest.h>

#include "editor_controller.h"
#include "serializer.h"
#include "shape_factory.h"
#include "shape.h"
#include "id_generator.h"

/// @brief Базовый тест генератора идентификаторов
TEST(TestIdGenerator, GeneratesUniqueIds)
{
    IdGenerator gen;
    std::string a = gen.Generate("line");
    std::string b = gen.Generate("line");
    EXPECT_FALSE(a.empty());
    EXPECT_FALSE(b.empty());
    EXPECT_NE(a, b);
}

/// @brief Smoke: controller calls (create/undo/redo)
TEST(TestController, CreateUndoRedo)
{
    DefaultShapeFactory factory;
    auto serializer = std::make_unique<SimpleTextSerializer>(factory);
    EditorController ctrl(factory, std::move(serializer));

    bool status = ctrl.NewDocument();
    if(status == false)
    {
        GTEST_FAIL() << std::string("NewDocument failed");
    }

    // Creating with auto id should not crash; content is irrelevant in stub mode
    std::string id = ctrl.CreateShapeAutoId(ShapeType::Line, "line");
    (void)id;

    // Undo/Redo should succeed as stubs
    status = ctrl.Undo();
    if(status == false)
    {
        GTEST_FAIL() << std::string("Undo failed");
    }

    status = ctrl.Redo();
    if(status == false)
    {
        GTEST_FAIL() << std::string("Redo failed");
    }

    // Optional: stubs report no history
    EXPECT_FALSE(ctrl.CanUndo());
    EXPECT_FALSE(ctrl.CanRedo());
}

/// @brief Smoke: delete and undo
TEST(TestController, DeleteUndo)
{
    DefaultShapeFactory factory;
    auto serializer = std::make_unique<SimpleTextSerializer>(factory);
    EditorController ctrl(factory, std::move(serializer));

    bool status = ctrl.NewDocument();
    if(status == false)
    {
        GTEST_FAIL() << std::string("NewDocument failed");
    }

    std::string id = ctrl.CreateShapeAutoId(ShapeType::Rectangle, "rect");
    (void)id;

    status = ctrl.DeleteShape(id);
    if(status == false)
    {
        GTEST_FAIL() << std::string("DeleteShape failed");
    }

    status = ctrl.Undo();
    if(status == false)
    {
        GTEST_FAIL() << std::string("Undo failed");
    }
}

/// @brief Smoke: export/import document
TEST(TestSerializer, SaveLoad)
{
    DefaultShapeFactory factory;
    auto serializer = std::make_unique<SimpleTextSerializer>(factory);
    EditorController ctrl(factory, std::move(serializer));

    bool status = ctrl.NewDocument();
    if(status == false)
    {
        GTEST_FAIL() << std::string("NewDocument failed");
    }

    std::string id1 = ctrl.CreateShapeAutoId(ShapeType::Line, "line");
    std::string id2 = ctrl.CreateShapeAutoId(ShapeType::Rectangle, "rect");
    (void)id1; (void)id2;

    const std::string path = std::string("test_doc.txt");
    status = ctrl.Export(path);
    if(status == false)
    {
        GTEST_FAIL() << std::string("Export failed");
    }

    status = ctrl.NewDocument();
    if(status == false)
    {
        GTEST_FAIL() << std::string("NewDocument failed (2)");
    }

    status = ctrl.Import(path);
    if(status == false)
    {
        GTEST_FAIL() << std::string("Import failed");
    }
}

