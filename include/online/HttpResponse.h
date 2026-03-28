#pragma once
#include <string>

struct HttpResponse {
    std::string body;
    long httpCode;
    bool success;
};
