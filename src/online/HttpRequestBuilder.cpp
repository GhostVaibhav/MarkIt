#include "HttpRequestBuilder.h"

#include <thread>

#include <spdlog/spdlog.h>
#include "config/UpdateConfig.h"

// --- Static rate-limiter state (process-global) ---
std::mutex HttpRequestBuilder::s_rateMtx;
std::chrono::steady_clock::time_point HttpRequestBuilder::s_lastRequestTime{};

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

static size_t writeCallback(void* ptr, size_t size, size_t count,
                             void* stream) {
  auto* str = static_cast<std::string*>(stream);
  str->append(static_cast<char*>(ptr), size * count);
  return size * count;
}

/// Perform a single curl request and return the response.
static HttpResponse doExecute(CURL* curl, const std::string& url,
                              const std::string& method, const std::string& body,
                              curl_slist* headers) {
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
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

  CURLcode res = curl_easy_perform(curl);
  if (res == CURLE_OK) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.httpCode);
    response.success = true;
    // Reset body buffer for potential retry
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
  }
  return response;
}

HttpResponse HttpRequestBuilder::execute() {
  using namespace std::chrono;
  using ms = milliseconds;

  // --- Global rate limiter: enforce kRequestMinIntervalMs AFTER the previous
  // response was received (not from when the previous request was sent).
  // Release the lock before sleeping to avoid blocking other threads. ---
  {
    std::unique_lock<std::mutex> lock(s_rateMtx);
    auto elapsed = duration_cast<ms>(steady_clock::now() - s_lastRequestTime).count();
    long toWait = UpdateConfig::kRequestMinIntervalMs - static_cast<long>(elapsed);
    lock.unlock();  // release before sleeping
    if (toWait > 0) {
      spdlog::debug("HttpRequestBuilder: throttling {}ms to respect rate limit", toWait);
      std::this_thread::sleep_for(ms(toWait));
    }
  }

  HttpResponse response = doExecute(curl, url, method, body, headers);

  // Update timestamp AFTER receiving the response — this is the key fix.
  // Measuring from response-received (not request-sent) ensures the full
  // kRequestMinIntervalMs gap exists between consecutive server interactions.
  {
    std::lock_guard<std::mutex> lock(s_rateMtx);
    s_lastRequestTime = steady_clock::now();
  }

  // --- 429 retry: wait kRateLimitRetryDelayMs and try once more ---
  if (response.success && response.httpCode == 429) {
    spdlog::warn("HttpRequestBuilder: HTTP 429 received — waiting {}ms before retry",
                 UpdateConfig::kRateLimitRetryDelayMs);
    std::this_thread::sleep_for(ms(UpdateConfig::kRateLimitRetryDelayMs));

    // Reset curl write buffer before retry
    response.body.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);

    response = doExecute(curl, url, method, body, headers);

    {
      std::lock_guard<std::mutex> lock(s_rateMtx);
      s_lastRequestTime = steady_clock::now();
    }

    if (response.success && response.httpCode == 429) {
      spdlog::error("HttpRequestBuilder: HTTP 429 on retry — server is still rate-limiting");
    }
  }

  return response;
}
