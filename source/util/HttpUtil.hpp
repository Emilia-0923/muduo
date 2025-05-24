#pragma once

#include "Buffer.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


static std::unordered_map<int, std::string> status_msg = {
    {100,  "Continue"},
    {101,  "Switching Protocol"},
    {102,  "Processing"},
    {103,  "Early Hints"},
    {200,  "OK"},
    {201,  "Created"},
    {202,  "Accepted"},
    {203,  "Non-Authoritative Information"},
    {204,  "No Content"},
    {205,  "Reset Content"},
    {206,  "Partial Content"},
    {207,  "Multi-Status"},
    {208,  "Already Reported"},
    {226,  "IM Used"},
    {300,  "Multiple Choice"},
    {301,  "Moved Permanently"},
    {302,  "Found"},
    {303,  "See Other"},
    {304,  "Not Modified"},
    {305,  "Use Proxy"},
    {306,  "unused"},
    {307,  "Temporary Redirect"},
    {308,  "Permanent Redirect"},
    {400,  "Bad Request"},
    {401,  "Unauthorized"},
    {402,  "Payment Required"},
    {403,  "Forbidden"},
    {404,  "Not Found"},
    {405,  "Method Not Allowed"},
    {406,  "Not Acceptable"},
    {407,  "Proxy Authentication Required"},
    {408,  "Request Timeout"},
    {409,  "Conflict"},
    {410,  "Gone"},
    {411,  "Length Required"},
    {412,  "Precondition Failed"},
    {413,  "Payload Too Large"},
    {414,  "URI Too Long"},
    {415,  "Unsupported Media Type"},
    {416,  "Range Not Satisfiable"},
    {417,  "Expectation Failed"},
    {418,  "I'm a teapot"},
    {421,  "Misdirected Request"},
    {422,  "Unprocessable Entity"},
    {423,  "Locked"},
    {424,  "Failed Dependency"},
    {425,  "Too Early"},
    {426,  "Upgrade Required"},
    {428,  "Precondition Required"},
    {429,  "Too Many Requests"},
    {431,  "Request Header Fields Too Large"},
    {451,  "Unavailable For Legal Reasons"},
    {501,  "Not Implemented"},
    {502,  "Bad Gateway"},
    {503,  "Service Unavailable"},
    {504,  "Gateway Timeout"},
    {505,  "HTTP Version Not Supported"},
    {506,  "Variant Also Negotiates"},
    {507,  "Insufficient Storage"},
    {508,  "Loop Detected"},
    {510,  "Not Extended"},
    {511,  "Network Authentication Required"}
};

static std::unordered_map<std::string, std::string> mime_msg = {
    {".aac",        "audio/aac"},
    {".abw",        "application/x-abiword"},
    {".arc",        "application/x-freearc"},
    {".avi",        "video/x-msvideo"},
    {".azw",        "application/vnd.amazon.ebook"},
    {".bin",        "application/octet-stream"},
    {".bmp",        "image/bmp"},
    {".bz",         "application/x-bzip"},
    {".bz2",        "application/x-bzip2"},
    {".csh",        "application/x-csh"},
    {".css",        "text/css"},
    {".csv",        "text/csv"},
    {".doc",        "application/msword"},
    {".docx",       "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".eot",        "application/vnd.ms-fontobject"},
    {".epub",       "application/epub+zip"},
    {".gif",        "image/gif"},
    {".htm",        "text/html"},
    {".html",       "text/html"},
    {".ico",        "image/vnd.microsoft.icon"},
    {".ics",        "text/calendar"},
    {".jar",        "application/java-archive"},
    {".jpeg",       "image/jpeg"},
    {".jpg",        "image/jpeg"},
    {".js",         "text/javascript"},
    {".json",       "application/json"},
    {".jsonld",     "application/ld+json"},
    {".mid",        "audio/midi"},
    {".midi",       "audio/x-midi"},
    {".mjs",        "text/javascript"},
    {".mp3",        "audio/mpeg"},
    {".mpeg",       "video/mpeg"},
    {".mpkg",       "application/vnd.apple.installer+xml"},
    {".odp",        "application/vnd.oasis.opendocument.presentation"},
    {".ods",        "application/vnd.oasis.opendocument.spreadsheet"},
    {".odt",        "application/vnd.oasis.opendocument.text"},
    {".oga",        "audio/ogg"},
    {".ogv",        "video/ogg"},
    {".ogx",        "application/ogg"},
    {".otf",        "font/otf"},
    {".png",        "image/png"},
    {".pdf",        "application/pdf"},
    {".ppt",        "application/vnd.ms-powerpoint"},
    {".pptx",       "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".rar",        "application/x-rar-compressed"},
    {".rtf",        "application/rtf"},
    {".sh",         "application/x-sh"},
    {".svg",        "image/svg+xml"},
    {".swf",        "application/x-shockwave-flash"},
    {".tar",        "application/x-tar"},
    {".tif",        "image/tiff"},
    {".tiff",       "image/tiff"},
    {".ttf",        "font/ttf"},
    {".txt",        "text/plain"},
    {".vsd",        "application/vnd.visio"},
    {".wav",        "audio/wav"},
    {".weba",       "audio/webm"},
    {".webm",       "video/webm"},
    {".webp",       "image/webp"},
    {".woff",       "font/woff"},
    {".woff2",      "font/woff2"},
    {".xhtml",      "application/xhtml+xml"},
    {".xls",        "application/vnd.ms-excel"},
    {".xlsx",       "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".xml",        "application/xml"},
    {".xul",        "application/vnd.mozilla.xul+xml"},
    {".zip",        "application/zip"},
    {".3gp",        "video/3gpp"},
    {".3g2",        "video/3gpp2"},
    {".7z",         "application/x-7z-compressed"}
};

namespace HttpUtil
{
    //字符串分割函数,将src按照sep进行分割,得到的字串放入arr中
    size_t split(const std::string& src, const std::string& sep, std::vector<std::string>& arry, bool token_compress = false) {
        size_t offset = 0;
        while (offset < src.size()) {
            size_t pos = src.find(sep, offset);
            if (pos == std::string::npos) {
                arry.emplace_back(src.substr(offset));
                return arry.size();
            }
            else {
                if (pos != offset) {
                    arry.emplace_back(src.substr(offset, pos - offset));
                }
                offset = pos + sep.size();
            }
        }
        return arry.size();
    }

    //读取文件
    bool read_file(const std::string& file_name, std::string& buffer) {
        std::ifstream ifs(file_name, std::ios::binary);
        if (ifs.is_open() == false) {
            logging.error("打开文件 %s 失败!", file_name.c_str());
            return false;
        }
        size_t file_size = 0;
        ifs.seekg(0, ifs.end);
        file_size = ifs.tellg();
        ifs.seekg(0, ifs.beg);

        buffer.resize(file_size);
        ifs.read(&buffer[0], file_size);
        if (ifs.good() == false) { 
            logging.error("读取文件 %s 出错!", file_name.c_str());
            ifs.close();
            return false;
        }
        ifs.close();
        return true;
    }

    //写入文件
    bool write_file(const std::string& file_name, const std::string& buffer) {
        std::ofstream ofs(file_name, std::ios::binary | std::ios::trunc);
        if (ofs.is_open() == false) {
            logging.error("打开文件 %s 失败!", file_name.c_str());
            return false;
        }
        ofs.write(buffer.c_str(), buffer.size());
        if (ofs.good() == false) { 
            logging.error("写入文件 %s 出错!", file_name.c_str());
            ofs.close();
            return false;
        }
        ofs.close();
        return true;
    }

    //url编码
    //. - _ ~ 字母 数字 是绝对不编码字符
    std::string url_encode(const std::string url, bool convert_space_to_plus) {
        std::string res;
        for (auto& c : url) {
            if (c == '.' || c == '-' || c == '_' || c == '~' || isalnum(c) == true) {
                res += c;
            }
            else if (c == ' ' && convert_space_to_plus == true) {
                res += '+';
            }
            else {
                char temp[4] = { 0 };
                snprintf(temp, 4, "%%%02X", c);
            }
        }
        return res;
    }

    int hex_to_int(char c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        else if (c > 'a' && c <= 'z') {
            return c - 'a' + 10;
        }
        else if (c > 'A' && c <= 'Z') {
            return c - 'A' + 10;
        }
        else {
            return -1;
        }
    }

    //url解码
    std::string url_decode(const std::string url, bool convert_space_to_plus) {
        std::string res;
        for (int i = 0; i < url.size(); i++) {
            if (url[i] == '%' && i + 2 < url.size()) {
                res += (HttpUtil::hex_to_int(url[i + 1]) << 4 + HttpUtil::hex_to_int(url[i + 2]));
                i += 2;
            }
            else if (url[i] == '+' && convert_space_to_plus == true) {
                res += ' ';
            }
            else {
                res += url[i];
            }
        }
        return res;
    }

    //状态码获取
    std::string status_desc(int status){
        auto it = status_msg.find(status);
        if (it != status_msg.end()) {
            return it->second;
        }
        else {
            return "Unknow";
        }
    }

    //根据文件后缀获取文件mime
    std::string extend_mime(const std::string& file_name) {
        size_t pos = file_name.find_last_of('.');
        if (pos == std::string::npos) {
            //二进制流
            return "application/octet-stream";
        }
        std::string extend = file_name.substr(pos);
        auto it = mime_msg.find(extend);
        if (it != mime_msg.end()) {
            return it->second;
        }
        else {
            return "application/octet-stream";
        }
    }

    //判断文件是否是一个目录
    bool is_directory(const std::string& file_name){
        struct stat st;
        int ret = stat(file_name.c_str(), &st);
        if (ret < 0) {
            return false;
        }
        else {
            //内置宏
            return S_ISDIR(st.st_mode);
        }
    }

    //判断文件是否是普通文件
    bool is_regular(const std::string& file_name) {
        struct stat st;
        int ret = stat(file_name.c_str(), &st);
        if (ret < 0) {
            return false;
        }
        else {
            //内置宏
            return S_ISREG(st.st_mode);
        }
    }

    //请求资源路径是否有效
    bool valid_path(const std::string& path) {
        std::vector<std::string> sub_dir;
        split(path, "/", sub_dir, true);
        int level = 0;
        for (auto& dir : sub_dir) {
            if (dir == "..") {
                level--;
                if (level < 0) {
                    return false;
                }
            }
            else {
                level++;
            }
        }
        return true;
    }

    std::string trim(const std::string& str) {
        if (str.empty()) return str;
        
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            return ""; // 全是空白字符
        }
        
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    // 获取文件的最后修改时间（HTTP格式）
    std::string last_modified_time(const std::string& file_path) {
        struct stat file_stat;
        if (stat(file_path.c_str(), &file_stat) != 0) {
            return "";
        }

        struct tm tm_gmt;
        if (!gmtime_r(&file_stat.st_mtime, &tm_gmt)) {
            return "";
        }

        char time_buf[64];
        strftime(time_buf, sizeof(time_buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_gmt);

        return std::string(time_buf);
    }

        // 获取规范化的绝对路径（解析符号链接、消除.和..）
    std::string canonical_path(const std::string& path) {
        char* resolved = realpath(path.c_str(), nullptr);
        if (!resolved) {
            return path; // 失败时返回原路径
        }
        
        std::string result(resolved);
        free(resolved);
        return result;
    }

    std::string http_date() {
        std::time_t now = std::time(nullptr);
        std::tm tm_gmt;
        gmtime_r(&now, &tm_gmt);
        char buf[32];
        strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_gmt);
        
        return std::string(buf);
    }
    std::string normalize_path(const std::string& path) {
        if (path.empty()) {
            return "/";
        }

        std::vector<std::string> components;
        split(path, "/", components, true);

        std::vector<std::string> normalized;
        for (const auto& component : components) {
            if (component.empty() || component == ".") {
                continue;
            }
            
            if (component == "..") {
                if (!normalized.empty()) {
                    normalized.pop_back();
                }
            } else {
                normalized.push_back(component);
            }
        }

        std::string result;
        for (const auto& component : normalized) {
            result += "/" + component;
        }

        if (result.empty()) {
            return "/";
        }
        return result;
    }
};