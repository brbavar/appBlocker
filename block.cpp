#include <string>

int main(int argc, char* argv[]) {
    system("taskkill /f /im blockStarter.exe");
    system("taskkill /f /im WindowsTerminal.exe");
    system("taskkill /f /im cmd.exe");
    system("taskkill /f /im powershell.exe");
    system("taskkill /f /im git-bash.exe");
    system("taskkill /f /im bash.exe");
    while(1) {
        std::string cmd = "taskkill /f /im ";
        /* Skip first command-line arg, which is not the name of
           an app we were asked to block, but rather the string
           "appBlocker.exe" that runs this program */
        int i = 1;
        while(i < argc) {
            int j = 0;
            char c = argv[i][j];
            while(c != ' ') {
                if((int)c > 32 && (int)c < 127) {
                    if(c == ':')
                        c = ' ';
                    cmd += c;
                }
                else
                    break;
                j++;
                c = argv[i][j];
            }
            i++;
            system(cmd.c_str());
            cmd = cmd.substr(0, 16);
        }
    }
}