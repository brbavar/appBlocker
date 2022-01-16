#include <fstream>
#include <string>
#include <algorithm>   // std::find 
#include <vector>

std::vector<std::string> getArgs(int, char* []);
std::vector<std::string> saveApps(std::vector<std::string>, std::vector<std::string>);
std::vector<std::string> getSavedApps();

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
    std::ofstream log("blocklist.txt", std::ios_base::app);
    if (log.is_open())
        for (std::string s : newApps) {
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
    std::ifstream log("blocklist.txt");
    if (log.is_open())
        while (!log.eof()) {
            std::getline(log, savedApp);
            if(savedApp != "" && savedApp != "\n")
                savedApps.push_back(savedApp);
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
    auto savedApps = getSavedApps();
    auto apps = saveApps(newApps, savedApps);
    block(apps);
}