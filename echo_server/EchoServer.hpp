#include "../source/module/TcpServer.hpp"

class EchoServer {
private:
    TcpServer server;

private:
    void on_connected(Connection::ptr conn) {
        loging(DEBUG, "====================");
        loging(DEBUG, "新连接: %p", conn.get());
        loging(DEBUG, "====================");
    }

    void on_message(Connection::ptr conn, Buffer& buf) {
        // char buffer[65536];
        // int size = buf.read_able_size();
        // buf.read(buffer, size);
        // buffer[size] = 0;
        // loging(DEBUG, "====================");
        // loging(INFO, "接收到信息: %s", buffer);
        // loging(DEBUG, "====================");
        // conn->send(buffer, size + 1);
        loging(DEBUG, "%d, %d", buf.get_read_idx(), buf.read_able_size());
        conn->send(buf.get_read_idx(), buf.read_able_size());
        buf.move_read(buf.read_able_size());
        conn->shutdown();
    }

    void on_close(Connection::ptr conn) {
        loging(DEBUG, "====================");
        loging(DEBUG, "连接: %p, 将要关闭了", conn.get());
        loging(DEBUG, "====================");
    }

public:
    EchoServer(int port)
        : server(port) {
        server.set_thread_count(0);
        server.enable_inactive_release(10);
        server.set_close_cb(std::bind(&EchoServer::on_close, this, std::placeholders::_1));
        server.set_conn_cb(std::bind(&EchoServer::on_connected, this, std::placeholders::_1));
        server.set_msg_cb(std::bind(&EchoServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
    }

    void start() {
        server.start();
    }
};