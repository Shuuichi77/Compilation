#ifndef SYMBOLS_H
#define SYMBOLS_H

/* ------------------------------------------------------- */

    #include "abstract_tree.h"

    #define MAXNAME 64
    #define MAXTYPE 64

    enum {
        SIMPLE,
        STRUCTURE,
        FUNCTION,
        ALL,
    } IdentType;

    enum {
        DECLARING_VAR,
        DECLARING_LOCAL_VAR,
        DECLARING_FUNCTION_PARAMETERS,
        DECLARING_STRUCT_PARAMETERS,
        IN_FUNCTION_BODY,
    } Declaring;

    typedef struct {
        int ident_type;
        char type[MAXTYPE];
        char name[MAXNAME];
        int character;
        int line;
    } SymbolTable;
    
    typedef struct {
        char name[MAXNAME];
        int size;
        SymbolTable *parameters;
    } StructTable;

/* ------------------------------------------------------- */

    extern int yylineno;  /* from lexical analyser */

    #define MAXSYMBOLS 256

    /* #define INTEGER 0
    #define REAL 1 */

    #define INT 0
    #define CHAR 1
    #define STRUC 2

/* ------------------------------------------------------- */
    /* Print "length mod 8" tabs depending (for print_symbol_table())*/
    void pad_tabs(int length);

    /*  */
    int has_main_function(SymbolTable *table, int size);

    /* Print the table "structTable" */
    void print_struct_table(StructTable *table, int size);
    
    /* Print the table "symbolTable" */
    void print_symbol_table(SymbolTable *table, int size);

    /* Check in "structTable" if the structure "name" has been declared
       - Return 0 if it hasn't been declared yet
       - Return 1 if the structure has been declared */
    int is_struct_declared(StructTable *table, int size, const char name[]);
    
    /*  */
    int is_function_declared(SymbolTable *table, int size, const char name[]);

    /*  */
    int is_var_declared(SymbolTable *global_table, SymbolTable *local_table, int g_size, int l_size, const char name[], int ident_type);

    /* Add the variable "name" with its type "type" in "symbolTable" */
    void add_var(SymbolTable **table, int *size, int total_size, int ident_type, const char name[], char type[], int line, int character);

    /* Add the structure "name" in "structTable" */
    void add_struct(StructTable **table, int *size, int total_size, const char name[], int line, int character);

    /* Traverse through the tree "node" to add variables in symbols tables "g_table", "l_table" and "s_table" */
    void create_symbols_tables(Node *node, SymbolTable **global_table, SymbolTable **local_table, StructTable **struct_table, int *global_size, int *local_size, int *struct_size, int display);

/* ------------------------------------------------------- */

#endif