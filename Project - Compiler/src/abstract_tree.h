#ifndef ABSTRACT_TREE_H
#define ABSTRACT_TREE_H

/* ------------------------------------------------------- */

	typedef enum {
		Program,
		GlobalVars,
		DeclVars,
		Declarateurs,
		DeclChamps,
		DeclFoncts,
		DeclFonct,
		EnTeteFonct,
		Parameters,
		ListTypVar,
		Corps,
		Instructions,
		Exp,
		TB,
		FB,
		M,
		E,
		T,
		F,
		LValue,
		Arguments,
		NoArguments,
		FunctionCall,
		ListExp,
		IntLiteral,
		CharLiteral,
		Identifier,
		AddSub,
		Addition,
		Substraction,
		Division,
		Reste,
		Multiplication,
		Integer,
		Character,
		Type,
		FonctName,
		StructName,
		Parameter,
		Not, /* '!' F */
		Assignement,
		Reade,
		Readc,
		Print,
		Return,
		Condition,
		If,
		ThenWhile,
		ThenIf,
		Else,
		While,
		Or,
		And,
		Comparison,
		Order,
		Struct,
		EndDeclChamps,
		FuncType
	} Kind;

/* ------------------------------------------------------- */

	typedef struct Node {
		Kind kind;
		union {
			int integer;
			char character;
			char identifier[64];
			char structName[64];
			char type[5];
			char comp[3];
		} u;
		struct Node *firstChild, *nextSibling;
		int line;
		int character;
	} Node;

/* ------------------------------------------------------- */

	Node *makeNode(Kind kind);
	void addSibling(Node *node, Node *sibling);
	void addChild(Node *parent, Node *child);
	void deleteTree(Node *node);
	void printTree(Node *node);

/* ------------------------------------------------------- */

	#define FIRSTCHILD(node) node->firstChild
	#define SECONDCHILD(node) node->firstChild->nextSibling
	#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling

/* ------------------------------------------------------- */

#endif