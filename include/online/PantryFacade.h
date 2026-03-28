#pragma once
#include <string>
#include "json.hpp"
#include "HttpRequestBuilder.h"

class PantryFacade {
public:
    explicit PantryFacade(const std::string& pantryId);

    bool bucketExists(const std::string& bucketName) const;
    bool createBucket(const std::string& bucketName) const;
    bool saveBucket(const std::string& bucketName, const nlohmann::json& data) const;
    nlohmann::json loadBucket(const std::string& bucketName) const;

private:
    std::string pantryId;
    std::string getBaseUrl(const std::string& bucketName) const;
};
