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

#ifndef CLOUD_FUNCTIONS_H
#define CLOUD_FUNCTIONS_H

#include "json.hpp"
#include "globalVariable.h"
#include "curl/curl.h"

/*! @brief For using it with Curl and writing response to a string
 *
 * @return size_t (Determining the size of the string)
 * @param void* Storing the pointer to the text to append
 * @param size_t Size of a character
 * @param size_t Count of a character
 * @param void* Main string to write
 */
size_t write_to_string(void *, size_t, size_t, void *);

/*! @brief Getting a bucket from an API call
 *
 * @return bool (If the bucket exists)
 * @param std::string Bucket name
 */
bool getBucket(const std::string &);

/*! @brief Creating a bucket through an API call
 *
 * @return int (Returning a creation code)
 * @param std::string Bucket name
 */
int createBucket(const std::string &);

/*! @brief Replacing an existing bucket through an API call
 *
 * @return bool (If the replacement was successful)
 * @param std::string Bucket name
 * @param nlohmann::json JSON object to replace
 */
bool replaceBucket(const std::string &, const nlohmann::json &);

/*! @brief Deleting a bucket through an API call
 *
 * @return bool (If the delete was successful)
 * @param std::string Bucket name
 */
bool deleteBucket(const std::string &);

/*! @brief Getting the bucket details (in the form of string) and serializing it to a JSON structure through an API call
 *
 * @return nlohmann::json (JSON object of bucket details)
 * @param std::string Bucket name
 */
nlohmann::json getBucketDetails(const std::string &);

/*! @brief Appending to the end of a bucket through an API call (useful in pushing to the cloud)
 *
 * @return bool (If the append was successful)
 * @param std::string Bucket name
 * @param nlohmann::json JSON object to append
 */
bool appendBucket(const std::string &, const nlohmann::json &);

#endif