#include "PantryFacade.h"

PantryFacade::PantryFacade(const std::string& id) : pantryId(id) {}

std::string PantryFacade::getBaseUrl(const std::string& bucketName) const {
  return "https://getpantry.cloud/apiv1/pantry/" + pantryId + "/basket/" +
         bucketName;
}

bool PantryFacade::bucketExists(const std::string& bucketName) const {
  HttpRequestBuilder builder;
  builder.setUrl(getBaseUrl(bucketName))
      .setMethod("GET")
      .addHeader("Content-Type: application/json");
  auto response = builder.execute();
  return response.success && response.httpCode == 200;
}

bool PantryFacade::createBucket(const std::string& bucketName) const {
  HttpRequestBuilder builder;
  builder.setUrl(getBaseUrl(bucketName))
      .setMethod("POST")
      .addHeader("Content-Type: application/json")
      .setBody("{}");
  auto response = builder.execute();
  return response.success && response.httpCode == 200;
}

bool PantryFacade::saveBucket(const std::string& bucketName,
                              const nlohmann::json& data) const {
  HttpRequestBuilder builder;
  builder.setUrl(getBaseUrl(bucketName))
      .setMethod("PUT")
      .addHeader("Content-Type: application/json")
      .setBody(data.dump());
  auto response = builder.execute();
  return response.success && response.httpCode == 200;
}

nlohmann::json PantryFacade::loadBucket(const std::string& bucketName) const {
  HttpRequestBuilder builder;
  builder.setUrl(getBaseUrl(bucketName))
      .setMethod("GET")
      .addHeader("Content-Type: application/json");
  auto response = builder.execute();
  if (response.success && response.httpCode == 200) {
    try {
      return nlohmann::json::parse(response.body);
    } catch (...) {
      return nlohmann::json();
    }
  }
  return nlohmann::json();
}
