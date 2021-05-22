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
        system("cls");
        cout << "- Compiling debug object file\n";
        cmd = "g++ -Iinclude -g -w -c " + fileName + ".cpp";
        system(cmd.c_str());
        if(!exist("test.o")) {
            cout << "\u001b[31m- Build Failed - Debug\u001b[0m\n";
            cout << "\u001b[31m- Reason: Compilation Error\u001b[0m\n";
            exit(0);
        }
        cout << "- Linking object debug file\n";
        cmd = "g++ include/sha256.cpp " + fileName + ".o -o " + fileName + "_debug -Llib -lcurl -lpdcurses";
        system(cmd.c_str());
        if(!exist("test_debug.exe")) {
            cout << "\u001b[31m- Build Failed - Debug\u001b[0m\n";
            cout << "\u001b[31m- Reason: Linker Error\u001b[0m\n";
            exit(0);
        }
        cout << "- Deleting object debug file\n";
        cmd = "del " + fileName + ".o";
        system(cmd.c_str());
        cout << "- Moving the debug package to \"bin\" directory\n";
        cmd = "move " + fileName + "_debug.exe bin > NUL";
        system(cmd.c_str());
        cout << "\u001b[32m- Build Success - Debug\u001b[0m\n";
        // cmd = "cd bin && call " + fileName + ".exe";
        // system(cmd.c_str());
        cout << "- Compiling release object file\n";
        cmd = "g++ -Iinclude -w -c " + fileName + ".cpp";
        system(cmd.c_str());
        if(!exist("test.o")) {
            cout << "\u001b[31m- Build Failed - Release\u001b[0m\n";
            cout << "\u001b[31m- Reason: Compilation Error\u001b[0m\n";
            exit(0);
        }
        cout << "- Linking object release file\n";
        cmd = "g++ include/sha256.cpp " + fileName + ".o -o " + fileName + " -Llib -lcurl -lpdcurses";
        system(cmd.c_str());
        if(!exist("test.exe")) {
            cout << "\u001b[31m- Build Failed - Release\u001b[0m\n";
            cout << "\u001b[31m- Reason: Linker Error\u001b[0m\n";
            exit(0);
        }
        cout << "- Deleting object release file\n";
        cmd = "del " + fileName + ".o";
        system(cmd.c_str());
        cout << "- Moving the release package to \"bin\" directory\n";
        cmd = "move " + fileName + ".exe bin > NUL";
        system(cmd.c_str());
        cout << "\u001b[32m- Build Success - Release\u001b[0m";
    }
    return 0;
}