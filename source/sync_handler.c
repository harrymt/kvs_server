#include "sync_handler.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include "kv.h"
#include "parser.h"
#include "debug.h"
#include "server.h"
#include "socket-helper.h"

enum RETURN_TYPE run_command(struct socket_info *data, void* message) {

	if(data->type == DATA) {
    	enum DATA_CMD cmd;
    	char *key[512], *text[512];
    	int is_success = parse_d(message, &cmd, key, text);

    	if(cmd == D_COUNT) {
    		sprintf(message, "%d\n", countItems());
    	} else if(cmd == D_EXISTS) {
    		int does_exist = itemExists(*key); // TODO incompatible pointer type :s, should be a const
    		sprintf(message, "%d\n", does_exist);
    	} else if(cmd == D_GET) {
    		DEBUG_PRINT(("Getting item %s.\n", *key));
    		char* r = findValue(*key);
    		sprintf(message, "%s\n", r);  // TODO incompatible pointer type :s, should be a const
    		if(r == NULL) {
    			printf("r is null :(\n");
    		} else {
    			printf("r is %d, %s\n", *r, r);
    		}
    		DEBUG_PRINT(("Item found %s.\n", findValue(*key)));
    	} else if(cmd == D_PUT) {
    		DEBUG_PRINT(("Creating Item %s %s.\n", *key, *text));
    		int is_error = createItem(*key, *text);
    		if(is_error == 0) {
    			sprintf(message, "Success.\n");
    		} else {
    			sprintf(message, "Error %d.\n", is_error);
    		}
    	} else if(cmd == D_DELETE) {
    		DEBUG_PRINT(("Deleting Item %s.\n", *key));
			int is_error = deleteItem(*key, false);
			if(is_error == 0) {
				sprintf(message, "Success.\n");
			} else {
				sprintf(message, "Error %d.\n", is_error);
			}
    	} else if(cmd == D_END)	{
    		return R_DEATH; // They want to exit

		} else if(cmd == D_ERR_OL) {
			sprintf(message, "Found cmd D_ERR_OL.\n");
		} else if(cmd == D_ERR_INVALID) {
			sprintf(message, "Found cmd D_ERR_INVALID.\n");
		} else if(cmd == D_ERR_SHORT) {
			sprintf(message, "Found cmd D_ERR_SHORT.\n");
		} else if(cmd == D_ERR_LONG) {
			sprintf(message, "Found cmd D_ERR_LONG.\n");
    	} else {
    		sprintf(message, "Command not found.\n");
    	}


	} else if (data->type == CONTROL) {
    	DEBUG_PRINT(("Is of type Control %d\n", data->type));

		// Parse a Control command
		enum CONTROL_CMD cmd = parse_c(message);
		printf("Parsed control command %d\n", cmd);
		if(cmd == C_ERROR) {
			// If a user types a wrong command, show error, otherwise if they hit return then close their connection
			 if(strlen(message) == 0) {
				 return R_DEATH;
			 } else {
			   sprintf(message, "Command not found.\n");
			 }

		} else if(cmd == C_SHUTDOWN) {
			sprintf(message, "Shutting down.\n");
			return R_SHUTDOWN;

		} else if(cmd == C_COUNT) {
			int items_in_kvs = countItems();
		    sprintf(message, "%d\n", items_in_kvs);
		} else {
		    sprintf(message, "Command not found.\n");
		}

	} else {
		DEBUG_PRINT(("Not a recognised type! %d\n", data->type));
	}

	return R_SUCCESS;
}
