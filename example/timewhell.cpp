#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>


using TaskFunc = std::function<void()>;
using ReleaseFunc = std::function<void()>;

class TimerTask {
private:
    uint64_t id; //定时器任务对象id
    uint32_t timeout; //定时任务的超时时间
    TaskFunc task_cb; //任务回调函数
    ReleaseFunc release_cb; //删除TimerWheel种保存的定时器对象信息
    bool status;
public:
    TimerTask(uint64_t _id, uint32_t _timeout, const TaskFunc& _task_cb)
        : id(_id), timeout(_timeout), task_cb(_task_cb), status(true) {}

    ~TimerTask() {
        if (status == true) {
            task_cb();
        }
        release_cb();
    }

    void cancel() {
        status = false;
    }

    uint32_t get_timeout() {
        return timeout;
    }

    void set_release(const ReleaseFunc& _release_cb) {
        release_cb = _release_cb;
    }
};

class TimerWheel {
private:
    using WeakTask = std::weak_ptr<TimerTask>;
    using SharedTask = std::shared_ptr<TimerTask>;

    int capacity; //轮盘容量
    int tick; //秒针，走到哪释放哪
    std::vector<std::vector<SharedTask>> wheel;
    std::unordered_map<uint64_t, WeakTask> timers;
public:
    TimerWheel()
        : capacity(60), tick(0), wheel(capacity) {}

    void remove_timer(uint64_t _id) {
        auto it = timers.find(_id);
        if (it != timers.end()) {
            timers.erase(it);
        }
    }

    void timer_add(uint64_t _id, uint32_t _timeout, const TaskFunc& _task_cb) {
        SharedTask task_ptr(new TimerTask(_id, _timeout, _task_cb));
        task_ptr->set_release(std::bind(&TimerWheel::remove_timer, this, _id));
        int delay = task_ptr->get_timeout();
        wheel[(tick + delay) % capacity].push_back(task_ptr);
        timers[_id] = WeakTask(task_ptr);
    }

    void timer_cancel(uint64_t _id) {
        auto it = timers.find(_id);
        if (it == timers.end()) {
            return;
        }
        SharedTask task_ptr = it->second.lock();
        task_ptr->cancel();
    }

    void timer_refresh(uint64_t _id) {
        //通过weakptr构造shared_ptr,添加到轮子中
        auto it = timers.find(_id);
        if (it == timers.end()) {
            return;
        }
        SharedTask task_ptr = it->second.lock();
        int delay = task_ptr->get_timeout();
        wheel[(tick + delay) % capacity].push_back(task_ptr);
    }

    void run_timer_task() {
        //每秒执行一次
        tick = (tick + 1) % capacity;
        wheel[tick].clear(); //清空指定位置的数组，通过析构函数自动调用任务
    }
};

class Test {
public:
    Test() {
        std::cout << "构造" << std::endl;
    }

    ~Test() {
        std::cout << "析构" << std::endl;
    }
};

void del_test(Test* t) {
    delete t;
}

int main() {
    TimerWheel tw;
    Test* t = new Test();
    tw.timer_add(888, 5, std::bind(del_test, t));

    for (int i = 0; i < 5; i++) {
        tw.timer_refresh(888);
        tw.run_timer_task();
        sleep(1);
        std::cout << "五秒后销毁" << std::endl;
    }

    tw.timer_cancel(888);
    
    while (true) {
        sleep(1);
        std::cout << "过了一秒" << std::endl;
        tw.run_timer_task();
    }
    return 0;
}