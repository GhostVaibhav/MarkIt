#pragma once
#include <curl/curl.h>

#include <chrono>
#include <mutex>
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

  // Process-global rate limiter — shared across ALL HttpRequestBuilder instances
  static std::mutex  s_rateMtx;
  static std::chrono::steady_clock::time_point s_lastRequestTime;
};
