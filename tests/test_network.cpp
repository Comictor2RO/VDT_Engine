#include <gtest/gtest.h>
#include "../Engine/Engine.hpp"
#include "../Catalog/Catalog.hpp"
#include "../Networking/NetworkServer.hpp"
#include <thread>
#include <chrono>
#include <cstdio>

class NetworkServerTest : public ::testing::Test {
protected:
    Catalog *catalog = nullptr;
    Engine *engine = nullptr;
    NetworkServer *server = nullptr;
    std::thread serverThread;
    const size_t TEST_PORT = 9998;

    void SetUp() override {
        cleanup();
        catalog = new Catalog();
        engine = new Engine(*catalog);
        engine->query("CREATE TABLE net_users (id INT, name STRING)");
        engine->query("INSERT INTO net_users VALUES (1, Alice)");
        engine->query("INSERT INTO net_users VALUES (2, Bob)");

        server = new NetworkServer(TEST_PORT, *engine);
        serverThread = std::thread([this]() { server->start(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void TearDown() override {
        server->stop();
        serverThread.join();
        delete server;
        delete engine;
        delete catalog;
        cleanup();
    }

    void cleanup() {
        std::remove("catalog.dat");
        std::remove("engine.wal");
        std::remove("net_users.db");
    }

    std::string sendQuery(const std::string &query) {
        asio::io_context ctx;
        tcp::socket sock(ctx);
        tcp::resolver resolver(ctx);
        auto endpoints = resolver.resolve("127.0.0.1", std::to_string(TEST_PORT));
        asio::connect(sock, endpoints);

        std::string msg = query + "\n";
        asio::write(sock, asio::buffer(msg));

        std::string response;
        asio::error_code ec;
        asio::read(sock, asio::dynamic_buffer(response), ec);
        return response;
    }
};

// Test 1: SELECT * returneaza toate rows
TEST_F(NetworkServerTest, SelectAllReturnsRows) {
    std::string response = sendQuery("SELECT * FROM net_users");
    EXPECT_NE(response.find("Alice"), std::string::npos);
    EXPECT_NE(response.find("Bob"), std::string::npos);
}

// Test 2: INSERT returneaza OK
TEST_F(NetworkServerTest, InsertReturnsOK) {
    std::string response = sendQuery("INSERT INTO net_users VALUES (3, Charlie)");
    EXPECT_EQ(response, "OK\n");
}

// Test 3: SELECT cu WHERE returneaza doar row-ul corect
TEST_F(NetworkServerTest, SelectWithConditionReturnsFilteredRow) {
    std::string response = sendQuery("SELECT * FROM net_users WHERE id = 1");
    EXPECT_NE(response.find("Alice"), std::string::npos);
    EXPECT_EQ(response.find("Bob"), std::string::npos);
}

// Test 4: query invalid returneaza ERROR
TEST_F(NetworkServerTest, InvalidQueryReturnsError) {
    std::string response = sendQuery("SELECT * FROM nonexistent_table");
    EXPECT_NE(response.find("ERROR"), std::string::npos);
}

// Test 5: DELETE returneaza OK si row-ul dispare
TEST_F(NetworkServerTest, DeleteReturnsOK) {
    std::string response = sendQuery("DELETE FROM net_users WHERE id = 1");
    EXPECT_EQ(response, "OK\n");

    std::string selectResponse = sendQuery("SELECT * FROM net_users");
    EXPECT_EQ(selectResponse.find("Alice"), std::string::npos);
    EXPECT_NE(selectResponse.find("Bob"), std::string::npos);
}
