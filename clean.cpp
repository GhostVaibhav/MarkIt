#include <iostream>
using namespace std;
int main() {
    #ifdef _WIN32
    system("cls");
    system("del *.o >nul 2>&1");
    system("del *.exe >nul 2>&1");
    system("del bin\\*.dat >nul 2>&1");
    system("del bin\\test.exe >nul 2>&1");
    system("del bin\\test_debug.exe >nul 2>&1");
    system("del *.dat >nul 2>&1");
    system("del *.out >nul 2>&1");
    system("del test_debug >nul 2>&1");
    system("del test >nul 2>&1");
    system("del generates >nul 2>&1");
    #else
    system("clear");
    system("rm *.exe > /dev/null 2>&1");
    system("rm bin/*.dat > /dev/null 2>&1");
    system("rm bin/test.exe > /dev/null 2>&1");
    system("rm bin/test_debug.exe > /dev/null 2>&1");
    system("rm *.out > /dev/null 2>&1");
    system("rm *.o > /dev/null 2>&1");
    system("rm *.dat > /dev/null 2>&1");
    system("rm test_debug > /dev/null 2>&1");
    system("rm test > /dev/null 2>&1");
    system("rm generates > /dev/null 2>&1");
    #endif
    cout << "\u001b[32m+++++++++++++++++++++++++++++++++++++++++++++\u001b[0m\n";
    cout << "\u001b[32m+               Clean Success               +\u001b[0m\n";
    cout << "\u001b[32m+ Now remove the clean application manually +\u001b[0m\n";
    cout << "\u001b[32m+++++++++++++++++++++++++++++++++++++++++++++\u001b[0m\n";
    return 0;
}