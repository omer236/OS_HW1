#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;


void ctrlZHandler(int sig_num) {
    // TODO: Add your implementation
    SmallShell &smash = SmallShell::getInstance();
    pid_t fg = smash.foreground_pid;
    if (fg == 0) {
        cout << "smash: got ctrl-Z" << std::endl;
        return;
    }
    cout << "smash: got ctrl-Z" << std::endl;
    JobsList::JobEntry *newJob = smash.fg_job;
    newJob->isStopped = true;
    if(smash.maxID==0)
    smash.jobsList.jobs_vec.insert(smash.jobsList.jobs_vec.begin(), newJob);
    else if(smash.maxID>0&&newJob->jobId==-1)
        smash.jobsList.jobs_vec.insert(smash.jobsList.jobs_vec.end(), newJob);
    else if(smash.maxID>0&&newJob->jobId!=-1&&(newJob->jobId)<smash.maxID)
        smash.jobsList.jobs_vec.insert(smash.jobsList.jobs_vec.begin()+newJob->jobId-1, newJob);
    else if(smash.maxID>0&&newJob->jobId!=-1&&(newJob->jobId)>smash.maxID)
        smash.jobsList.jobs_vec.insert(smash.jobsList.jobs_vec.begin()+smash.maxID-1, newJob);
    else if(smash.maxID>0&&newJob->jobId!=-1&&(newJob->jobId)==smash.maxID){
        smash.jobsList.jobs_vec.insert(smash.jobsList.jobs_vec.begin()+smash.maxID-1, newJob);
        smash.jobsList.maxId++;
        }
    kill(fg,SIGSTOP);
    smash.foreground_pid=0;
    smash.cmd= nullptr;
    cout<<"smash: process "<<fg <<" was stopped"<<std::endl;
}
void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
  SmallShell &smash = SmallShell::getInstance();
  pid_t fg=smash.foreground_pid;
  if(fg==0){
      cout<<"smash: got ctrl-C"<<std::endl;
      return;
  }
  kill(fg,SIGKILL);
  smash.foreground_pid=0;
  smash.cmd= nullptr;
  cout<<"smash: got ctrl-C"<<std::endl;
  cout<<"smash: process "<<fg <<" was killed"<<std::endl;
}

void alarmHandler(int sig_num) {
    // TODO: Add your implementation
    bool alarm=false;
    if (SmallShell::getInstance().timeVec.empty())
        return;
    std::vector<TimeOut *>::iterator it = SmallShell::getInstance().timeVec.begin();
   // while (it != SmallShell::getInstance().timeVec.end()) {
       // if ((kill((*it)->pid, 0) == 0) && (waitpid((*it)->pid, nullptr, WNOHANG)) > 0) {
            //delete (*it);
         //   it = SmallShell::getInstance().timeVec.erase((it));
       // } else
         //   it++;
    //}
    SmallShell::getInstance().jobsList.removeFinishedJobs();
         it = SmallShell::getInstance().timeVec.begin();
        while (it != SmallShell::getInstance().timeVec.end()) {
            if (((*it)->beginTime + (*it)->durationTime) == time(nullptr)) {
                if((kill((*it)->pid,0)==0)&&waitpid((*it)->pid, nullptr, WNOHANG)>0) {
                return;
            }
                    cout << "smash got an alarm" << std::endl;

                kill((*it)->pid, SIGKILL);
                cout << "smash: " << (*it)->cmd_line << "timed out!" << std::endl;
                it = SmallShell::getInstance().timeVec.erase(it);
            }
            else
            it++;
        }
    }
