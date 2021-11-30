#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <string>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    struct sigaction s_action;
            s_action.sa_handler=alarmHandler;
            s_action.sa_flags=SA_RESTART;
    if(sigaction(SIGALRM,&s_action, nullptr)==-1){
        perror("smash error: failed to set alarm handler");
    }

    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    while(true) {
        std::cout << smash.getPt() << "> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}