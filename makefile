CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra
CPPFLAGS += -DCOMPILER_ROOT='"$(CURDIR)"' -DDEFAULT_CC='"$(firstword $(CC))"' -DDEFAULT_RUNTIME='"$(CURDIR)/build/runtime.o"'
LDLIBS += -lm
ifeq ($(OS),Windows_NT)
EXEEXT = .exe
endif

SOURCES = driver.c src/lexer/lexer.c src/parser/parser.c src/compiler/compiler.c src/platform/platform.c
HEADERS = $(wildcard src/lexer/*.h src/parser/*.h src/compiler/*.h src/compiler/*.inc src/platform/*.h)
TARGET = build/compiler$(EXEEXT)

.PHONY: all clean test
all: $(TARGET) stage1exe$(EXEEXT)

build/runtime.o: src/runtime/runtime.S
	mkdir -p build
	$(CC) -c -x assembler-with-cpp $< -o $@

$(TARGET): $(SOURCES) $(HEADERS) build/runtime.o makefile
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

stage1exe$(EXEEXT): $(TARGET)
	cp $(TARGET) $@

test: $(TARGET)
	python tests/test_compiler.py --compiler $(TARGET)

clean:
	rm -f build/compiler build/compiler.exe build/*.o stage1exe stage1exe.exe
