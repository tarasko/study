#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <thread>
#include <vector>
#include <algorithm>
#include <iostream>
#include <memory>

using namespace std;
using namespace boost::asio;

static const unsigned HARDWARE_CONCURRENCY = 2;
static const unsigned short LISTEN_PORT = 10001;
static const char STOP[] = "stop";

static io_service     g_iosvc;
static vector<thread> g_thrpool;

template<typename Array>
static void initiate_read(
    const shared_ptr<ip::tcp::socket>& sock, 
    const shared_ptr<Array>& read_buf
    )
{
    sock->async_read_some(
        buffer(*read_buf), 
        [read_buf, sock](const boost::system::error_code& ec, size_t bytes_transferred) 
        {
            if (ec)
                cout << sock->remote_endpoint() << ": async read failed: " << ec << endl;
            else
            {
                if (bytes_transferred > 4 && !memcmp(STOP, read_buf->begin(), sizeof(STOP) - 1))
                {
                    cout << sock->remote_endpoint() << ": stop requested\n";
                    g_iosvc.stop();
                    return;
                }
                
                boost::system::error_code wr_ec;
                sock->write_some(buffer(*read_buf, bytes_transferred), wr_ec);
                
                if (wr_ec) 
                    cout << sock->remote_endpoint() << ": write failed: " << wr_ec << endl;
                else
                    initiate_read(sock, read_buf);
            }
        });
}

static void initiate_accept(ip::tcp::acceptor& acceptor)
{
    auto sock = make_shared<ip::tcp::socket>(ref(g_iosvc));
    acceptor.async_accept(
        *sock, 
        [sock, &acceptor](const boost::system::error_code& ec) 
        {
            if (ec)
            {
                g_iosvc.stop();
                cout << acceptor.local_endpoint() << ": accept failed: " << ec << endl;
            }
            else 
            {        
                initiate_accept(acceptor);
                cout << acceptor.local_endpoint() << ": socket accepted from " << sock->remote_endpoint() << endl;
                initiate_read(sock, make_shared<array<char, 1024>>());
            }
        });
}

int main(int argc, char* argv[])
{   
    io_service::work work(g_iosvc);
    ip::tcp::endpoint ep(ip::tcp::v4(), LISTEN_PORT);
    ip::tcp::acceptor acceptor(g_iosvc, ep);

    initiate_accept(acceptor);
    
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) 
        g_thrpool.emplace_back([]() 
            { 
                for (;;) try 
                { 
                    g_iosvc.run(); 
                    break;
                }                               
                catch (exception& e) 
                {
                    cout << "Unhandled exception in pool thread: " << e.what() << endl;
                }
            } );
    
    for_each(g_thrpool.begin(), g_thrpool.end(), [](thread& t) { t.join(); });
    
    return 0;
}
