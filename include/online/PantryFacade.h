#pragma once
#include <string>
#include <vector>
#include <utility>

#include "HttpRequestBuilder.h"
#include "json.hpp"

/// Result of a loadBucket call.
/// `ok`       – true when the HTTP request succeeded with HTTP 200.
/// `notFound` – true when the server responded with HTTP 404 (bucket doesn't
///              exist yet, e.g. a brand-new user). ok will be false.
/// `data`     – parsed JSON body; empty object when the bucket is new/empty.
struct BucketResult {
  bool ok = false;
  bool notFound = false;
  nlohmann::json data;
};

/// Result of a getPantryDetails call.
/// `ok`      – true when the HTTP request succeeded with HTTP 200.
/// `baskets` – list of (basket name, TTL in seconds) pairs.
///             TTL is reset to ~2592000 (30 days) on every GET/POST/PUT.
struct PantryDetails {
  bool ok = false;
  std::vector<std::pair<std::string, long>> baskets;
};

class PantryFacade {
 public:
  explicit PantryFacade(const std::string& pantryId);

  bool bucketExists(const std::string& bucketName) const;
  bool createBucket(const std::string& bucketName) const;
  bool saveBucket(const std::string& bucketName,
                  const nlohmann::json& data) const;
  BucketResult loadBucket(const std::string& bucketName) const;

  /// Fetch pantry-level details including the list of baskets and their TTLs.
  PantryDetails getPantryDetails() const;

  /// Ensure the named bucket exists, creating it if it is absent (expired or
  /// brand-new). Returns true if the bucket is ready for use.
  bool ensureBucketExists(const std::string& bucketName) const;

 private:
  std::string pantryId;
  std::string getBaseUrl(const std::string& bucketName) const;
  std::string getPantryUrl() const;
};

