#include "../Engine/Engine.hpp"
#include "NetworkServer.hpp"
#include <iostream>

NetworkServer::NetworkServer(Engine &engine)
    : engine(engine), acceptor(io_context)
{};

void NetworkServer::prepare()
{
    io_context.restart();
    openServer();
    acceptConnections();
}

void NetworkServer::run()
{
    io_context.run();
}

size_t NetworkServer::getPort() const
{
    return port;
}

void NetworkServer::setLogCallback(std::function<void(const std::string&)> callback)
{
    logCallback = callback;
}

void NetworkServer::stop()
{
    acceptor.close();
    io_context.stop();
    std::cout << "Server stopped." << std::endl;
}

// Opens the server, auto-detecting a free port in range [3000, 8000]
void NetworkServer::openServer()
{
    for (size_t p = 3000; p <= 8000; p++) {
        try {
            acceptor.open(tcp::v4());
            acceptor.bind(tcp::endpoint(tcp::v4(), p));
            acceptor.listen();
            port = p;
            return;
        }
        catch (const asio::system_error &) {
            if (acceptor.is_open())
                acceptor.close();
        }
    }
    throw std::runtime_error("No free port found in range 3000-8000");
}

// Accepts incoming connections and handles them async
void NetworkServer::acceptConnections()
{
    acceptor.async_accept(io_context, [this](asio::error_code ec, tcp::socket socket) {
        if (!ec) {
            handleClient(std::move(socket));
            if (logCallback) logCallback("[SERVER] Client connected.");
        }
        else if (ec == asio::error::operation_aborted) {
            return;
        }
        else {
            if (logCallback) logCallback("[SERVER ERROR] " + ec.message());
        }

        acceptConnections();
    });
}

// Handles communication with a connected client
void NetworkServer::handleClient(tcp::socket socket)
{
    auto sharedSocket = std::make_shared<tcp::socket>(std::move(socket));
    auto buffer = std::make_shared<asio::streambuf>();

    asio::async_read_until(*sharedSocket, *buffer, "\n", [this, sharedSocket, buffer](asio::error_code ec, std::size_t bytes_transferred) {
        if (!ec && bytes_transferred > 0) {
            std::istream is(buffer.get());
            std::string message;
            std::getline(is, message);
            if (!message.empty() && message.back() == '\r')
                message.pop_back();

            auto response = std::make_shared<std::string>(executeQuery(message));
            asio::async_write(*sharedSocket, asio::buffer(*response), [this, sharedSocket, response](asio::error_code ec, std::size_t) {
                if (ec && logCallback) logCallback("[SERVER ERROR] Send failed: " + ec.message());
            });
        }
        else {
            if (logCallback) logCallback("[SERVER ERROR] Read failed: " + ec.message());
        }
    });
}

std::string NetworkServer::executeQuery(std::string &query)
{
    std::lock_guard<std::mutex> lock(engineMutex);
    try {
        std::vector<Row> rows = engine.query(query);

        if (rows.empty())
            return "OK\n";

        std::string result;
        for (const Row &row : rows) {
            for (size_t i = 0; i < row.values.size(); i++) {
                if (i > 0) result += "|";
                result += row.values[i];
            }
            result += "\n";
        }
        return result;
    }
    catch (const std::exception &e) {
        return std::string("ERROR: ") + e.what() + "\n";
    }
}


