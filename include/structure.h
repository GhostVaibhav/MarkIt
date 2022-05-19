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

#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "json.hpp"

struct todo
{
    std::string name;                                                   //!< Storing the name of Todo
    std::string desc;                                                   //!< Storing the description of Todo
    std::string time;                                                   //!< Automatically generating the time for a Todo
    bool isComplete;                                                    //!< Marking the Todo as "complete" or "not complete"
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(todo, name, desc, time, isComplete); //!< For serializing and deserializing JSON from Todo structure and vice-versa
};

#endif