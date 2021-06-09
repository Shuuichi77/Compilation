#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "abstract_tree.h"

extern int yylineno;
extern int nb_character;
extern int ident_character;

/* ------------------------------------------------------- */

static const char *StringFromKind[] = {
	"Program",
	"GlobalVars",
	"DeclVars",
	"Declarateurs",
	"DeclChamps",
	"DeclFoncts",
	"DeclFonct",
	"EnTeteFonct",
	"Parameters",
	"ListTypVar",
	"Corps",
	"Instructions",
	"Exp",
	"TB",
	"FB",
	"M",
	"E",
	"T",
	"F",
	"LValue",
	"Arguments",
	"NoArguments",
	"FunctionCall",
	"ListExp",
	"IntLiteral",
	"CharLiteral",
	"Identifier",
	"AddSub",
	"Addition",
	"Substraction",
	"Division",
	"Reste",
	"Multiplication",
	"Integer",
	"Character",
	"Type",
	"FonctName",
	"StructName",
	"Parameter",
	"Not", /* '!' F */
	"Assignement",
	"Reade",
	"Readc",
	"Print",
	"Return",
	"Condition",
	"If",
	"ThenWhile",
	"ThenIf",
	"Else",
	"While",
	"Or",
	"And",
	"Comparison",
	"Order",
	"Struct",
	"EndDeclChamps",
	"FuncType"
};

/* ------------------------------------------------------- */

Node *makeNode(Kind kind) {
	Node *node = malloc(sizeof(Node));

	if (!node) {
		printf("Run out of memory\n");
		exit(3);
	}

	node->kind = kind;
	node->firstChild = node->nextSibling = NULL;
	node->line = yylineno;

	switch(kind) {
		case Identifier: node->character = ident_character; break;
		default: node->character = nb_character; break;
	}

	return node;
}

/* ------------------------------------------------------- */

void addSibling(Node *node, Node *sibling) {
	Node *curr = node;

	while (curr->nextSibling != NULL) {
		curr = curr->nextSibling;
	}

	curr->nextSibling = sibling;
}

/* ------------------------------------------------------- */

void addChild(Node *parent, Node *child) {
	if (parent->firstChild == NULL) {
		parent->firstChild = child;
	}

	else {
		addSibling(parent->firstChild, child);
	}
}

/* ------------------------------------------------------- */

void deleteTree(Node *node) {
	if (node->firstChild) {
		deleteTree(node->firstChild);
	}

	if (node->nextSibling) {
		deleteTree(node->nextSibling);
	}

	free(node);
}

/* ------------------------------------------------------- */

void printTree(Node *node) {
	static bool rightmost[128]; // current node is rightmost sibling
	static int depth = 0;       // depth of current node
	
	/* EndDeclChamps is a node to prevent adding struct's parameters in symbolTable. 
	We don't want to print this kind of node in the tree. */
	if (node->kind != EndDeclChamps) {
		for (int i = 1; i < depth; i++) { // 2502 = vertical line
			printf(rightmost[i] ? "    " : "\u2502   ");
		}

		if (depth > 0) { // 2514 = up and right; 2500 = horiz; 251c = vertical and right 
			printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
		}

		printf("%s", StringFromKind[node->kind]);

		switch (node->kind) {
			case IntLiteral: printf(": %d", node->u.integer); break;
			case CharLiteral: printf(": '%c'", node->u.character); break;
			case Identifier: printf(": %s", node->u.identifier); break;
			case Type: printf(": %s", node->u.type); break;
			case FonctName:  printf(": %s", node->u.identifier); break;
			case StructName:  printf(": %s", node->u.identifier); break;
			case Parameters: 
				printf(":");
				if (strcmp(node->u.identifier, "void") == 0) { 
					printf(" None");
				} 
				break;

			case Parameter: printf(": %s", node->u.identifier); break;
			case FunctionCall: printf(": %s()", node->u.identifier); break;
			case Comparison: printf(": %s", node->u.comp); break;
			case Order: printf(": %s", node->u.comp); break;
			default: break;
		}
		printf("\n");
	}
	
	depth++;

	for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
		rightmost[depth] = (child->nextSibling) ? false : true;
		printTree(child);
	}
	
	depth--;
}

/* ------------------------------------------------------- */