#include "protocol_manager.h"

#include <stdbool.h>
#include "kv.h"
#include "parser.h"
#include "server.h"
#include "safe_functions.h"

// Lock to make the KVS threadsafe.
pthread_mutex_t mutex_kvs = PTHREAD_MUTEX_INITIALIZER;

void lock_store() {
	pthread_mutex_lock_safe(&mutex_kvs);
}

void unlock_store() {
	pthread_mutex_unlock_safe(&mutex_kvs);
}

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
int parse_message_with_control_protocol(void *message, int message_size) {

	// Parse a Control command
	enum CONTROL_CMD cmd = parse_c(message);

	// Error parsing the command
	if(cmd == C_ERROR) {

		// If a user types a wrong command, show error,
		// otherwise if they hit return then close their connection
		 if(strnlen(message, message_size) == 0) {
			 sprintf_safe(message, "Goodbye.\n");
			 return R_DEATH;  // They want to exit

		 } else {
			 // Just show a command not found error
			 sprintf_safe(message, "Command not found.\n");
		 }

	// If the user has issued the SHUTDOWN command.
	} else if(cmd == C_SHUTDOWN) {

		// Send them back a shutdown message.
		sprintf_safe(message, "Shutting down.\n");
		return R_SHUTDOWN;

	// If the user issued COUNT command
	} else if(cmd == C_COUNT) {

		// countItems() should never fail.
		lock_store();
		sprintf_arg_safe(message, "%d\n", countItems());
		unlock_store();

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
 *			- "Goodbye."
 *
 */
int parse_message_with_data_protocol(void* message, int message_size) {

	// Parse the command, storing the results in key and text.
	enum DATA_CMD cmd;
	char *key[512];
	char *text = (char*) malloc_safe(LINE * sizeof(char));

	int is_success = parse_d(message, &cmd, key, &text);
	if(is_success == 2 || is_success == 3) {
		DEBUG_PRINT(("WARNING parse_d() in protocol_manger.c: Is_success %d\n", is_success));
	}

	// If user issued COUNT command
	if(cmd == D_COUNT) {

		// countItems should never fail
		lock_store();
		int itemCount = countItems();
		unlock_store();

		sprintf_arg_safe(message, "%d\n", itemCount);

	// If user issued EXISTS command
	} else if(cmd == D_EXISTS) {

		// itemExists should never fail
		lock_store();
		int does_exist = itemExists(*key);
		unlock_store();

		sprintf_arg_safe(message, "%d\n", does_exist);

	// If user issued GET command
	} else if(cmd == D_GET) {

		// findValue could return NULL
		lock_store();
		char* result = findValue(*key);
		unlock_store();

		// Check if the key exists
		if(result == NULL) {
			sprintf_safe(message, "No such key.\n");
		} else {
			sprintf_arg_str_safe(message, "%s\n", result);
		}

	// If user issued PUT command
	} else if(cmd == D_PUT) {

		// Create the text to store on the heap
		char* copytext = malloc_safe(strnlen(text, message_size) + 1);
		strncpy(copytext, text, strnlen(text, message_size) + 1);

		// If item exists, or table is full then -1
		lock_store();
		int is_error = createItem(*key, copytext);
		unlock_store();

		// Check if result exists
		if(is_error == 0) {
			sprintf_safe(message, "Success.\n");
		} else {
			sprintf_safe(message, "Error storing key.\n");
		}

	// If user issued DELETE command
	} else if(cmd == D_DELETE) {

		// -1 if item doesn't exist.
		lock_store();
		int is_error = deleteItem(*key, false);
		unlock_store();

		// Check if item exists
		if(is_error == 0) {
			sprintf_safe(message, "Success.\n");
		} else {
			sprintf_safe(message, "Error, no key found.\n");
		}

	// If user just hit return
	} else if(cmd == D_END)	{
		sprintf_safe(message, "Goodbye.\n");
		return R_DEATH; // They want to exit

	// Error messages
	} else if(cmd == D_ERR_OL) {
		sprintf_safe(message, "Error, can't find EOL, line too long.\n");

	} else if(cmd == D_ERR_INVALID) {
		sprintf_safe(message, "Error, command not found.\n");

	} else if(cmd == D_ERR_SHORT) {
		sprintf_safe(message, "Error, command too short.\n");

	} else if(cmd == D_ERR_LONG) {
		sprintf_safe(message, "Error, command too long.\n");

	} else {
		sprintf_safe(message, "Command not found.\n");
	}

	return R_SUCCESS;
}


/**
 * Parses the message based on the socket protocol, either DATA or CONTROL,
 * stores the result to give back to the client in the message parameter,
 * returns an enum about what to do next.
 */
enum RETURN_TYPE run_command(int type, void* message, int message_size) {

	if(type == DATA) {
		return parse_message_with_data_protocol(message, message_size);

	} else if (type == CONTROL) {
    	return parse_message_with_control_protocol(message, message_size);
	}

	return R_ERROR;
}
