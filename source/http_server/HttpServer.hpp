#pragma once
#include "../tcp_server/TcpServer.hpp"
#include "HttpContext.hpp"

class HttpServer
{
private:
    using Handler = std::function<void(const HttpRequest&, HttpResponse&)>;
    using Handler_Map = std::vector<std::pair<std::regex, Handler>>;

    static const int default_timeout = 30;
    
    Handler_Map get_route;
    Handler_Map post_route;
    Handler_Map put_route;
    Handler_Map delete_route;
    Handler_Map options_route;

    std::string base_dir;
    std::string base_page = "index.html";
    std::string error_page = "error.html";
    TcpServer server;

private:
    //错误处理回调
    void error_handler(HttpRequest& req, HttpResponse& rsp){
        //读取错误页面
        std::string path = base_dir + error_page;
        bool ret = HttpUtil::read_file(path, rsp.body);
        if(ret == false){
            logging.error("读取错误页面失败!");
            return;
        }
        std::string mime = HttpUtil::extend_mime(path);
        rsp.set_header("Content-Type", mime);
    }

    //设置上下文
    void on_connected(const Connection::ptr& conn) {
        conn->set_context(HttpContext());
    }

    //缓冲区数据解析+处理
    void on_message(const Connection::ptr& conn, Buffer& buf) {
        while(buf.read_able_size() > 0){
            HttpContext *context = conn->get_context().get<HttpContext>();
            //读取缓冲区的内容, 把内容解析写入到req中以构建req
            context->recv_http_request(buf);
            HttpRequest& req = context->get_request();
            HttpResponse rsp(context->response_status());
            //状态码>=400代表解析出错了
            if (rsp.status >= 400) {
                //先构建一个错误页面
                error_handler(req, rsp);
                //设置rsp头部字段
                set_response(req, rsp);
                //序列化rsp,并写入到缓冲区
                write_response(conn, rsp);
                context->reset();
                buf.move_read(buf.read_able_size());
                conn->shutdown();
                return;
            }

            //请求还没有接收完成,还要等完整的请求才能处理
            if(context->get_recv_status() != RECV_HTTP_OVER){
                return;
            }

            //如果没出错,则先根据路由表先进行路由, 根据请求类型来调用不同的函数填充rsp的中的字段
            route(req, rsp);
            //设置rsp头部字段
            set_response(req, rsp);
            //序列化rsp,并写入到缓冲区
            write_response(conn, rsp);
            //这次请求处理完了, 重置协议处理类
            context->reset();
            // 短连接则关闭连接
            if (rsp.close() == true) {
                conn->shutdown();
                return;
            }
        }
    }

    //路由
    void route(HttpRequest& req, HttpResponse& rsp) {
        //分辨req请求的是静态资源请求还是功能性请求

        //判断req请求的是否是静态资源请求
        if(is_file_handler(req) == true){
            //调用静态资源处理函数
            file_handler(req, rsp);
            return;
        }
        
        //不是静态资源请求
        std::unordered_map<std::string, Handler_Map> routes = {
            {"GET", get_route},
            {"POST", post_route},
            {"PUT", put_route},
            {"DELETE", delete_route}
        };

        //在路由中查找handler回调函数
        auto it = routes.find(req.method);
        if (it != routes.end()) {
            dispatcher(req, rsp, it->second);
        }
        else {
            //返回错误页面
            rsp.status = 405;
            error_handler(req, rsp);
        }
    }

    bool is_file_handler(const HttpRequest& req){
        //必须设置了静态资源根目录
        if(base_dir.empty()){
            return false;
        }
        //必须为GET/HEAD
        if(req.method != "GET" && req.method != "HEAD"){
            return false;
        }
        if(HttpUtil::valid_path(req.path) == false){
            return false;
        }

        // 如果路径为/,则访问base_page
        std::string path = base_dir + req.path;
        if(req.path == "/"){
            path += base_page;
        }
        
        if(HttpUtil::is_regular(path) == false){
            return false;
        }
        return true;
    }

    //静态资源请求处理
    bool file_handler(HttpRequest& req, HttpResponse& rsp) {
        std::string path = base_dir + req.path;
        
        // 防止路径遍历攻击
        if (req.path.find("../") != std::string::npos || 
            req.path.find("..\\") != std::string::npos) {
            rsp.status = 403; // Forbidden
            return false;
        }
        
        // 处理默认页面
        if (req.path == "/") {
            path += base_page;
        }
        
        // 检查文件是否存在且是常规文件
        if (!HttpUtil::is_regular(path)) {
            rsp.status = 404;
            return false;
        }
        
        // 检查文件是否在base_dir目录下
        std::string canonical_path = HttpUtil::canonical_path(path);
        std::string canonical_base = HttpUtil::canonical_path(base_dir);
        
        if (canonical_path.find(canonical_base) != 0) {
            rsp.status = 403; // Forbidden
            return false;
        }
        
        // 读取文件
        if (!HttpUtil::read_file(path, rsp.body)) {
            rsp.status = 500;
            return false;
        }
        
        // 设置MIME类型
        std::string mime = HttpUtil::extend_mime(path);
        rsp.set_header("Content-Type", mime);
        
        // 设置Last-Modified头
        auto last_modified = HttpUtil::last_modified_time(path);
        if (!last_modified.empty()) {
            rsp.set_header("Last-Modified", last_modified);
        }
        
        // 处理HEAD请求 - 不发送正文
        if (req.method == "HEAD") {
            rsp.body.clear();
        }
        
        return true;
    }
 
    //功能性请求分类
    void dispatcher(HttpRequest& req, HttpResponse& rsp, Handler_Map& handler_map) { 
        for(auto& handler : handler_map){
            std::regex& e = handler.first;
            Handler& functor = handler.second;
            //正则表达式匹配req的path
            bool ret = std::regex_match(req.path, req.smatches, e);
            if (ret == true) {
                //匹配成功, 执行处理函数
                functor(req, rsp);
                return;
            }
        }
        rsp.status = 404;
        error_handler(req, rsp);
        logging.error("请求的资源: %s 不存在!", req.path.c_str());
    }

    void set_response(HttpRequest& req, HttpResponse& rsp) {
        rsp.version = req.version;
        
        // 连接管理
        rsp.set_header("Connection", req.close() ? "close" : "keep-alive");
        
        // 日期头 (RFC 7231要求)
        rsp.set_header("Date", HttpUtil::http_date());
        
        // 服务器标识
        rsp.set_header("Server", "MyHttpServer/1.0");
        
        // 内容相关头
        if (!rsp.body.empty()) {
            if (!rsp.has_header("Content-Length")) {
                rsp.set_header("Content-Length", std::to_string(rsp.body.size()));
            }
            
            if (!rsp.has_header("Content-Type")) {
                rsp.set_header("Content-Type", "application/octet-stream");
            }
        }
        
        // 重定向
        if (rsp.redirect_flag) {
            rsp.set_header("Location", rsp.redirect_url);
        }
    }

    //将http_response中的数据按照http协议的格式进行组织和发送
    void write_response(const Connection::ptr& conn, HttpResponse& rsp) {
        //首行
        std::stringstream rsp_stream;
        rsp_stream << rsp.version << " " << std::to_string(rsp.status) << " " << HttpUtil::status_desc(rsp.status) << "\r\n";
        //头部
        for(auto& head : rsp.headers){
            rsp_stream << head.first << ": " << head.second << "\r\n";
        }
        //空行
        rsp_stream << "\r\n";
        //正文
        rsp_stream << rsp.body;

        conn->send(rsp_stream.str().c_str(), rsp_stream.str().size());
    }

public:
    HttpServer(int port, int timeout = default_timeout)
        : server(port) {
        server.set_conn_cb(std::bind(&HttpServer::on_connected, this, std::placeholders::_1));
        server.set_msg_cb(std::bind(&HttpServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
        server.enable_inactive_release(timeout);
    }

    void set_base_dir(const std::string& path) {
        bool ret = HttpUtil::is_directory(path);
        if(ret == false){
            logging.fatal("文件根目录: %s 不存在!", path.c_str());
            abort();
        }
        base_dir = path;
    }

    //设置请求(请求的正则表达式)和处理函数的映射管理
    void method_get(const std::string& pattern, const Handler& handler) {
        get_route.emplace_back(std::regex(pattern), handler);
    }

    void method_post(const std::string& pattern, const Handler& handler) {
        post_route.emplace_back(std::regex(pattern), handler);
    }
    
    void method_put(const std::string& pattern, const Handler& handler) {
        put_route.emplace_back(std::regex(pattern), handler);
    }
    
    void method_delete(const std::string& pattern, const Handler& handler) {
        delete_route.emplace_back(std::regex(pattern), handler);
    }

    void method_options(const std::string& pattern, const Handler& handler) {
        options_route.emplace_back(std::regex(pattern), handler);
    }

    void set_thread_count(int count) {
        server.set_thread_count(count);
    }

    void set_base_page(const std::string& page) {
        base_page = page;
    }

    void set_error_page(const std::string& page) {
        error_page = page;
    }

    void set_inactive_release(int timeout) {
        server.enable_inactive_release(timeout);
    }

    void listen(){
        server.start();
    }
};