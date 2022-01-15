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
        for(char c : apps[i])
            if((int)c > 32 && (int)c != 127 && (int)c != 255) {
                inputEmpty = false;
                break;
            }
        if(inputEmpty)
            break;
        i++;
    }

    return apps;
}

int main() {
    std::cout << "Let's start blocking some apps. Type the name of the app you want to block, and then press "
        << "the enter/return key exactly once. Repeat this process for every app you want to block. Once you "
        << "have entered the final app's name, press enter/return twice to start blocking all the apps you've "
        << "listed." << std::endl;
    auto apps = getApps();
    for(std::string s : apps)
        std::cout << s << std::endl;
}