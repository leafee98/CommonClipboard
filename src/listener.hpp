#include <boost/beast.hpp>
#include <iostream>
#include "html_def.hpp"

class listener
{
private:
    std::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
    void handle_accept(boost::system::error_code ec, boost::asio::ip::tcp::socket sock);
    virtual void handle(boost::system::error_code ec, boost::asio::ip::tcp::socket &sock) = 0;

public:
    listener(boost::asio::io_context &ioc, const boost::asio::ip::tcp::endpoint &endpoint);

    void start();
};

listener::listener(boost::asio::io_context &ioc, const boost::asio::ip::tcp::endpoint &endpoint)
    : _acceptor(new boost::asio::ip::tcp::acceptor(ioc, endpoint))
{
}

void listener::start()
{
    this->_acceptor->async_accept(std::bind(&listener::handle_accept, this,
                                            std::placeholders::_1,
                                            std::placeholders::_2));
}

void listener::handle_accept(boost::system::error_code ec, boost::asio::ip::tcp::socket sock)
{
    this->_acceptor->async_accept(std::bind(&listener::handle_accept, this,
                                            std::placeholders::_1,
                                            std::placeholders::_2));
    this->handle(ec, sock);
}

class ws_listener
    : public listener
{
private:
    boost::beast::flat_buffer content;

    void handle(boost::system::error_code ec, boost::asio::ip::tcp::socket &sock) override
    {
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws(std::move(sock));
        boost::beast::flat_buffer instruction;

        ws.accept();

        std::cout << "ws_srv: new connection" << std::endl;

        try
        {
            while (true)
            {
                ws.read(instruction);
                std::string instruction_str =
                    std::string(static_cast<char *>(instruction.data().data()), instruction.size());
                instruction.consume(instruction.size());

                if (instruction_str == "SAVE")
                {
                    std::cout << "ws_srv: recive SAVE instruction" << std::endl;
                    content.consume(content.size());
                    ws.read(content);
                }
                else if (instruction_str == "LOAD")
                {
                    std::cout << "ws_srv: recive LOAD instruction" << std::endl;
                    ws.write(content.data());
                }
                else
                {
                    std::cout << "ws_srv: unknown instruction: " << instruction_str << std::endl;
                }
            }
        }
        catch (boost::system::system_error &error)
        {
            std::cerr << "ws_srv: websocket disconnected, reason follow:" << std::endl;
            std::cerr << error.what() << std::endl;
        }
    }

public:
    using listener::listener;
};

class http_listener
    : public listener
{
private:
    void handle(boost::system::error_code ec, boost::asio::ip::tcp::socket &sock) override
    {
        try
        {
            boost::asio::mutable_buffer buffer((void *)(response_str.data()), response_str.size());
            boost::asio::write(sock, buffer);
            std::cout << "http_srv: write http response to client" << std::endl;

            sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            sock.close();
        }
        catch (const boost::system::system_error &e)
        {
            std::cerr << "http_srv: socket disconnect, reason follow:" << std::endl;
            std::cerr << e.what() << '\n';
        }
    }

public:
    using listener::listener;
};
