#include <iostream>
#include <sys/stat.h>
using namespace std;
inline bool exist(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}
int main()
{
#ifdef _WIN32
    system("cls");
    system("del src\\*.o >nul 2>&1");
    system("del src\\*.exe >nul 2>&1");
    system("del bin\\*.dat >nul 2>&1");
    system("del bin\\main.exe >nul 2>&1");
    system("del bin\\main_debug.exe >nul 2>&1");
    system("del *.dat >nul 2>&1");
    system("del *.out >nul 2>&1");
    system("del main_debug >nul 2>&1");
    system("del build.exe >nul 2>&1");
    system("del main >nul 2>&1");
    system("del generates >nul 2>&1");
    system("del clean.exe >nul 2>&1");
#else
    system("clear");
    system("rm *.exe > /dev/null 2>&1");
    system("rm src/*.exe > /dev/null 2>&1");
    system("rm bin/*.dat > /dev/null 2>&1");
    system("rm bin/main.exe > /dev/null 2>&1");
    system("rm bin/main_debug.exe > /dev/null 2>&1");
    system("rm *.out > /dev/null 2>&1");
    system("rm *.o > /dev/null 2>&1");
    system("rm *.dat > /dev/null 2>&1");
    system("rm main_debug > /dev/null 2>&1");
    system("rm src/main > /dev/null 2>&1");
    system("rm tools/main > /dev/null 2>&1");
    system("rm build > /dev/null 2>&1");
    system("rm generates > /dev/null 2>&1");
    system("rm clean > /dev/null 2>&1");
#endif
#ifdef _WIN32
    if (!exist("bin/main.exe") && !exist("bin/main_debug.exe") && !exist("src/main") && !exist("main_debug") && !exist("main.o") && !exist("main.out") && !exist("main_debug.out") && !exist("clean") && !exist("build") && !exist("build.exe") && !exist("data.dat") && !exist("bin/data.dat"))
    {
        cout << "\u001b[32m   ________                          __\u001b[0m\n";
        cout << "\u001b[32m  / ____/ /__  ____ _____  ___  ____/ /\u001b[0m\n";
        cout << "\u001b[32m / /   / / _ \\/ __ `/ __ \\/ _ \\/ __  /\u001b[0m\n";
        cout << "\u001b[32m/ /___/ /  __/ /_/ / / / /  __/ /_/ /\u001b[0m\n";
        cout << "\u001b[32m\\____/_/\\___/\\__,_/_/ /_/\\___/\\__,_/\u001b[0m\n\n";
        cout << "\u001b[32m      Remove clean.exe manually\u001b[0m\n";
    }
    else
    {
        cout << "\u001b[31m    _   __      __     ________                          __\u001b[0m\n";
        cout << "\u001b[31m   / | / /___  / /_   / ____/ /__  ____ _____  ___  ____/ /\u001b[0m\n";
        cout << "\u001b[31m  /  |/ / __ \\/ __/  / /   / / _ \\/ __ `/ __ \\/ _ \\/ __  /\u001b[0m\n";
        cout << "\u001b[31m / /|  / /_/ / /_   / /___/ /  __/ /_/ / / / /  __/ /_/ /\u001b[0m\n";
        cout << "\u001b[31m/_/ |_/\\____/\\__/   \\____/_/\\___/\\__,_/_/ /_/\\___/\\__,_/\u001b[0m\n\n";
    }
#elif __APPLE__
    if (!exist("bin/main.exe") && !exist("bin/main_debug.exe") && !exist("main") && !exist("main_debug") && !exist("main.o") && !exist("main.out") && !exist("main_debug.out") && !exist("generates") && !exist("clean") && !exist("clean.exe") && !exist("build") && !exist("build.exe") && !exist("data.dat") && !exist("bin/data.dat"))
    {
        cout << "   ________                          __\n";
        cout << "  / ____/ /__  ____ _____  ___  ____/ /\n";
        cout << " / /   / / _ \\/ __ `/ __ \\/ _ \\/ __  /\n";
        cout << "/ /___/ /  __/ /_/ / / / /  __/ /_/ /\n";
        cout << "\\____/_/\\___/\\__,_/_/ /_/\\___/\\__,_/\n\n";
    }
    else
    {
        cout << "    _   __      __     ________                          __\n";
        cout << "   / | / /___  / /_   / ____/ /__  ____ _____  ___  ____/ /\n";
        cout << "  /  |/ / __ \\/ __/  / /   / / _ \\/ __ `/ __ \\/ _ \\/ __  /\n";
        cout << " / /|  / /_/ / /_   / /___/ /  __/ /_/ / / / /  __/ /_/ /\n";
        cout << "/_/ |_/\\____/\\__/   \\____/_/\\___/\\__,_/_/ /_/\\___/\\__,_/\n\n";
    }
#else
    if (!exist("bin/main.exe") && !exist("bin/main_debug.exe") && !exist("src/main") && !exist("main_debug") && !exist("main.o") && !exist("main.out") && !exist("main_debug.out") && !exist("generates") && !exist("clean") && !exist("clean.exe") && !exist("build") && !exist("build.exe") && !exist("data.dat") && !exist("bin/data.dat"))
    {
        cout << "\u001b[32m   ________                          __\u001b[0m\n";
        cout << "\u001b[32m  / ____/ /__  ____ _____  ___  ____/ /\u001b[0m\n";
        cout << "\u001b[32m / /   / / _ \\/ __ `/ __ \\/ _ \\/ __  /\u001b[0m\n";
        cout << "\u001b[32m/ /___/ /  __/ /_/ / / / /  __/ /_/ /\u001b[0m\n";
        cout << "\u001b[32m\\____/_/\\___/\\__,_/_/ /_/\\___/\\__,_/\u001b[0m\n\n";
    }
    else
    {
        cout << "\u001b[31m    _   __      __     ________                          __\u001b[0m\n";
        cout << "\u001b[31m   / | / /___  / /_   / ____/ /__  ____ _____  ___  ____/ /\u001b[0m\n";
        cout << "\u001b[31m  /  |/ / __ \\/ __/  / /   / / _ \\/ __ `/ __ \\/ _ \\/ __  /\u001b[0m\n";
        cout << "\u001b[31m / /|  / /_/ / /_   / /___/ /  __/ /_/ / / / /  __/ /_/ /\u001b[0m\n";
        cout << "\u001b[31m/_/ |_/\\____/\\__/   \\____/_/\\___/\\__,_/_/ /_/\\___/\\__,_/\u001b[0m\n\n";
    }
#endif
    return 0;
}