#ifndef NETWORK_SERVER_HPP
#define NETWORK_SERVER_HPP

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include "Engine.hpp"

using asio::ip::tcp;

class NetworkServer {
    public:
        NetworkServer(size_t port, Engine &engine);

        void start();
    private:
        size_t port;
        asio::io_context io_context;
        tcp::acceptor acceptor;
        Engine &engine;

        void openServer();
        void acceptConnections();
};

#endif
