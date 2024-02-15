#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2020A7PS2005P                       Ohiduz Zaman
# 2020A7PS2682P                       Priyansh Patel
# 2020A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick

var = gcc -c #change to "clang" if you're using Clang to compile
all: src/lexer/preprocessor.c src/lexer/lexer.c src/parser/parser.c driver.c
	make clean
	mkdir -p build
	
	$(var) src/lexer/preprocessor.c -o build/preprocessor.o
	$(var) src/lexer/lexer.c -o build/lexer.o
	$(var) src/parser/parser.c -o build/parser.o
	$(var) driver.c -o build/driver.o

	gcc build/*.o -o stage1exe
	
clean:
	rm -f build/*.o
	rm -f compile