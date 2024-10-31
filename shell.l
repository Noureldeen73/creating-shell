%{
#include <string.h>
#include "y.tab.h"
%}

%%

\n {
	return NEWLINE;
}

[ \t] {
	/* Discard spaces and tabs */
}

">" {
	return GREAT;
}

"<" {
	return LESS;
}

"|" {
	return PIPE;
}

">>" {
	return APPEND;
}

"&" {
	return AMP;
}

"&>>" { 
	return AMPAPPEND;
}

">&" {
	return GREATAMP;
}

[^ \t\n][^ \t\n]* {
	/* Assume that file names have only alpha chars */
	yylval.string_val = strdup(yytext);
	return WORD;
}

. {
	/* Invalid character in input */
	return NOTOKEN;
}

%%

