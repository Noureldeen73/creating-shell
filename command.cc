#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
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

	if (_errFile)
	{
		free(_errFile);
	}

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
	if (_errFile)
	{
		printf("  %-12s %-12s %-12s %-12s\n", _errFile,
			   _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
			   _background ? "YES" : "NO");
	}
	else
		printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
			   _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
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
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		int pid = fork();
		if (pid == -1)
		{
			perror("fork");
			exit(2);
		}
		if (pid == 0)
		{
			int defaultin = dup(0);
			int defaultout = dup(1);
			int defaulterr = dup(2);
			if (_inputFile)
			{
				int fd = open(_inputFile, O_RDONLY);
				dup2(fd, 0);
				close(fd);
			}
			else
			{
				dup2(defaultin, 0);
			}
			if (_outFile)
			{
				if (_append)
				{
					int fd = open(_outFile, O_CREAT | O_WRONLY | O_APPEND, 0644);
					dup2(fd, 1);
					close(fd);
				}
				else
				{
					int fd = open(_outFile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
					dup2(fd, 1);
					close(fd);
				}
			}
			else
			{
				dup2(defaultout, 1);
			}
			if (_errFile)
			{
				if (_append)
				{
					int fd = open(_errFile, O_CREAT | O_WRONLY | O_APPEND, 0644);
					dup2(fd, 2);
					close(fd);
				}
				else
				{
					int fd = open(_errFile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
					dup2(fd, 2);
					close(fd);
				}
			}
			else
			{
				dup2(defaulterr, 2);
			}
			execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
			perror("execvp");
			_exit(1);

			// Restore in/out defaults
			dup2(defaultin, 0);
			dup2(defaultout, 1);
			dup2(defaulterr, 2);

			close(defaultin);
			close(defaultout);
			close(defaulterr);
		}
		if (_background)
		{
			// do not wait for the child process
			continue;
		}
		waitpid(pid, 0, 0);
	}
	// Clear to prepare for next command
	clear();

	// Print new prompt
	prompt();
}

// Shell implementation

void Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

int main()
{
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}
