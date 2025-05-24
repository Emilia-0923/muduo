#include <iostream>
#include <functional>
#include <unistd.h>

using func_t = std::function<void()>;

class A {
public:
	int* pi;
	func_t func;

	A(const int _i)
		: pi(new int(_i)) {
		std::cout << "A构造, i = " << *pi << std::endl;
	}
	~A() {
		std::cout << "A析构" << std::endl;
		delete pi;
	}
};

void _func(A& _a) {
	std::cout << *_a.pi << std::endl;
}

int main() {
	A a(10);
	a.func = std::bind(_func, a);
	while (true) {
		sleep(1);
		a.func();
	}
	return 0;
}