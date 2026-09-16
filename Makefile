# Target to build the final executable
myAssembler: myAssembler.o firstPass.o secondPass.o preAssembler.o file.o 
	gcc -g -ansi -Wall -pedantic myAssembler.o firstPass.o secondPass.o preAssembler.o file.o -o myAssembler

# Object file compilations
myAssembler.o: myAssembler.c passes.h preAssembler.h
	gcc -g -ansi -Wall -pedantic -c myAssembler.c -o myAssembler.o

file.o: file.c file.h
	gcc -g -ansi -Wall -pedantic -c file.c -o file.o

firstPass.o: firstPass.c passes.h file.h 
	gcc -g -ansi -Wall -pedantic -c firstPass.c -o firstPass.o

secondPass.o: secondPass.c passes.h file.h 
	gcc -g -ansi -Wall -pedantic -c secondPass.c -o secondPass.o


preAssembler.o: preAssembler.c preAssembler.h passes.h file.h
	gcc -g -ansi -Wall -pedantic -c preAssembler.c -o preAssembler.o

# Safety net: Wipes old builds to guarantee a 100% fresh debug environment
clean:
	rm -f *.o myAssembler