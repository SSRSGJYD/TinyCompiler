all: parser

parser.cpp : parser.y
	bison -d -o $@ $<

parser.hpp : parser.cpp

lexer.cpp : lexer.l parser.hpp
	flex -o $@ $<

parser : parser.cpp lexer.cpp main.cpp
	g++ -std=c++11 -o $@ $^

clean :
	$(RM) -rf parser.cpp parser.hpp parser lexer.cpp *.output

