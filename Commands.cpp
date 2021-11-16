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
#define MAX_COMMAND_STR_LNT(255)
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
Command::Command(const char *cmd_line) {
    numArg=_parseCommandLine(cmd_line, cmdArgs);
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

bool _isBackgroundComamnd(const char* cmd_line) {
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


SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

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
    else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
	// For example:
/*


  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

}
void ChpromptCommand::execute() {
    if (numArg==0)
        ptMessage="smash";
    if (numArg>=1)
        ptMessage=cmdArgs[0];
}
void ShowPidCommand::execute() {
   pid_t pid=SmallShell::getInstance().getPidSmash();
   string  pid_str= to_string(pid);
    std::cout << "smash pid is " << pid_str << endl;
}
void pwdCommand::execute(){
    char* buffer;
    getcwd(buffer);
    std::cout << buffer.c_str();

}
void ChangeDirCommand::execute() {
    char* buffer;
    getcwd(buffer);
    string curent_dir=buffer.c_str();
    if(cmdArgs[1]=='-'){
        string prev_dir2= SmallShell::getInstance().prev_dir;
        SmallShell::getInstance().prev_dir=current_dir;
        chdir(prev_dir2);

    }
    if(cmdArgs[1]=='..') {
        int i=0,pos;
        while(curent_dir[i]!='/0'){
            if(curent_dir[i]=='/'){
                pos=i;
            }
            i++;
        }
        SmallShell::getInstance().prev_dir=current_dir;
        string short_path=curent_dir.substr(0, pos-1);
        chdir(short_path);
    }
    else{
        SmallShell::getInstance().prev_dir=current_dir;
        chdir(cmdArgs[2].c_str());
    }
}

