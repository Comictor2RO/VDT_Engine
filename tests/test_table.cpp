#include <gtest/gtest.h>
#include "../Table/Table.hpp"

//Create Table with Schema
TEST(TableTest, CreateTableWithValidSchema) {
    std::vector<Columns> schema = {
        {"id","INT"},
        {"name", "STRING"}
    };

    Table table("test_users", schema);

    SUCCEED();
}

//Test 2: Insert Valid Row
TEST(TableTest, InsertValidRow) {
    std::vector<Columns> schema = {
      {"id", "INT"},
        {"name", "STRING"}
    };

    Table table("test_insert", schema);

    Row row;
    row.values = {"1", "John"};
    auto result = table.insertRow(row);

    EXPECT_TRUE(result.has_value());
}

//Test 3: Insert Row with Schema Mismatch
TEST(TableTest, InsertSchemaMismatch) {
    std::vector<Columns> schema = {
        {"id", "INT"},
        {"name", "STRING"}
    };
    Table table("test_mismatch", schema);

    Row row;
    row.values = {"1"};
    auto result = table.insertRow(row);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), TableError::SCHEME_MISMATCH);
}

//Test 4: Select All Rows
TEST(TableTest, SelectAllRows) {
    std::vector<Columns> schema = {
        {"id", "INT"},
        {"name", "STRING"}
    };
    Table table("test_select", schema);

    // Clean up any existing data
    table.dropStorage();

    table.insertRow({{"1", "John"}});
    table.insertRow({{"2", "Jane"}});

    auto result = table.selectRow(nullptr);
    EXPECT_EQ(result.size(), 2);
}

//Test 5: Select with CONDITION
TEST(TableTest, SelectWithCondition) {
    std::vector<Columns> schema = {
        {"id", "INT"},
        {"name", "STRING"}
    };
    Table table("test_condition", schema);

    // Clean up any existing data
    table.dropStorage();

    table.insertRow({{"1", "John"}});
    table.insertRow({{"2", "Jane"}});

    Condition cond = {"id", "1", "="};

    auto result = table.selectRow(&cond);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].values[1], "John");
}

// Test 6: Insert with invalid type (non-INT for INT column)
TEST(TableTest, InsertInvalidType) {
    std::vector<Columns> schema = {
        {"id", "INT"},
        {"name", "STRING"}
    };
    Table table("test_invalid_type", schema);
    table.dropStorage();

    Row row;
    row.values = {"abc", "John"}; // Invalid INT
    auto result = table.insertRow(row);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), TableError::TYPE_VALIDATION_FAILED);
}

// Test 7: Delete with condition
TEST(TableTest, DeleteWithCondition) {
    std::vector<Columns> schema = {
        {"id", "INT"},
        {"name", "STRING"}
    };
    Table table("test_delete", schema);
    table.dropStorage();

    table.insertRow({{"1", "John"}});
    table.insertRow({{"2", "Jane"}});
    table.insertRow({{"3", "Bob"}});

    Condition cond = {"id", "2", "="};
    table.deleteRow(&cond);

    auto result = table.selectRow(nullptr);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].values[0], "1");
    EXPECT_EQ(result[1].values[0], "3");
}

// Test 8: Delete all rows (no condition)
TEST(TableTest, DeleteAllRows) {
    std::vector<Columns> schema = {
        {"id", "INT"},
        {"name", "STRING"}
    };
    Table table("test_delete_all", schema);
    table.dropStorage();

    table.insertRow({{"1", "John"}});
    table.insertRow({{"2", "Jane"}});

    table.deleteRow(nullptr);

    auto result = table.selectRow(nullptr);
    EXPECT_EQ(result.size(), 0);
}

// Test 9: Update with condition
TEST(TableTest, UpdateWithCondition) {
    std::vector<Columns> schema = {
        {"id", "INT"},
        {"name", "STRING"}
    };
    Table table("test_update", schema);
    table.dropStorage();

    table.insertRow({{"1", "John"}});
    table.insertRow({{"2", "Jane"}});

    Condition cond = {"id", "1", "="};
    std::vector<std::pair<std::string, std::string>> assignments = {{"name", "Johnny"}};
    table.updateRow(&cond, assignments);

    auto result = table.selectRow(&cond);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].values[1], "Johnny");
}

// Test 10: Select with different operators (>, <, >=, <=, !=)
TEST(TableTest, SelectWithDifferentOperators) {
    std::vector<Columns> schema = {
        {"id", "INT"},
        {"age", "INT"}
    };
    Table table("test_operators", schema);
    table.dropStorage();

    table.insertRow({{"1", "18"}});
    table.insertRow({{"2", "25"}});
    table.insertRow({{"3", "30"}});

    // Test >
    Condition cond1 = {"age", "20", ">"};
    auto result1 = table.selectRow(&cond1);
    EXPECT_EQ(result1.size(), 2);

    // Test <
    Condition cond2 = {"age", "26", "<"};
    auto result2 = table.selectRow(&cond2);
    EXPECT_EQ(result2.size(), 2);

    // Test !=
    Condition cond3 = {"age", "25", "!="};
    auto result3 = table.selectRow(&cond3);
    EXPECT_EQ(result3.size(), 2);
}