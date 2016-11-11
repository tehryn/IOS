CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic

all: proj2

proj2: proj2.o proj2_shared.o proj2_arguments.o
	$(CC) $(CFLAGS) proj2.o proj2_shared.o proj2_arguments.o -o proj2 -lpthread
	
proj2.o: proj2.c proj2.h
	$(CC) $(CFLAGS) -c proj2.c -lpthread
	
proj2_shared.o: proj2_shared.c proj2.h
	$(CC) $(CFLAGS) -c proj2_shared.c -lpthread

proj2_arguments.o: proj2_arguments.c proj2.h
	$(CC) $(CFLAGS) -c proj2_arguments.c

.PHONY: clean zip
clean:
	rm *.o proj2
zip:
	zip xmatej52.zip proj2.c proj2.h proj2_shared.c proj2_arguments.c Makefile
