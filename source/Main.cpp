#include "http_server/HttpServer.hpp"

const std::string www_root = "../source/www_root/";

std::string request_str(const HttpRequest &req) {
    std::stringstream ss;
    ss << req.method << " " << req.path << " " << req.version << "\r\n";
    for(auto& it : req.headers){
        ss << it.first << ": " << it.second << "\r\n";
    }
    ss << "\r\n";
    ss << req.body;
    ss << ss.str();
    ss << ss.str();
    ss << ss.str();
    ss << ss.str();
    ss << ss.str();
    ss << ss.str();
    return ss.str();
}

void get_method(const HttpRequest& req, HttpResponse& rsp){
    rsp.set_content(request_str(req), "text/plain");
}

void post_method(const HttpRequest& req, HttpResponse& rsp){
    rsp.set_content(request_str(req), "text/plain");
}

void put_method(const HttpRequest& req, HttpResponse& rsp){
    //rsp.set_content(request_str(req), "text/plain");
    std::string path_name = www_root + "put.txt";
    loging(DEBUG, "path: %s", path_name.c_str());
    HttpUtil::write_file(path_name, req.body);
}

void delete_method(const HttpRequest& req, HttpResponse& rsp){
    rsp.set_content(request_str(req), "text/plain");
}

void usage(std::string proc)
{
    std::cout << "\n\rUsage: " << proc << " port[1024+]" << " level[0~4]\n" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3)
    {
        usage(argv[0]);
        exit(0);
    }
    log_status = static_cast<int>(*argv[2] - '0');

    HttpServer server(std::stoi(argv[1]));
    server.set_thread_count(12);
    server.set_base_dir(www_root);
    server.method_get("/get", get_method);
    server.method_post("/post", post_method);
    server.method_put("/put", put_method);
    server.method_delete("/delete", delete_method);
    server.listen();
    return 0;
}
