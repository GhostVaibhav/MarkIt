<h1 align="center">
<b> Todo </b>
</h1>
<h4 align="center">
<b> A cross-platform TODO app in teminal with simple interface and lightning fast speed for everyone </b>
</h4>
&nbsp;

## **Index**
1. [Dependencies](https://github.com/GhostVaibhav/Todo#--dependencies--)
2. [Building the project](https://github.com/GhostVaibhav/Todo#--building-the-project--)

    1. [Setting the environment](https://github.com/GhostVaibhav/Todo#1-setting-the-environment)

        1. [On Windows](https://github.com/GhostVaibhav/Todo#on-windows)
        2. [On UNIX-based systems](https://github.com/GhostVaibhav/Todo#on-unix-based-systems)
    2. [Compiling the required files for building project](https://github.com/GhostVaibhav/Todo#2-compiling-the-required-files-for-building-project)

        1. [In Windows](https://github.com/GhostVaibhav/Todo#in-windows-use)
        2. [In UNIX-based systems](https://github.com/GhostVaibhav/Todo#in-unix-based-systems-use)
3. [Licenses](https://github.com/GhostVaibhav/Todo#-licenses--)

    1. [Todo license](https://github.com/GhostVaibhav/Todo#todo-license)
    2. [The curl license](https://github.com/GhostVaibhav/Todo#the-curl-license)
    3. [JSON for Modern C++ license](https://github.com/GhostVaibhav/Todo#json-for-modern-c-license)
    4. [PDCurses](https://github.com/GhostVaibhav/Todo#pdcurses)
    5. [Ncurses license](https://github.com/GhostVaibhav/Todo#ncurses-license)

<h2 align="center"> <b> Dependencies </b> </h2>

### This project uses:
* [libcurl](https://github.com/curl/curl)
* [JSON for Modern C++](https://github.com/nlohmann/json)
* [PDCurses](https://github.com/wmcbrine/PDCurses) for Windows
* ncurses for UNIX-based system
<h2 align="center"> <b> Building the project </b> </h2>

## **1. Setting the environment**
### **On Windows**
You can simply use the libraries and the header files which come preinstalled in the ```include``` and the ```lib``` folder. However, if you wish to build them yourself, you can have a look at the repository's page mentioned above and build them on your own.
> Note: All the libraries which come preinstalled are compiled as **x86** for maximal compatibility.
### **On UNIX-based systems**

You can simply get the libraries using the **sudo apt install** command in **Debian Linux**, **Ubuntu**.

Just do:
```bash
sudo apt install g++
sudo apt install libcurl4-openssl-dev
sudo apt-get install libncurses5-dev libncursesw5-dev
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
sudo yum group install "Development Tools"
sudo yum install libcurl
sudo yum install ncurses-devel
```
Or if you are using **Fedora Linux 22.x+** do:
```bash
sudo dnf install gcc-c++
sudo yum install libcurl
sudo dnf install ncurses-devel
```
Or if you are using **Arch linux** or **Manjaro** do:
```bash
sudo pacman -Sy gcc
sudo pacman -S curl
sudo pacman -S ncurses
```
The header files are **platform-independent** and hence the above commands will **install/update** your libraries.

This will setup the environment required for building the project.

## **2. Compiling the required files for building project**

After this, compile ```build.cpp``` by launching **terminal in the same folder**:
```powershell
g++ build.cpp -o build
```
You can run the ```build``` file with the file to be built i.e.:
### In Windows use:
```bash
build test
```
### In UNIX-based systems use:
```bash
./build test
```
will compile the **test.cpp** while linking it with the required libraries and including the required header files.

You can know the functionality of all files in the [**FILE_STRUCTURE**]() file.
&nbsp;

&nbsp;

<h2 align="center"><b> Licenses </b> </h2>

## **Todo license**
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