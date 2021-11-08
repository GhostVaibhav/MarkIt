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

#include "tabulate.hpp"
#include "globalVariable.h"
#include "json.hpp"
#include "fileHandlers.h"
#include "structure.h"
#include "cli.h"

#define CLI
using json = nlohmann::json;

// ------------------------------------------------------------------------
// --------------------------CLI FUNCTIONALITY-----------------------------
// ------------------------------------------------------------------------

namespace cli
{
    void test(const std::vector<std::string>& args)
    {
        tabulate::Table t;
        t.add_row({"Starting test mode (only for dev-builds)"});
        t.add_row({"Only call it when you know what you are doing"});
        t.add_row({"Arguments got: "});
        for (std::string arg : args)
            t.add_row({arg});
        t[0][0].format().font_color(tabulate::Color::blue).font_style({tabulate::FontStyle::bold}).font_align(tabulate::FontAlign::center);
        t[1][0].format().font_color(tabulate::Color::red).font_style({tabulate::FontStyle::bold}).font_align(tabulate::FontAlign::center);
        t[2][0].format().font_color(tabulate::Color::green);
        std::cout << t << std::endl;
    }
    void version(std::vector<std::string>& args)
    {
        if (args.size() > 1)
        {
            std::transform(args[1].begin(), args[1].end(), args[1].begin(), [](char &a)
                           { return std::tolower(a); });
            if (args[1] == "classic" || args[1] == "simple")
            {
                std::cout << "MarkIt!" << std::endl;
                std::cout << "Version: " << APP_VERSION << std::endl;
            }
            else if (args[1] == "modern")
            {
                tabulate::Table t;
                t.add_row({"MarkIt!", "", ""});
                t.add_row({"", "Version", APP_VERSION});
                for (int i = 0; i < 3; i++)
                {
                    t[0][i].format().font_align(tabulate::FontAlign::center).font_color(tabulate::Color::yellow).font_style({tabulate::FontStyle::bold});
                    t[1][i].format().font_align(tabulate::FontAlign::center).font_color(tabulate::Color::yellow).font_style({tabulate::FontStyle::bold});
                }
                t[0][1].format().font_background_color(tabulate::Color::yellow);
                t[0][2].format().font_background_color(tabulate::Color::yellow);
                t[1][0].format().font_background_color(tabulate::Color::yellow);
                std::cout << t << std::endl;
            }
            else
            {
                std::cout << "No format \"" << args[1] << "\" known" << std::endl;
            }
        }
        else
        {
            tabulate::Table t;
            t.add_row({"MarkIt!", "", ""});
            t.add_row({"", "Version", APP_VERSION});
            for (int i = 0; i < 3; i++)
            {
                t[0][i].format().font_align(tabulate::FontAlign::center).font_color(tabulate::Color::yellow).font_style({tabulate::FontStyle::bold});
                t[1][i].format().font_align(tabulate::FontAlign::center).font_color(tabulate::Color::yellow).font_style({tabulate::FontStyle::bold});
            }
            t[0][1].format().font_background_color(tabulate::Color::yellow);
            t[0][2].format().font_background_color(tabulate::Color::yellow);
            t[1][0].format().font_background_color(tabulate::Color::yellow);
            std::cout << t << std::endl;
        }
    }
    void display(const std::vector<std::string>& args)
    {
        if (!exist(storageFile))
        {
            std::cout << "No data found" << std::endl;
            return;
        }
        tabulate::Table t;
        json j = json::parse(_read_from_file(storageFile));
        std::vector<todo> temp = j["data"];
        t.add_row({"Name", "Description", "Time", "Completed"});
        for (int i = 0; i < temp.size(); i++)
        {
            if (temp[i].isComplete)
            {
                t.add_row({temp[i].name, temp[i].desc, temp[i].time, "Yes"});
                t[i + 1][3].format().font_color(tabulate::Color::green);
            }
            else
            {
                t.add_row({temp[i].name, temp[i].desc, temp[i].time, "No"});
                t[i + 1][3].format().font_color(tabulate::Color::red);
            }
        }
        for (int i = 0; i < 4; i++)
        {
            t.column(i).format().font_align(tabulate::FontAlign::center);
            t[0][i].format().font_color(tabulate::Color::blue).font_style({tabulate::FontStyle::bold});
        }
        std::cout << t << std::endl;
    }
}