#include <iostream>
#include <sys/stat.h>
using namespace std;
inline bool exist (const std::string& name) {
    struct stat buffer;   
    return (stat (name.c_str(), &buffer) == 0); 
}
int main(int argc,char* argv[]) {
    for(int i = 1 ; i < argc ; i++) {
        string fileName = argv[1];
        string cmd;
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
        cout << "- Compiling debug object file\n";
        cmd = "g++ -Iinclude -g -w -c " + fileName + ".cpp";
        system(cmd.c_str());
        if(!exist("test.o")) {
            cout << "\u001b[31m- Build Failed - Debug\u001b[0m\n";
            cout << "\u001b[31m- Reason: Compilation Error\u001b[0m\n";
            exit(0);
        }
        cout << "- Linking object debug file\n";
        #ifdef _WIN32
        cmd = "g++ include/sha256.cpp " + fileName + ".o -o " + fileName + "_debug -Llib -lcurl -lpdcurses";
        #else
        cmd = "g++ include/sha256.cpp " + fileName + ".o -o " + fileName + "_debug -lcurl -lncurses";
        #endif
        system(cmd.c_str());
        #ifdef _WIN32
        if(!exist("test_debug.exe")) {
            cout << "\u001b[31m- Build Failed - Debug\u001b[0m\n";
            cout << "\u001b[31m- Reason: Linker Error\u001b[0m\n";
            exit(0);
        }
        #else
        if(!exist("test_debug")) {
            cout << "\u001b[31m- Build Failed - Debug\u001b[0m\n";
            cout << "\u001b[31m- Reason: Linker Error\u001b[0m\n";
            exit(0);
        }
        #endif
        cout << "- Deleting object debug file\n";
        #ifdef _WIN32
        cmd = "del " + fileName + ".o";
        #else
        cmd = "rm " + fileName + ".o";
        #endif
        system(cmd.c_str());
        #ifdef _WIN32
        cout << "- Moving the debug package to \"bin\" directory\n";
        cmd = "move " + fileName + "_debug.exe bin > NUL";
        system(cmd.c_str());
        #endif
        cout << "\u001b[32m- Build Success - Debug\u001b[0m\n";
        cout << "- Compiling release object file\n";
        cmd = "g++ -Iinclude -w -c " + fileName + ".cpp";
        system(cmd.c_str());
        if(!exist("test.o")) {
            cout << "\u001b[31m- Build Failed - Release\u001b[0m\n";
            cout << "\u001b[31m- Reason: Compilation Error\u001b[0m\n";
            exit(0);
        }
        cout << "- Linking object release file\n";
        #ifdef _WIN32
        cmd = "g++ include/sha256.cpp " + fileName + ".o -o " + fileName + " -Llib -lcurl -lpdcurses";
        #else
        cmd = "g++ include/sha256.cpp " + fileName + ".o -o " + fileName + " -lcurl -lncurses";
        #endif
        system(cmd.c_str());
        #ifdef _WIN32
        if(!exist("test.exe")) {
            cout << "\u001b[31m- Build Failed - Release\u001b[0m\n";
            cout << "\u001b[31m- Reason: Linker Error\u001b[0m\n";
            exit(0);
        }
        #else
        if(!exist("test")) {
            cout << "\u001b[31m- Build Failed - Release\u001b[0m\n";
            cout << "\u001b[31m- Reason: Linker Error\u001b[0m\n";
            exit(0);
        }
        #endif
        cout << "- Deleting object release file\n";
        #ifdef _WIN32
        cmd = "del " + fileName + ".o";
        #else
        cmd = "rm " + fileName + ".o";
        #endif
        system(cmd.c_str());
        #ifdef _WIN32
        cout << "- Moving the release package to \"bin\" directory\n";
        cmd = "move " + fileName + ".exe bin > NUL";
        system(cmd.c_str());
        #endif
        cout << "\u001b[32m- Build Success - Release\u001b[0m\n";
    }
    return 0;
}