#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <cerrno>
#include <time.h>

#include "command.h"

char *remove_backslashes(const char *str)
{
	size_t len = strlen(str);
	char *result = (char *)malloc(len + 1); // Allocate memory for the new string
	char *dest = result;

	for (const char *src = str; *src != '\0'; ++src)
	{
		if (*src == '\\' && (*(src + 1) == ' '))
		{
			// Skip the backslash and copy the space
			++src;
		}
		*dest++ = *src;
	}
	*dest = '\0'; // Null-terminate the new string

	return result;
}

void insertLog(int pid, int status)
{
	int log_fd = open("/home/noureldeen/Data/term 7/Operating systmes/Labs/Lab3 assignment/lab3-src/shell.log", O_WRONLY | O_APPEND | O_CREAT, 0666);
	if (log_fd == -1)
	{
		perror("open log file");
		return;
	}

	time_t now = time(NULL);
	char *timestamp = ctime(&now);
	timestamp[strlen(timestamp) - 1] = '\0';

	char log_entry[256];
	snprintf(log_entry, sizeof(log_entry), "Child with PID %d terminated with status %d at %s\n", pid, status, timestamp);

	write(log_fd, log_entry, strlen(log_entry));

	close(log_fd);
}

void sigchld_handler(int signum)
{
	int status;
	pid_t pid;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) // msh bykhosh henaaa
	{
		printf("pid: %d\n", pid);
		insertLog(pid, status);
	}
}

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
	signal(SIGCHLD, sigchld_handler);
}

void SimpleCommand::insertArgument(char *argument)
{
	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments,
									  _numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}
void ignore(int signum)
{
	printf("please use exit function\n");
	Command::_currentCommand.prompt();
}
Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
													_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}

	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}
	_errFile = false;
	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
		printf("\n");
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
		   _inputFile ? _inputFile : "default", _errFile ? _outFile : "default",
		   _background ? "YES" : "NO");
	printf("\n\n");
}

void Command::execute()
{
	// Don't do anything if there are no simple commands
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	int pid;
	int fdpipe[2];
	int defaultin = dup(0);
	int defaultout = dup(1);
	int defaulterr = dup(2);
	int lastInput = defaultin;
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		if (i < _numberOfSimpleCommands - 1)
		{
			if (pipe(fdpipe) == -1)
			{
				perror("pipe");
				exit(1);
			}
		}
		pid = fork();
		if (pid == -1)
		{
			perror("fork");
			exit(2);
		}
		if (pid == 0)
		{
			if (i == 0 && _inputFile)
			{
				int fd = open(_inputFile, O_RDONLY);
				dup2(fd, 0);
				close(fd);
			}
			else
			{
				dup2(lastInput, 0);
			}
			if (i == _numberOfSimpleCommands - 1)
			{
				if (_outFile)
				{
					if (_append)
					{
						int fd = open(_outFile, O_WRONLY | O_APPEND | O_CREAT, 0666);
						if (_errFile)
							dup2(fd, 2);
						dup2(fd, 1);
						close(fd);
					}
					else
					{
						int fd = open(_outFile, O_WRONLY | O_TRUNC | O_CREAT, 0666);
						if (_errFile)
							dup2(fd, 2);
						dup2(fd, 1);
						close(fd);
					}
				}
				else
				{
					dup2(defaultout, 1);
					dup2(defaulterr, 2);
				}
			}
			else
			{
				dup2(fdpipe[1], 1);
				close(fdpipe[0]);
			}
			execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
			perror("execvp");
			exit(0);
		}
		else
		{
			if (lastInput != defaultin)
				close(lastInput);
			lastInput = fdpipe[0];
			close(fdpipe[1]);
			if (!_background)
			{
				int status;
				insertLog(pid, status);
				waitpid(pid, &status, 0);
			}
		}
	}
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	// Clear to prepare for next command
	clear();

	// Print new prompt
	prompt();
}

// Shell implementation

void Command::prompt()
{
	printf("myshell");
	if (_currentDir != NULL)
	{
		printf(":%s", _currentDir);
	}
	printf("> ");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

int main()
{
	signal(SIGINT, ignore);
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}
