#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>

#include "listener.hpp"

int main(int argc, const char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "usage: server <address> <websocket port> <http port>\n";
        return EXIT_FAILURE;
    }

    const boost::asio::ip::address listen_addr = boost::asio::ip::make_address(argv[1]);
    const unsigned short ws_port = static_cast<unsigned short>(std::atoi(argv[2]));
    const unsigned short http_port = static_cast<unsigned short>(std::atoi(argv[3]));

    boost::asio::io_context ioc;
    boost::asio::ip::tcp::endpoint ws_endpoint(listen_addr, ws_port);
    boost::asio::ip::tcp::endpoint http_endpoint(listen_addr, http_port);

    ws_listener ws_srv(ioc, ws_endpoint);
    http_listener http_srv(ioc, http_endpoint);

    ws_srv.start();
    http_srv.start();

    std::cout << "server started.\n";
    ioc.run();
}
