#include <string>

int main(int argc, char* argv[]) {
    while(1) {
        std::string cmd = "taskkill /F /im ";
        /* Skip first command-line arg, which is not the name of
           an app we were asked to block, but rather the string
           "appBlocker.exe" that runs this program */
        int i = 1;
        while(i < argc - 1) {
            char c = *argv[i];
            while(c != ' ') {
                if((int)c > 32 && (int)c != 127 && (int)c != 255) {
                    if(c == ':')
                        c = ' ';
                    cmd += c;
                }
                i++;
            }
            system(cmd.c_str());
            cmd = cmd.substr(0, 16);
        }
    }
}