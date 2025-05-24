#pragma once
#include <string>
#include <unordered_map>
#include <regex>

class HttpRequest
{
public:
    std::string method; //请求方法
    std::string path; //资源路径
    std::string version; //协议版本
    std::string body; //请求正文
    std::smatch smatches; //资源路径的正则提取数据
    std::unordered_map<std::string, std::string> headers; //头部字段
    std::unordered_map<std::string, std::string> params; //查询字符串

public:
    HttpRequest()
        : version("HTTP/1.1") {}

    void reset() {
        method.clear();
        path.clear();
        version = "HTTP/1.1";
        body.clear();
        std::smatch temp;
        smatches.swap(temp);
        headers.clear();
        params.clear();
    }

    //头部字段
    void set_header(const std::string& key, const std::string& val) {
        headers.emplace(key, val);
    }

    bool has_header(const std::string& key) {
        auto it = headers.find(key);
        if (it == headers.end()) {
            return false;
        }
        return true;
    }

    std::string get_header(const std::string& key) {
        auto it = headers.find(key);
        if (it == headers.end()) {
            return "";
        }
        return it->second;
    }

    //查询字符串
    void set_param(const std::string& key, const std::string& val) {
        params.emplace(key, val);
    }

    bool has_param(const std::string& key) {
        auto it = params.find(key);
        if (it == params.end()) {
            return false;
        }
        return true;
    }

    std::string get_param(const std::string& key) {
        auto it = params.find(key);
        if (it == params.end()) {
            return "";
        }
        return it->second;
    }

    //正文长度
    size_t content_size() {
        bool ret = has_header("Content-Length");
        if (ret == false) {
            return 0;
        }
        return std::stol(get_header("Content-Length"));
    }

    //是否是短连接
    bool close() {
        if (has_header("Connection") == true && get_header("Connection") == "keep-alive") {
            return false;
        }
        return true;
    }

        // 获取客户端IP (如果有代理需要考虑X-Forwarded-For)
    std::string get_client_ip() {
        if (has_header("X-Forwarded-For")) {
            return get_header("X-Forwarded-For");
        }
        return ""; // 实际使用时应该从连接中获取真实IP
    }
    
    // 检查内容类型
    bool is_json() {
        std::string content_type = get_header("Content-Type");
        return content_type.find("application/json") != std::string::npos;
    }
    
    // 获取cookie
    std::string get_cookie(const std::string& name) {
        if (!has_header("Cookie")) return "";
        
        std::string cookies = get_header("Cookie");
        std::vector<std::string> cookie_list;
        HttpUtil::split(cookies, ";", cookie_list, true);
        
        for (const auto& cookie : cookie_list) {
            size_t eq_pos = cookie.find('=');
            if (eq_pos == std::string::npos) continue;
            
            std::string key = HttpUtil::trim(cookie.substr(0, eq_pos));
            if (key == name) {
                return HttpUtil::trim(cookie.substr(eq_pos + 1));
            }
        }
        
        return "";
    }
};