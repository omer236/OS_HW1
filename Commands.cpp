#include <string.h>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <sys/param.h>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <fcntl.h>
#include <signal.h>
#include "signals.h"
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

JobsList::JobEntry::JobEntry(int jobId, const char *cmd_line,Command* command, time_t time, int jobPid, bool isStopped): jobId(jobId), cmd_line(strdup(cmd_line)),command(command),time(time), jobPid(jobPid),
isStopped(isStopped){};


void JobsList::addJob(Command* cmd, pid_t jobPid, bool isStopped) {
    JobsList::JobEntry* newJob= new JobsList::JobEntry(SmallShell::getInstance().maxID+1, cmd->commmand_line, cmd,time(nullptr), jobPid, isStopped);
    SmallShell::getInstance().maxID++;
    JobsList::jobs_vec.push_back(newJob);
}

JobsCommand::JobsCommand(const char *cmd_line):BuiltInCommand(cmd_line){};

void JobsCommand::execute() {
    SmallShell &smash= SmallShell::getInstance();
    time_t now = time(nullptr);
    if(smash.jobsList.jobs_vec.empty())
        return;
    smash.jobsList.removeFinishedJobs();
    string stopped = "";
    for (std::vector<JobsList::JobEntry*>::iterator it = smash.jobsList.jobs_vec.begin(); it !=   smash.jobsList.jobs_vec.end(); it++){
        if ((*it)->isStopped) {
            stopped = " (stopped)";
        }
    cout << '[' << (*it)->jobId << "] " << (*it)->cmd_line << " : " << (*it)->jobPid <<" "<< (int)difftime(now, (*it)->time)<< " secs"
         << stopped << std::endl;
    stopped = "";
    }
}
SmallShell::SmallShell() {
// TODO: add your implementation
    setpgrp();
   pidSmash=getpgrp();
   }

SmallShell::~SmallShell() {
// TODO: add your implementation
}
Command::Command(const char *cmd_line){
    commmand_line=strdup(cmd_line);
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
     if(std::string(cmd_line).find(">")!= std::string::npos)
        return new RedirectionCommand(cmd_line);
    else if(std::string(cmd_line).find('|')!= std::string::npos)
        return new PipeCommand(cmd_line);
    else if(firstWord.compare("timeout")==0)
        return new TimeoutCommand(cmd_line);
    else if(firstWord.compare("head")==0)
        return new HeadCommand(cmd_line);
    else if(firstWord.compare("chprompt")==0)
        return new ChpromptCommand(cmd_line);
    else if (firstWord.compare("pwd") == 0)
        return new GetCurrDirCommand(cmd_line);
    else if (firstWord.compare("showpid") == 0)
        return new ShowPidCommand(cmd_line);
    else if (firstWord.compare("cd") == 0)
        return new ChangeDirCommand(cmd_line,&prev_dir);
    else if (firstWord.compare("jobs") == 0)
        return new JobsCommand(cmd_line);
    else if (firstWord.compare("kill") == 0)
            return new KillCommand(cmd_line);
    else if (firstWord.compare("fg") == 0)
        return new ForegroundCommand(cmd_line);
    else if (firstWord.compare("bg") == 0)
        return new BackgroundCommand(cmd_line);
    else if (firstWord.compare("quit") == 0)
        return new QuitCommand(cmd_line);
     else
         return new ExternalCommand(cmd_line);
}
KillCommand::KillCommand(const char *cmd_line): BuiltInCommand(cmd_line){};
ForegroundCommand::ForegroundCommand(const char *cmd_line): BuiltInCommand(cmd_line){//new
    this->fgCommandLine=cmd_line;
};
BackgroundCommand::BackgroundCommand(const char *cmd_line): BuiltInCommand(cmd_line) {//new
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
bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}
void KillCommand::execute() {
    if (numArg != 2)
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
    else if (cmdArgs[1][0] != '-') {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
    }
    else if(!is_number(cmdArgs[1]+1)){
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
    }
    else if(cmdArgs[2][0]=='-'&&!is_number(cmdArgs[2]+1)){
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
    }
    else if(!is_number(cmdArgs[2])&&cmdArgs[2][0]!='-'){
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
    }
    else {
        SmallShell::getInstance().jobsList.removeFinishedJobs();
        char *no_makaf = cmdArgs[1] + 1;
        int sig_num = atoi(no_makaf);
        int jobId = atoi(cmdArgs[2]);
        if(SmallShell::getInstance().jobsList.getJobById(jobId)==nullptr) {
            std::cerr << "smash error: kill: job-id " << jobId << " does not exist" << std::endl;
            return;
        }
        int pid = SmallShell::getInstance().jobsList.getJobById(jobId)->jobPid;
        if(kill(pid, sig_num)!=0){
            perror("smash error: kill failed");
            return;
        }
        cout << "signal number " << sig_num << " was sent to pid " << pid << std::endl;
        if (sig_num == SIGKILL) {
            SmallShell::getInstance().jobsList.removeJobById(jobId);
        }
        if (sig_num == SIGSTOP) {
            SmallShell::getInstance().jobsList.getJobById(jobId)->isStopped = true;
        }
    }
}
void ForegroundCommand::execute() {
    SmallShell &smash = SmallShell::getInstance();
    if (numArg == 0) {
        if (smash.jobsList.jobs_vec.empty()) {
            std::cerr << "smash error: fg: jobs list is empty" << std::endl;
            return;
        }
        JobsList::JobEntry *job = smash.jobsList.jobs_vec.back();
        cout << job->cmd_line << " : " << job->jobPid << std::endl;
        if(kill(job->jobPid, SIGCONT)!=0){
            perror("smash error: kill falied");
            return;
        }
        smash.fg_job=job;
        smash.foreground_pid = job->jobPid;
        smash.cmd=job->command;

        if(waitpid(job->jobPid, nullptr, WUNTRACED)<0){
            perror("smash error: waitpid failed");
            return;
        }
        smash.foreground_pid=0;
        smash.cmd=nullptr;
        smash.jobsList.removeJobById(job->jobId);
    } else if (numArg == 1) {
        JobsList::JobEntry *job = smash.jobsList.getJobById(atoi(cmdArgs[1]));
        if (job == nullptr) {
            std::cerr << "smash error: fg: job-id " << cmdArgs[1] << " does not exist" << std::endl;
            return;
        }

        cout << job->cmd_line << " : " << job->jobPid << std::endl;
        if(kill(job->jobPid, SIGCONT)!=0){
            perror("smash error: kill falied");
            return;
        }
        smash.fg_job=job;
        smash.foreground_pid = job->jobPid;
        smash.cmd=job->command;
        if(waitpid(job->jobPid, nullptr, WUNTRACED)<0){
            perror("smash error: waitpid failed");
            return;
        }
        smash.jobsList.removeJobById(job->jobId);
        smash.foreground_pid=0;
        smash.cmd=nullptr;
    } else {
        std::cerr << "smash error: fg: invalid arguments" << std::endl;


    }
}
int getLastStoppedJob() {
    std::vector<JobsList::JobEntry*>::iterator it =SmallShell::getInstance().jobsList.jobs_vec.begin();
     int maxJobId=-1;
    while (it !=SmallShell::getInstance().jobsList.jobs_vec.end()) {
        if ((*it)->isStopped) {
            maxJobId = (*it)->jobId;
        }
        it++;
    }
    return maxJobId;
}
void BackgroundCommand::execute() {
    SmallShell &smash=SmallShell::getInstance();
    if(numArg==0) {
        int job_stopped_id= getLastStoppedJob();
        if(smash.jobsList.getJobById(job_stopped_id)== nullptr){
            std::cerr<<"smash error: bg: there is no stopped jobs to resume"<<std::endl;
            return;
        }
        else {
            cout << smash.jobsList.getJobById(job_stopped_id)->cmd_line << " : "
                 << smash.jobsList.getJobById(job_stopped_id)->jobPid << std::endl;
            if(kill(smash.jobsList.getJobById(job_stopped_id)->jobPid, SIGCONT)!=0){
                perror("smash error: kill falied");
                return;
            }
            smash.jobsList.getJobById(job_stopped_id)->isStopped = false;
        }
    }
    else if(numArg==1){
        JobsList::JobEntry* job =smash.jobsList.getJobById(atoi(cmdArgs[1]));
        if(job== nullptr){
            std::cerr<<"smash error: bg: job-id "<< cmdArgs[1]<<" does not exist"<<std::endl;
            return;
        }
        if(job->isStopped==false) {
            std::cerr << "smash error: bg: job-id " << cmdArgs[1] << " is already running in the background"<< std::endl;
            return;
        }
        cout << job->cmd_line << " : " << job->jobPid << std::endl;
        if(kill(job->jobPid,SIGCONT)!=0){
            perror("smash error: kill falied");
            return;
        }
        job->isStopped= false;
    }
    else
    std::cerr<<"smash error: bg: invalid arguments"<<std::endl;
}

void JobsList::killAllJobs() {
    if(this->jobs_vec.empty()){
        std::cout << "smash: sending SIGKILL signal to 0 jobs:" << std::endl;
        return;
    }
    std::vector<JobsList::JobEntry*>::iterator it =jobs_vec.begin();
    while (it !=this->jobs_vec.end()){
        std::cout << (*it)->jobPid << ": " << (*it)->cmd_line << std:: endl;
        if(kill((*it)->jobPid,SIGKILL)!=0){
            perror("smash error: kill falied");
            return;
        }
            this->jobs_vec.erase((it));
    }
    }


QuitCommand::QuitCommand(const char *cmd_line): BuiltInCommand(cmd_line){};

void QuitCommand::execute() {///// free memory !!!!
    SmallShell::getInstance().jobsList.removeFinishedJobs();
    if(numArg>=1&&std::string(cmdArgs[1])=="kill"){
        std::cout << "smash: sending SIGKILL signal to "<< SmallShell::getInstance().jobsList.jobs_vec.size() <<" jobs:" << std::endl;
        SmallShell::getInstance().jobsList.killAllJobs();
    }

        exit(0);
}


 void JobsList::removeJobById(int jobId){

     std::vector<JobsList::JobEntry*>::iterator it =this->jobs_vec.begin();
     while (it !=this->jobs_vec.end()){
         if((*it)->jobId==jobId) {
             this->jobs_vec.erase((it));
             break;         }
         else
             it++ ;
     }
     if(this->jobs_vec.empty()) {
         SmallShell::getInstance().maxID=0;
     }
     else {
         SmallShell::getInstance().maxID= this->jobs_vec.back()->jobId;
     }
}
void JobsList::removeFinishedJobs() {
    if (JobsList::jobs_vec.empty())
        return;
    std::vector<JobsList::JobEntry*>::iterator it =jobs_vec.begin();
     while (it !=this->jobs_vec.end()){
         if((kill((*it)->jobPid,0)==0)&&(waitpid((*it)->jobPid, nullptr,WNOHANG))>0){
             //delete (*it);
             it=this->jobs_vec.erase((it));
         }else
             it++ ;
     }
     if(jobs_vec.empty())
         SmallShell::getInstance().maxID=0;
     else
         SmallShell::getInstance().maxID=this->jobs_vec.back()->jobId;
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command* cmd= CreateCommand(cmd_line);
    SmallShell::getInstance().cmd=cmd;
    cmd->execute();

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
    if(getcwd(buffer,PATH_MAX)== nullptr){
        perror("smash error: getcwd failed");
        return;
    }
    std::cout << buffer << "\n";//changed from  b.c_str
}
void ChangeDirCommand::execute() {
    if(numArg==0){
        std::cerr << "smash error: " <<commmand_line<<std::endl;
    }
    else if (numArg >= 2) {
        std::cerr << "smash error: cd: too many arguments" << std::endl;

    } else {
        SmallShell &smash = SmallShell::getInstance();
        char buffer[PATH_MAX];
        if(getcwd(buffer, PATH_MAX)==nullptr){
            perror("smash error: getcwd failed");
            return;
        }
        char *current_dir = buffer;//changed from  b.c_str
        if (*(cmdArgs[1]) == '-') {
            if (smash.prev_dir == NULL) {
                perror("smash error: cd: OLDPWD not set");
                return;
            }
            char *prev_dir2 = smash.prev_dir;
            smash.prev_dir = current_dir;
            if(chdir(prev_dir2)!=0){
                perror("smash error: chdir failed");
                return;
            }
        } else if (cmdArgs[1] == "..") {
            int i = 0, pos;
            while (current_dir[i] != '/0') {
                if (current_dir[i] == '/') {
                    pos = i;
                }
                i++;
            }
            smash.prev_dir = current_dir;
            string short_path = std::string(current_dir);
            short_path = short_path.substr(0, pos - 1);
            if(chdir(short_path.c_str())!=0){
                perror("smash error: chdir failed");
                return;
            }
        } else {
            smash.prev_dir = buffer;
            if(chdir(cmdArgs[1])!=0){
                perror("smash error: chdir failed");
                return;
            }
        }
    }
}

ExternalCommand::ExternalCommand(const char *cmd_line):Command(cmd_line) {
};

void ExternalCommand::execute() {
    SmallShell::getInstance().jobsList.removeFinishedJobs();
    bool is_background=false;
    if(_isBackgroundCommand(this->commmand_line))
        is_background=true;
    pid_t pid=fork();
    if(pid<0){
        perror("smash error:fork failed");
        return;
    }
    if (pid==0){
        setpgrp();
        _removeBackgroundSign((char*)this->commmand_line);
        const char* arg[]={"/bin/bash","-c",this->commmand_line, nullptr};
        char** arguments=(char **) arg;
        execv("/bin/bash",arguments);
    }
    if(pid>0){
        if(!is_background){
            JobsList::JobEntry* newJob= new JobsList::JobEntry(SmallShell::getInstance().maxID+1, this->commmand_line, this,time(nullptr), pid, false);
            SmallShell::getInstance().fg_job=newJob;
            SmallShell::getInstance().foreground_pid=pid;
            SmallShell::getInstance().cmd=this;
            if(waitpid(pid, nullptr,WUNTRACED)<0){
                perror("smash error: waitpid failed");
                return;
            }
            SmallShell::getInstance().foreground_pid=0;
            SmallShell::getInstance().cmd=nullptr;

        }
        else{
            //Command *cmd=SmallShell::getInstance().CreateCommand(this->commmand_line);
            SmallShell::getInstance().jobsList.addJob(this,pid,false);
        }

    }

}
RedirectionCommand::RedirectionCommand(const char *cmd_line): Command(cmd_line) {};
void RedirectionCommand::execute() {
    SmallShell &smash=SmallShell::getInstance();
    if (std::string(this->commmand_line).find(">>") != std::string::npos) {
        string cmdLine = _trim(this->commmand_line);
        std::string command = _rtrim(cmdLine.substr(0, cmdLine.find(">>")));
        _removeBackgroundSign((char*)command.c_str());
        std::string filename = _ltrim(cmdLine.substr(cmdLine.find(">>") + 2));
        int stdout=dup(1);
        if(stdout<0){
            perror("smash error: dup failed");
            return;
        }
        if(close(1)==-1){
            perror("smash error: close failed");
            return;
        }
        if(open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666)<0){
            perror("smash error: open failed");
            if(dup2(stdout, 1)<0){
                perror("smash error: dup2 failed");
                return;
            }
            if(close(stdout)==-1){
                perror("smash error: close failed");
                return;
            }
            return;
        }
        smash.executeCommand(command.c_str());
        if(dup2(stdout, 1)<0){
            perror("smash error: dup2 failed");
            return;
        }
        if(close(stdout)==-1){
            perror("smash error: close failed");
            return;
        }
    } else {
        string cmdLine = _trim(this->commmand_line);
        std::string command = _rtrim(cmdLine.substr(0, cmdLine.find(">")));
        _removeBackgroundSign((char*)command.c_str());
        std::string filename = _ltrim(cmdLine.substr(cmdLine.find(">") + 1));
        int stdout=dup(1);
        if(stdout<0){
            perror("smash error: dup failed");
            return;
        }
        if(close(1)==-1){
            perror("smash error: close failed");
            return;
        }
        if(open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666)<0){
            perror("smash error: open failed");
            if(dup2(stdout, 1)<0){
                perror("smash error: dup2 failed");
                return;
            }
            if(close(stdout)==-1){
                perror("smash error: close failed");
                return;
            }
            return;
        }
        smash.executeCommand(command.c_str());
        if(dup2(stdout, 1)<0){
            perror("smash error: dup2 failed");
            return;
        }
        if(close(stdout)==-1){
            perror("smash error: close failed");
            return;
        }
    }
}

PipeCommand::PipeCommand(const char *cmd_line): Command(cmd_line) {};

void PipeCommand::execute() {
    SmallShell &smash = SmallShell::getInstance();
    if (std::string(this->commmand_line).find("|&") != std::string::npos) {
        std::string cmdLine = _trim(this->commmand_line);
        std::string command1 = _rtrim(cmdLine.substr(0, cmdLine.find("|")));
        std::string command2 = _ltrim(cmdLine.substr(cmdLine.find("|") + 2));
        _removeBackgroundSign((char *) command1.c_str());
        _removeBackgroundSign((char *) command2.c_str());
        int my_pipe[2];
        pipe(my_pipe);
        pid_t pid1=fork();
        if(pid1<0) {
            perror("smash error:fork failed");
            return;
        }
        if (pid1 == 0) {
            setpgrp();
            if(dup2(my_pipe[1], 2)<0){
                perror("smash error: dup2 failed");
                return;
            }
            if(close(my_pipe[0])==-1){
                perror("smash error: close failed");
                return;
            }
            if(close(my_pipe[1])==-1){
                perror("smash error: close failed");
                return;
            }
            smash.executeCommand(command1.c_str());
            exit(0);
        }
        pid_t pid2=fork();
        if(pid2<0) {
            perror("smash error:fork failed");
            return;
        }
        if(pid2==0) {
            setpgrp();
            if(dup2(my_pipe[0], 0)<0){
                perror("smash error: dup2 failed");
                return;
            }
            if(close(my_pipe[1])==-1){
                perror("smash error: close failed");
                return;
            }
            if(close(my_pipe[0])==-1){
                perror("smash error: close failed");
                return;
            }
            smash.executeCommand(command2.c_str());
            exit(0);
        }
        if(close(my_pipe[0])==-1){
            perror("smash error: close failed");
            return;
        }
        if(close(my_pipe[1])==-1){
            perror("smash error: close failed");
            return;
        }
        if(waitpid(pid1, nullptr,WUNTRACED)<0){
            perror("smash error: waitpid failed");
            return;
        }
        if(waitpid(pid2, nullptr,WUNTRACED)<0){
            perror("smash error: waitpid failed");
            return;
        }
    }

    else {
        std::string cmdLine = _trim(this->commmand_line);
        std::string command1 = _rtrim(cmdLine.substr(0, cmdLine.find("|")));
        std::string command2 = _ltrim(cmdLine.substr(cmdLine.find("|") + 1));
        _removeBackgroundSign((char *) command1.c_str());
        _removeBackgroundSign((char *) command2.c_str());
        int my_pipe[2];
        pipe(my_pipe);
        pid_t pid1=fork();
        if(pid1<0) {
            perror("smash error:fork failed");
            return;
        }
        if (pid1 == 0) {
            setpgrp();
            if(dup2(my_pipe[1], 1)<0){
                perror("smash error: dup2 failed");
                return;
            }
            if(close(my_pipe[0])==-1){
                perror("smash error: close failed");
                return;
            }
            if(close(my_pipe[1])==-1){
                perror("smash error: close failed");
                return;
            }
            smash.executeCommand(command1.c_str());
            exit(0);
        }
        pid_t pid2=fork();
        if(pid2<0) {
            perror("smash error:fork failed");
            return;
        }
         if(pid2==0) {
            setpgrp();
            if(dup2(my_pipe[0], 0)<0){
                perror("smash error: dup2 failed");
                return;
            }
            if(close(my_pipe[1])==-1){
                perror("smash error: close failed");
                return;
            }
            if(close(my_pipe[0])==-1){
                perror("smash error: close failed");
                return;
            }
            smash.executeCommand(command2.c_str());
            exit(0);
        }
        if(close(my_pipe[0])==-1){
            perror("smash error: close failed");
            return;
        }
        if(close(my_pipe[1])==-1){
            perror("smash error: close failed");
            return;
        }
        if(waitpid(pid1, nullptr,WUNTRACED)<0){
            perror("smash error: waitpid failed");
            return;
        }
        if(waitpid(pid2, nullptr,WUNTRACED)<0){
            perror("smash error: waitpid failed");
            return;
        }
        }
    }

HeadCommand::HeadCommand(const char *cmd_line): BuiltInCommand(cmd_line){};
void HeadCommand::execute() {
    if(numArg==0){
        std::cerr<<"smash error: head: not enough arguments"<<std::endl;
        return;
    }
    int numLines = 10;
     const char* no_makaf=cmdArgs[1]+1;
    if (numArg != 1) {
        numLines = atoi(no_makaf);
    }
    int file = open((const char*)cmdArgs[numArg], O_RDONLY);
    if(file==-1){
        perror("smash error: open failed");
        return;
    }
    char *buff = (char*)malloc(1);
    int read_status=read(file, buff, 1);
    if(read_status==-1){
        perror("smash error: read failed");
        return;
    }
    while( read_status==1&&(numLines!=0)) {
        if(read_status==-1){
            perror("smash error: read failed");
            return;
        }
        if (*buff == '\n')
            numLines--;
        int status = write(1, buff, 1);
        if(status==-1){
            perror("smash error: write failed");
            return;
        }
        if (status < 0) {
            perror("smash error write failed");
            break;
        }
        read_status=read(file, buff, 1);
        if(read_status==-1){
            perror("smash error: read failed");
            return;
        }
    }
    if(read_status==0&&numLines==0)
        cout<<std::endl;
}

TimeOut::TimeOut(const char *cmd_line):cmd_line(cmd_line){};
void TimeOut::execute() {
    ExternalCommand* cmd=new ExternalCommand(cmd_line);
    cmd->TimeOutExecute(this);
}

TimeoutCommand::TimeoutCommand(const char *cmd_line): BuiltInCommand(cmd_line){
};
void TimeoutCommand::execute() {

    SmallShell &smash=SmallShell::getInstance();
    TimeOut *timeout= new TimeOut(commmand_line);
    timeout->execute();
    timeout->durationTime=atoi(cmdArgs[1]);
    timeout->beginTime= time(nullptr);
    alarm(difftime(timeout->beginTime+timeout->durationTime,time(nullptr)));
    smash.timeVec.push_back(timeout);
}
void ExternalCommand::TimeOutExecute(TimeOut* time_out){
    bool is_background=false;
    if(_isBackgroundCommand(this->commmand_line))
        is_background=true;
    pid_t pid=fork();
    if(pid<0){
        perror("smash error:fork failed");
        return;
    }
    if (pid==0){
        setpgrp();
        _removeBackgroundSign((char*)this->commmand_line);
        const char* arg[]={"/bin/bash","-c",this->commmand_line, nullptr};
        char** arguments=(char **) arg;
        execv("/bin/bash",arguments);
    }
    if(pid>0){
        if(!is_background){
            time_out->pid=pid;
            SmallShell::getInstance().foreground_pid=pid;
            if(waitpid(pid, nullptr,WUNTRACED)<0){
                perror("smash error: waitpid failed");
                return;
            }

        }


    }

}

