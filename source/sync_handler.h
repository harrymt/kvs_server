
#include "server.h"

#ifndef _sync_handler_h_
#define _sync_handler_h_

enum RETURN_TYPE { R_SUCCESS, R_ERROR, R_SHUTDOWN, R_DEATH };
enum RETURN_TYPE run_command(struct socket_info *data, void* message);

#endif
