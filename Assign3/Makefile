all: master

master: master.o utility.o
	gcc -pthread -o master master.o utility.o

master.o: master.c utility.h common_structs.h master.h
	gcc -pthread -c master.c

utility.o: utility.c utility.h common_structs.h master.h
	gcc -c utility.c

clean:
	rm *.o master
