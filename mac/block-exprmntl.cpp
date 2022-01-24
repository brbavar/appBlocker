#include <fstream>
#include <cstdio>
#include <string>
#include <vector>

// HEADERS BELOW ARE PURELY FOR DEBUGGING
#include <iostream>


std::string run(std::string, int);
std::string getPlistPath(std::string, std::string);
std::vector<std::string> block(std::vector<std::string>);
void kill(std::vector<std::string>);

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

std::string getPlistPath(std::string app, std::string dir) {
    std::string xtnsnMtchs = run("ls " + dir + " | grep '^" + app + ".app$'");
    if(xtnsnMtchs.size())
        app += ".app";
    std::string bndlid = run("defaults read " + dir + "/'" + app + "'/Contents/Info CFBundleIdentifier");
    if(bndlid.substr(bndlid.size() - 14) == "does not exist")
        return "";
    return run("launchctl dumpstate | grep -iA2 '" + bndlid + " = {' | awk '/path =/ {print $3}'");
}

std::vector<std::string> block(std::vector<std::string> apps) {
    std::vector<std::string> stubbornApps;
    std::string cmd;
    for(std::string app : apps) {
        std::string path = getPlistPath(app, "/Applications");
        if(path.size() == 0) {
            path = getPlistPath(app, "/System/Library/CoreServices");
            if(path.size() == 0) {
                stubbornApps.push_back(app);
                continue;
            }
        }
                
        if (path.find("Agents") != std::string::npos) {
            cmd = "launchctl bootout gui/`id -u` " + path;
            system(cmd.c_str());
        }
        else
            if(path.find("Daemons") != std::string::npos) {
                cmd = "launchctl unload " + path;
                system(cmd.c_str());
            }
            else
                stubbornApps.push_back(app);
    }
    return stubbornApps;
}

void kill(std::vector<std::string> stubbornApps) {
    while (1)
        for (std::string s : stubbornApps)
            std::string cmd = "killall " + s;
}

int main(int argc, char* argv[]) {
    std::string spawnExec(argv[0]);
    std::string spawn = spawnExec.substr(2);
    spawn = spawn.substr(6);
    std::string baseCmd = "ps -ce | awk '/[[:space:]]\\" + spawn + "$/ {print $1}'";
    std::string cmd = "(" + baseCmd + ") > .proc/postSpawnPIDs.txt";
    system(cmd.c_str());
    
    std::vector<std::string> apps;
    /* Skip first command-line arg, which is not name of app
        we were asked to block, but rather of spawned proc */
    for (int i = 1; i < argc; i++) {
        std::string app(argv[i]);
        if (app.find(':') != std::string::npos) {
            app = "";
            for (int j = 0; j < argc; j++) {
                char c = argv[i][j];
                if(c == ':')
                    c = ' ';
                app += c;
            }
        }
        apps.push_back(app);
    }
    
    std::vector<std::string> stubbornApps = block(apps);
    kill(stubbornApps);
}