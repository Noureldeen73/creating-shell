
%union {
	char *string_val;
}

%token <string_val> WORD
%token NOTOKEN GREAT NEWLINE LESS PIPE APPEND AMP AMPAPPEND GREATAMP EXIT CD

%{
extern "C" {
	int yylex();
	void yyerror(const char *s);
}
#define yylex yylex
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "command.h"
#include <unistd.h>
#include<bits/stdc++.h>
#include <cstring>
%}

%%

goal:
	commands
	;

commands:
	command
	| commands command 
	;

command:
	simple_command
	| change_directory
	;
change_directory:
	CD WORD NEWLINE {
			if(chdir(remove_backslashes($2)) == -1)
			{
				printf("   Yacc: Directory not found\n");
				Command::_currentCommand.prompt();
			}
			else
			{
				char newDir[1024];
				if(strcmp($2, ".") == 0)
				{
					snprintf(newDir, sizeof(newDir), "%s", Command::_currentCommand._currentDir);
				}
				else if (strcmp($2, "..") == 0) {
					char *lastSlash = strrchr(Command::_currentCommand._currentDir, '/');
					if (lastSlash != NULL) {
						*lastSlash = '\0';
					}
					snprintf(newDir, sizeof(newDir), "%s", Command::_currentCommand._currentDir);
				} else {
					char *cleaned = remove_backslashes($2);
					for(int i = 0; i < strlen(cleaned); i++)
					{
						newDir[i] = cleaned[i];
					}
					snprintf(newDir, sizeof(newDir), "%s/%s", Command::_currentCommand._currentDir, remove_backslashes($2));
					free(cleaned);
				}
				Command::_currentCommand._currentDir = strdup(newDir);
				printf("   Yacc: Change directory to %s\n", Command::_currentCommand._currentDir);
				Command::_currentCommand.prompt();
			}
		}
	| CD NEWLINE {
		printf("   Yacc: Change directory to HOME\n");
		chdir(getenv("HOME"));
		Command::_currentCommand._currentDir = getenv("HOME");
		Command::_currentCommand.prompt();
	}
	;
simple_command:
	command_and_args piping iomodifier_opt amp NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE
	| error NEWLINE { yyerrok; }
	| EXIT NEWLINE {
		printf("   Yacc: GOODBYE!\n");
		exit(0);
	}
	;
piping:
	PIPE piped_command {
		printf("   Yacc: insert pipe\n");
	} 
	| /* can be empty */
	;

piped_command:
command_and_args piping{
	}
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
		printf("   Yacc: insert argument \"%s\"\n", $1);
		char *newArg = remove_backslashes($1);
		Command::_currentSimpleCommand->insertArgument(newArg);
	}
	;

command_word:
	WORD {
		printf("   Yacc: insert command \"%s\"\n", $1);
		Command::_currentSimpleCommand = new SimpleCommand();
		Command::_currentSimpleCommand->insertArgument($1);
	}
	;

iomodifier_opt:
	iomodifier_opt GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $3);
		Command::_currentCommand._outFile = $3;
	}
	| iomodifier_opt LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $3);
		Command::_currentCommand._inputFile = $3;
	}
	| iomodifier_opt APPEND WORD {
		printf("   Yacc: insert append \"%s\"\n", $3);
		Command::_currentCommand._outFile = $3;
		Command::_currentCommand._append = 1;
	}
	| iomodifier_opt AMPAPPEND WORD {
		printf("   Yacc: insert error append \"%s\"\n", $3);
		Command::_currentCommand._errFile = true;
		Command::_currentCommand._outFile = $3;
		Command::_currentCommand._append = 1;
	}
	| iomodifier_opt GREATAMP WORD {
		printf("   Yacc: insert output and error \"%s\"\n", $3);
		Command::_currentCommand._errFile = true;
		Command::_currentCommand._outFile = $3;
	}
	| /* can be empty */
	;
amp:
	AMP {
		printf("   Yacc: insert background\n");
		Command::_currentCommand._background = 1;
	}
	| /* can be empty */{
		Command::_currentCommand._background = 0;
	}
	;

%%

void yyerror(const char *s) {
	fprintf(stderr, "%s", s);
}

#if 0
main() {
	yyparse();
}
#endif
