#include "httpclient.h"

bool s_curl_init = false;

namespace http {

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
    if(NULL == str || NULL == buffer)
    {
        return -1;
    }

    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    return nmemb;
}


Result Get(const std::string& url, const Headers &headers)
{
    if (!s_curl_init) {
        s_curl_init = true;
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return Result(CURLcode::CURLE_FAILED_INIT,"");
    }

    std::string strResponse;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);

    struct curl_slist *headerlist = NULL;
    for (auto item : headers)
    {
        std::string header_builder;
        header_builder.append(item.first);
        if ( !item.second.empty() ) {
            header_builder.append(": ");
            header_builder.append(item.second);
        } else {
            header_builder.append(";");
        }
        headerlist = curl_slist_append(headerlist, header_builder.c_str());
    }
    if (headerlist) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    }
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 12);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 12);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return Result(res, strResponse);
}

Result Post(const std::string& url, const std::string &body, const Headers &headers, const std::string &content_type)
{
    if (!s_curl_init) {
        s_curl_init = true;
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return Result(CURLcode::CURLE_FAILED_INIT,"");
    }

    std::string strResponse;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);

    // 设置http发送的内容类型为JSON
    Headers newheaders = headers;
    if (!content_type.empty()) {
        newheaders.emplace("Content-Type", content_type);
    }

    struct curl_slist *headerlist = NULL;
    for (auto item : newheaders)
    {
        std::string header_builder;
        header_builder.append(item.first);
        if ( !item.second.empty() ) {
            header_builder.append(": ");
            header_builder.append(item.second);
        } else {
            header_builder.append(";");
        }
        headerlist = curl_slist_append(headerlist, header_builder.c_str());
    }
    if (headerlist) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    }

    // 设置要POST的JSON数据
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 12);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 12);
    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return Result(res, strResponse);
}

}
