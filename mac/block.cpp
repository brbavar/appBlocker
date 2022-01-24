#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <algorithm>

const std::array<std::string,4> appDir = { "/Applications", "/System/Applications", 
    "/System/Library/CoreServices", "~/Downloads" };

std::string run(std::string, int);
std::vector<std::vector<std::string> > getApps(int, char* []);
void block(std::vector<std::vector<std::string> >);
std::vector<std::string> getListItems(std::string);
std::string readPlist(std::pair<std::string,int>);
std::vector<std::string> getExes(std::array<std::string,4>);
bool isBndl(std::vector<std::string>&);
bool noApp(std::string);

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

std::vector<std::vector<std::string> > getApps(int argc, char* argv[]) {
    std::vector<std::vector<std::string> > apps;
    /* Skip first command-line arg, which is not name of app
        we were asked to block, but rather of spawned proc */
    for (int i = 1; i < argc; i++) {
        std::string app(argv[i]);
        if (app.find(':') != std::string::npos) {
            app = "";
            for (int j = 0; j < argc; j++) {
                char c = argv[i][j];
                if (c == ':')
                    c = ' ';
                app += c;
            }
        }
        std::vector<std::string> v{app};
        apps.push_back(v);
    }
    return apps;
}

void block(std::vector<std::vector<std::string> > apps) {
    while (1)
        for (auto v : apps)
            for (std::string s : v) {
                std::string cmd = "killall " + s;
                system(cmd.c_str());
            }
}

std::vector<std::string> getListItems(std::string list) {
    std::vector<std::string> items;
    std::string item = "";
    for (char c : list)
        if (c != '\n')
            item += c;
        else {
            items.push_back(item);
            item = "";
        }
    return items;
}

std::string readPlist(std::pair<std::string,int> match) {
    std::string app = match.first;
    std::string cmd = "defaults read \"/" + appDir[match.second];
    cmd += app + "/Content/Info\" CFBundleExecutable";
    std::string exe = run(cmd);
    return exe;
}

std::vector<std::string> getExes(std::array<std::string,4> srchRes) {
    std::vector<std::pair<std::string,int> > matches;
    std::vector<std::string> exes;
    for (int i = 0; i < srchRes.size(); i++)
        for (std::string item : getListItems(srchRes[i])) {
            std::pair<std::string,int> match { item, i };
            matches.push_back(match);
        }
    if (matches.size() <= 1)
        for (auto m : matches)
            return { readPlist(m) };
    else {
        std::cout << "Does any of these look like the app you want to block? Type in every name listed below that you think " <<
            "might be the one. Press the return key exactly once after each entry, except for the last, after which you should" <<
            " press the return key twice." << '\n' << '\n';
        for (int i = 0; i < 4; i++)
	        std::cout << "Drawn from " << appDir[i] << '\n' << srchRes[i] << '\n';
        std::string entry = "";
        std::array<std::vector<std::pair<std::string,int> >::iterator,4> it;
        std::array<bool,4> matched;
        bool inputEmpty = false;
        while (!inputEmpty) {
            std::getline(std::cin, entry);
            if (e.size() <= 3 || e.substr(e.size() - 4) != ".app")
                e += ".app";
            if (e.size() == 4 || e == "\n.app" || e == "\n.app\n") {
                inputEmpty = true;
                break;
            }

            std::pair<std::string,int> m;
            m.first = entry;
            for(int i = 0; i < 4; i++) {
                m.second = i;
                it[i] = std::find(matches.begin(), matches.end(), m);
                matched[i] = it[i] != matches.end();
                if(matched[i])
                    break;
            }
            exes.push_back(readPlist(m));
        }
    }
    return exes;
}

bool isBndl(std::vector<std::string>& name) {
    bool confirmed = false, deeperToGo = true;
    
    for (int d = 1; !confirmed && deeperToGo; d++) {
        std::string beg = "find ";
        std::array<std::string,2> depth = { "-maxdepth " + std::to_string(d), 
            "-maxdepth " + std::to_string(d + 1) };
        std::string end = " | grep '" + name[0] + "$' | grep -v \.app";

        std::array<std::string,8> cmd;
        std::array<std::string,4> srchRes;
        std::array<bool,4> numResGrows;
        for (int i = 0; i < 4; i++) {
            std::string cmd[i] = beg + appDir[i] + depth[0] + end;
            srchRes[i] = run(cmd[i]);
            int ind = (i + 1) * 2 - 1;
            cmd[ind - 1] = beg + appDir[i] + depth[1];
            cmd[ind] = beg + appDir[i] + depth[0];
            numResGrows[i] = cmd[ind - 1].size() > cmd[ind].size();
        }

        confirmed = srchRes[0].size() || srchRes[1].size() || srchRes[2].size() || srchRes[3].size();
        if (confirmed) {
            name.clear();
            std::vector<std::string> newNames = getExes(srchRes);
            for (std::string exe : newNames)
                name.push_back(exe);
            break;
        }

        deeperToGo = numResGrows[0] || numResGrows[1] || numResGrows[2] || numResGrows[3];
    }
        
    return confirmed;
}

bool noApp(std::string name) {
    if (name.substr(name.size() - 4) != ".app")
        name += ".app";
    std::string nameTmp = name;
    name.insert(name.size() - 4, "\\");

    std::string beg = "ls -R ";
    std::string end = " | grep '^" + name + "\.app$'";

    std::array<std::string,4> cmd;
    std::array<std::string,4> srchRes;
    std::array<bool,4> foundIn;
    for (int i = 0; i < 4; i++) {
        cmd[i] = beg + appDir[i] + end;
        srchRes[i] = run(cmd[i]);
        foundIn[i] = find(srchRes[i].begin(), srchRes[i].end(), nameTmp) != srchRes[i].end();
    }

    return foundIn[0] || foundIn[1] || foundIn[2] || foundIn[3];
}

int main(int argc, char* argv[]) {
    std::string spawnExecution(argv[0]);
    std::string spawn = spawnExecution.substr(2);
    spawn = spawn.substr(6);
    std::string baseCmd = "ps -ce | pgrep '[[:space:]]" + spawn + "$' | awk '{print $1}'";
    std::string cmd = "(" + baseCmd + ") > .proc/postSpawnPIDs.txt";
    system(cmd.c_str());
    
    std::vector<std::vector<std::string> > apps = getApps(argc, argv);
    std::unordered_map<std::string,std::vector<std::string> > correct;
    for (auto v : apps) {
        if (noApp(v[0])) {
            std::cout << "The name \"" + v[0] + "\" does not match any app on your device. ";
            std::vector<std::string> b{v[0]};
            if (isBndl(b)) {
                correct[v[0]] = b;
                break;
            }
            std::cout << "Try entering a different name. Hint: If you look in your /Applications (the most likely location), " <<
                "/System/Applications, /System/Library/CoreServices, and ~/Downloads folders, you should be able " <<
                "to find the exact name of the app you're attempting to block. Just be sure to leave " <<
                "off the extension \".app\" when you enter the name, if there is an extension." << '\n';
            std::string name;
            std::getline(std::cin, name);
            correct[v[0]] = {name};
        }
    }
    for (auto mapIt : correct) {
        std::vector<std::string> key{mapIt.first};
        auto vecIt = find(apps.begin(), apps.end(), key);
        if (vecIt != apps.end())
            apps[vecIt - apps.begin()] = correct[mapIt.first];
    }
    block(apps);
}