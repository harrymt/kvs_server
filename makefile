
# Name of source file
SOURCE=server

PARAMETERS=8000 5000

OBJECTS=server-utils.o parser.o kv.o server.o

# C compiler optimisations and warnings
# -lnsl Library needed for socket(), connect(), etc
# -pthread Library needed for pthread commands
CFLAGS=-pthread -Wall -Wextra -std=c99 -O3

# First instruction is default
# default: build

run: build permissions
	./$(SOURCE) $(PARAMETERS)

# tests: permissions
#	./$(SOURCE) $(CASE_1) && \
#	./$(SOURCE) $(CASE_2) && \
#	./$(SOURCE) $(CASE_3) && \
#	./$(SOURCE) $(CASE_4)

permissions:
	chmod 777 $(SOURCE)

%.o : %.c
	gcc $(CFLAGS) -c $< -o $@

build: $(OBJECTS) makefile
	$(CC) -o $(SOURCE) $(OBJECTS)
