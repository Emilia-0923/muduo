#pragma once
#include "../util/Buffer.hpp"
#include "../util/HttpUtil.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

enum Http_Status {
    RECV_HTTP_ERROR,
    RECV_HTTP_LINE,
    RECV_HTTP_HEAD,
    RECV_HTTP_BODY,
    RECV_HTTP_OVER
};

class HttpContext {
private:
    static const size_t max_line = 8192;
    static const size_t max_headers = 100; // 最大请求头数量
    
    int rsp_status;
    Http_Status recv_status;
    HttpRequest request;

private:
    bool recv_http_line(Buffer& buf) {
        std::string line = buf.get_line();
        if (line.empty()) {
            if (buf.read_able_size() > max_line) {
                recv_status = RECV_HTTP_ERROR;
                rsp_status = 414; // URI Too Long
                return false;
            }
            return true;
        }
        
        if (line.size() > max_line) {
            recv_status = RECV_HTTP_ERROR;
            rsp_status = 414;
            return false;
        }
        
        bool ret = parse_http_line(line);
        if (!ret) {
            return false;
        }
        
        recv_status = RECV_HTTP_HEAD;
        return true;
    }

    bool parse_http_line(const std::string& line) {
        // 更高效的正则表达式，只匹配必要部分
        static const std::regex e("^(GET|HEAD|POST|PUT|DELETE|OPTIONS|PATCH|TRACE|CONNECT) "
                                "((?:\\/[^\\s?#]*)(?:\\?[^\\s#]*)?(?:#[^\\s]*)?) "
                                "(HTTP\\/1\\.[01])\r?\n?$", 
                                std::regex::icase);
        
        std::smatch matches;
        if (!std::regex_match(line, matches, e)) {
            recv_status = RECV_HTTP_ERROR;
            rsp_status = 400;
            return false;
        }
        
        // 方法验证
        request.method = matches[1];
        std::transform(request.method.begin(), request.method.end(), 
                    request.method.begin(), ::toupper);
        
        // 路径安全性检查
        std::string full_path = matches[2];
        if (full_path.empty() || full_path[0] != '/') {
            recv_status = RECV_HTTP_ERROR;
            rsp_status = 400;
            return false;
        }
        
        // 分离路径和查询字符串
        size_t query_pos = full_path.find('?');
        try {
            request.path = HttpUtil::url_decode(full_path.substr(0, query_pos), false);
        } catch (...) {
            recv_status = RECV_HTTP_ERROR;
            rsp_status = 400;
            return false;
        }
        
        // 防止路径遍历攻击
        if (request.path.find("../") != std::string::npos || 
            request.path.find("..\\") != std::string::npos ||
            request.path.find("//") != std::string::npos) {
            recv_status = RECV_HTTP_ERROR;
            rsp_status = 403;
            return false;
        }
        
        // 规范化路径
        request.path = HttpUtil::normalize_path(request.path);
        request.version = matches[3];
        
        // 解析查询字符串
        if (query_pos != std::string::npos) {
            std::string query_string = full_path.substr(query_pos + 1);
            std::vector<std::string> query_pairs;
            HttpUtil::split(query_string, "&", query_pairs, true);
            
            for (auto& pair : query_pairs) {
                size_t eq_pos = pair.find('=');
                if (eq_pos == std::string::npos) {
                    continue; // 允许没有值的参数
                }
                
                std::string key = HttpUtil::url_decode(pair.substr(0, eq_pos), true);
                std::string val = HttpUtil::url_decode(pair.substr(eq_pos + 1), true);
                request.set_param(key, val);
            }
        }
        
        return true;
    }

    bool recv_http_head(Buffer& buf) {
        if (recv_status != RECV_HTTP_HEAD) {
            return false;
        }
        
        while (true) {
            std::string line = buf.get_line();
            if (line.empty()) {
                if (buf.read_able_size() > max_line) {
                    recv_status = RECV_HTTP_ERROR;
                    rsp_status = 414;
                    return false;
                }
                return true;
            }
            
            if (line.size() > max_line) {
                recv_status = RECV_HTTP_ERROR;
                rsp_status = 414;
                return false;
            }
            
            if (line == "\n" || line == "\r\n") {
                break;
            }
            
            if (!parse_http_head(line)) {
                return false;
            }
        }
        
        // 检查Host头是否存在 (HTTP/1.1要求)
        if (request.version == "HTTP/1.1" && !request.has_header("Host")) {
            recv_status = RECV_HTTP_ERROR;
            rsp_status = 400;
            return false;
        }
        
        recv_status = RECV_HTTP_BODY;
        return true;
    }

    bool parse_http_head(const std::string& line) {
        if (request.headers.size() >= max_headers) {
            recv_status = RECV_HTTP_ERROR;
            rsp_status = 431;
            return false;
        }

        std::string trimmed = line;
        if (!trimmed.empty() && trimmed.back() == '\n') {
            trimmed.pop_back();
        }
        if (!trimmed.empty() && trimmed.back() == '\r') {
            trimmed.pop_back();
        }
        
        size_t pos = trimmed.find(": ");
        if (pos == std::string::npos) {
            recv_status = RECV_HTTP_ERROR;
            rsp_status = 400; // Bad Request
            return false;
        }
        
        std::string key = trimmed.substr(0, pos);
        std::string val = trimmed.substr(pos + 2);
        request.set_header(key, val);
        return true;
    }

    bool recv_http_body(Buffer& buf) {
        if (recv_status != RECV_HTTP_BODY) {
            return false;
        }
        
        size_t content_length = request.content_size();
        if (content_length == 0) {
            recv_status = RECV_HTTP_OVER;
            return true;
        }
        
        // 检查Content-Length是否合理
        if (content_length > 10 * 1024 * 1024) { // 限制10MB
            recv_status = RECV_HTTP_ERROR;
            rsp_status = 413; // Payload Too Large
            return false;
        }
        
        size_t remaining = content_length - request.body.size();
        size_t available = buf.read_able_size();
        
        if (available >= remaining) {
            request.body.append(buf.read_string(remaining));
            recv_status = RECV_HTTP_OVER;
        } else {
            request.body.append(buf.read_string(available));
        }
        
        return true;
    }

public:
    HttpContext() : rsp_status(200), recv_status(RECV_HTTP_LINE) {}

    void reset() {
        rsp_status = 200;
        recv_status = RECV_HTTP_LINE;
        request.reset();
    }

    int response_status() const { return rsp_status; }
    Http_Status get_recv_status() const { return recv_status; }
    HttpRequest& get_request() { return request; }
    const HttpRequest& get_request() const { return request; }

    void recv_http_request(Buffer& buf) {
        switch (recv_status) {
        case RECV_HTTP_LINE:
            if (!recv_http_line(buf)) break;
            [[fallthrough]];
        case RECV_HTTP_HEAD:
            if (!recv_http_head(buf)) break;
            [[fallthrough]];
        case RECV_HTTP_BODY:
            recv_http_body(buf);
            break;
        default:
            break;
        }
    }
};