#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

std::vector<std::string> getApps();

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
            if ((int)c > 32 && c != 127 && (int)c != 255)
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
    std::cout << "\nLet's start blocking some apps. Type the name\nof the app you want to block, and then\npress "
        << "the enter/return key exactly once.\nRepeat this process for every app you want\nto block. Once you "
        << "have entered the final\napp's name, press enter/return twice to start\nblocking all the apps you've "
        << "listed.\n" << std::endl; /* \n(Warning: If you have Windows Terminal, Command Prompt,\nWindows Powershell, Git Bash, or " 
        << "any other command\nshell open, don't submit your list of apps\nuntil you're ready for that to be closed.)\n" << std::endl; */
    auto apps = getApps();
    HWND console = GetConsoleWindow();
    ShowWindow(console, SW_HIDE);
    system("taskkill /f /im appBlocker.exe");   // Kill any extant instance of appBlocker proc
    std::string cmd = "g++ block.cpp -o appBlocker.exe && appBlocker.exe";
    for (std::string s : apps)
        cmd += " " + s;
    system(cmd.c_str());
}
