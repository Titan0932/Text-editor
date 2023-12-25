EXEC = editor
OBJ = kilo.o

all: ${OBJ}
		gcc -Wall -Wextra -o ${EXEC} ${OBJ}

kilo.o: kilo.c
		gcc -c -Wall -Wextra kilo.c

clean: 
		rm -f ${OBJ} ${EXEC}