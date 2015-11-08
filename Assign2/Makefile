all: producer consumer

producer: producer.o common_sem.o common_shm.o
	gcc -o producer producer.o common_sem.o common_shm.o

consumer: consumer.o common_sem.o common_shm.o
	gcc -o consumer consumer.o common_sem.o common_shm.o

producer.o: producer.c common_sem.h common_shm.h
	gcc -c producer.c

consumer.o: consumer.c common_sem.h common_shm.h
	gcc -c consumer.c

common_sem.o: common_sem.c common_sem.h
	gcc -c common_sem.c

common_shm.o: common_shm.c common_shm.h
	gcc -c common_shm.c

clean:
	rm producer consumer producer.o consumer.o common_sem.o common_shm.o _text_output.txt
