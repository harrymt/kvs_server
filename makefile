
# Name of source file
SOURCE=server
S_DIR=source
B_DIR=build

PARAMETERS=8000 5000

OBJECTS=$(B_DIR)/server-utils.o $(B_DIR)/parser.o $(B_DIR)/kv.o $(B_DIR)/server.o

# C compiler optimisations and warnings
# -lnsl Library needed for socket(), connect(), etc
# -pthread Library needed for pthread commands
CFLAGS=-pthread -Wall -Wextra -std=c99 -O3

# First instruction is default
run: build permissions
	./$(B_DIR)/$(SOURCE) $(PARAMETERS)

# tests: permissions
#	./$(SOURCE) $(CASE_1) && \
#	./$(SOURCE) $(CASE_2) && \
#	./$(SOURCE) $(CASE_3) && \
#	./$(SOURCE) $(CASE_4)

build: create_dir $(OBJECTS) makefile
	$(CC) -o $(B_DIR)/$(SOURCE) $(OBJECTS)

permissions:
	chmod 777 $(B_DIR)/$(SOURCE)

$(B_DIR)/%.o : $(S_DIR)/%.c
	gcc $(CFLAGS) -c $< -o $@

create_dir:
	mkdir -p $(B_DIR)

clean:
	rm -rf $(B_DIR)
