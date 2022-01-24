#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>


std::string run(std::string, int);
std::vector<std::vector<std::string> > getApps(int, char* []);
void block(std::vector<std::vector<std::string> >);
std::vector<std::string> getListItems(std::string);
std::string readPlist(std::pair<std::string,int>);
std::vector<std::string> getExes(std::vector<std::pair<std::string,int> >);
bool isBundle(std::string&);
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
    for(char c : list)
        if(c != '\n')
            item += c;
        else {
            items.push_back(item);
            item = "";
        }
    return items;
}

std::string readPlist(std::pair<std::string,int> match) {
    std::string app = match.first;
    int pathNum = match.second;
    std::string path = !pathNum ? "Applications/" : ( pathNum == 1 ? "System/Applications/" : "System/Libary/CoreServices/" );
    std::string cmd = "defaults read \"/" + path;
    cmd += app + "/Content/Info\" CFBundleExecutable";
    std::string exe = run(cmd);
    return exe;
}

std::vector<std::string> getExes(std::vector<std::string> srchRes) {
    std::vector<std::pair<std::string,int> > matches;
    std::vector<std::string> exes;
    for (int i = 0; i < srchRes.size(); i++)
        for(std::string item : getListItems(srchRes[i])) {
            std::pair<std::string,int> match{item, i};
            matches.push_back(match);
        }
    if (matches.size() <= 1)
        for (auto m : matches)
            return {readPlist(m)};
    else {
        std::cout << "Does any of these look like the app you want to block? Type in every name listed below that you think " <<
            "might be the one. Press the return key exactly once after each entry, except for the last, after which you should" <<
            " press the return key twice." << '\n' << '\n';
        for (int i = 0; i < srchRes.size(); i++) {
            std::cout << "Drawn from " << ( i == 0 ? "/Applications" : ( i == 1 ? "/System/Applications" : "/System/Library/CoreServices" ));
            std::cout << '\n' << srchRes[i] << '\n';
        }
        std::string entry = "";
        std::vector<std::pair<std::string,int> >::iterator it1, it2, it3;
        bool inputEmpty = false;
        while ( !inputEmpty || ( it1 == matches.end() && it2 == matches.end() && it3 == matches.end() )) {
            std::getline(std::cin, entry);
            if (entry.size() <= 3 || entry.substr(entry.size() - 4) != ".app")
                entry += ".app";
            if(entry.size() == 4 || entry == "\n.app" || entry == "\n.app\n") {
                inputEmpty = true;
                break;
            }
            std::pair<std::string,int> match1{entry,0}, match2{entry,1}, match3{entry,2};
            auto it1 = find(matches.begin(), matches.end(), match1), it2 = find(matches.begin(), matches.end(), match2),
                it3 = find(matches.begin(), matches.end(), match3);
            exes.push_back(readPlist( it1 != matches.end() ? match1 : ( it2 != matches.end() ? match2 : match3 )));
        }
    }
    return exes;
}

bool isBundle(std::vector<std::string>& name) {
    bool answer = false, deeperToGo = true;
    
    for (int d = 1; !answer && deeperToGo; d++) {
        std::string end = " -maxdepth " + std::to_string(d) + " | grep '" + name[0] + "$' | grep -v \.app";
        std::string cmd1 = "find /Applications" + end, cmd2 = "find /System/Applications" + end,
            cmd3 = "find /System/Library/CoreServices" + end;
        std::string srchRes1 = run(cmd1), srchRes2 = run(cmd2), srchRes3 = run(cmd3);
        answer = srchRes1.size() || srchRes2.size() || srchRes3.size();
        
        if (answer) {
            name.clear();
            std::vector<std::string> newNames = getExes({srchRes1, srchRes2, srchRes3});
            for (std::string exe : newNames)
                name.push_back(exe);
            break;
        }

        std::string beg1 = "find /Applications -maxdepth ", beg2 = "find /System/Applications -maxdepth ", 
            beg3 = "find /System/Library/CoreServices -maxdepth ";
        std::string end1 = std::to_string(d + 1), end2 = std::to_string(d);
        cmd1 = beg1 + end1, cmd2 = beg1 + end2, cmd3 = beg2 + end1;
        std::string cmd4 = beg2 + end2, cmd5 = beg3 + end1, cmd6 = beg3 + end2;
        deeperToGo = run(cmd1).size() > run(cmd2).size() || run(cmd3).size() > run(cmd4).size() || run(cmd5).size() > run(cmd6).size();
    }
        
    return answer;
}

bool noApp(std::string name) {
    if(name.substr(name.size() - 4) != ".app")
        name += ".app";
    std::string nameTmp = name;
    name.insert(name.size() - 4, "\\");
    std::string end = " | grep '^" + name + "\.app$'";
    std::string cmd1 = "ls -R /Applications" + end, cmd2 = "ls -R /System/Applications" + end,
        cmd3 = "ls -R /System/Library/CoreServices" + end;
    std::string srchRes1 = run(cmd1), srchRes2 = run(cmd2), srchRes3 = run(cmd3);
    return find(srchRes1.begin(), srchRes1.end(), nameTmp) != srchRes1.end() || 
        find(srchRes2.begin(), srchRes2.end(), nameTmp) != srchRes2.end() || 
        find(srchRes3.begin(), srchRes3.end(), nameTmp) != srchRes3.end();
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
            if (isBundle(b)) {
                correct[v[0]] = b;
                break;
            }
            std::cout << "Try entering a different name. Hint: If you look in your /Applications (the most likely location), " <<
                "/System/Applications, and /System/Library/CoreServices folders, you should be able " <<
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
