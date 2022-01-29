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
bool canBlock(std::vector<std::string>&);

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
        int pos = app.find(':');
        while (pos != std::string::npos) {
            app.insert(app.begin() + pos, ' ');
            app.erase(app.begin() + pos + 1);
            pos = app.find(':');
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
                if(!s.size())
                    continue;
                else
                    if (s.size() > 3 && s.substr(s.size() - 4) == ".app")
                        s = s.substr(0, s.size() - 4);
                std::cout << "Blocking " << s;
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
    std::string cmd = "defaults read \"" + app;
    cmd += "/Contents/Info\" CFBundleExecutable";
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
        std::cout << "Does any of these look like the app you want to block? Type, in full, every path listed below that you think " <<
            "might lead to that app. Press the return key exactly once after each entry, except for the last, after which you should" <<
            " press the return key twice." << '\n' << '\n';
        for (int i = 0; i < 4; i++) {
            if(srchRes[i].size())
	            std::cout << "NAMES DRAWN FROM " << appDir[i] << '\n' << '\n' << srchRes[i] << '\n';
        }
        std::string entry = "";
        std::array<std::vector<std::pair<std::string,int> >::iterator,4> it;
        std::array<bool,4> matched;
        bool inputEmpty = false;
        while (!inputEmpty) {
            std::getline(std::cin, entry);
            if (entry.size() <= 3 || entry.substr(entry.size() - 4) != ".app")
               entry+= ".app";
            if (entry.size() == 4 || entry == "\n.app" || entry == "\n.app\n")
                return exes;

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

bool canBlock(std::vector<std::string>& nameVec) {
    std::string name = nameVec[0];
    std::string appName = name, bndlName = name;

    if (name.substr(name.size() - 4) != ".app")
        appName += ".app";
    appName.insert(appName.size() - 4, "\\");

    if (name.substr(name.size() - 4) == ".app")
        bndlName = name.substr(0, name.size() - 4);

    bool deeperToGo = true;
    std::string beg = "find '";
    std::string appEnd = " | grep '" + appName + "$'";
    std::string bndlEnd = bndlName + "' -regex '.*\\.app$' | grep -v '\\.app/'";

    for (int d = 1; deeperToGo; d++) {
        std::array<std::string,2> depth = { " -maxdepth " + std::to_string(d), 
            " -maxdepth " + std::to_string(d + 1) };

        std::array<std::string,8> appCmd;
        std::array<std::string,8> cmd;
        std::array<std::string,4> appSrchRes;
        std::array<bool,4> numResGrows;
        for (int i = 0; i < 4; i++) {
            appCmd[i] = beg + appDir[i] + '\'' + depth[0] + appEnd;
            appSrchRes[i] = run(appCmd[i]);
            if(appSrchRes[i].size()) { 
                nameVec.clear();
                std::vector<std::string> newNames = getExes(appSrchRes);
                for (std::string exe : newNames)
                    nameVec.push_back(exe);
                return true;
            }

            int ind = (i + 1) * 2 - 1;
            cmd[ind - 1] = beg + appDir[i] + depth[1];
            cmd[ind] = beg + appDir[i] + depth[0];
            numResGrows[i] = cmd[ind - 1].size() > cmd[ind].size();
        }
        deeperToGo = numResGrows[0] || numResGrows[1] || numResGrows[2] || numResGrows[3];
    }


    std::array<std::string,4> bndlSrchRes;
    std::array<std::string,8> bndlCmd;
    for (int i = 0; i < 4; i++) {
        bndlCmd[i] = beg + appDir[i] + '/' + bndlEnd;
        bndlSrchRes[i] = run(bndlCmd[i]);
        if (bndlSrchRes[i].size()) {
            nameVec.clear();
            std::vector<std::string> newNames = getExes(bndlSrchRes);
            for (std::string exe : newNames)
                nameVec.push_back(exe);
            return true;
        }
    }

    return false;
}

int main(int argc, char* argv[]) {
    std::string spawnExecution(argv[0]);
    std::string spawn = spawnExecution.substr(2);
    spawn = spawn.substr(6);
    std::string baseCmd = "ps -ce | pgrep '[[:space:]]" + spawn + "$' | awk '{print $1}'";
    std::string cmd = "(" + baseCmd + ") > .proc/postSpawnPIDs.txt";
    system(cmd.c_str());
    
    auto apps = getApps(argc, argv);
    std::unordered_map<std::string,std::vector<std::string> > correct;
    for (auto v : apps) {
        std::string entry = v[0];
        while (v.size() ==  1 && !canBlock(v)) {
            std::cout << "The name \"" + v[0] + "\" does not match any app on your device. Try entering a different" <<
                " name. Hint: If you look in your /Applications (the most likely location), /System/Applications, " <<
                "/System/Library/CoreServices, and ~/Downloads folders, you should be able to find the exact name of" << 
                " the app you're attempting to block." << '\n' << '\n';
            std::getline(std::cin, v[0]);
        }
        correct[entry] = v;
    }
    for (auto mapIt : correct) {
        std::vector<std::string> key { mapIt.first };
        auto vecIt = find(apps.begin(), apps.end(), key);
        if (vecIt != apps.end())
            apps[vecIt - apps.begin()] = correct[mapIt.first];
    }
    block(apps);
}