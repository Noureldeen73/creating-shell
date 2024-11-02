
#ifndef command_h
#define command_h
#include <string.h>

// Command Data Structure
struct SimpleCommand
{
	// Available space for arguments currently preallocated
	int _numberOfAvailableArguments;

	// Number of arguments
	int _numberOfArguments;
	char **_arguments;

	SimpleCommand();
	void insertArgument(char *argument);
};

struct Command
{
	int _numberOfAvailableSimpleCommands;
	int _numberOfSimpleCommands;
	SimpleCommand **_simpleCommands;
	char *_outFile;
	char *_inputFile;
	char *_currentDir = NULL;
	bool _errFile;
	int _background = 0;
	int _append;

	void prompt();
	void print();
	void execute();
	void clear();

	Command();
	void insertSimpleCommand(SimpleCommand *simpleCommand);
	static Command _currentCommand;
	static SimpleCommand *_currentSimpleCommand;
};
void sigchld_handler(int signum);
void ignore(int signum);

#endif
