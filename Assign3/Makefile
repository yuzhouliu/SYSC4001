all: master

master: master.o
	gcc -o master master.o

master.o: master.c common_structs.h
	gcc -pthread -c master.c

clean:
	rm *.o master
