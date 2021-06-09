#ifndef GENERATE_ASM_H
#define GENERATE_ASM_H

/* ------------------------------------------------------- */

    #include <stdio.h>
    #include "abstract_tree.h"
    #include "symbols.h"

    #define POP(x)      fprintf(file, "\tpop %s\n", x)
    #define PUSH(x)     fprintf(file, "\tpush %s\n", x)
    #define MOV(x, y)   fprintf(file, "\tmov %s, %s\n", x, y)
    #define ADD(x, y)   fprintf(file, "\tadd %s, %s\n", x, y)
    #define SUB(x, y)   fprintf(file, "\tsub %s, %s\n", x, y)
    #define IMUL(x, y)  fprintf(file, "\timul %s, %s\n", x, y)
    #define IDIV(x)     fprintf(file, "\tidiv %s\n", x)
    #define CALL(x)     fprintf(file, "\tcall %s\n", x)
    #define CMP(x, y)   fprintf(file, "\tcmp %s, %s\n", x, y)
    #define SET(x, y)   fprintf(file, "\tset%s %s\n", x, y)

    #define JMP(x, y)               fprintf(file, "\tjmp %s%d\n", x, y)
    #define JMP_L(x, y)             fprintf(file, "\tj%s L%d\n", x, y)
    #define JMP_WHILE(x, y)         fprintf(file, "\tj%s WHILE%d\n", x, y)
    #define JMP_LOOP(x, y)          fprintf(file, "\tj%s LOOP%d\n", x, y)
    #define JMP_ENDWHILE(x, y)      fprintf(file, "\tj%s ENDWHILE%d\n", x, y)
    #define JMP_ENDIF(x, y)         fprintf(file, "\tj%s ENDIF%d\n", x, y)
    #define JMP_ELSE(x, y)          fprintf(file, "\tj%s ELSE%d\n", x, y)

    #define NEW_L(x)                fprintf(file, "\nL%d:\n", x)
    #define NEW_WHILE(x)            fprintf(file, "\nWHILE%d:\n", x)
    #define NEW_LOOP(x)             fprintf(file, "\nLOOP%d:\n", x)
    #define NEW_ENDWHILE(x)         fprintf(file, "\nENDWHILE%d:\n", x)
    #define NEW_ENDIF(x)            fprintf(file, "\nENDIF%d:\n", x)
    #define NEW_ELSE(x)             fprintf(file, "\nELSE%d:\n", x)
    
    #define NB_MAX_FUNCTION 30

    typedef struct {
        char type[5];
        char name[64];
    } FuncParameters;

    typedef struct {
        char name[64];
        char return_type[5];
        int nb_parameter;
        FuncParameters parameters[8]; /* Max nb of registers stored in generate_asm.c for parameters */
    } FunctionInfo;

/* ------------------------------------------------------- */
    
    int write_asm_file(FILE* file, Node* node, SymbolTable **g_symtab, int *g_size);
    int write_intro(FILE* file);
    Node* write_typesvars(FILE *file, Node *node);
    void write_global_variable(FILE *file, char *name, char *type);
    int write_function(FILE *file, Node *node, SymbolTable **g_symtab, int *g_size, FunctionInfo functions[]);
    Node* write_declfunc(FILE *file, Node *node, SymbolTable **table, int *size, FunctionInfo *function);
    Node* push_local_variables(FILE *file, Node *node, SymbolTable **table, int *size, int *push_size);
    char* global_var_type(char *name, SymbolTable *table, int size);
    int var_offset(SymbolTable *table, int size, int check_parameters, char *name);
    char* get_first_var(char *name, SymbolTable *g_symtab, SymbolTable *p_symtab, SymbolTable *l_symtab, int g_size, int p_size, int l_size);
    int parameter_is_same_type(FunctionInfo functions[], char *f_name, int parameter);
    char* return_comparison_or_order(char *comp);
    int function_instructions(FILE *file, Node *node, SymbolTable *g_symtab, SymbolTable *p_symtab, SymbolTable *l_symtab, int g_size, int p_size, int l_size, FunctionInfo functions[]);
    void write_read(FILE *file, char *var, char *type);
    void write_print(FILE *file, char *type);

/* ------------------------------------------------------- */

#endif