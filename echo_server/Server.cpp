#include "EchoServer.hpp"


void usage(std::string proc)
{
    std::cout << "\n\rUsage: " << proc << " port[1024+]" << " level[0~4]\n" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        usage(argv[0]);
        exit(0);
    }
    log_status = static_cast<int>(*argv[2] - '0');
    EchoServer server(std::stoi(argv[1]));
    server.start();
    return 0;
}