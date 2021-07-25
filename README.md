<h1 align="center">
<b> Mark It! </b>
</h1>
<h3 align="center">
<b> A cross-platform TODO app in teminal with simple interface and lightning fast speed for everyone </b>
</h3>
&nbsp;


[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ff29f2d480744511ada59ea48fbcfc0b)](https://www.codacy.com/gh/GhostVaibhav/Todo/dashboard?utm_source=github.com&utm_medium=referral&utm_content=GhostVaibhav/Todo&utm_campaign=Badge_Grade)
[![CodeQL](https://github.com/GhostVaibhav/Todo/actions/workflows/codeql-analysis.yml/badge.svg?branch=master)](https://github.com/GhostVaibhav/Todo/actions/workflows/codeql-analysis.yml)
[![Code Inspector](https://github.com/GhostVaibhav/Todo/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/GhostVaibhav/Todo/actions/workflows/ci.yml)
[![C/C++ CI](https://github.com/GhostVaibhav/Todo/actions/workflows/c-cpp.yml/badge.svg?branch=master)](https://github.com/GhostVaibhav/Todo/actions/workflows/c-cpp.yml)

&nbsp;

<h3 align="center">
<b> Successfully built and tested on Windows, MacOS, Ubuntu, Arch Linux, Kali Linux, Debian, CentOS, Alpine, Fedora Linux, openSUSE Leap and openSUSE Tumbleweed </b>
</h3>

&nbsp;

## **Index**

- [Dependencies](#--dependencies--)
- [Building the project](#--building-the-project--)
  - [Setting the environment](#1-setting-the-environment)
    - [On Windows](#on-windows)
      - [Building automatically](#building-automatically)
      - [Building manually](#building-manually)
    - [On UNIX-based systems](#on-unix-based-systems)
  - [Compiling the required files for building project](#2-compiling-the-required-files-for-building-project)
    - [In Windows](#in-windows)
    - [In UNIX-based systems](#in-unix-based-systems-use)
- [Visualising the project](#--visualising-the-project--)
- [Licenses](#--licenses--)
  - [Todo license](#todo-license)
  - [The curl license](#the-curl-license)
  - [JSON for Modern C++ license](#json-for-modern-c-license)
  - [PDCurses](#pdcurses)
  - [Ncurses license](#ncurses-license)

<h2 align="center"> <b> Dependencies </b> </h2>

### This project uses

- [curl](https://github.com/curl/curl)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [PDCurses](https://github.com/wmcbrine/PDCurses) for Windows
- ncurses for UNIX-based system
<h2 align="center"> <b> Building the project </b> </h2>

## **1. Setting the environment**

### **On Windows**

You can simply use the libraries and the header files which come preinstalled in the **`include`** and the **`lib`** folder. However, if you wish to build them yourself, you can have a look at the repository's page mentioned above and build them on your own.

> Note: All the libraries which come preinstalled are compiled as **x86** for maximum compatibility.

- ### **Building automatically**
  You can automatically build the libraries if you want by just executing [**`prereq.bat`**](https://github.com/GhostVaibhav/Todo/blob/master/prereq.bat). It will download the latest version of all libraries and build it using **Visual Studio**.
  > Note: The script runs on two tools: **Git** and **Visual Studio** (optionally **MinGW**).

> Note: For PDCurses, you can build it through **MinGW** or **Visual Studio**, you will be prompted to enter the choice during the build process.

- ### **Building manually**
  You can manually build and install all the libraries by visiting their Github pages and following their build rules.
  > Note: For building most of the libraries you will be required to install **Visual Studio**.

### **On UNIX-based systems**

You can simply get the libraries using the **sudo apt install** command in **Debian Linux**, **Ubuntu**.

Just do:

```bash
apt install g++
apt install libcurl4-openssl-dev
apt-get install libncurses5-dev libncursesw5-dev
```

Or in **MacOS**, first install [**Homebrew**](https://brew.sh/) and then do:

```bash
brew install g++
brew install make
brew install curl
brew install ncurses
```

Or if you are using **Red Hat linux**, **CentOS** or **Fedora Linux 21 or older** do:

```bash
yum group install "Development Tools"
yum install libcurl
yum install ncurses-devel
```

Or if you are using **Fedora Linux 22.x+** do:

```bash
dnf install gcc-c++
yum install libcurl
dnf install ncurses-devel
```

Or if you are using **Arch linux** or **Manjaro** do:

```bash
pacman -Sy gcc
pacman -S curl
pacman -S ncurses
```

The header files are **platform-independent** and hence the above commands will **install/update** your libraries.

This will setup the environment required for building the project.

## **2. Compiling the required files for building project**


### **In Windows**
Compile [**`build.cpp`**](https://github.com/GhostVaibhav/Todo/blob/master/build.cpp) by launching **terminal in the same folder**:

```powershell
g++ build.cpp -o build
```

You can run the **`build`** file with the file to be built i.e.:


```bash
build main
```

### **In UNIX-based systems use**

```
CMake for building the project.
```

You can know the functionality of all files in the [**FILE_STRUCTURE**](https://github.com/GhostVaibhav/Todo/blob/master/FILE_STRUCTURE.md) file.

&nbsp;

<h2 align="center"> <b> Visualising the project </b> </h2>

&nbsp;

You can visit [**Whimsical**](https://whimsical.com/todo-working-6xZbAEdTgBjz2d8a5LQhRV) for an _interactive_ design diagram.

&nbsp;

<h2 align="center"> <b> Licenses </b> </h2>

## **Mark It! license**

Check the [**LICENSE**](https://github.com/GhostVaibhav/Todos/blob/master/LICENSE) file

## **The curl license**

Copyright &copy; 1996 - 2021, Daniel Stenberg, [daniel@haxx.se](mailto:daniel@haxx.se), and many contributors, see the THANKS file.

All rights reserved.

Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall not be used in advertising or otherwise to promote the sale, use or other dealings in this Software without prior written authorization of the copyright holder.

## **JSON for Modern C++ license**

Copyright &copy; 2013-2021 Niels Lohmann

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## **PDCurses**

Special thanks to the maintainer of the repository William McBrine [wmcbrine@gmail.com](mailto:wmcbrine@gmail.com) and Chris Szurgot [szurgot@itribe.net](mailto:szurgot@itribe.net) for porting it to the Windows console.

## **Ncurses license**

Copyright &copy; 2018-2019,2020 Thomas E. Dickey

Copyright &copy; 1998-2016,2017 Free Software Foundation, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, distribute with modifications sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name(s) of the above copyright holders shall not be used in advertising or otherwise to promote the sale, use or other dealings in this Software without prior written authorization.
