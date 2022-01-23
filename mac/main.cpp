#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>   // std::find


std::string run(std::string, int);
std::vector<std::string> addApps();
std::vector<std::string> saveApps(std::vector<std::string>, std::vector<std::string>);
std::vector<std::string> getSavedApps();
std::vector<std::string> getListItems(std::string);
void spawnProc(std::vector<std::string>, std::vector<std::string>);

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

std::vector<std::string> addApps() {
    std::vector<std::string> apps;
    apps.reserve(50);
    bool inputEmpty = false;
    while (!inputEmpty) {
        inputEmpty = true;
        std::string app = "";
        std::getline(std::cin, app);
        for (char c : app) {
            if ((int)c > 32 && (int)c != 127 && (int)c != 255)
                inputEmpty = false;
            else {
                if (c == '\n')
                    break;
                if (c == ' ')
                    c = ':';
            }
        }
        if(!inputEmpty)
            apps.push_back(app);
    }
    return apps;
}

std::vector<std::string> saveApps(std::vector<std::string> newApps, std::vector<std::string> savedApps) {
    std::vector<std::string> apps;
    apps.reserve(newApps.size() + savedApps.size());
    system("chmod 200 .blocklist.txt");
    std::ofstream log(".blocklist.txt", std::ios_base::app);
    if (log.is_open())
        for (std::string s : newApps) {
            if (find(savedApps.begin(), savedApps.end(), s) == savedApps.end()) {
                log << s << '\n';
                apps.push_back(s);
            }
        }
    log.close();
    system("chmod 000 .blocklist.txt");
    for (std::string s : savedApps)
        apps.push_back(s);
    return apps;
}

std::vector<std::string> getSavedApps() {
    std::vector<std::string> savedApps;
    system("chmod 400 .blocklist.txt");
    std::ifstream log(".blocklist.txt");
    if (log.is_open())
        while (!log.eof()) {
            std::string savedApp;
            std::getline(log, savedApp);
            if (savedApp != "" && savedApp != "\n")
                savedApps.push_back(savedApp);
        }
    log.close();
    system("chmod 000 .blocklist.txt");
    return savedApps;
}

std::vector<std::string> getListItems(std::string list) {
    std::vector<std::string> items;
    std::string item = "";
    for(char c : list)
        if(c != '\n')
            item += c;
        else {
            items.push_back(item);
            item = "";
        }
    return items;
}

void spawnProc(std::vector<std::string> procNames, std::vector<std::string> apps) {
    // Start by picking a random (running) process name
    unsigned int seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::default_random_engine re(seed);
    std::uniform_int_distribution<unsigned long> nameDistro(0, procNames.size() - 1);
    std::string proc = procNames[nameDistro(re)];
    
    // Save all PIDs associated with proc before spawn, so new PID can be identified later
    std::string baseCmd = "ps -ce | pgrep '[[:space:]]" + proc + "$' | awk '{print $1}'";
    std::string cmd = "mkdir .proc && (" + baseCmd + ") > .proc/preSpawnPIDs.txt";
    std::string pidList = run(baseCmd);
    std::vector<std::string> pids = getListItems(pidList);
    system(cmd.c_str());
    
    if (find(pids.begin(), pids.end(), "") != pids.end())
        spawnProc(procNames, apps);
    
    cmd = "g++ -std=c++11 block.cpp -o " + proc + " && mv " + proc + " .proc && ./.proc/" + proc;
    for (std::string s : apps)
        cmd += " " + s;
    system(cmd.c_str());
}


int main() {
    std::cout << '\n' << "Let's start blocking some apps. Type the name" << '\n' << "of the app you want to block, and then" << '\n' << "press " <<
        "the enter/return key exactly once." << '\n' << "Repeat this process for every app you want" << '\n' << "to block. Once you " <<
        "have entered the final" << '\n' << "app's name, press enter/return twice to start" << '\n' << "blocking all the apps you've " <<
        "listed." << '\n' << '\n'; /* (Warning: If you have Windows Terminal, Command Prompt,\nWindows Powershell, Git Bash, or "
        << "any other command\nshell open, don't submit your list of apps\nuntil you're ready for that to be closed.)" << '\n' << '\n'; */
    auto newApps = addApps();
    auto savedApps = getSavedApps();
    auto apps = saveApps(newApps, savedApps);
    
    /* HWND console = GetConsoleWindow();    // Need Mac-friendly alternative
    ShowWindow(console, SW_HIDE); */
    
    std::string procList = run("ps -ce | grep -v grep | awk '{print $4}'");
    auto procNames = getListItems(procList);
    spawnProc(procNames, apps);
}
