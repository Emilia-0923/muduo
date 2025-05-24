#pragma once
#include <string>
#include <unordered_map>
#include <regex>

class HttpResponse
{
public:
    int status;
    int redirect_flag;
    std::string version;
    std::string body;
    std::string redirect_url;
    std::unordered_map<std::string, std::string> headers;
public:
    HttpResponse()
        : redirect_flag(false), status(200) {}

    HttpResponse(int status)
        : redirect_flag(false), status(status) {}

    void reset() {
        status = 200;
        redirect_flag = false;
        body.clear();
        redirect_url.clear();
        headers.clear();
    }

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

    void set_content(const std::string& body, const std::string& type) {
        this->body = body;
        set_header("Content-Type", type);
    }

    void set_redirect(const std::string& url, int status = 302) {
        this->status = status;
        redirect_flag = true;
        redirect_url = url;
    }

    void set_cookie(const std::string& name, const std::string& value, const std::string& path = "/", int max_age = 86400, bool http_only = true) {
        std::string cookie = name + "=" + value + "; Path=" + path + 
                            "; Max-Age=" + std::to_string(max_age);
        if (http_only) {
            cookie += "; HttpOnly";
        }
        set_header("Set-Cookie", cookie);
    }

    bool close() {
        if (has_header("Connection") == true && get_header("Connection") == "keep-alive") {
            return false;
        }
        return true;
    }
};