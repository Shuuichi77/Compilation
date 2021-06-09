#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "generate_asm.h"

/* All the global variables for the function_instructions() function which */
char g_type[10];        
char g_type_first_var[10];
char g_first_var[64];
char g_struct_name[64];
char g_function_called[64];
int g_nb_function = 0;
int g_declaring_struct = 0;
int g_is_first_var = 1;
int g_is_first_type = 0;
int g_is_parameter = 0;
int g_last_registre = 0;
int g_last_param_registre = 0;
int g_jump = 0;
int g_jump_while = 0;
int g_jump_if = 0;
int g_if_has_else = 0;

int g_condition;
int g_lst_jmp_then[64]; /* To remember in which function we should jmp to when encountering a "Then" Node */
int g_nb_then = 0;

int g_in_if = 0;
int g_in_else = 0;
int g_in_while = 0;
int g_returned = 0;
char g_returned_type[5];

int padd_size_16_bit = 0;
char *local_registre_4bit[9] = {"ebx", "r12d", "r13d", "r14d", "r15d", "r8d", "r9d", "r10d", "r11d"};
char *local_registre_1bit[9] = {"bl", "r12b", "r13b", "r14b", "r15b", "r8b", "r9b", "r10b", "r11b"};
char *param_registre_4bit[8] = {"edi", "esi", "edx", "ecx", "r8d", "r9d", "r10d", "r11d"};
char *param_registre_1bit[8] = {"dil", "sil", "dl", "cl", "r8b", "r9b", "r10b", "r11b"};

/* ------------------------------------------------------- */

int write_asm_file(FILE* file, Node* node, SymbolTable **g_symtab, int *g_size) {
    FunctionInfo functions[NB_MAX_FUNCTION]; /* Max nb of function which can be declared in a file */

    write_intro(file);
    node = write_typesvars(file, node);

    fprintf(file, "\nsection .text\n");
    fprintf(file, "global main\n");

    for (Node *child = node->firstChild ; child != NULL ; child = child->nextSibling) {
        g_nb_function++;

        if (g_nb_function >= NB_MAX_FUNCTION) {
            printf("Error : nb max of function allowed (%d)\n", NB_MAX_FUNCTION);
            exit(3);
        }
        write_function(file, child, g_symtab, g_size, functions);
    }
}

/* ------------------------------------------------------- */

int write_intro(FILE *file) {
    fprintf(file, "extern printf\n");
    fprintf(file, "extern scanf\n\n");
    fprintf(file, "section .data\n");
    fprintf(file, "\tprintInt: db \"%%d\", 10, 0\n");
    fprintf(file, "\tprintChar: db \"%%c\", 10, 0\n");
    fprintf(file, "\treadInt: db \"%%d\", 0\n");
    fprintf(file, "\treadChar: db \"%%c\", 0\n\n");
    fprintf(file, "section .bss\n");
    fprintf(file, "\tscanfInt: resd 1\n");
    fprintf(file, "\tscanfChar: resb 1\n");
}

/* ------------------------------------------------------- */

Node* write_typesvars(FILE *file, Node *node) {
    char str[128];
    Node *res;

	switch (node->kind) {
		case Identifier:
            if (!g_declaring_struct) {
                write_global_variable(file, node->u.identifier, g_type);
            }
            
            break;

        case Type: 
            strcpy(g_type, node->u.type);
            break;

        case DeclFoncts:
            return node;

        case StructName:
            strcpy(g_struct_name, node->u.identifier);
            break;

        case DeclChamps:
            g_declaring_struct = 1;
            break;
        
        case EndDeclChamps:
            g_declaring_struct = 0;
            break;

        
    }

	for (Node *child = node->firstChild ; child != NULL ; child = child->nextSibling) {
		res = write_typesvars(file, child);

        if (res) {
            return res;
        }
	}

    return NULL;
}

/* ------------------------------------------------------- */

void write_global_variable(FILE *file, char *name, char *type) { 
    if (!strcmp(type, "char") || !strcmp(type, "int")) {
        fprintf(file, "\t%s: %s 1\n", name, !strcmp(type, "int") ? "resd" : "resb");
    }
}

/* ------------------------------------------------------- */

int write_function(FILE *file, Node *node, SymbolTable **g_symtab, int *g_size, FunctionInfo functions[]) {
    SymbolTable *p_symtab = NULL, *l_symtab = NULL;
    int push_size = 0, p_size = 0, l_size = 0, i;

    l_symtab = malloc(MAXSYMBOLS * sizeof * l_symtab);
    p_symtab = malloc(MAXSYMBOLS * sizeof * p_symtab);

    if (!p_symtab || !l_symtab) {
        printf("Run out of memory\n");
        exit(3);
    }

    /* Write function's name in asm file and go throught the AST until function's Body */
    g_is_first_type = 1;
    
    functions[g_nb_function - 1].nb_parameter = 0;
    node = write_declfunc(file, node, &p_symtab, &p_size, &functions[g_nb_function - 1]); /* node = Body's function */

    /* Go throught the AST of function's DeclVar */
    node = push_local_variables(file, node, &l_symtab, &l_size, &push_size); /* node = First body's instruction of the function */
    padd_size_16_bit = 16 - (push_size % 16);
    fprintf(file, "\tsub rsp, %d\n", push_size + padd_size_16_bit);
    
    /* Then go throught the AST of function's Body's Instructions */
    g_is_first_var = 1;
    g_last_registre = 0;
    g_returned = 0;
    function_instructions(file, node, *g_symtab, p_symtab, l_symtab, *g_size, p_size, l_size, functions);

    /* If function return wasn't void, and returned nothing */
    if (strcmp(functions[g_nb_function - 1].return_type, "void") && !g_returned) {
        printf("Error : semantic error, control reaches end of non-void function in \"%s()\" function \n", functions[g_nb_function - 1].name);
        exit(2);
    }

    else if (!strcmp(functions[g_nb_function - 1].return_type, "char") && !strcmp(g_returned_type, "int")) {
        printf("Warning : Returned an int whereas the return value should have been char in function \"%s\"\n", functions[g_nb_function - 1].name);
    }
    
    free(l_symtab);
    free(p_symtab);

    return 1;
}

/* ------------------------------------------------------- */

Node* write_declfunc(FILE *file, Node *node, SymbolTable **table, int *size, FunctionInfo *function) {
    Node* res;

    // printf("%s\n", StringFromKind[node->kind]);

    switch(node->kind) {
        case Corps:
            return node;

        case Type:
            if (g_is_first_type) {
                strcpy(function->return_type, node->u.type);
                g_is_first_type = 0;
            }

            else {
                function->nb_parameter++;
                strcpy(function->parameters[function->nb_parameter - 1].type, node->u.type);
            }

            strcpy(g_type, node->u.type);
            break;

        case Identifier:    
            add_var(table, size, *size, SIMPLE, node->u.identifier, g_type, node->line, node->character);
            strcpy(function->parameters[function->nb_parameter - 1].name, node->u.identifier);
            break;

        case FonctName:
            fprintf(file, "\n%s:\n", node->u.identifier);
            strcpy(function->name, node->u.identifier);
            PUSH("rbp");
            MOV("rbp", "rsp");
            break;
    }

    for (Node *child = node->firstChild ; child != NULL ; child = child->nextSibling) {
		res = write_declfunc(file, child, table, size, function);

        if (res) {
            return res;
        }
	}

    return NULL;
}

/* ------------------------------------------------------- */

Node* push_local_variables(FILE *file, Node *node, SymbolTable **table, int *size, int *push_size) {
    Node *res;

    // printf("%s\n", StringFromKind[node->kind]);

    switch(node->kind) {
        case Instructions:
            return node;
        
        case Type:
            strcpy(g_type, node->u.type);
            break;

        case Identifier:
            *push_size += (!strcmp(g_type, "char") ? 1 : 4);
            add_var(table, size, *size, SIMPLE, node->u.identifier, g_type, node->line, node->character);
            break;
    }

    for (Node *child = node->firstChild ; child != NULL ; child = child->nextSibling) {
		res = push_local_variables(file, child, table, size, push_size);

        if (res) {
            return res;
        }
	}

    return NULL;
}


/* ------------------------------------------------------- */

int var_offset(SymbolTable *table, int size, int check_parameters, char *name) {
    int res = 0, i;

    for (i = 0; i < size ; i++) {
        res += (!strcmp(table[i].type, "char") || check_parameters ? 1 : 4);

        if (!strcmp(table[i].name, name)) {
            strcpy(g_type, table[i].type);
            return res;
        }
    }

    return -1;
}

/* ------------------------------------------------------- */

char* global_var_type(char *name, SymbolTable *table, int size) {
    int i;

    for (i = 0; i < size ; i++) {
        if (!strcmp(table[i].name, name)) {
            return table[i].type;
        }
    }
}
/* ------------------------------------------------------- */

char* get_first_var(char *name, SymbolTable *g_symtab, SymbolTable *p_symtab, SymbolTable *l_symtab, int g_size, int p_size, int l_size) {
    int offset;
    int length;
    char *str = NULL;

    if ((offset = var_offset(p_symtab, p_size, 1, name)) > 0) {
        return (!strcmp(g_type, "char") ? param_registre_1bit[offset - 1] : param_registre_4bit[offset - 1]);
    }

    else if ((offset = var_offset(l_symtab, l_size, 0, name)) > 0) {
        length = snprintf(NULL, 0, "%d", offset);
        if ((str = malloc(length + 8 + 1)) == NULL) {
            exit(EXIT_FAILURE);
        }

        snprintf(str, length + 8 + 1, "[rbp - %d]", offset);
        return str;
    }

    else {
        strcpy(g_type, global_var_type(name, g_symtab, g_size));

        if ((str = malloc(strlen(name) + 2 + 1)) == NULL) {
            exit(EXIT_FAILURE);
        }

        snprintf(str, strlen(name) + 2 + 1, "[%s]", name);
        return str;
    }
}
/* ------------------------------------------------------- */

int parameter_is_same_type(FunctionInfo functions[], char *f_name, int parameter) {
    int i, j;

    for (i = 0 ; i < g_nb_function ; i++) {
        if (!strcmp(functions[i].name, f_name)) {
           
            if (!strcmp(functions[i].parameters[parameter].type, "")) {
                return 1;
            }

            return 0;
                
            
        }
    }   

    /* Shouldn't arrive here since we already know the function does exist (or else there would already be a semantic error previously) */
    return 0;
}

/* ------------------------------------------------------- */

char* return_comparison_or_order(char *comp) {
    if (!strcmp(comp, "==")) {
        return "e";
    }

    else if (!strcmp(comp, "!=")) {
        return "ne";
    }
    
    else if (!strcmp(comp, ">")) {
        return "g";
    }

    else if (!strcmp(comp, ">=")) {
        return "ge";
    }

    else if (!strcmp(comp, "<")) {
        return "l";
    }

    else if (!strcmp(comp, "<=")) {
        return "le";
    }

    return NULL;

}

/* ------------------------------------------------------- */

int function_instructions(FILE *file, Node *node, SymbolTable *g_symtab, SymbolTable *p_symtab, SymbolTable *l_symtab, int g_size, int p_size, int l_size, FunctionInfo functions[]) {
    char str[64];
    
    if (node->kind == If) {
        g_in_if++;
        g_condition = If;
        g_lst_jmp_then[g_nb_then++] = g_jump_if++;

        if (node->firstChild->nextSibling->nextSibling) {
            g_if_has_else = 1;
        }
    }

    else if (node->kind == Else) {
        /* If in an "if else", and not an "if else if", and also if not in another "if" 
        It will help to know if we did return something, which wasn't in an "if" (it can be in an "if else", but not an "if else if" */
        if (node->firstChild->kind != If && g_in_if == 1) {
            g_in_else = 1;
        }

        g_if_has_else = 0;
        NEW_ELSE(g_lst_jmp_then[g_nb_then - 1]);
    }

    /* Encounter a While -> create a loop "CX" and enter it to check the condition (X = i-th while) */
    else if (node->kind == While) {
        g_in_while = 1;
        g_condition = While;
        g_lst_jmp_then[g_nb_then++] = g_jump_while++;
        NEW_WHILE(g_jump_while - 1);
    }

    for (Node *child = node->firstChild ; child != NULL ; child = child->nextSibling) {
		function_instructions(file, child, g_symtab, p_symtab, l_symtab, g_size, p_size, l_size, functions);
	}

    // printf("%s\n", StringFromKind[node->kind]);
    
    switch(node->kind) {
        case Identifier:
            strcpy(str, get_first_var(node->u.identifier, g_symtab, p_symtab, l_symtab, g_size, p_size, l_size));
            
            if (g_is_first_var) {
                strcpy(g_first_var, str);
                strcpy(g_type_first_var, g_type);
                g_is_first_var = 0;
            }
            
            if (g_is_parameter) {
                /* If using a int variable whereas the parameter should be a char, print a warning */
                !strcmp(g_type, "char") ? MOV(param_registre_1bit[g_last_param_registre++], str) :
                                          MOV(param_registre_4bit[g_last_param_registre++], str);
            }

            else {
                !strcmp(g_type, "char") ? MOV(local_registre_1bit[g_last_registre++], str) :
                                          MOV(local_registre_4bit[g_last_registre++], str);
            }
            
            break;

        case CharLiteral:
            strcpy(g_type, "char");
            sprintf(str, "'%c'", node->u.character);
            if (g_is_parameter) {
                MOV(param_registre_1bit[g_last_param_registre++], str);
            }

            else {
                MOV(local_registre_1bit[g_last_registre++], str);
            }
            
            break;

        case IntLiteral:
            strcpy(g_type, "int");
            sprintf(str, "%d", node->u.integer);

            if (g_is_parameter) {
                MOV(param_registre_4bit[g_last_param_registre++], str);
            }

            else {
                MOV(local_registre_4bit[g_last_registre++], str);
            }

            break;
        
        case Addition:
            ADD(local_registre_4bit[g_last_registre - 2], local_registre_4bit[g_last_registre - 1]);
            g_last_registre--;
            break;

        case Reste:
            MOV("eax", local_registre_4bit[g_last_registre - 2]);
            fprintf(file, "\tcdq\n");
            IDIV(local_registre_4bit[g_last_registre - 1]);
            MOV(local_registre_4bit[g_last_registre - 2], "edx");
            
            g_last_registre--;
            break;

        case Division:
            MOV("eax", local_registre_4bit[g_last_registre - 2]);
            fprintf(file, "\tcdq\n");
            IDIV(local_registre_4bit[g_last_registre - 1]);
            MOV(local_registre_4bit[g_last_registre - 2], "eax");
            
            g_last_registre--;
            break;

        case Multiplication:
            IMUL(local_registre_4bit[g_last_registre - 2], local_registre_4bit[g_last_registre - 1]);
            g_last_registre--;
            break;
        
        case Substraction:
            SUB(local_registre_4bit[g_last_registre - 2], local_registre_4bit[g_last_registre - 1]);
            g_last_registre--;
            
            break;

        case Assignement:
            /* Assigning a char variable with an int */
            if (!strcmp(g_type_first_var, "char") && !strcmp(g_type, "int")) {
                printf("Warning : assigning char variable with an int\n");
            }

            !strcmp(g_type_first_var, "char") ? MOV(g_first_var, local_registre_1bit[g_last_registre - 1]) :
                                                MOV(g_first_var, local_registre_4bit[g_last_registre - 1]);
            g_last_registre = 0;
            g_is_first_var = 1;
            break;

        case Print:
            write_print(file, g_type);

            g_last_registre = 0;
            g_is_first_var = 1;
            break;

        case Reade:
            /* Assigning a char variable with an int */
            if (!strcmp(g_type, "char")) {
                printf("Warning : using reade on a char variable\n");
            }

            write_read(file, g_first_var, g_type);
            
            g_last_registre = 0;
            g_is_first_var = 1;
            break;
        
        case Readc:
            write_read(file, g_first_var, g_type);

            g_last_registre = 0;
            g_is_first_var = 1;
            break;


        case FunctionCall:
            strcpy(g_function_called, node->u.identifier);
            g_is_parameter = 1;
            break;

        case NoArguments:
        case Arguments:
            CALL(g_function_called);
            g_last_param_registre = 0;
            g_is_parameter = 0;

            !strcmp(g_type, "char") ? MOV(local_registre_1bit[g_last_registre++], "al") :
                                      MOV(local_registre_4bit[g_last_registre++], "eax");

            break;

        case Order:
        case Comparison:
            CMP(local_registre_4bit[g_last_registre - 2], local_registre_4bit[g_last_registre - 1]);
            g_last_registre -= 2;
            
            SET(return_comparison_or_order(node->u.comp), local_registre_1bit[g_last_registre++]);
            break;

        case Or:
            /* If 1st register TRUE (== 1), jmp to "g_jump" */
            CMP(local_registre_4bit[g_last_registre - 2], "1");
            JMP_L("e", g_jump);

            /* If 2nd register TRUE (== 1), jmp to "g_jump" */
            CMP(local_registre_4bit[g_last_registre - 1], "1");
            JMP_L("e", g_jump);

            g_last_registre -= 2;

            /* "OR" is FALSE */
            /* Put 0 in local_registre_1bit[g_last_registre] */
            MOV("rax", "1");
            CMP("rax", "1");
            SET("ne", local_registre_1bit[g_last_registre]);
            JMP_L("mp", g_jump + 1);
            
            /* "OR" is TRUE */
            /* Put 1 in local_registre_1bit[g_last_registre] */
            NEW_L(g_jump++);
            MOV("rax", "1");
            CMP("rax", "1");
            SET("e", local_registre_1bit[g_last_registre++]);

            /* End of "OR" */
            NEW_L(g_jump++);
            break;

        case And:
            /* If 1st register FALSE (== 0), jmp to "g_jump" */
            CMP(local_registre_4bit[g_last_registre - 2], "0");
            JMP_L("e", g_jump);

            /* If 2nd register FALSE (== 0), jmp to "g_jump" */
            CMP(local_registre_4bit[g_last_registre - 1], "0");
            JMP_L("e", g_jump);

            g_last_registre -= 2;

            /* "AND" is TRUE */
            /* Put 1 in local_registre_1bit[g_last_registre] */
            MOV("rax", "1");
            CMP("rax", "1");
            SET("e", local_registre_1bit[g_last_registre]);
            JMP_L("mp", g_jump + 1);
            
            /* "AND" is FALSE */
            /* Put 0 in local_registre_1bit[g_last_registre] */
            NEW_L(g_jump++);
            MOV("rax", "1");
            CMP("rax", "1");
            SET("ne", local_registre_1bit[g_last_registre++]);

            /* End of "AND" */
            NEW_L(g_jump++);
            break;

        
        case Condition:
            /* Check if condition is TRUE */
            CMP(local_registre_4bit[g_last_registre - 1], "1");

            /* If the condition is a WHILE */
            if (g_condition == While) {
                /* If TRUE, go to "DX", loop in while (X = i-th while) */
                JMP_LOOP("e", g_jump_while - 1);

                /* If FALSE, go to "EX" end of while (X = i-th while) */
                JMP_ENDWHILE("mp", g_jump_while - 1);
                NEW_LOOP(g_jump_while - 1);
            }

            else if (g_condition == If) {
                if (g_if_has_else) {
                    JMP_ELSE("ne", g_jump_if - 1);
                }

                else {
                    JMP_ENDIF("ne", g_jump_if - 1);
                }
            }

            break;

        case ThenWhile:
            /* End of while loop, check if condition is still true so go in "CX" */
            JMP_WHILE("mp", g_lst_jmp_then[g_nb_then - 1]);
            break;

        case While:  
            g_in_while = 0;
            g_nb_then--;
            NEW_ENDWHILE(g_lst_jmp_then[g_nb_then]);  
            break;

        case ThenIf:
            JMP("ENDIF", g_lst_jmp_then[g_nb_then - 1]);
            break;

        case If:
            g_in_if--;
            g_nb_then--;
            NEW_ENDIF(g_lst_jmp_then[g_nb_then]); 
            break;

        case Else:
            g_in_else = 0;
            break;

        case Return:
            if ((!g_in_if && !g_in_while) || g_in_else) {
                g_returned = 1;
            }
            
            strcpy(g_returned_type, g_type);

            MOV("eax", "ebx");
            MOV("rsp", "rbp");
            POP("rbp");
            fprintf(file, "\tret\n");
            return 1;
    }
    
    // printf("\n");

    return 1;
}

/* ------------------------------------------------------- */

void write_read(FILE *file, char *var, char *type) {
    fprintf(file, "\tmov rdi, %s\n", (!strcmp(type, "char")) ? "readChar" : "readInt");
    fprintf(file, "\tmov rsi, %s\n", (!strcmp(type, "char")) ? "scanfChar" : "scanfInt");
    fprintf(file, "\tmov rax, 0\n");
    fprintf(file, "\tcall scanf\n");
    fprintf(file, "\tmov %s, [%s]\n", (!strcmp(type, "char")) ? "bl" : "ebx", (!strcmp(type, "char")) ? "scanfChar" : "scanfInt");
    fprintf(file, "\tmov %s, %s\n", var, (!strcmp(type, "char")) ? "bl" : "ebx");
}

/* ------------------------------------------------------- */

void write_print(FILE *file, char *type) {
    fprintf(file, "\tmov rdi, %s\n", (!strcmp(type, "char")) ? "printChar" : "printInt");
    (!strcmp(type, "char")) ? fprintf(file, "\tmov %s, %s\n", "sil", "bl") :
                              fprintf(file, "\tmov %s, %s\n", "rsi", "rbx");
    fprintf(file, "\tmov rax, 0\n");
    fprintf(file, "\tcall printf\n");
    
}
/* ------------------------------------------------------- */

