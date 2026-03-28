#pragma once
#include <curl/curl.h>
#include <string>
#include "HttpResponse.h"

class HttpRequestBuilder {
public:
    HttpRequestBuilder();
    ~HttpRequestBuilder();
    
    HttpRequestBuilder& setUrl(const std::string& url);
    HttpRequestBuilder& setMethod(const std::string& method);
    HttpRequestBuilder& setBody(const std::string& bodyStr);
    HttpRequestBuilder& addHeader(const std::string& header);
    
    HttpResponse execute();

private:
    CURL* curl;
    std::string url;
    std::string method;
    std::string body;
    curl_slist* headers = nullptr;
    static size_t writeCallback(void* ptr, size_t size, size_t count, void* stream);
};
