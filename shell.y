%union {
	char *string_val;
}

%token <string_val> WORD
%token NOTOKEN GREAT NEWLINE LESS PIPE APPEND AMP AMPAPPEND GREATAMP

%{
extern "C" {
	int yylex();
	void yyerror(const char *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
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
	;
// ls aaaa | grep | cat > a.txt

simple_command:
	command_and_args piping iomodifier_opt amp NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE
	| error NEWLINE { yyerrok; }
	;
piping:
	PIPE piped_command {
		printf("   Yacc: insert pipe\n");
		// Command::_currentCommand.insertPipeCommand(Command::_currentSimpleCommand);
	} 
	| /* can be empty */
	;

piped_command:
command_and_args piping{
		// Command::_currentCommand.insertPipeCommand(Command::_currentSimpleCommand);
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
		Command::_currentSimpleCommand->insertArgument($1);
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
		Command::_currentCommand._errFile = $3;
		Command::_currentCommand._append = 1;
	}
	| iomodifier_opt GREATAMP WORD {
		printf("   Yacc: insert output and error \"%s\"\n", $3);
		Command::_currentCommand._errFile = $3;
	}
	| /* can be empty */
	;

amp:
	AMP {
		printf("   Yacc: insert background\n");
		Command::_currentCommand._background = true;
	}
	| /* can be empty */
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
