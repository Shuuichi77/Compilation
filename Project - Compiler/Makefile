CC=gcc
CFLAGS=

LEX=flex
LFLAGS=-lfl

BISON=bison
BFLAGS=-d

SRC =./src/

EXEC = tpcc
OBJ = as.tab.c abstract_tree.o generate_asm.o lex.yy.o symbols.o 

##########################################################

tpcc: $(OBJ)
	$(CC) -o $@ $^ $(LFLAGS)
	mv -t ./obj/ $(OBJ) as.tab.h lex.yy.c
	mv $(EXEC) ./bin/ 
	
as.tab.c: $(SRC)as.y
	$(BISON) $(BFLAGS) $<

lex.yy.c: $(SRC)as.lex 
	$(LEX) $<

abstract_tree.o : $(SRC)abstract_tree.c $(SRC)abstract_tree.h
	$(CC) -c $< $(CFLAGS)

generate_asm.o : $(SRC)generate_asm.c $(SRC)generate_asm.h $(SRC)abstract_tree.h $(SRC)symbols.h 
	$(CC) -c $< $(CFLAGS)

symbols.o : $(SRC)symbols.c $(SRC)symbols.h $(SRC)abstract_tree.h
	$(CC) -c $< $(CFLAGS)

##########################################################

clean:
	rm -f lex.yy.*
	rm -f ./obj/*.o
	rm -f ./obj/as.tab*
	rm -f ./obj/lex.yy*

mrproper: clean
	rm -f ./bin/$(EXEC)
	rm -f _anonymous.asm
