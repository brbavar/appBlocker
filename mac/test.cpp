#include <stdlib.h>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>

// HEADERS BELOW ARE PURELY FOR DEBUGGING
#include <iostream>


std::string run(std::string, int);
std::string getPlistPath(std::string, std::string);
std::vector<std::string> block(std::vector<std::string>);

std::string run(std::string cmd, int size = 100) {
    std::string output = "";
    char buf[size];
    FILE* stream = popen(cmd.c_str(), "r");
    if (stream) {
        std::cout << "The command \"" << cmd << "\" generates a stream" << '\n';
        while (!feof(stream)) {
            // if (fgets(buf, size, stream) != NULL) {
                std::cout << "fgets does" << (fgets(buf, size, stream) == NULL ? " " : " NOT ") << "return NULL" << '\n';
                output += buf;
                std::string strBuf(buf);
            std::cout << "std::string version of buf = " << strBuf << '\n' << "So output + buf = " << output << '\n';
            // }
        }
        std::ofstream outFile("noNullTest.txt");
        outFile << output;
        outFile.close();
    }
    pclose(stream);
    return output;
}

std::string getPlistPath(std::string app, std::string dir) {
    std::string xtnsnMtchs = run("ls " + dir + " | grep '^" + app + ".app$'");
    if(xtnsnMtchs.size())
        app += ".app";
    std::string bndlid = run("defaults read " + dir + "/'" + app + "'/Contents/Info CFBundleIdentifier");
    std::cout << "1 bndlid = " << bndlid << '\n'; //
    if(bndlid[bndlid.size() - 1] == '\n') {
        bndlid = bndlid.substr(0, bndlid.size() - 1);    // Remove trailing newline character
        std::cout << "2 bndlid = " << bndlid << '\n'; //
    }
    if(bndlid.size() > 14) {
        std::cout << "3 bndlid = " << bndlid << '\n';
        if(bndlid.substr(bndlid.size() - 14) == "does not exist") {
            // std::cout << "Contents does NOT exist, bndlid = " << bndlid << '\n';
            std::cout << "4 bndlid = " << bndlid << '\n';
            return "";
        }
    }
    std::cout << "Contents DOES exist, bndlid = " << bndlid << '\n'; //
    std::cout << "the size of bndlid = " << bndlid.size() << '\n'; //
    
    return run("launchctl dumpstate | grep -iA2 '" + bndlid + " = {' | awk '/path =/ {print $3}'");
}

std::vector<std::string> block(std::vector<std::string> apps) {
    std::vector<std::string> stubbornApps;
    std::string cmd;
    for(std::string app : apps) {
        std::string path = getPlistPath(app, "/Applications");
        if(path.size() == 0) {
            std::cout << "app = " << app << '\n'; //
            std::cout << "no path found in /Applications" << '\n'; //
            path = getPlistPath(app, "/System/Library/CoreServices");
            if(path.size() == 0) {
                std::cout << "no path found in /System/Library/CoreServices" << '\n'; //
                stubbornApps.push_back(app);
                return stubbornApps; //
                continue;
            }
            return stubbornApps; //
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
            else {
                std::cout << app << " is stubborn!";
                stubbornApps.push_back(app);
            }
        return stubbornApps; //
    }
    return stubbornApps;
}


int main(int argc, char* argv[]) {
  /* while (1)
    for (int i = 1; i < argc; i++) {
      std::string app(argv[i]);
      std::string cmd = "killall '" + app + "'";
      system(cmd.c_str());
    } */
    
    std::vector<std::string> apps { "Kindle" };
    block(apps);
}
