
#Use GNU compiler
cc = gcc -g
CC = g++ -g

all: shell

shell: clean
	touch key1
	touch key2
	touch key3
	touch key4
	$(CC) producer.cpp -o p
	$(CC) consumer.cpp -o c

clean:
	rm -f p c
	rm -f key1 key2 key3 key4

