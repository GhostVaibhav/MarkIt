#  **File**  *structure*


**\*  -  Only for Windows / Required in Windows**


#  *Files*


-  ##  **main.cpp**


###  This is the main file in which the whole program is contained a.k.a **monolithic programming**.


&nbsp;


-  ##  **build.cpp**


###  As the name suggests this file **builds the project.**


> ### Compatible on all platforms


&nbsp;


-  ##  **clean.cpp**


###  This is the file used for cleaning the codebase i.e getting rid of ***.exe** files in **Windows** or name executables in **UNIX-platforms**.


>  ###  Usually used just before commiting in the repo


&nbsp;


-  ##  **prereq.bat***


###  This is the file which will build all the **dependencies** of this project.


>  ###  For using this, you require either of the two compilers: **MinGW** or **MSVC** and **Git** installed


&nbsp;


-  ##  **CMakeLists.txt** - :warning:**NOT READY YET**:warning:


###  This is the file which will build this project including all the dependencies. Still experimenting since I don't know how to use CMake.


&nbsp;


#  *Folders*


-  ##  **_include/_**


###  This is the folder which contains all the header files.


&nbsp;


-  ##  **_lib/_***


###  This is the folder which contains all the static libraries.


&nbsp;


-  ##  **_bin/_***


###  This is the folder where the produced executables go.


> ### In UNIX-platforms the executable is produced in the main folder itself.