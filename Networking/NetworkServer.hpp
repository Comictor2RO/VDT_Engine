#ifndef NETWORK_SERVER_HPP
#define NETWORK_SERVER_HPP

#include <iostream>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif
#ifndef NOUSER
#define NOUSER
#endif
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <string>
#include <mutex>
#include <functional>

class Engine;

using asio::ip::tcp;

class NetworkServer {
    public:
        NetworkServer(Engine &engine);

        void prepare();
        void run();
        void stop();
        size_t getPort() const;
        void setLogCallback(std::function<void(const std::string&)> callback);

    private:
        size_t port = 0;
        asio::io_context io_context;
        tcp::acceptor acceptor;
        Engine &engine;
        std::mutex engineMutex;
        std::function<void(const std::string&)> logCallback;

        void openServer();
        void acceptConnections();
        void handleClient(tcp::socket socket);
        std::string executeQuery(std::string &query);
};

#endif
