#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> getApps();

std::vector<std::string> getApps() {
    std::vector<std::string> apps;
    apps.reserve(50);
    int i = 0;
    bool inputEmpty = false;
    while(!inputEmpty) {
        apps.push_back("");
        inputEmpty = true;
        std::getline(std::cin, apps[i]);
        for(int j = 0; j < apps[i].size(); j++) {
            char c = apps[i][j];
            if((int)c > 32 && (int)c != 127 && (int)c != 255)
                inputEmpty = false;
            else
                apps[i][j] = ':';
        }
        i++;
    }

    return apps;
}

int main() {
    std::cout << "\nLet's start blocking some apps. Type the name \nof the app you want to block, and then \npress "
        << "the enter/return key exactly once. \nRepeat this process for every app you want \nto block. Once you "
        << "have entered the final \napp's name, press enter/return twice to start \nblocking all the apps you've "
        << "listed." << std::endl;
    auto apps = getApps();
    std::string cmd = "g++ block.cpp -o appBlocker.exe && appBlocker.exe";
    for(std::string s : apps)
        cmd += " " + s;
    system(cmd.c_str());
}