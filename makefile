
# Name of source file
SOURCE=server
S_DIR=source
B_DIR=build
T_DIR=tests

PARAMETERS=8000 5000

OBJECTS=$(B_DIR)/server-utils.o $(B_DIR)/parser.o $(B_DIR)/kv.o $(B_DIR)/server.o

# C compiler optimisations and warnings
# -lnsl Library needed for socket(), connect(), etc
# -pthread Library needed for pthread commands
CFLAGS=-pthread -std=gnu99 -O3
CLINTS=-Wall -Wextra


build: clean $(OBJECTS) makefile
	gcc -o $(B_DIR)/$(SOURCE) $(OBJECTS)

run:
	./$(B_DIR)/$(SOURCE) $(PARAMETERS)
	
run_tests:
	./$(T_DIR)/$(T_DIR)

build_tests:
	gcc -I . $(CFLAGS) -o $(T_DIR)/$(T_DIR) $(T_DIR)/$(T_DIR).c


$(B_DIR)/%.o : $(S_DIR)/%.c
	gcc $(CFLAGS) $(CLINTS) -c $< -o $@

#create_dir:
#	mkdir -p $(B_DIR)

clean:
	rm -f $(B_DIR)/*
