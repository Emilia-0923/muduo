#include <iostream>
#include <vector>
#include <algorithm>
#include "../source/util/Log.hpp"
#include "../source/tcp_server/Socket.hpp"
#include "../source/util/Buffer.hpp"
#include "../source/tcp_server/EventLoop.hpp"
#include "../source/tcp_server/Connection.hpp"

int main() {
    for (int i = 0; i < 3; i++){
        pid_t pid = fork();
        if(pid < 0){
            return -1;
        }
        else if(pid == 0){
            Socket cli_socket;
            cli_socket.create_client(Socket::IPV4_TCP, "127.0.0.1", 40627);
            std::string str = "GET /get HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n";
            for (int i = 0; i < 1; i++) {
                assert(cli_socket.send(str.c_str(), str.size()) != -1);
                char buffer[8192] = { 0 };
                assert(cli_socket.recv(buffer, 8191) != -1);
                loging(DEBUG, "%s", buffer);
                sleep(1);
            }
            exit(0);
        }
    }
    sleep(15);
    return 0;
}

// #include <iostream>
// #include <vector>
// #include <algorithm>
// #include "../source/util/Log.hpp"
// #include "../source/tcp_server/Socket.hpp"
// #include "../source/util/Buffer.hpp"
// #include "../source/tcp_server/EventLoop.hpp"
// #include "../source/tcp_server/Connection.hpp"
// #include "../source/util/HttpUtil.hpp"

// int main() {
//     Socket cli_socket;
//     cli_socket.create_client(Socket::IPV4_TCP, "127.0.0.1", 40627);
//     std::string str = "PUT /put HTTP/1.1\r\nConnection: keep-alive\r\n"; //PUT方法无法运行
//     std::string body;
//     HttpUtil::read_file("./hello.txt", body);
//     str += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
//     str += body;
//     cli_socket.send(str.c_str(), str.size());
//     char buffer[8192] = {0};
//     cli_socket.recv(buffer, 8191);
//     loging(DEBUG, "%s", buffer);
//     sleep(1);
//     return 0;
// }