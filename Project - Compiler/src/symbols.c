#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "symbols.h"

/* Variables for "create_table_symbols()" : */
char g_type[10];                /* Keep the the ident type for the next "Identifier" node */
char g_struct_name[64];         /* Keep the struct name when we declare a new struct */
char g_current_function_id[64]; /* Keep the function's name we are currently declaring to prevent calling this same function inside itself */
int g_ident_type = -1;          /* Keep the Identifier type : function, structure, or simple type (char/int) */
int g_declaring = DECLARING_VAR;/* Allow us to know what to do when we encounter an "Identifier" node 
                                    - Declaring global variable, so add the Identifier in the global symbol table
                                    - Declaring local variable, so add the Identifier in the local symbol table
                                    - Declaring structure's parameters, so add the next Identifier in the struct table 
                                    - Declaring function's parameters, so add the next Identifiers in the local symbol table
                                    - Currently in function's body, so if we use an Identifier, check if it exists in the symbols tables
                                */
StructTable *current_struct;
/* ------------------------------------------------------- */

void pad_tabs(int length) {
    int i;

    for (i = length / 8 ; i < 3 ; i++) {
        printf("\t");
    }
}

/* ------------------------------------------------------- */

int has_main_function(SymbolTable *table, int size) {
    int i;
    
    for (i = 0; i < size ; i++) {
        if (!strcmp(table[i].name, "main") && table[i].ident_type == FUNCTION) {
            return 1;
        }
    }

    return 0;
}

/* ------------------------------------------------------- */

void print_struct_table(StructTable *table, int size) {
    int i;

    printf("\n|--------------------- STRUCT TABLE ----------------------|\n");
    for (i = 0; i < size ; i++) {
        printf("\nStruct : %s\n", table[i].name);
        print_symbol_table(table[i].parameters, table[i].size);
        free(table[i].parameters);
        printf("\n");
    }
}

/* ------------------------------------------------------- */

int type_is_a_struct(char type[]) {
    return strcmp(type, "int") && strcmp(type, "char") && strcmp(type, "void");
}

/* ------------------------------------------------------- */

void print_symbol_table(SymbolTable *table, int size) {
    int i;
    
    for (i = 0; i < size ; i++) {
        switch (table[i].ident_type) {
            case SIMPLE:       
                printf("Type\t\t: %s\t\t\t Id. : %s\n", table[i].type, table[i].name);
                break;

            case STRUCTURE:     
                printf("Type\t\t: struct %s", table[i].type);
                pad_tabs(strlen(table[i].type) + 8);
                printf(" Id. : %s\n", table[i].name);
                break;

            case FUNCTION:      
                printf("Funct. Type\t: %s%s", (type_is_a_struct(table[i].type)) ? "struct " : "", table[i].type); 
                pad_tabs(strlen(table[i].type) + (type_is_a_struct(table[i].type) ? 8 : 0)); 
                printf(" Id. : %s()\n", table[i].name); 
                break;
        }
    }
}

/* ------------------------------------------------------- */

int is_struct_declared(StructTable *table, int size, const char name[]) {
    int i;
    
    for (i = 0 ; i < size ; i++) {
        if (!strcmp(table[i].name, name)) {
            return 1;
        }
    }

    return 0;
}

/* ------------------------------------------------------- */

int is_function_declared(SymbolTable *table, int size, const char name[]) {
    int i;
}

/* ------------------------------------------------------- */

int is_var_declared(SymbolTable *global_table, SymbolTable *local_table, int g_size, int l_size, const char name[], int ident_type) {
    int i;

    for (i = 0 ; i < l_size ; i++) {
        if (!strcmp(local_table[i].name, name)) {
            /* - If ident_type is not function for both ident_type, it means they are both variables : then the variable has been declared.
            - However, if one of them is a function, as long as the var we want to declare is local, we can allow
            a function and a variable to have the same identifier.
            - HOWEVER, the "reverse" isn't true : 
            if we want to add a FUNCTION with the same name as another function, OR a global variable, it means the variable has been declared
            (the ident_type ALL exists for this possibility) */
            if (ident_type == ALL || (local_table[i].ident_type == FUNCTION) == (ident_type == FUNCTION)) {
                return 1;
            }
        }
    }

    for (i = 0 ; i < g_size ; i++) {
        if (!strcmp(global_table[i].name, name)) {
            if (ident_type == ALL || (global_table[i].ident_type == FUNCTION) == (ident_type == FUNCTION)) {
                return 1;
            }
        }
    }

    return 0;
}

/* ------------------------------------------------------- */

void add_var(SymbolTable **table, int *size, int total_size, int ident_type, const char name[], char type[], int line, int character) {
    int i;

    /* Check if the var "name" is already in the symbol table */
    for (i = 0 ; i < *size ; i++) {
        if (!strcmp((*table)[i].name, name)) {
            /* Check if we didn't found a function whereas the var we want to add is a local variable 
            (we can add a local variable with the same name as a function) */
            if (((*table)[i].ident_type == FUNCTION) == (ident_type == FUNCTION)) {
                printf("Error near line %d character %d : semantic error, redefinition of variable \"%s\"\n", line, character, name);
                exit(2);  
            }
        }
    }
    
    if (total_size + 1 > MAXSYMBOLS) {
        printf("Warning : too many variables\n");
        exit(3);
    }
    
    *size += 1;
    (*table)[*size - 1].ident_type = ident_type;
    (*table)[*size - 1].line = line;
    (*table)[*size - 1].character = character;
    strcpy((*table)[*size - 1].name, name);
    strcpy((*table)[*size - 1].type, type);
}

/* ------------------------------------------------------- */

void add_struct(StructTable **table, int *size, int total_size, const char name[], int line, int character) {
    int i;
    
    for (i = 0 ; i < *size ; i++) {
        if (!strcmp((*table)[i].name, name)) {
            printf("Error near line %d character %d : semantic error, redefinition of struct \"%s\"\n", line, character, name);
            exit(2); 
        }
    }

    if (total_size + 1 > MAXSYMBOLS) {
        printf("Warning : too many variables\n");
        exit(3);
    }
    
    *size += 1;

    strcpy((*table)[*size - 1].name, name);
    (*table)[*size - 1].size = 0;
    (*table)[*size - 1].parameters = malloc(MAXSYMBOLS * sizeof * (*table)[*size - 1].parameters);

    if (!(*table)[*size - 1].parameters) {
        printf("Run out of memory\n");
        exit(3);
    }

    /* To add struct's parameters */
    current_struct = &(*table)[*size - 1];
}

/* ------------------------------------------------------- */

void create_symbols_tables(Node *node, SymbolTable **global_table, SymbolTable **local_table, StructTable **struct_table, int *global_size, int *local_size, int *struct_size, int display) {
	static int depth = 0;       /* depth of current node */

	switch (node->kind) {
		case Identifier:
            /* Function's parameters */
            switch(g_declaring) {
                case DECLARING_VAR:
                    if (g_ident_type == STRUCTURE) {
                        if (!is_struct_declared(*struct_table, *struct_size, g_type)) {
                            printf("Error near line %d character %d : semantic error, undefined \"%s\" struct\n", node->line, node->character, g_type);
                            exit(2);
                        }
                    }

                    add_var(global_table, global_size, *local_size + *global_size, g_ident_type, node->u.identifier, g_type, node->line, node->character);
                    break;

                case DECLARING_LOCAL_VAR:
                    if (g_ident_type == STRUCTURE) {
                        if (!is_struct_declared(*struct_table, *struct_size, g_type)) {
                            printf("Error near line %d character %d : semantic error, undefined \"%s\" struct\n", node->line, node->character, g_type);
                            exit(2);
                        }
                    }

                    add_var(local_table, local_size, *local_size + *global_size, g_ident_type, node->u.identifier, g_type, node->line, node->character);
                    break;

                case DECLARING_FUNCTION_PARAMETERS:
                    if (g_ident_type == STRUCTURE) {
                        if (!is_struct_declared(*struct_table, *struct_size, g_type)) {
                            printf("Error near line %d character %d : semantic error, undefined \"%s\" struct\n", node->line, node->character, g_type);
                            exit(2);
                        }
                    }

                    add_var(local_table, local_size, *local_size + *global_size, g_ident_type, node->u.identifier, g_type, node->line, node->character);
                    break;

                case DECLARING_STRUCT_PARAMETERS:
                    add_var(&(current_struct->parameters), &(current_struct->size), current_struct->size, g_ident_type, node->u.identifier, g_type, node->line, node->character);
                    break;

                case IN_FUNCTION_BODY: /* If we are in a function's body, for each ident, we check if the variable has been declared in symbols tables (local or global) */
                    if (!is_var_declared(*global_table, *local_table, *global_size, *local_size, node->u.identifier, SIMPLE)) {
                        printf("Error near line %d character %d : semantic error, undefined variable \"%s\"\n", node->line, node->character, node->u.identifier);
                        exit(2);
                    }
                    break;
            }

            break;

		case Type: 
            strcpy(g_type, node->u.type);
            g_ident_type = SIMPLE; /* For now, varType is SimpleType : it might change in case we are actually declaring a function or structure */
            break;

        case StructName:
            /* We keep the struct name in case the next node is DeclChamps, which means we'll declare a new struct */
            strcpy(g_type, node->u.identifier);
            g_ident_type = STRUCTURE;
            break;

        case FonctName:
            strcpy(g_current_function_id, node->u.identifier);
            if (display) { printf("\n|----------------- %s() SYMBOLS TABLE -----------------|\n\n", node->u.identifier); }
            g_ident_type = FUNCTION;

            /* In case the function's name is already used by another global variable or function
            (we don't need to search in local table) */
            if (is_var_declared(*global_table, NULL, *global_size, 0, node->u.identifier, ALL)) {
                printf("Error near line %d character %d : semantic error, identifier \"%s\" is already used by another variable\n", node->line, node->character, node->u.identifier);
                exit(2);
            }
            
            /* Add the function in the symbol table */
            add_var(global_table, global_size, *local_size + *global_size, g_ident_type, node->u.identifier, g_type, node->line, node->character);
            break;

		case GlobalVars:
            g_declaring = DECLARING_VAR; 
            break; 

        case DeclVars:
            g_declaring = DECLARING_LOCAL_VAR; 
            break; 
        
        /* We are declaring a struct's parameter : 
        - first we add the struct in the structTable
        - then we indicate that we are not declaring variables ; the next variables are struct's parameters. */
        case DeclChamps: 
            add_struct(struct_table, struct_size, *local_size + *global_size + *struct_size, g_type, node->line, node->character);
            /* current_struct = struct_table[*struct_size - 1]; */
            g_declaring = DECLARING_STRUCT_PARAMETERS;
            break;  
        
        /* We declared all struct's parameters, we can keep declaring variables if we are not done */
        case EndDeclChamps: 
            g_declaring = DECLARING_VAR;
            break;  

        case DeclFonct:
            g_declaring = DECLARING_LOCAL_VAR;

            /* If it's not the first time we use the local symbol table and it's not empty, we print and free it */
            if (*local_size != 0) {
                if (display) { print_symbol_table(*local_table, *local_size); }
                free(*local_table);
                *local_size = 0;
            }
            
            (*local_table) = malloc(MAXSYMBOLS * sizeof * (*local_table));

            if (!(*local_table)) {
                printf("Run out of memory\n");
                exit(3);
            }
            
            break;

        case Parameters:
            g_declaring = DECLARING_FUNCTION_PARAMETERS;
            break;

		case Instructions: 
            g_declaring = IN_FUNCTION_BODY;
            break;    
            
        case FunctionCall:        
            /* Check if function has been declared (a function can only be in the global table, so we don't look in the local_table) */
            if (!is_var_declared(*global_table, NULL, *global_size, 0, node->u.identifier, FUNCTION)) {
                printf("Error near line %d character %d : semantic error, undefined function \"%s\"\n", node->line, node->character, node->u.identifier);
                exit(2);
            }

            break;
    }

	for (Node *child = node->firstChild ; child != NULL ; child = child->nextSibling) {
		create_symbols_tables(child, global_table, local_table, struct_table, global_size, local_size, struct_size, display);
	}
}

/* ------------------------------------------------------- */