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

#ifndef FILE_HANDLERS_H
#define FILE_HANDLERS_H
#include "json.hpp"

/*! @brief For writing to the local file
 *
 * @return NOTHING
 * @param nlohmann::json The JSON object to be written to the file
 * @param std::string The file name to be written to
 */
void _write_to_file(nlohmann::json, std::string);

/*! @brief For reading from the local file
 *
 * @return std::string (The JSON object read from the file in the form of a string)
 * @param std::string The file name to be read from
 */
std::string _read_from_file(std::string);

/*! @brief For deleting the file's contents
 *
 * @return NOTHING
 * @param std::string The file to be deleted
 */
void _delete_file(std::string);

/*! @brief Checks if a file exists
 *
 * @return bool (return true, if file exists)
 * @param std::string File name
 */
bool exist(const std::string&);

#endif