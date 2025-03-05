#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <map>
#include <memory>
#include <curl/curl.h>

namespace http {

using Headers = std::multimap<std::string, std::string>;

struct Result {
    CURLcode code_;
    std::string body_;

    Result(CURLcode code, std::string body) : code_(code), body_(body){}
};

Result Get(const std::string& url, const Headers &headers);

Result Post(const std::string& url, const std::string &body, const Headers &headers, const std::string &content_type);


}



#endif // HTTPCLIENT_H
