#include <fstream>
#include <cstdio>
#include <string>
#include <algorithm>   // std::find 
#include <vector>

/* HEADERS BELOW PURELY FOR DEBUGGING */
#include <iostream>


std::string run(std::string, int);
std::vector<std::string> getArgs(int, char* []);
std::vector<std::string> saveApps(std::vector<std::string>, std::vector<std::string>);
std::vector<std::string> getSavedApps();

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

std::vector<std::string> getArgs(int argc, char* argv[]) {
    std::vector<std::string> args;
    args.reserve(argc - 1);
    std::string arg = "";
    /* Skip first command-line arg, which is not the name of
           an app we were asked to block, but rather the string
           "appBlocker.exe" that runs this program */
    int i = 1;
    while (i < argc) {
        int j = 0;
        char c = argv[i][j];
        while (c != ' ') {
            if ((int)c > 32 && (int)c < 127) {
                if (c == ':')
                    c = ' ';
                arg += c;
            }
            else
                break;
            j++;
            c = argv[i][j];
        }
        if(arg != "")
            args.push_back(arg);
        arg = "";
        i++;
    }
    return args;
}

std::vector<std::string> saveApps(std::vector<std::string> newApps, std::vector<std::string> savedApps) {
    std::vector<std::string> apps;
    apps.reserve(newApps.size() + savedApps.size());

    std::string uid = run("echo %username%");
    uid = uid.substr(0, uid.size() - 1);         // Remove terminating newline character
    std::string startupPath = "\"C:\\Users\\" + uid + "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\";
    startupPath += "Programs\\Startup\"";

    // std::string

    /* std::string wd = run("cd");
    std::string cmd = "(for /f \"skip=7 tokens=5\" %a in('dir " + wd + "') do echo %a)";
    std::string thisDir = run(cmd);
    cmd = "(for /f \"skip=7 tokens=5\" %a in('dir " + startupPath + "') do echo %a)";
    std::string startupDir = run(cmd);
    
    if (wd == startupPath&&)
        std::string */
        
    std::ofstream log("blocklist.txt", std::ios_base::app);
    if (log.is_open()) {
        std::cout << "log opened in saveApps" << '\n';
        std::string cmd = "attrib +h " + startupPath + "\\blocklist.txt";
        system(cmd.c_str());
        for (std::string s : newApps)
            if (find(savedApps.begin(), savedApps.end(), s) == savedApps.end()) {
                log << s << '\n';
                apps.push_back(s);
            }
    }
    log.close();

    for (std::string s : savedApps)
        apps.push_back(s);
    return apps;
}

std::vector<std::string> getSavedApps() {
    std::vector<std::string> savedApps;
    std::string savedApp;

    /* std::string uid = run("echo %username%");
    uid = uid.substr(0, uid.size() - 1);         // Remove terminating newline character
    std::string logPath = "\\Users\\" + uid + "\\AppData\\Roaming\\Microsoft\\Windows\\\"Start Menu\"\\";
    logPath += "Programs\\Startup\\blocklist.txt"; */

    std::ifstream log("blocklist.txt");
    if (log.is_open()) {
        std::cout << "log opened in getSavedApps" << '\n';
        while (!log.eof()) {
            std::getline(log, savedApp);
            if (savedApp != "" && savedApp != "\n")
                savedApps.push_back(savedApp);
        }
    }
    log.close();

    return savedApps;
}

void block(std::vector<std::string> apps) {
    while (1) {
        std::string baseCmd = "taskkill /f /im ";
        std::string cmd;
        for (std::string s : apps) {
            cmd = baseCmd + s;
            system(cmd.c_str());
        }
    }
}


int main(int argc, char* argv[]) {
    system("taskkill /f /im blockStarter.exe");
    /* system("taskkill /f /im WindowsTerminal.exe");
    system("taskkill /f /im cmd.exe");
    system("taskkill /f /im powershell.exe");
    system("taskkill /f /im git-bash.exe");
    system("taskkill /f /im bash.exe"); */

    auto newApps = getArgs(argc, argv);
    std::cout << "New apps given below:" << '\n';
    for (std::string s : newApps)
        std::cout << s << '\n';
    auto savedApps = getSavedApps();
    std::cout << "Saved apps given below:" << '\n';
    for (std::string s : savedApps)
        std::cout << s << '\n';
    auto apps = saveApps(newApps, savedApps);
    std::cout << "Total set of apps given below:" << '\n';
    for (std::string s : apps)
        std::cout << s << '\n';
    block(apps);
}