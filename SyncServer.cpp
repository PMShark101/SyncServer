
#include <boost/asio.hpp>
#include <iostream>
#include <set>
#include <memory>

using boost::asio::ip::tcp;
const int max_length = 1024;
typedef std::shared_ptr<tcp::socket> socket_ptr; //智能指针
std::set<std::shared_ptr<std::thread>> thread_set;
using namespace std;

void session(socket_ptr sock) {
    try {
        for (;;) {
            char data[max_length];
            memset(data, '\0', max_length);
            boost::system::error_code error;
            //size_t length = boost::asio::read(sock, boost::asio::buffer(data, max_length), error);
            size_t length = sock->read_some(boost::asio::buffer(data, max_length), error);
            if (error == boost::asio::error::eof) {
                std::cout << "Connection closed by peer" << std::endl;
                break;
            }

            else if (error) {
                throw boost::system::system_error(error);
            }

            std::cout << "Receive from " << sock->remote_endpoint().address().to_string() << std::endl;
            std::cout << "Receive message is " << data << std::endl;
            //将收到的数据回传给对方
            boost::asio::write(*sock, boost::asio::buffer(data, length));
        }
    }
    catch (exception& e) {
        std::cerr << "Exception: " << e.what() << endl;
    }
}

void server(boost::asio::io_context& io_context, unsigned short port) {
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
    for (;;) {
        socket_ptr socket(new tcp::socket(io_context));
        a.accept(*socket); //处理新来的连接
        //session(socket); //容易阻塞
        auto t = std::make_shared<std::thread>(session, socket); //用一个子线程去处理
        thread_set.insert(t);
    }
}

int main()
{
    try {
        boost::asio::io_context ioc;
        server(ioc, 10086);
        for (auto& t : thread_set) {
            t->join(); //主线程会等t这个子线程执行完了之后再退出
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}
