#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;


void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    SmallShell &smash = SmallShell::getInstance();
    pid_t fg=smash.foreground_pid;
    SmallShell::getInstance().jobsList.addJob(SmallShell::getInstance().cmd,fg,true);
    kill(fg,SIGSTOP);
    cout<<"smash: got ctrl-Z"<<std::endl;
    cout<<"smash: process "<<fg <<" was stopped"<<std::endl;
}
void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
  SmallShell &smash = SmallShell::getInstance();
  pid_t fg=smash.foreground_pid;
  kill(fg,SIGKILL);
  cout<<"smash: got ctrl-C"<<std::endl;
  cout<<"smash: process "<<fg <<" was killed"<<std::endl;
}

void alarmHandler(int sig_num) {
    // TODO: Add your implementation
    cout << "smash got an alarm" << std::endl;
    std::vector<TimeOut *>::iterator it = SmallShell::getInstance().timeVec.begin();
    while (it != SmallShell::getInstance().timeVec.end()) {
        if (((*it)->beginTime + (*it)->durationTime) >= time(nullptr)) {

            kill((*it)->pid, SIGKILL);
            cout << "smash: " << (*it)->cmd_line << " timed out" << std::endl;
            it=SmallShell::getInstance().timeVec.erase(it);
            break;
        }
        it++;
    }
    SmallShell::getInstance().jobsList.removeFinishedJobs();
}
