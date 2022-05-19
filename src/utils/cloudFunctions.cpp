/*
 *      __  ___         __    ____ __  __
 *     /  |/  /__ _____/ /__ /  _// /_/ /
 *    / /|_/ / _ `/ __/  '_/_/ / / __/_/
 *   /_/  /_/\_,_/_/ /_/\_\/___/ \__(_)
 *   
 *   MIT License
 *   
 *   Copyright (c) 2021 Vaibhav Sharma
 *   
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *   
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *   
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 */

#include "cloudFunctions.h"

size_t write_to_string(void *ptr, size_t size, size_t count, void *stream)
{
    ((std::string *)stream)->append((char *)ptr, 0, size * count);
    return size * count;
}

bool getBucket(const std::string &bucketName)
{
    CURL *curl;
    CURLcode res;
    std::string resp;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        const char *data = "";
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
        if (res != 0)
            throw curl_easy_strerror(res);
    }
    curl_easy_cleanup(curl);
    if (resp.find("Could not get basket") != resp.npos)
        return false;
    else
        return true;
}

int createBucket(const std::string &bucketName)
{
    std::string resp;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        const char *data = "";
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    return (int)res;
}

bool replaceBucket(const std::string &bucketName, const nlohmann::json &bucket)
{
    if (!getBucket(bucketName))
        return false;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string resp;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        std::string update = bucket.dump();
        const char *data = update.c_str();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    return true;
}

bool deleteBucket(const std::string &bucketName)
{
    CURL *curl;
    CURLcode res;
    std::string resp;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        const char *data = "";
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    if (resp.find("Could not delete") != std::string::npos)
        return false;
    else
        return true;
}

nlohmann::json getBucketDetails(const std::string &bucketName)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    nlohmann::json temp;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        const char *data = "";
        std::string result;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
        if (res != 0)
            throw curl_easy_strerror(res);
        temp = nlohmann::json::parse(result);
    }
    curl_easy_cleanup(curl);
    return temp;
}

bool appendBucket(const std::string &bucketName, const nlohmann::json &patch)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string resp;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        std::string update = patch.dump();
        const char *data = update.c_str();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    if (resp.find("does not exist") != resp.npos)
        return false;
    else
        return true;
}