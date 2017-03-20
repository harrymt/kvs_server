#include "protocol_manager.h"

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


/**
 * Parses the message based on the CONTROL protocol.
 *
 * COUNT
 * 		– Gets number of items in store, same as DATA protocol.
 *  	- Returns:
 *  		- "<int>", e.g. "2"
 *
 *  SHUTDOWN
 *  	- Stop accepting any new connections on the data port.
 *  	- Terminates as soon as current connections have ended.
 *  	- Returns:
 *  		- "Shutting down."
 *
 */
int parse_message_with_control_protocol(void *message) {

	// Parse a Control command
	enum CONTROL_CMD cmd = parse_c(message);

	// Error parsing the command
	if(cmd == C_ERROR) {

		// If a user types a wrong command, show error,
		// otherwise if they hit return then close their connection
		 if(strlen(message) == 0) {
			 return R_DEATH;

		 } else {
			 // Just show a command not found error
		     sprintf(message, "Command not found.\n");
		 }

	// If the user has issued the SHUTDOWN command.
	} else if(cmd == C_SHUTDOWN) {

		// Send them back a shutdown message.
		sprintf(message, "Shutting down.\n");
		return R_SHUTDOWN;

	// If the user issued COUNT command
	} else if(cmd == C_COUNT) {

		// countItems() should never fail.
		sprintf(message, "%d\n", countItems()); // TODO synchronise

	} else {
		DEBUG_PRINT(("ERROR, should never happen. cmd %d", cmd));
	}

	return R_SUCCESS;
}

/**
 * Parses the message based on the DATA protocol.
 *
 *  The data protocol:
 *
 *  PUT key value
 *  	- Store value under key, should succeed whether or not the key already exists, if it does overwrite old value.
 *  	- Returns:
 *  		- "Stored key successfully.", if success
 *  		- "Error storing key.", if error
 *
 *	GET key
 *		– Gets value from key
 *		- Returns:
 *			- "<value>", if key exists
 *			- "No such key.", if doesn't
 *
 *  COUNT
 *  	– Gets number of items in store
 *  	- Returns:
 *  		- "<int>", e.g. "2".
 *
 *	DELETE key
 *		– Deletes key
 *		- Returns:
 *			- "Success.", if key exists
 *			- "Error, no key found.", if not.
 *
 *	EXISTS key
 *		– Check if key exists in store
 *		- Returns:
 *			- "1", if key exists,
 *			- "0", otherwise.
 *
 *	(empty line)
 *		- Close connection.
 *	    - Returns:
 *			- "goodbye."
 *
 */
int parse_message_with_data_protocol(void* message) {

	// Parse the command, storing the results in key and text.
	enum DATA_CMD cmd;
	char *key[512];
	char *text = (char*) malloc(LINE * sizeof(char));

	int is_success = parse_d(message, &cmd, key, &text);

	// TODO remove
//	DEBUG_PRINT(("Is_success %d\n", is_success));


	// If user issued COUNT command
	if(cmd == D_COUNT) {


		// countItems should never fail
		int itemCount = countItems(); // TODO synchronise


		sprintf(message, "%d\n", itemCount);

	// If user issued EXISTS command
	} else if(cmd == D_EXISTS) {

		// itemExists should never fail
		int does_exist = itemExists(*key); // TODO synchronise


		sprintf(message, "%d\n", does_exist);

	// If user issued GET command
	} else if(cmd == D_GET) {

		// findValue could return NULL
		char* result = findValue(*key); // TODO synchronise

		// Check if the key exists
		if(result == NULL) {
			sprintf(message, "No such key.\n");
		} else {
			sprintf(message, "%s\n", result);
		}

	// If user issued PUT command
	} else if(cmd == D_PUT) {

		// Create the text to store on the heap
		char* copytext = malloc(strlen(text) + 1);
		strncpy(copytext, text, strlen(text) + 1);

		// If item exists, or table is full then -1
		int is_error = createItem(*key, copytext); // TODO synchronise

		// Check if result exists
		if(is_error == 0) {
			sprintf(message, "Success.\n");
		} else {
			sprintf(message, "Error storing key.\n");
		}

	// If user issued DELETE command
	} else if(cmd == D_DELETE) {

		// -1 if item doesn't exist.
		int is_error = deleteItem(*key, false); // TODO synchronise

		// Check if item exists
		if(is_error == 0) {
			sprintf(message, "Success.\n");
		} else {
			sprintf(message, "Error, no key found.\n");
		}

	// If user just hit return
	} else if(cmd == D_END)	{
		return R_DEATH; // They want to exit

	/** ERRORS **/


	} else if(cmd == D_ERR_OL) {
		sprintf(message, "Found cmd D_ERR_OL, Possibly need a C null terminator backslash zero at end!\n");

	} else if(cmd == D_ERR_INVALID) {
		sprintf(message, "Found cmd D_ERR_INVALID.\n");

	} else if(cmd == D_ERR_SHORT) {
		sprintf(message, "Found cmd D_ERR_SHORT.\n");

	} else if(cmd == D_ERR_LONG) {
		sprintf(message, "Found cmd D_ERR_LONG.\n");

	} else {
		sprintf(message, "Command not found.\n");
	}

	return R_SUCCESS;
}


/**
 * Parses the message based on the socket protocol, either DATA or CONTROL,
 * stores the result to give back to the client in the message parameter,
 * returns an enum about what to do next.
 */
enum RETURN_TYPE run_command(int type, void* message) {

	if(type == DATA) {
		return parse_message_with_data_protocol(message);

	} else if (type == CONTROL) {
    	return parse_message_with_control_protocol(message);

	} else {
		DEBUG_PRINT(("Not a recognised type! %d\n", type));
	}

	return R_SUCCESS;
}
