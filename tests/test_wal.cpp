#include <gtest/gtest.h>
#include "../WALManager/WALManager.hpp"
#include "../Engine/Engine.hpp"
#include "../Catalog/Catalog.hpp"
#include <fstream>
#include <cstdio>

class WALManagerTest : public ::testing::Test {
protected:
    std::string testWalFile = "test_engine.wal";

    void SetUp() override {
        std::remove(testWalFile.c_str());
    }

    void TearDown() override {
        std::remove(testWalFile.c_str());
    }

    // Helper to read raw file content for verification
    std::vector<std::string> readWalLines() {
        std::vector<std::string> lines;
        std::ifstream file(testWalFile);
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        return lines;
    }
};

TEST_F(WALManagerTest, LogInsertCreatesEntry) {
    WALManager wal(testWalFile);
    wal.logInsert("users", "1|Alice|25");

    auto lines = readWalLines();
    ASSERT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0], "INSERT|users|1|Alice|25|0");
}

TEST_F(WALManagerTest, CommitUpdatesEntry) {
    WALManager wal(testWalFile);
    wal.logInsert("users", "1|Alice|25");
    wal.commit();

    auto lines = readWalLines();
    ASSERT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0], "INSERT|users|1|Alice|25|1");
}

TEST_F(WALManagerTest, MultipleInsertsAndCommit) {
    WALManager wal(testWalFile);
    wal.logInsert("users", "1|Alice|25");
    wal.logInsert("users", "2|Bob|30");
    wal.commit(); // Commits the last one

    auto lines = readWalLines();
    ASSERT_EQ(lines.size(), 2);
    EXPECT_EQ(lines[0], "INSERT|users|1|Alice|25|0");
    EXPECT_EQ(lines[1], "INSERT|users|2|Bob|30|1");
}
TEST_F(WALManagerTest, RecoverReplaysUncommitted) {
    {
        WALManager wal(testWalFile);
        wal.logInsert("users", "1|Alice|25");
    }

    Catalog catalog;
    std::vector<Columns> cols = {{"id", "INT"}, {"name", "TEXT"}, {"age", "INT"}};
    catalog.createTable("users", cols);

    {
        Table table("users", cols);
        table.dropStorage();
    }

    std::remove("engine.wal");
    std::rename(testWalFile.c_str(), "engine.wal");

    Engine engine(catalog);

    auto results = engine.query("SELECT * FROM users");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].values[0], "1");
    EXPECT_EQ(results[0].values[1], "Alice");
    EXPECT_EQ(results[0].values[2], "25");

    std::remove("engine.wal");
}