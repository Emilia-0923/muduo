#include <iostream>
#include <cstdint>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if(efd < 0) {
        perror("efd 错误");
    }
    uint64_t val = 1;
    write(efd, &val, sizeof(val));
    write(efd, &val, sizeof(val));
    write(efd, &val, sizeof(val));
    uint64_t res = 0;
    read(efd, &res, sizeof(res));
    std::cout << res << std::endl;
    return 0;
}