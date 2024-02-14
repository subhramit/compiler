#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2020A7PS2005P                       Ohiduz Zaman
# 2020A7PS2682P                       Priyansh Patel
# 2020A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick

builder = gcc
all: src/lexer/preprocessor.c src/lexer/lexer.c src/parser/parser.c driver.c

	$(builder) src/lexer/preprocessor.c -o build/preprocessor.o
	$(builder) src/lexer/lexer.c -c -o build/lexer.o
	$(builder) src/parser/parser.c -c -o build/parser.o
	$(builder) driver.c -c -o build/driver.o
	$(builder) -o compile preprocessor.o lexer.o driver.o parser.o

	$(builder) build/*.o -o compile
clean:
	rm -f build/*.o
	rm -f compile