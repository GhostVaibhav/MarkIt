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

// ------------------------------------------------------------------------
// --------------------CORE CLOUD SERVICE FUNCTIONS------------------------
// ------------------------------------------------------------------------
// 1. write_to_string() - For using it with Curl and writing response to a string
// --------------------------------------------------------
// | Returns: size_t (Determining the size of the string) |
// | Parameters:                                          |
// | void* - Storing the pointer to the text to append    |
// | size_t - Size of a character                         |
// | size_t - Count of a character                        |
// | void* - Main string to write                         |
// --------------------------------------------------------
// 2. getBucket() - Getting a bucket from an API call
// -----------------------------------------
// | Returns: bool - If the bucket exists  |
// | Parameters: std::string - Bucket name |
// -----------------------------------------
// 3. createBucket() - Creating a bucket through an API call
// --------------------------------------------
// | Returns: int - Returning a creation code |
// | Parameters: std::string - Bucket name    |
// --------------------------------------------
// 4. replaceBucket() - Replacing an existing bucket through an API call
// -----------------------------------------------------------------------
// | Returns: bool - If the replacement was successful                   |
// | Parameters: std::string, json - Bucket name, JSON object to replace |
// -----------------------------------------------------------------------
// 5. deleteBucket() - Deleting a bucket through an API call
// ------------------------------------------------
// | Returns: bool - If the delete was successful |
// | Parameters: std::string - Bucket name        |
// ------------------------------------------------
// 6. getBucketDetails() - Getting the bucket details (in the form of string) and serializing it to a JSON structure through an API call
// -------------------------------------------------
// | Returns: json - JSON object of bucket details |
// | Parameters: std::string - Bucket name         |
// -------------------------------------------------
// 7. appendBucket() - Appending to the end of a bucket through an API call (useful in pushing to the cloud)
// ----------------------------------------------------------------------
// | Returns: bool - If the append was successful                       |
// | Parameters: std::string, json - Bucket name, JSON object to append |
// ----------------------------------------------------------------------

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

bool replaceBucket(const std::string &bucketName, const json &bucket)
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

json getBucketDetails(const std::string &bucketName)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    json temp;
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
        temp = json::parse(result);
    }
    curl_easy_cleanup(curl);
    return temp;
}

bool appendBucket(const std::string &bucketName, const json &patch)
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