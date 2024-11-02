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

"&>" {
	return GREATAMP;
}
"exit" {
	return EXIT;
}
"cd" {
	return CD;
}
" " {
	return NOTOKEN;
}
\"([^\\\"]|\\.)*\" {
    // Create a copy of the string without the quotes
    size_t len = strlen(yytext);
    yylval.string_val = strndup(yytext + 1, len - 2); // Exclude the quotes
    return WORD;
}
[^ &|<>\t\n][^ &|<>\t\n]* {
	/* Assume that file names have only alpha chars */
	yylval.string_val = strdup(yytext);
	return WORD;
}

. {
	/* Invalid character in input */
	return NOTOKEN;
}

%%

