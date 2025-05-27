#pragma once

#include "Connector.hpp"
#include "../package/Connection.hpp"

namespace muduo {

class TcpClient {
private:
    using conn_func = Connection::conn_func;
    using msg_func = Connection::msg_func;
    using close_func = Connection::close_func;
    
    EventLoop* loop_;
    std::shared_ptr<Connector> connector_;
    std::string server_ip_;
    uint16_t server_port_;
    bool retry_;
    bool connected_;
    std::mutex mutex_;
    Connection::ptr connection_;
    conn_func conn_cb_;
    msg_func msg_cb_;
    close_func close_cb_;

public:

    TcpClient(EventLoop* loop, const std::string& ip, uint16_t port)
        : loop_(loop)
        ,  connector_(new Connector(loop, ip, port))
        ,  server_ip_(ip)
        ,  server_port_(port)
        ,  retry_(false)
        ,  connected_(true) {
        connector_->set_new_conn_cb(std::bind(&TcpClient::new_connection, this, std::placeholders::_1));
    }

    ~TcpClient() {
        disconnect();
    }

    void connect() {
        connected_ = true;
        connector_->start();
    }

    void disconnect() {
        connected_ = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (connection_) {
                connection_->shutdown();
            }
        }
        connector_->stop();
    }

    void enable_retry() {
        retry_ = true;
    }

    void set_conn_cb(conn_func cb) {
        conn_cb_ = cb;
    }

    void set_msg_cb(msg_func cb) {
        msg_cb_ = cb;
    }

    void set_close_callback(close_func cb) {
        close_cb_ = cb;
    }

    void stop() {
        connected_ = false;
        connector_->stop();
    }

private:
    void new_connection(int _sockfd) {
        loop_->assert_in_loop();

        Connection::Info info;
        info.fd = _sockfd;
        info.ip = server_ip_;
        info.port = server_port_;

        Connection::ptr new_conn = std::make_shared<Connection>(loop_, 0, info);
        new_conn->set_conn_cb(conn_cb_);
        new_conn->set_msg_cb(msg_cb_);
        new_conn->set_close_cb(close_cb_);
        new_conn->set_svr_close_cb(std::bind(&TcpClient::remove_connection, this, std::placeholders::_1));
        {
            std::lock_guard<std::mutex> lock(mutex_);
            connection_ = new_conn;
        }

        new_conn->established();
    }

    void remove_connection(const Connection::ptr& conn) {
        loop_->assert_in_loop();
        if(loop_ == conn->get_loop()) {
            logging.fatal("remove_connection");
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (connection_ == conn) {
                logging.fatal("remove_connection");
                abort();
            }
            connection_.reset();
        }
        
        loop_->push_task(std::bind(&Connection::release, conn));

        if (retry_ && connected_) {
            logging.info("尝试重新连接 %s:%d", server_ip_.c_str(), server_port_);
            connector_->restart();
        }
    }
};

}