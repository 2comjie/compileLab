.PHONY:all clean
all:main
main:
	g++ main.cpp lexical.h util.h -o main
clean:
	-rm main