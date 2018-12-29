all: compiler

OBJS = parser.o \
	lexer.o \
	codegen.o \
	main.o \

LLVMCONFIG = /home/chengjialiang/llvm/bin/llvm-config
CPPFLAGS = `$(LLVMCONFIG) --cppflags` -std=c++11 -I/home/chengjialiang/llvm/include/
LDFLAGS = `$(LLVMCONFIG) --ldflags` -lpthread -ldl -lz -rdynamic -L/usr/local/lib
LIBS = `$(LLVMCONFIG) --libs`

codegen.cpp: codegen.h ast.h

parser.cpp : parser.y
	bison -d -o $@ $<

parser.hpp : parser.cpp

lexer.cpp : lexer.l parser.hpp
	flex -o $@ $<

%.o: %.cpp
	g++ -c $(CPPFLAGS) -o $@ $<

compiler : $(OBJS)
	g++ $(CPPFLAGS)  -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

clean :
	$(RM) -rf parser.cpp parser.hpp compiler lexer.cpp *.output *.ll *.o

