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

#include <fstream>
#include <sys/stat.h>
#include "fileHandlers.h"

// ------------------------------------------------------------------------
// ---------------------CORE FILE SERVICE FUNCTIONS------------------------
// ------------------------------------------------------------------------
// This section consists of three internal file modification functions:
// 1. _write_to_file() - For writing to the local file
// -----------------------------------------------------------------------
// | Returns: NOTHING                                                    |
// | Parameters:                                                         |
// | json object - The JSON object to be written to the file             |
// | std::string - The file name to be written to                        |
// -----------------------------------------------------------------------
// 2. _read_from_file() - For reading from the local file
// -------------------------------------------------------------------------------------
// | Returns: std::string - The JSON object read from the file in the form of a string |
// | Parameters: std::string - The file name to be read from                           |
// -------------------------------------------------------------------------------------
// 3. _delete_file() - For deleting the file's contents
// ----------------------------------------------------
// | Returns: NOTHING                                 |
// | Parameters: std::string - The file to be deleted |
// ----------------------------------------------------
// 4. exist() - checks if a file exists
// -----------------------------------------------
// | Returns: bool - return true, if file exists |
// | Parameters: std::string - file name         |
// -----------------------------------------------

void _write_to_file(json temp, std::string STORAGE_FILE)
{
    std::ofstream f1(STORAGE_FILE);
    f1 << temp.dump();
    f1.close();
}

std::string _read_from_file(std::string STORAGE_FILE)
{
    std::ifstream f1(STORAGE_FILE.c_str());
    std::string _read_string;
    if (!f1.is_open())
        _read_string = "File does not exist";
    else
        std::getline(f1, _read_string);
    f1.close();
    return _read_string;
}

void _delete_file(std::string STORAGE_FILE)
{
    std::ofstream f1;
    f1.open(STORAGE_FILE, std::ofstream::out | std::ofstream::trunc);
    f1.close();
}

bool exist(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}