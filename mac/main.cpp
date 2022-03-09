#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <string>
#include <array>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>   // std::find

/*  */
const std::array<std::string,4> appDir = { "/Applications", "/System/Applications", 
    "/System/Library/CoreServices", "~/Downloads" };

std::string run(std::string, int);
std::vector<std::string> getApps();
std::vector<std::string> getListItems(std::string);
std::string readPlist(std::string);
std::vector<std::string> getNames(std::string, std::string);
bool canBlock(std::string, std::vector<std::string>&, std::vector<std::string>);
std::vector<std::string> getSavedNames();
std::vector<std::string> saveNames(std::vector<std::string>, std::vector<std::string>);
void spawnProc(std::vector<std::string>, std::vector<std::string>);

// Return the output of a shell command, namely cmd.
std::string run(std::string cmd, int size = 100) {
    std::string output = "";
    char buf[size];
    FILE* stream = popen(cmd.c_str(), "r");
    if (stream)
        while (!feof(stream))
            if (fgets(buf, size, stream) != NULL)
                output += buf;
    pclose(stream);
    if (output[output.size() - 1] == '\n' || output[output.size() - 1] == '\r')
        output.erase(output.size() - 1, 1);   // Remove trailing newline/carriage return
    return output;
}

/*  */
std::vector<std::string> getApps() {
    std::vector<std::string> apps;
    apps.reserve(50);
    bool inputEmpty = false;
    while (!inputEmpty) {
        inputEmpty = true;
        std::string app = "";
        std::getline(std::cin, app);
        for (int i = 0; i < app.size(); i++) {
            char c = app[i];
            if (c == '\n')
                break;
            if ((int)c > 32 && (int)c != 127 && (int)c != 255)
                inputEmpty = false;
        }
        if (!inputEmpty)
            apps.push_back(app);
    }
    return apps;
}

/* Extract from a string all lines separated by newline characters, and return vector 
   of those lines.*/
std::vector<std::string> getListItems(std::string list) {
    std::vector<std::string> items;
    std::stringstream listream(list);
    std::string line;
    while (std::getline(listream, line, '\n'))
        items.push_back(line);
    return items;
}

/* Read property list file for this app. Return name of bundle or executable, depending on 
   whether the string app - the app name entered by the user - contains spaces. */
std::string readPlist(std::string app, bool nameHasSpaces) {
    std::string cmd = "defaults read \"" + app;
    cmd += "/Contents/Info\" CFBundle";
    cmd += nameHasSpaces ? "Executable" : "Name";
    std::string name = run(cmd);
    return name;
}

/* Get names of executables (for app entries with spaces) or of bundles (for app entries without spaces)
   from blocklist. */
std::vector<std::string> getNames(std::string middle, std::string end, bool nameHasSpaces) {
    std::string cmd = "find " + middle + end;
    std::string srchRes = run(cmd);
    auto apps = getListItems(srchRes);
    std::vector<std::string> names;
    if(apps.size()) {
        if (apps.size() == 1)
            for (std::string s : apps)
                return { readPlist(s, nameHasSpaces) };
        else {
            std::cout << '\n' << "Does any of these look like the app you want to block? Type, in full, every path listed below that you think " <<
                "might lead to that app. Press the return key exactly once after each entry, except for the last, after which you should" <<
                " press the return key twice." << '\n' << '\n' << srchRes << '\n';

            std::string entry = "";
            bool matchFound = false;
            while (!matchFound) {
                std::getline(std::cin, entry);
                if (entry.size() <= 3 || entry.substr(entry.size() - 4) != ".app")
                    entry += ".app";
                if (entry.size() == 4 || !entry.size() || entry == "\n.app" || entry == "\n.app\n")
                    return names;

                auto it = std::find(apps.begin(), apps.end(), entry);
                matchFound = it != apps.end();

                names.push_back(readPlist(entry, nameHasSpaces));
            }
        }
    }
    return names;
}

/*  */
bool canBlock(std::string name, std::vector<std::string>& names, std::vector<std::string> allNames) {    
    if (find(allNames.begin(), allNames.end(), name) != allNames.end()) {
        names.push_back(name);
        return true;
    }

    std::string appName = name, bndlName = name;

    if (name.size() <= 3 || name.substr(name.size() - 4) != ".app") {
        appName += ".app";
        appName.insert(appName.begin() + appName.size() - 4, '\\');
    }
    if (name.size() > 3 && name.substr(name.size() - 4) == ".app")
        bndlName = name.substr(0, name.size() - 4);

    bool deeperToGo = true, nameHasSpaces = appName.find(' ') != std::string::npos;
    std::string end = " 2>/dev/null | grep '" + appName + "$'";
    std::string middle;

    std::cout << "Loading..." << '\n';
    for (int d = 1; deeperToGo; d++) {
        std::array<std::string,2> depth = { " -maxdepth " + std::to_string(d), 
            " -maxdepth " + std::to_string(d + 1) };

        std::array<std::string,8> cmd;
        std::array<bool,4> numResGrows;
        for (int i = 0; i < 4; i++) {
            middle = appDir[i] + depth[0];
            // std::cout << "middle = " << middle << '\n';
            std::vector<std::string> newNames = getNames(middle, end, nameHasSpaces);
            for (std::string s : newNames)
                names.push_back(s);
            if (newNames.size())
                return true;

            int ind = (i + 1) * 2 - 1;
            cmd[ind - 1] = "find " + appDir[i] + depth[1] + " 2>/dev/null";
            cmd[ind] = "find " + middle + " 2>/dev/null";
            numResGrows[i] = run(cmd[ind - 1]).size() > run(cmd[ind]).size();
        }
        deeperToGo = numResGrows[0] || numResGrows[1] || numResGrows[2] || numResGrows[3];
    }

    end = bndlName + "' -regex '.*\\.app$' 2>/dev/null | grep -v '\\.app/'";
    nameHasSpaces = bndlName.find(' ') != std::string::npos;

    for (int i = 0; i < 4; i++) {
        middle = appDir[i] + '/';
        std::vector<std::string> newNames = getNames(middle, end, nameHasSpaces);
        for (std::string s : newNames)
            names.push_back(s);
        if (newNames.size())
            return true;
    }

    return false;
}

// Pull all names from list of apps to block. Then return a vector of all those names.
std::vector<std::string> getSavedNames() {
    std::vector<std::string> savedNames;
    system("chmod 400 .blocklist.txt 2>/dev/null");
    std::ifstream log(".blocklist.txt");
    if (log.is_open())
        while (!log.eof()) {
            std::string savedName;
            std::getline(log, savedName);
            if (savedName != "" && savedName != "\n")
                savedNames.push_back(savedName);
        }
    log.close();
    system("chmod 000 .blocklist.txt 2>/dev/null");
    return savedNames;
}

// Add user-specified names to list of apps to block. Then return a vector of all apps in list.
std::vector<std::string> saveNames(std::vector<std::string> newNames, std::vector<std::string> savedNames) {
    std::vector<std::string> apps;
    apps.reserve(newNames.size() + savedNames.size());
    system("chmod 200 .blocklist.txt 2>/dev/null");
    std::ofstream log(".blocklist.txt", std::ios_base::app);
    if (log.is_open())
        for (std::string s : newNames) {
            if (find(savedNames.begin(), savedNames.end(), s) == savedNames.end()) {
                log << s << '\n';
                apps.push_back(s);
            }
        }
    log.close();
    system("chmod 000 .blocklist.txt 2>/dev/null");
    for (std::string s : savedNames)
        apps.push_back(s);
    return apps;
}

/*  */
void spawnProc(std::vector<std::string> procNames, std::vector<std::string> names) {    
    std::string cmd = "g++ -std=c++11 block.cpp -o block && ./block";/* && mv block /usr/local/libexec && /usr/libexec/PlistBuddy -c ";
    cmd += "'add Label string block' -c 'add AbandonProcessGroup bool true' -c 'add KeepAlive bool true' -c 'add RunAtLoad bool true' "; 
    cmd += "-c 'add ProgramArguments array' -c 'add ProgramArguments: string /usr/local/libexec/block' block.plist && ";
    cmd += "chmod 644 block.plist && chown `whoami` block.plist && ";
    cmd += "mv block.plist ~/Library/LaunchAgents && launchctl bootstrap gui/`id -u` ~/Library/LaunchAgents/block.plist && ";
    cmd += "/usr/local/libexec/block"; */
    for (std::string s : names) {
        int pos = s.find(' ');
        while (pos != std::string::npos) {
            s.insert(s.begin() + pos, ':');
            s.erase(s.begin() + pos + 1);
            pos = s.find(' ');
        }
        cmd += " " + s;
    }
    system(cmd.c_str());
}

int main() {
    std::string output = "Let's start blocking some apps. Type the name of the app you want to block, and then ";
    output += "press the enter/return key exactly once. Repeat this process for every app you want to block. ";
    output += "Once you have entered the final app's name, press enter/return twice to start blocking all the ";
    output += "apps you've listed.";
    std::string cmd = "echo \"" + output;
    cmd += "\" | fold -s";
    std::cout << '\n';
    system(cmd.c_str());
    std::cout << '\n';
    auto newApps = getApps();

    std::cout << "Loading..." << '\n';
    std::vector<std::string> allNames;
    for (int i = 0; i < 4; i++) {
        std::string find = "find " + appDir[i];
        find += " -maxdepth 3 -regex '" + appDir[i];
        find += "/.*\\.app/Contents/Info.plist$'";
        auto plists = getListItems(run(find));
        for (std::string s : plists)
            allNames.push_back(run("defaults read '" + s + "' CFBundleExecutable 2>/dev/null"));
    }

    int size = newApps.size();
    std::vector<std::string> newNames;
    for (int i = 0; i < size; i++) {
        std::string name = newApps[i];
        while (!canBlock(name, newNames, allNames)) {
            std::cout << '\n' << "The name \"" << name << "\" does not match any app on your device. Try entering a different" <<
                " name. Hint: If you look in your /Applications (the most likely location), /System/Applications, " <<
                "/System/Library/CoreServices, and ~/Downloads folders, you should be able to find the exact name of" << 
                " the app you're attempting to block." << '\n' << '\n';
            std::getline(std::cin, name);
            newApps.push_back(name);
        }
    }

    auto savedNames = getSavedNames();
    auto names = saveNames(newNames, savedNames);

    std::string procList = run("ps -ce | grep -v grep | awk '{print $4}'");
    auto procNames = getListItems(procList);
    spawnProc(procNames, names);
}