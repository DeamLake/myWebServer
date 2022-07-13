#pragma once

#include <unordered_map>
#include <unordered_set>
#include <mysql/mysql.h>  //mysql
#include <regex>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnRAII.h"

class HttpRequest 
{
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH
    };

    enum HTTP_CODE {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESPONSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

    HttpRequest() { Init();}
    ~HttpRequest() = default;

    void Init();
    bool parse(Buffer& buff);

    std::string path() const { return path_; }
    std::string& path() { return path_; }
    std::string method() const { return method_; }
    std::string version() const { return version_; }
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;
    
    bool IsKeepAlive() const;

private:
    bool ParseRequestLine(const std::string& line);
    void ParseHeader(const std::string& line);
    void ParseBody(const std::string& line);

    void ParsePath();
    void ParsePost();
    void ParseFromUrlencoded(); 

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);

    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);
};