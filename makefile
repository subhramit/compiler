all: src/lexer/preprocessor.c src/lexer/lexer.c src/parser/parser.c driver.c
	gcc src/lexer/preprocessor.c -o preprocessor.o
	gcc src/lexer/lexer.c -c -o lexer.o
	gcc src/parser/parser.c -c -o parser.o
	gcc driver.c -c -o driver.o
	gcc -o compile preprocessor.o lexer.o driver.o parser.o

clean:
	rm -f *.o
	rm -f compile