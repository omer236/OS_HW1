#include <string.h>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sstream>
#include <sys/param.h>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#define WHITESPACE " "
using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string retrieveString( char* buf) {

    size_t len = 0;
    while( (len < 255) && (buf[ len ] != '\0') ) {
        len++;
    }

    return string( buf, len );

}
string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}


int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}
bool _isBackgroundCommand(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

JobsList::JobEntry::JobEntry(int jobId, const char *cmd_line, time_t time, int jobPid, bool isStopped): jobId(jobId), cmd_line(strdup(cmd_line)), time(time), jobPid(jobPid),
isStopped(isStopped){};


void JobsList::addJob(Command* cmd, pid_t jobPid, bool isStopped) {
    JobsList::JobEntry* newJob= new JobsList::JobEntry(JobsList::maxId+1, cmd->commmand_line, time(nullptr), jobPid, isStopped);
    maxId++;
    JobsList::jobs_vec.push_back(newJob);
}

JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs):BuiltInCommand(cmd_line), jobs_list(jobs){};

void JobsCommand::execute() {
    time_t now = time(nullptr);
    //this->jobs_list->removeFinishedJobs();
    string stopped = "";
    for (std::vector<JobsList::JobEntry*>::iterator it =this->jobs_list->jobs_vec.begin(); it !=  this->jobs_list->jobs_vec.end(); ++it){
        if ((*it)->isStopped) {
            stopped = " (stopped)";
        }
    cout << '[' << (*it)->jobId << "] " << (*it)->cmd_line << " : " << (*it)->jobPid << difftime(now, (*it)->time) << " secs"
         << stopped << std::endl;
    stopped = "";
    }
}

SmallShell::SmallShell() {
// TODO: add your implementation
    setpgrp();
   pidSmash=getpgrp();
   jobsList=new JobsList();
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}
Command::Command(const char *cmd_line){
    commmand_line=cmd_line;
    cmdArgs=new char*[COMMAND_MAX_ARGS+1];//////remember to distruct
    int numArgsTemp= _parseCommandLine(cmd_line,cmdArgs);
    if (!_isBackgroundCommand(cmd_line))
        numArg=numArgsTemp-1;
    else {
        if(*(cmdArgs[numArgsTemp-1])=='&'){
            numArg=numArgsTemp-2;
        }
        else{
            numArg=numArgsTemp-1;
        }
    }
}
Command::~Command() {}//release cmdargs???
BuiltInCommand::BuiltInCommand(const char *cmd_line):Command(cmd_line){};
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if(firstWord.compare("chprompt")==0)
        return new ChpromptCommand(cmd_line);
    else if (firstWord.compare("pwd") == 0)
        return new GetCurrDirCommand(cmd_line);
    else if (firstWord.compare("showpid") == 0)
        return new ShowPidCommand(cmd_line);
    else if (firstWord.compare("cd") == 0)
        return new ChangeDirCommand(cmd_line,&prev_dir);
    else if (firstWord.compare("jobs") == 0)
        return new JobsCommand(cmd_line, this->jobsList);
    else if (firstWord.compare("kill") == 0)
            return new KillCommand(cmd_line, this->jobsList);
    else if (firstWord.compare("fg") == 0)
        return new ForegroundCommand(cmd_line, this->jobsList);
    else if (firstWord.compare("bg") == 0)
        return new BackgroundCommand(cmd_line, this->jobsList);
    else if (firstWord.compare("quit") == 0)
        return new QuitCommand(cmd_line,this->jobsList);
     else
         return new ExternalCommand(cmd_line);
}
KillCommand::KillCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line), jobs_list(jobs){};
ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line),jobs_list(jobs) {//new
    this->fgCommandLine=cmd_line;
};
BackgroundCommand::BackgroundCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line),jobs_list(jobs) {//new
    this->bgCommandLine=cmd_line;
};

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    for (std::vector<JobsList::JobEntry *>::iterator it = this->jobs_vec.begin(); it != this->jobs_vec.end(); ++it) {
        if((*it)->jobId==jobId) {
            return (*it);
        }
    }
    return nullptr;
}
void KillCommand::execute() {
    char* no_makaf=cmdArgs[1]+1;
    int sig_num=atoi(no_makaf);
    int jobId= atoi(cmdArgs[2]);
    int pid=SmallShell::getInstance().jobsList->getJobById(jobId)->jobPid;
        kill(pid, sig_num);
        cout << "signal number " << sig_num << " was sent to pid " << pid << std::endl;
}
void ForegroundCommand::execute() {
        SmallShell &smash=SmallShell::getInstance();
        if(numArg==0) {
            JobsList::JobEntry* job=smash.jobsList->jobs_vec.back();
            if(job== nullptr){
                perror("smash error:fg:jobs list is empty");
            }
            cout << job->cmd_line << " : " << job->jobPid << std::endl;
            kill(job->jobPid,SIGCONT);
            waitpid(job->jobPid,nullptr,WUNTRACED);
            smash.jobsList->removeJobById(job->jobId);
        }
    else if(numArg==1){////efshar lezamzem
            JobsList::JobEntry* job =smash.jobsList->getJobById(atoi(cmdArgs[1]));
            if(job== nullptr){
                //perror(message);
            }
            cout << job->cmd_line << " : " << job->jobPid << std::endl;
           kill(job->jobPid,SIGCONT);
            waitpid(job->jobPid,nullptr,WUNTRACED);
           // smash.jobsList->removeJobById(job->jobId);
        }
    }

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    if(this->jobs_vec.empty()){
        return nullptr;
    }
    for (std::vector<JobsList::JobEntry *>::iterator it = this->jobs_vec.end(); it != this->jobs_vec.begin(); --it){
        if((*it)->isStopped) {
            *jobId=(*it)->jobId;
            return (*it);
        }
    }
    return nullptr;
}
void BackgroundCommand::execute() {
    SmallShell &smash=SmallShell::getInstance();
    if(numArg==0) {
        int* job_stopped_id= nullptr;
        JobsList::JobEntry* job=smash.jobsList->getLastStoppedJob(job_stopped_id);
        if(job== nullptr){
            perror("smash error:bg:jobs list is empty");
        }
        cout << job->cmd_line << " : " << job->jobPid << std::endl;
        kill(job->jobPid,SIGCONT);
        job->isStopped= false;
    }
    else if(numArg==1){
        JobsList::JobEntry* job =smash.jobsList->getJobById(atoi(cmdArgs[1]));
        if(job== nullptr){
            //perror(message);
        }
        cout << job->cmd_line << " : " << job->jobPid << std::endl;
        kill(job->jobPid,SIGCONT);
        job->isStopped= false;
    }
}

void JobsList::killAllJobs() {
    if(this->jobs_vec.empty()){
        std::cout << "smash: sending SIGKILL signal to 0 jobs:" << std::endl;
        return;
    }
    for (std::vector<JobsList::JobEntry *>::iterator it = this->jobs_vec.begin(); it != this->jobs_vec.end(); ++it) {
        std::cout << (*it)->jobPid << " : " << (*it)->cmd_line << std:: endl;
        kill((*it)->jobPid,SIGKILL);///if dosent work try to put it out loop
    }
}

QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line), jobs_list(jobs) {};

void QuitCommand::execute() {
    SmallShell::getInstance().jobsList->removeFinishedJobs();
    if(cmdArgs[1]=="kill"){
        std::cout << "smash: sending SIGKILL signal to "<< SmallShell::getInstance().jobsList->jobs_vec.size() <<" jobs:" << std::endl;
        SmallShell::getInstance().jobsList->killAllJobs();
    }

}


 void JobsList::removeJobById(int jobId){
    SmallShell &smash= SmallShell::getInstance();
     if(smash.jobsList->jobs_vec.size()==0)
         return;
     for (std::vector<JobsList::JobEntry*>::iterator it =smash.jobsList->jobs_vec.begin(); it !=smash.jobsList->jobs_vec.end(); ++it) {
         if (((*it)->jobId) == jobId) {
             delete (*it);
             smash.jobsList->jobs_vec.erase((it));
             if((*it)->jobId==maxId)
                maxId=smash.jobsList->jobs_vec.back()->jobId;
         }
     }
}
void JobsList::removeFinishedJobs() {
     if(this->jobs_vec.empty())
         return;
     for (std::vector<JobsList::JobEntry*>::iterator it =this->jobs_vec.begin(); it !=this->jobs_vec.end(); ++it){
         if(waitpid((*it)->jobPid,nullptr,WNOHANG)>0){
             delete (*it);
             this->jobs_vec.erase((it));
         }
     }
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command* cmd= CreateCommand(cmd_line);
    cmd->execute();
    delete cmd;
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}


void ChpromptCommand::execute() {
    std::string stemp="smash";
    if (numArg==0)
        SmallShell::getInstance().setPt(stemp);
    if (numArg>=1)
        SmallShell::getInstance().setPt(retrieveString(cmdArgs[1]));
}

ChpromptCommand::ChpromptCommand(const char *cmd_line): BuiltInCommand(cmd_line){};
void ShowPidCommand::execute() {
   pid_t pid=SmallShell::getInstance().getPidSmash();
   string  pid_str= to_string(pid);
    std::cout << "smash pid is " << pid_str << endl;
}
void GetCurrDirCommand::execute(){
    char buffer[PATH_MAX];
    getcwd(buffer,PATH_MAX);
    std::cout << buffer << "\n";//changed from  b.c_str
}
void ChangeDirCommand::execute() {
    SmallShell &smash=SmallShell::getInstance();
    char buffer[PATH_MAX];
    getcwd(buffer,PATH_MAX);
    char* current_dir=buffer;//changed from  b.c_str
    if(*(cmdArgs[1])=='-'){
        char* prev_dir2= smash.prev_dir;
        smash.prev_dir=current_dir;
        chdir(prev_dir2);
    }
    else if(cmdArgs[1]=="..") {
        int i=0,pos;
        while(current_dir[i]!='/0'){
            if(current_dir[i]=='/'){
                pos=i;
            }
            i++;
        }
        smash.prev_dir=current_dir;
        string short_path=std::string(current_dir);
         short_path=short_path.substr(0, pos-1);
        chdir(short_path.c_str());
    }
    else{
        smash.prev_dir=buffer;
        chdir(cmdArgs[1]);
    }
}

ExternalCommand::ExternalCommand(const char *cmd_line): Command(cmd_line) {
    this->cmd_line=cmd_line;
};

void ExternalCommand::execute() {
    bool is_background=false;
    if(_isBackgroundCommand(cmd_line))
        is_background=true;
    pid_t pid=fork();
    if(pid<0){
        perror("error");/////change
        return;
    }
    if (pid==0){
        setpgrp();
        const char* arg[]={"/bin/bash","-c",cmd_line, nullptr};
        char** arguments=(char **) arg;
        execv("/bin/bash",arguments);
    }
    if(pid>0){
        if(!is_background){
            waitpid(pid, nullptr,WUNTRACED);
        }
        else{
            Command *cmd=SmallShell::getInstance().CreateCommand(cmd_line);
            SmallShell::getInstance().jobsList->addJob(cmd, pid, false);
        }
    }

}

