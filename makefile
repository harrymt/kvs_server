
# Name of source file
SOURCE=server
S_DIR=source
B_DIR=build
T_DIR=tests

PARAMETERS=8000 5000

ALL_OBJECTS=$(OBJECTS) $(B_DIR)/main.o
OBJECTS=$(B_DIR)/socket-helper.o $(B_DIR)/protocol_manager.o $(B_DIR)/parser.o $(B_DIR)/kv.o $(B_DIR)/server.o

# C compiler optimisations and warnings
# -lnsl Library needed for socket(), connect(), etc
# -pthread Library needed for pthread commands
CFLAGS=-Wall -Wextra -pthread -std=gnu99 -O3


build: clean $(ALL_OBJECTS) makefile
	gcc -o $(B_DIR)/$(SOURCE) $(ALL_OBJECTS)

run:
	./$(B_DIR)/$(SOURCE) $(PARAMETERS)
	
run_tests:
	./$(T_DIR)/$(T_DIR)

build_tests:
	gcc -I . $(CFLAGS) -o $(T_DIR)/$(T_DIR) $(T_DIR)/test_client.c $(T_DIR)/$(T_DIR).c $(OBJECTS)


$(B_DIR)/%.o : $(S_DIR)/%.c
	gcc $(CFLAGS) $(CLINTS) -c $< -o $@

#create_dir:
#	mkdir -p $(B_DIR)

clean:
	rm -f $(B_DIR)/*
