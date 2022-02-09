#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>   // std::find

std::vector<std::string> getApps(int, char* []);
void block(std::vector<std::string>);

std::vector<std::string> getApps(int argc, char* argv[]) {
    std::vector<std::string> apps;
    /* Skip first command-line arg, which is not name of app
        we were asked to block, but rather of spawned proc */
    for (int i = 1; i < argc; i++) {
        std::string app(argv[i]);
        int pos = app.find(':');
        while (pos != std::string::npos) {
            app.insert(app.begin() + pos, ' ');
            app.erase(app.begin() + pos + 1);
            pos = app.find(':');
        }
        apps.push_back(app);
    }
    return apps;
}

void block(std::vector<std::string> apps) {
    while (1)
        for (std::string s : apps) {
            if (!s.size())
                continue;
            else
                if (s.size() > 3 && s.substr(s.size() - 4) == ".app")
                    s = s.substr(0, s.size() - 4);
            std::string cmd = "killall " + s;
            cmd += " 2>/dev/null";
            std::cout << "The specified apps are now blocked." << '\n' << '\n';
            system(cmd.c_str());
        }
}

int main(int argc, char* argv[]) {
    auto apps = getApps(argc, argv);
    block(apps);
}