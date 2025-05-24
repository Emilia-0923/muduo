#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/timerfd.h>

int main() {
    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd < 0) {
        perror("timerfd_create error");
    }
    struct itimerspec itime;
    itime.it_value.tv_sec = 1;
    itime.it_value.tv_nsec = 0;
    itime.it_interval.tv_sec = 1;
    itime.it_interval.tv_nsec = 0;
    timerfd_settime(timerfd, 0, &itime, nullptr);
    while (true) {
        __uint64_t times;
        int ret = read(timerfd, &times, 8);
        if (ret < 0) {
            perror("read 错误");
        }
        printf("超时了，距离上次超时了%ld次\n", times);
    }
    close(timerfd);
    return 0;
}