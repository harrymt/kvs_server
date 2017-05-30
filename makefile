
# Name of source file
SOURCE=server
S_DIR=source
B_DIR=build
T_DIR=tests

PARAMETERS=8000 5000

ALL_OBJECTS=$(OBJECTS) $(B_DIR)/main.o

# Objects containing no main() functions
OBJECTS=$(B_DIR)/safe_functions.o $(B_DIR)/socket_helper.o $(B_DIR)/protocol_manager.o $(B_DIR)/parser.o $(B_DIR)/kv.o $(B_DIR)/message_manager.o $(B_DIR)/server_helpers.o $(B_DIR)/queue.o $(B_DIR)/server.o

# C compiler optimisations and warnings
# -pthread Library needed for pthread commands
CFLAGS=-Wall -Wextra -std=c99
CLIBRARIES=-pthread


build: clean $(ALL_OBJECTS)
	gcc -o $(B_DIR)/$(SOURCE) $(ALL_OBJECTS) $(CLIBRARIES)

tests: $(T_DIR)/test_client.c $(T_DIR)/$(T_DIR).c $(OBJECTS)
	gcc -I . $(CFLAGS) -o $(B_DIR)/$(T_DIR) $(T_DIR)/test_client.c $(T_DIR)/$(T_DIR).c $(OBJECTS) $(CLIBRARIES)

run:
	./$(B_DIR)/$(SOURCE) $(PARAMETERS)
	
run_tests:
	./$(B_DIR)/$(T_DIR) $(PARAMETERS)

$(B_DIR)/%.o : $(S_DIR)/%.c
	gcc $(CFLAGS) $(CLINTS) -c $< -o $@

clean:
	rm -f $(B_DIR)/*
