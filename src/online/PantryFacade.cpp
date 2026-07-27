#include "PantryFacade.h"

#include <spdlog/spdlog.h>

PantryFacade::PantryFacade(const std::string& id) : pantryId(id) {}

std::string PantryFacade::getBaseUrl(const std::string& bucketName) const {
  return "https://getpantry.cloud/apiv1/pantry/" + pantryId + "/basket/" +
         bucketName;
}

std::string PantryFacade::getPantryUrl() const {
  return "https://getpantry.cloud/apiv1/pantry/" + pantryId;
}

/// Returns a safe preview of a response body for logging (max 200 chars).
static std::string bodyPreview(const std::string& body) {
  if (body.empty()) return "<empty>";
  return body.size() > 200 ? body.substr(0, 200) + "..." : body;
}

bool PantryFacade::bucketExists(const std::string& bucketName) const {
  HttpRequestBuilder builder;
  builder.setUrl(getBaseUrl(bucketName))
      .setMethod("GET")
      .addHeader("Content-Type: application/json");
  auto response = builder.execute();
  if (response.success && response.httpCode == 200) return true;

  // GetPantry signals a missing basket with HTTP 404 or HTTP 400 + "does not exist"
  bool isNotFound =
      (response.success && response.httpCode == 404) ||
      (response.success && response.httpCode == 400 &&
       response.body.find("does not exist") != std::string::npos);

  if (isNotFound) {
    spdlog::debug("PantryFacade::bucketExists: basket not found (HTTP {})", response.httpCode);
    return false;
  }
  spdlog::debug("PantryFacade::bucketExists: HTTP {} | curl_ok={} | body={}",
                response.httpCode, response.success,
                bodyPreview(response.body));
  return false;
}

bool PantryFacade::createBucket(const std::string& bucketName) const {
  HttpRequestBuilder builder;
  builder.setUrl(getBaseUrl(bucketName))
      .setMethod("POST")
      .addHeader("Content-Type: application/json")
      .setBody("{}");
  auto response = builder.execute();
  if (response.success && response.httpCode == 200) return true;
  spdlog::warn("PantryFacade::createBucket: HTTP {} | curl_ok={} | body={}",
               response.httpCode, response.success,
               bodyPreview(response.body));
  return false;
}

bool PantryFacade::saveBucket(const std::string& bucketName,
                              const nlohmann::json& data) const {
  HttpRequestBuilder builder;
  builder.setUrl(getBaseUrl(bucketName))
      .setMethod("PUT")
      .addHeader("Content-Type: application/json")
      .setBody(data.dump());
  auto response = builder.execute();
  if (response.success && response.httpCode == 200) return true;
  spdlog::error("PantryFacade::saveBucket FAILED: HTTP {} | curl_ok={} | body={}",
                response.httpCode, response.success,
                bodyPreview(response.body));
  return false;
}

BucketResult PantryFacade::loadBucket(const std::string& bucketName) const {
  HttpRequestBuilder builder;
  builder.setUrl(getBaseUrl(bucketName))
      .setMethod("GET")
      .addHeader("Content-Type: application/json");
  auto response = builder.execute();
  if (response.success && response.httpCode == 200) {
    BucketResult result;
    result.ok = true;
    try {
      result.data = nlohmann::json::parse(response.body);
    } catch (...) {
      // Body was not valid JSON (e.g. newly created empty bucket) — ok=true,
      // data stays as default empty object.
    }
    return result;
  }

  // GetPantry returns HTTP 404 *or* HTTP 400 with "does not exist" in the body
  // when a basket is absent (expired or never created).
  bool isNotFound =
      (response.success && response.httpCode == 404) ||
      (response.success && response.httpCode == 400 &&
       response.body.find("does not exist") != std::string::npos);

  if (isNotFound) {
    spdlog::info("PantryFacade::loadBucket: HTTP {} — bucket absent or expired | body={}",
                 response.httpCode, bodyPreview(response.body));
    return BucketResult{false, true, {}};
  }

  // Transport failure (curl error, httpCode=0) or unexpected HTTP status.
  spdlog::warn("PantryFacade::loadBucket FAILED: HTTP {} | curl_ok={} | body={}",
               response.httpCode, response.success,
               bodyPreview(response.body));
  return BucketResult{false, false, {}};
}

PantryDetails PantryFacade::getPantryDetails() const {
  HttpRequestBuilder builder;
  builder.setUrl(getPantryUrl())
      .setMethod("GET")
      .addHeader("Content-Type: application/json");
  auto response = builder.execute();

  PantryDetails details;
  if (!response.success || response.httpCode != 200) {
    spdlog::warn("PantryFacade::getPantryDetails FAILED: HTTP {} | curl_ok={} | body={}",
                 response.httpCode, response.success,
                 bodyPreview(response.body));
    return details;
  }

  try {
    auto body = nlohmann::json::parse(response.body);
    details.ok = true;
    if (body.contains("baskets") && body["baskets"].is_array()) {
      for (const auto& b : body["baskets"]) {
        std::string name = b.value("name", "");
        long ttl = b.value("ttl", 0L);
        if (!name.empty()) {
          details.baskets.emplace_back(name, ttl);
        }
      }
    }
    spdlog::info("PantryFacade: getPantryDetails returned {} basket(s)",
                 details.baskets.size());
  } catch (...) {
    spdlog::error("PantryFacade: getPantryDetails failed to parse response body={}",
                  bodyPreview(response.body));
    details.ok = false;
  }
  return details;
}

bool PantryFacade::ensureBucketExists(const std::string& bucketName) const {
  // First check if the bucket already exists
  if (bucketExists(bucketName)) {
    spdlog::info("PantryFacade: Bucket '{}' already exists", bucketName);
    return true;
  }
  // Bucket is absent (expired or never created) — recreate it
  spdlog::warn(
      "PantryFacade: Bucket '{}' is missing (expired or new). Recreating...",
      bucketName);
  bool created = createBucket(bucketName);
  if (created) {
    spdlog::info("PantryFacade: Bucket '{}' successfully recreated",
                 bucketName);
  } else {
    spdlog::error("PantryFacade: Failed to recreate bucket '{}'", bucketName);
  }
  return created;
}


