
# Name of source file
SOURCE=server

PARAMETERS=8000 5000


# C compiler optimisations and warnings
# -lnsl Library needed for socket(), connect(), etc
# -pthread Library needed for pthread commands
CC_OPTS=-pthread -Wall -Wextra -std=c99 -O3

# First instruction is default
default: build

run: build permissions
	./$(SOURCE) $(PARAMETERS)

# tests: permissions
#	./$(SOURCE) $(CASE_1) && \
#	./$(SOURCE) $(CASE_2) && \
#	./$(SOURCE) $(CASE_3) && \
#	./$(SOURCE) $(CASE_4)

permissions:
	chmod 777 $(SOURCE)

build:
	gcc $(SOURCE).c -o $(SOURCE) $(CC_OPTS)

