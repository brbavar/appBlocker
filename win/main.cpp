#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <windows.h>


std::string run(std::string, int);
std::vector<std::string> getApps();

std::string run(std::string cmd, int size = 100) {
    std::string output = "";
    char buf[size];
    FILE* stream = popen(cmd.c_str(), "r");
    if (stream)
        while (!feof(stream))
            if (fgets(buf, size, stream) != NULL)
                output += buf;
    pclose(stream);
    return output;
}

std::vector<std::string> getApps() {
    std::vector<std::string> apps;
    apps.reserve(50);
    int i = 0;
    bool inputEmpty = false;
    while (!inputEmpty) {
        apps.push_back("");
        inputEmpty = true;
        std::getline(std::cin, apps[i]);
        for (int j = 0; j < apps[i].size(); j++) {
            char c = apps[i][j];
            if ((int)c > 32 && (int)c != 127 && (int)c != 255)
                inputEmpty = false;
            else {
                if (c == '\n')
                    break;
                apps[i][j] = ':';
            }
        }
        i++;
    }
    return apps;
}


int main() {
    std::cout << '\n' << "Let's start blocking some apps. Type the name" << '\n' << "of the app you want to block, and then" << '\n' << "press " <<
        "the enter/return key exactly once." << '\n' << "Repeat this process for every app you want" << '\n' << "to block. Once you " <<
        "have entered the final" << '\n' << "app's name, press enter/return twice to start" << '\n' << "blocking all the apps you've " <<
        "listed." << '\n' << '\n'; /* (Warning: If you have Windows Terminal, Command Prompt,\nWindows Powershell, Git Bash, or "
        << "any other command\nshell open, don't submit your list of apps\nuntil you're ready for that to be closed.)" << '\n' << '\n'; */
    auto apps = getApps();

    HWND console = GetConsoleWindow();
    ShowWindow(console, SW_HIDE);
    
    system("taskkill /f /im appBlocker.exe");    // Kill any extant instance of appBlocker proc
    
    std::string uid = run("echo %username%");
    uid = uid.substr(0, uid.size() - 1);         // Remove terminating newline character
    std::string startupPath = "\"\\Users\\" + uid + "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu";
    startupPath += "\\Programs\\Startup";
    std::string cmd = "move block.cpp " + startupPath + "\" && g++ -std=c++11 " + startupPath + "\\block.cpp\" -o " + startupPath + "\\appBlocker.exe\"";
    cmd += " && move " + startupPath + "\\block.cpp\" . && " + startupPath + "\\appBlocker.exe\"";
    for (std::string s : apps)
        cmd += " " + s;
    system(cmd.c_str());
}
