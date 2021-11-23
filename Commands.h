#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_
#include <vector>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <time.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)


class Command {
 public:
    int numArg;
    char** cmdArgs;
    const char* commmand_line;
    Command(const char* cmd_line);
    virtual ~Command();
    virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members
char** lastPw;
public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd):BuiltInCommand(cmd_line),lastPw(plastPwd){}
  virtual ~ChangeDirCommand() {}
  void execute() override;
};
class ChpromptCommand : public BuiltInCommand {
public:
    ChpromptCommand(const char *cmd_line);
    virtual ~ChpromptCommand() {}
    void execute() override;
};
class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  QuitCommand(const char* cmd_line);
  virtual ~QuitCommand() {}
  void execute() override;

};




class JobsList {
 public:
  class JobEntry {
  public:
      int jobId;
      const char* cmd_line;
      time_t time;
      int jobPid;
      bool isStopped;
      JobEntry(int jobId,const char* cmd_line,time_t time,int jobPid,bool isStopped);
      ~JobEntry()=default;
   // TODO: Add your data members
  };
    std::vector<JobEntry*> jobs_vec;
    int maxId=0;
 // TODO: Add your data members
  JobsList()= default;
  ~JobsList() =default;
  void addJob(Command* cmd, pid_t jobPid, bool isStopped);
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry *getLastStoppedJob(int *jobId);

  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
    const char* fgCommandLine;
  ForegroundCommand(const char* cmd_line);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
    const char* bgCommandLine;
  BackgroundCommand(const char* cmd_line);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class HeadCommand : public BuiltInCommand {
 public:
  HeadCommand(const char* cmd_line);
  virtual ~HeadCommand() {}
  void execute() override;
};


class SmallShell {
 private:
    pid_t pidSmash;
    std::string ptMessage="smash";
  // TODO: Add your data members
  SmallShell();
 public:
    JobsList jobsList;
    void setPt(std::string s){
        ptMessage=s;
    }
    std::string getPt(){
        return ptMessage;
    }
    char* prev_dir=NULL;
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
    pid_t getPidSmash(){
        return pidSmash;
    }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
};



#endif //SMASH_COMMAND_H_
