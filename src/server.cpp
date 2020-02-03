#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>

void handle(boost::asio::ip::tcp::socket & sock);

int main(int argc, const char * argv[]) {
    if (argc != 3) {
        std::cerr << "usage: server <address> <port>\n";
        return EXIT_FAILURE;
    }

    const boost::asio::ip::address listen_addr = boost::asio::ip::make_address(argv[1]);
    const unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));

    boost::asio::io_context ioc;
    boost::asio::ip::tcp::endpoint endpoint(listen_addr, port);
    boost::asio::ip::tcp::acceptor acceptor(ioc, endpoint);

    std::cout << "server started.\n";
    while (true) {
        boost::asio::ip::tcp::socket sock(ioc);
        acceptor.accept(sock);

        std::thread(std::bind(&handle, std::move(sock))).detach();
    }

}


void handle(boost::asio::ip::tcp::socket & sock) {
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws(std::move(sock));
    static boost::beast::flat_buffer content, instruction;

    ws.accept();
    std::cout << "websocket connected" << std::endl;

    try {
        while (true) {
            ws.read(instruction);

            std::string instruction_str = static_cast<char*>(instruction.data().data());
            instruction.consume(instruction.size());
            std::cout << "instruction: " << instruction_str << '\n';

            if (instruction_str == "SAVE") {
                content.consume(content.size());
                ws.read(content);
            } else if (instruction_str == "LOAD") {
                ws.write(content.data());
            }
        }
    } catch (boost::system::system_error & er) {
        std::cout << "websocket disconnected" << std::endl;
    }
}