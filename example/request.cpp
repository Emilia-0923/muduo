#include <iostream>
#include <string>
#include <regex>

int main() {
    //GET /Emilia/login?user=Emilia&password=12345678 HTTP/1.1\r\n
    std::string str = "GET /Emilia/login?user=Emilia&password=12345678 HTTP/1.1";
    std::regex e("(GET|HEAD|POST|PUT|DELETE) ([^?]*)(?:\\?(.*)) (HTTP/1\\.[01])(?:\n|\r\n)?");
    std::smatch matches;

    bool ret = std::regex_match(str, matches, e);
    if (ret == false) {
        return -1;
    }
    for (auto& s : matches) {
        std::cout << s << std::endl;
    }
    return 0;
}