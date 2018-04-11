all:
	gcc -Wall -Wextra -pedantic -pthread -o pos2 main.c cmdparser.c

clean:
	rm pos2