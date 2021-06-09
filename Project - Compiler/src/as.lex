%{

#include "src/abstract_tree.h" 
#include "as.tab.h"

char line[512];
int nb_character = 0;
int ident_character = 0;
int last_string = 0;

%}

%option yylineno
%option nounput
%option noinput

%x COMMENTAIRE COMMENTAIRE_BIS QUOTE

character \'.\'
num [0-9]+
type "int"|"char"
eq "=="|"!="
order "<"|"<="|">"|">="
addsub "+"|"-"
divstar "*"|"/"|"%"
struct "struct"
or "||"
and "&&"
void "void"
print "print"
if "if"
else "else"
while "while"
return "return"
reade "reade"
readc "readc"
ident [a-zA-Z][a-zA-Z0-9_]*
blanc [ \t] 

%%

"/*" 											{ BEGIN(COMMENTAIRE); }
"//"											{ BEGIN(COMMENTAIRE_BIS); }
\" 												{ BEGIN(QUOTE); } 

<COMMENTAIRE>"*/"								{ BEGIN(INITIAL); }
<COMMENTAIRE_BIS>"\n" 							{ BEGIN(INITIAL); }
<QUOTE>\" 										{ BEGIN(INITIAL); }

<COMMENTAIRE,COMMENTAIRE_BIS,QUOTE>.			{ ; }
<COMMENTAIRE,QUOTE>[ \t\n] 						{ ; }

{character} 					{ 	nb_character += strlen(yytext); yylval.character = yytext[1]; return CHARACTER; }
{num} 							{ 	nb_character += strlen(yytext); sscanf(yytext, "%d", &(yylval.num)); return NUM; }
{type}  						{ 	nb_character += strlen(yytext); strcpy(yylval.type, yytext); return SIMPLETYPE; }
{eq} 							{ 	nb_character += strlen(yytext); strcpy(yylval.comp, yytext); return EQ; }
{order} 						{ 	nb_character += strlen(yytext); strcpy(yylval.comp, yytext); return ORDER; }
{addsub} 						{ 	nb_character += strlen(yytext); yylval.character = yytext[0]; return ADDSUB; }
{divstar} 						{ 	nb_character += strlen(yytext); yylval.character = yytext[0]; return DIVSTAR; }
{struct} 						{ 	nb_character += strlen(yytext); strcpy(yylval.type, yytext); return STRUCT; }
{or} 							{ 	nb_character += strlen(yytext); return OR; }
{and} 							{ 	nb_character += strlen(yytext); return AND; }
{void} 							{ 	nb_character += strlen(yytext); strcpy(yylval.type, yytext); return VOID; }
{print} 						{ 	nb_character += strlen(yytext); return PRINT; }
{if} 							{ 	nb_character += strlen(yytext); return IF; }
{else} 							{ 	nb_character += strlen(yytext); return ELSE; }
{while} 						{ 	nb_character += strlen(yytext); return WHILE; }
{return} 						{ 	nb_character += strlen(yytext); return RETURN; }
{reade} 						{ 	nb_character += strlen(yytext); return READE; }
{readc} 						{ 	nb_character += strlen(yytext); return READC; }
{ident} 						{ 	ident_character = nb_character + 1; nb_character += strlen(yytext); strcpy(yylval.ident, yytext); return IDENT; }

^.+/\n							{	strcpy(line, yytext); REJECT; }		

\n 								{   nb_character = 0; }

{blanc}							{   nb_character += strlen(yytext); }								

. 								{   nb_character += strlen(yytext); return yytext[0]; }

%%

int yywrap(void) { return 1; }

