all:
	gcc -Wall -Wextra -pedantic -pthread -o pos2 main.c

clean:
	rm pos2