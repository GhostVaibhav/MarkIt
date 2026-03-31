#include "HttpRequestBuilder.h"

HttpRequestBuilder::HttpRequestBuilder() { curl = curl_easy_init(); }

HttpRequestBuilder::~HttpRequestBuilder() {
  if (curl) curl_easy_cleanup(curl);
  if (headers) curl_slist_free_all(headers);
}

HttpRequestBuilder& HttpRequestBuilder::setUrl(const std::string& u) {
  url = u;
  return *this;
}

HttpRequestBuilder& HttpRequestBuilder::setMethod(const std::string& m) {
  method = m;
  return *this;
}

HttpRequestBuilder& HttpRequestBuilder::setBody(const std::string& b) {
  body = b;
  return *this;
}

HttpRequestBuilder& HttpRequestBuilder::addHeader(const std::string& header) {
  headers = curl_slist_append(headers, header.c_str());
  return *this;
}

size_t HttpRequestBuilder::writeCallback(void* ptr, size_t size, size_t count,
                                         void* stream) {
  auto* str = static_cast<std::string*>(stream);
  str->append(static_cast<char*>(ptr), size * count);
  return size * count;
}

HttpResponse HttpRequestBuilder::execute() {
  HttpResponse response{"", 0, false};
  if (!curl) return response;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());

  if (headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  if (!body.empty()) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  CURLcode res = curl_easy_perform(curl);
  if (res == CURLE_OK) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.httpCode);
    response.success = true;
  }

  return response;
}
