%{

    #include "src/abstract_tree.h"
    #include "src/generate_asm.h"
    #include "src/symbols.h"
    #include <getopt.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    void yyerror (char const *s);
    extern char line[512];
    extern int last_string;
    extern int nb_character;
    extern int yylineno;
    int yylex();
    int type_global;
    Node *tree;

    /* Pour les options -s, -t, -h */
    enum options {
        t,
        s, 
        h
    };

%}

%define parse.error verbose // %error-verbose
%start Prog

%union {
    int num;
    char character;
    char ident[64];
    char type[10];
    char comp[3];
    Node *node;
}

%token <num> NUM
%token <character> CHARACTER ADDSUB DIVSTAR
%token <ident> IDENT
%token <comp> EQ ORDER
%token <type> SIMPLETYPE VOID STRUCT

%type <node> Prog DeclVars Declarateurs
             Type TypesVars DeclChamps
             DeclFoncts DeclFonct
             EnTeteFonct Parametres ListTypVar
             Corps SuiteInstr 
             Instr
             Exp
             TB FB M E T 
             F LValue
             Arguments ListExp

%token  OR AND PRINT IF ELSE WHILE RETURN READE READC
%right "then" ELSE

%%
    Prog: TypesVars DeclFoncts  {   $$ = makeNode(Program);
                                    addChild($$, $1);
                                    addChild($$, $2);
                                    
                                    tree = $$;
                                }
        ;

    TypesVars: TypesVars Type Declarateurs ';'                  {   $$ = $1;
                                                                    addChild($$, $2);
                                                                    addChild($2, $3);
                                                                }
            | TypesVars STRUCT IDENT '{' DeclChamps '}' ';'     {   $$ = $1;
                                                                    Node *s = makeNode(Type);
                                                                    strcpy(s->u.type, $2);
                                                                    
                                                                    Node *s_name = makeNode(StructName);
                                                                    strcpy(s_name->u.identifier, $3);

                                                                    addChild(s, s_name);
                                                                    addChild(s, $5);

                                                                    addChild($$, s);
                                                                    /* Le node EndDeclChamps nous permet de savoir quand on a terminé de déclarer
                                                                    les champs d'une structure. Cela permettra lorsqu'on remplira la table
                                                                    des symboles de ne pas intégrer les champs d'une structure, et de reprendre
                                                                    la déclaration des variables à la fin de la déclaration de ces champs */
                                                                    addChild($$, makeNode(EndDeclChamps));
                                                                }
            |  {   $$ = makeNode(GlobalVars); }
            ;

    Type: SIMPLETYPE    {   $$ = makeNode(Type);
                            strcpy($$->u.type, $1);
                        }
        | STRUCT IDENT  {   $$ = makeNode(Type);
                            strcpy($$->u.type, $1);

                            Node *structName = makeNode(StructName);
                            strcpy(structName->u.structName, $2);
                            
                            addChild($$, structName);
                        }
        ;

    Declarateurs: Declarateurs ',' IDENT    {   Node *ident = makeNode(Identifier);
                                                strcpy(ident->u.identifier, $3);
                                                addSibling($$, ident);
                                            }

                | IDENT                     {   $$ = makeNode(Identifier);
                                                strcpy($$->u.identifier, $1); 
                                            }
                ;


    DeclChamps : DeclChamps SIMPLETYPE Declarateurs ';' {   Node *type = makeNode(Type);
                                                            strcpy(type->u.type, $2);
                                                            
                                                            addChild(type, $3);
                                                            addChild($$, type);
                                                        }
               |  SIMPLETYPE Declarateurs ';'           {   $$ = makeNode(DeclChamps); 
                                                            Node *type = makeNode(Type);
                                                            strcpy(type->u.type, $1);

                                                            addChild($$, type);
                                                            addChild(type, $2);
                                                        }
            ;

    DeclFoncts: DeclFoncts DeclFonct    {   addChild($$, $2); }
              | DeclFonct               {   $$ = makeNode(DeclFoncts);
                                            addChild($$, $1);
                                        }
              ;

    DeclFonct: EnTeteFonct Corps    {   $$ = $1;
                                        addChild($$, $2);
                                    }
             ;

    EnTeteFonct: Type IDENT '(' Parametres ')'  {   $$ = makeNode(DeclFonct);
                                                    Node *nom = makeNode(FonctName);
                                                    strcpy(nom->u.identifier, $2);

                                                    addChild($$, $1);
                                                    addChild($$, nom);
                                                    addChild($$, $4);
                                                }
               | VOID IDENT '(' Parametres ')'  {   $$ = makeNode(DeclFonct);

                                                    Node *f_type = makeNode(Type);
                                                    Node *f_name = makeNode(FonctName);
                                                    
                                                    strcpy(f_type->u.type, $1);
                                                    strcpy(f_name->u.identifier, $2);

                                                    addChild($$, f_type);
                                                    addChild($$, f_name);
                                                    addChild($$, $4);
                                                }
                ;

    Parametres: VOID        {   $$ = makeNode(Parameters);
                                strcpy($$->u.identifier, $1);
                            }
              | ListTypVar  {   $$ = makeNode(Parameters);
                                strcpy($$->u.identifier, "Not void");
                                addChild($$, $1);
                            }
              ;

    ListTypVar: ListTypVar ',' Type IDENT           {   $$ = $1;
                                                        addSibling($$, $3);
                                                        
                                                        Node *ident = makeNode(Identifier);
                                                        strcpy(ident->u.identifier, $4);

                                                        addChild($3, ident);
                                                    }
              | Type IDENT                          {   Node *ident = makeNode(Identifier);
                                                        strcpy(ident->u.identifier, $2);

                                                        addChild($1, ident);
                                                    }           
              ;

    Corps: '{' DeclVars SuiteInstr '}'  {   $$ = makeNode(Corps);
                                            addChild($$, $2);
                                            addChild($$, $3);
                                        }
         ;

    DeclVars: DeclVars Type Declarateurs ';'    {   $$ = $1;
                                                    addChild($$, $2);
                                                    addChild($2, $3);
                                                }
            | {   $$ = makeNode(DeclVars); }
            ;

    SuiteInstr: SuiteInstr Instr    {   $$ = $1; 
                                        addChild($$, $2);
                                    }
              |                     {   $$ = makeNode(Instructions); }
              ; 


    Instr: LValue '=' Exp ';'               {   $$ = makeNode(Assignement); 
                                                addChild($$, $1);
                                                addChild($$, $3);
                                            } 
         |  READE '(' IDENT ')' ';'         {   $$ = makeNode(Reade); 
                                                Node *child = makeNode(Identifier);
                                                strcpy(child->u.identifier, $3);
                                                addChild($$, child);
                                            } 
         |  READC '(' IDENT ')' ';'         {   $$ = makeNode(Readc); 
                                                Node *child = makeNode(Identifier);
                                                strcpy(child->u.identifier, $3);
                                                addChild($$, child);
                                            } 
         |  PRINT '(' Exp ')' ';'           {   $$ = makeNode(Print); 
                                                addChild($$, $3);
                                            } 
         |  IF '(' Exp ')' Instr            %prec "then" 
                                            {   $$ = makeNode(If);
                                                Node *child_condition = makeNode(Condition);
                                                addChild(child_condition, $3);

                                                Node *child_then = makeNode(ThenIf);
                                                addChild(child_then, $5);

                                                addChild($$, child_condition);
                                                addChild($$, child_then);
                                            } 
         |  IF '(' Exp ')' Instr ELSE Instr {   $$ = makeNode(If);
                                                Node *child_condition = makeNode(Condition);
                                                addChild(child_condition, $3);

                                                Node *child_then = makeNode(ThenIf);
                                                addChild(child_then, $5);

                                                Node *child_else = makeNode(Else);
                                                addChild(child_else, $7);
                                                
                                                addChild($$, child_condition);
                                                addChild($$, child_then);
                                                addChild($$, child_else);
                                            } 
         |  WHILE '(' Exp ')' Instr         {   $$ = makeNode(While);
                                                Node *child_condition = makeNode(Condition);
                                                addChild(child_condition, $3);

                                                Node *child_then = makeNode(ThenWhile);
                                                addChild(child_then, $5);

                                                addChild($$, child_condition);
                                                addChild($$, child_then);
                                            } 
         |  IDENT '(' Arguments ')' ';'     {   $$ = makeNode(FunctionCall);
                                                strcpy($$->u.identifier, $1);
                                                addSibling($$, $3); 
                                            } 
         |  RETURN Exp ';'                  {   $$ = makeNode(Return); 
                                                addChild($$, $2);
                                            } 
         |  RETURN ';'                      {   $$ = makeNode(Return); }
         |  '{' SuiteInstr '}'              {   $$ = $2; } 
         |  ';'                             {   $$ = makeNode(Instructions); }
         ;

    Exp :  Exp OR TB    {   $$ = makeNode(Or);
                            addChild($$, $1);
                            addChild($$, $3);
                        }
        |  TB           {   $$ = $1; }
        ;

    TB  :  TB AND FB    {   $$ = makeNode(And);
                            addChild($$, $1);
                            addChild($$, $3);
                        }
        |  FB           {   $$ = $1; }
        ;

    FB  :  FB EQ M      {   $$ = makeNode(Comparison);
                            strcpy($$->u.comp, $2);
                            addChild($$, $1);
                            addChild($$, $3);
                        }
        |  M            {   $$ = $1; }
        ;

    M   :  M ORDER E    {   $$ = makeNode(Order);
                            strcpy($$->u.comp, $2);
                            addChild($$, $1);
                            addChild($$, $3);
                        }
        |  E            {   $$ = $1; }
        ;

    E   :  E ADDSUB T   {   $$ = makeNode(($2 == '+') ? Addition : Substraction);
                            $$->u.character = $2;
                            addChild($$, $1);
                            addChild($$, $3);
                        }
        |  T            {   $$ = $1; }
        ;    

    T   :  T DIVSTAR F  {   if ($2 == '/') {
                                $$ = makeNode(Division);
                            }

                            else if ($2 == '%') {
                                $$ = makeNode(Reste);
                            }

                            else {
                                $$ = makeNode(Multiplication);
                            }

                            $$->u.character = $2;
                            addChild($$, $1);
                            addChild($$, $3);
                        }
        |  F            {   $$ = $1; }
        ;


    F   :  ADDSUB F     {   $$ = makeNode(($1 == '+') ? Addition : Substraction);
                            addChild($$, $2);
                        }
        |  '!' F        {   $$ = makeNode(Not); 
                            addChild($$, $2); 
                        }
        |  '(' Exp ')'  {   $$ = $2; }
        |  NUM          {   $$ = makeNode(IntLiteral);
                            $$->u.integer = $1; }
        |  CHARACTER    {   $$ = makeNode(CharLiteral);
                            $$->u.character = $1; }
        |  LValue       {   $$ = $1; }
        |  IDENT '(' Arguments ')'  {   $$ = makeNode(FunctionCall);
                                        strcpy($$->u.identifier, $1);
                                        addSibling($$, $3); }
        ;

    LValue: IDENT   {   $$ = makeNode(Identifier);
                        strcpy($$->u.identifier, $1);
                    }
          ;

    Arguments: ListExp  { $$ = $1; }
             |          { $$ = makeNode(NoArguments); }
             ;

    ListExp: ListExp ',' Exp    {   $$ = $1;
                                    addChild($$, $3);
                                }
            |  Exp              {   $$ = makeNode(Arguments);
                                    addChild($$, $1); }
            ;
%%


void yyerror(char const *s) {
    int i;

    fprintf(stderr, "Error near line %d character %d : %s\n", yylineno, nb_character, s); 
    printf("%s\n", line);

    for (i = 0 ; i < nb_character - 1; i++) {
        printf(" ");
    }

    printf("^\n");
}


int main(int argc, char *const argv[]) {
    SymbolTable *global_symbol_table, *local_symbol_table;
    StructTable *struct_table;
	int res, long_index = 0, i, global_size = 0, local_size = 0, struct_size = 0;
    int options[3] = {0}, val;
	const char *optstring = "tsh";
    FILE *asm_file = NULL;

    /* Create, then close, an empty asm file (TEMP) */
    asm_file = fopen("_anonymous.asm", "w+");

    if (!asm_file) {
        printf("Run out of memory\n");
        exit(3);
    }


    static struct option long_options[] = {
        {"tree",    no_argument,    0,  't' },
        {"symtabs", no_argument,    0,  's' },
        {"help",    no_argument,    0,  'h' },
        {0,         0,              0,  0   }
    };

	while (EOF != (val = getopt_long(argc, argv, optstring, long_options, &long_index))) {
		switch(val) { 
		case 't':
			options[t] = 1; break;

		case 's':
			options[s] = 1; break;

		case 'h':
			printf("--------------------- HELP ----------------------\n\n");
            printf("— Pour lancer le compilateur, on utilise la ligne de commande \"./tpcc [OPTIONS]\" et le compilateur va lire sur l’entrée standard\n\n");
            printf("— Options :\n");
            printf("\t-t, --tree affiche l’arbre abstrait sur la sortie standard\n");
            printf("\t-s, --symtabs affiche toutes les tables des symboles sur la sortie standard\n");
            printf("\t-h, --help affiche une description de l’interface utilisateur et termine l’exécution\n\n");
            printf("— Un script de déploiement des tests nommé \"test.sh\" se trouve dans le dossier ./src/ (\"bash src/test.sh\") qui produit un rapport unique \"test_results.txt\"\n");
            printf(" (situé dans le dossier \"./test/\") donnant les résultats de tous les fichiers des 4 dossiers tests du dossier \"./test/\"\n");
            printf("\n-------------------------------------------------\n");
            return 0;

		case '?':
            return 3;
            break;
		}
	}

    res = yyparse();
    
	if (res == 0) {
        if (options[t]) {
            printf("||------------------------- PRINT TREE --------------------------||\n\n");
            printTree(tree);            
            printf("\n||---------------------------------------------------------------||\n\n");
        }

        
        local_symbol_table = malloc(MAXSYMBOLS * sizeof * local_symbol_table);
        global_symbol_table = malloc(MAXSYMBOLS * sizeof * global_symbol_table);
        struct_table = malloc(MAXSYMBOLS * sizeof * struct_table);

        if (!global_symbol_table || !local_symbol_table || !struct_table) {
            printf("Run out of memory\n");
            exit(3);
        }

        create_symbols_tables(tree, &global_symbol_table, &local_symbol_table, &struct_table, &global_size, &local_size, &struct_size, options[s]);

        if (!has_main_function(global_symbol_table, global_size)) {
            printf("Warning : semantic error, function \"main\" undefined\n");
            exit(2);
        }
        
        if (options[s]) {
            if (local_symbol_table) {
                print_symbol_table(local_symbol_table, local_size);
            }

            printf("\n|----------------- GLOBAL SYMBOLS TABLE -----------------|\n\n");
            print_symbol_table(global_symbol_table, global_size);
            print_struct_table(struct_table, struct_size);
            printf("|---------------------------------------------------------|\n");            
        }

        write_asm_file(asm_file, tree, &global_symbol_table, &global_size);
        fclose(asm_file);
        deleteTree(tree);
        free(local_symbol_table);
        free(global_symbol_table);
        free(struct_table);
	}
    
	return res;
}
