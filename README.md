## A Key-value-store server

A C implementation of a Key-Value-Store server, full outline available [here](overview.pdf).

Requires Make and gcc are installed on your system.

Program is black box tested, using my own [test harness](/tests).


### TODO

- `poll()` instead of `accept()`
- DONE: Create an array of worker threads at the start, and then use an array of structs
- For testing, create a worker pool
- Create another thread pool for the control thread
- Check error, if I close a terminal, all terminals exit!
- Make kvs threadsafe

### Build

`make`

### Run

`./build/server <control port number> <data port number>`
 - or `make run`


### Tests

- Build: `make tests`
- Test: `./tests/tests <control port number> <data port number>`
- Or manually access the server using `telnet`, e.g. `telnet <host> <control port number>/<data port number>`


### Usage

We start the server in one terminal...

```
$ ./server 8000 5000
Server started.
(blocks until we get a connection)
```

Then connect to it, from another.

```
$ telnet localhost 8000
Welcome to the server.
```

Then the server spits out...

```
Got a connection.
Delegating to worker 1.
Worker 1 executing task.
(blocked)
```

From the other terminal we can type commands.

```
Welcome to the KV store.
$ COUNT
0
$ PUT name Harry
Success.
$ GET name
Harry
$ EXISTS name
1
$ EXISTS office
0
$ (empty line)
Connection closed.
```

To shutdown the server, we connect to the data port number, and we can execute `SHUTDOWN`.

```
$ telnet localhost 5000
Welcome to the server.
$ SHUTDOWN
Shutting down.
Connection closed.
```

Finally the server then unblocks and prints out...

```
(unblocked)
Shutting down.
```


### Program Overview

There are two ports that are available to connect. The Control Port and the Data Port. Each of these have different protocols.

#### Control Protocol

- `COUNT`
  - Print a line, an integer, stating number of items currently in the store.

- `SHUTDOWN`
  - Stop accepting new connections on the dataport and terminate as soon as all current connections have ended.

> After reading and processing a single command on the control port, your program should terminate the connection on this port. While a connection on the control port is open, your program does not have to listen for new connections on the data port (i.e. you can handle the control port in your main thread). However, while there is no connection open on the control port, your program must be able to handle multiple simultaneous connections on the data port.
> You will probably want to `poll()` the control and data ports in your main thread and then handle whichever port you get a connection on.
> Use the provided function `enum CONTROL_CMD parse_c(char* buffer);` to parse the line you get on the control port. It returns one of `C_SHUTDOWN`, `C_COUNT` or `C_ERROR`. Note that this function may modify the buffer contents.


#### Data Protocol

- `PUT <key> <value>`
  - Store given value under given key
  - Overwrites old value if exists
  - Returns: `"Stored key successfully."` or `"Error storing key."` depending on if the operation was successful or not.
- `GET <key>`
  - Returns: `<key>` or `"No such key."`
- `COUNT`
  - Returns: `<number of items in store>`
- `DELETE <key>`
  - Returns: `"Key deleted."` if key exists, else `"Error deleting key."`
- `EXISTS <key>`
  - Returns: `1` if key exists, `0` of doesn't
- `<empty line>`
  - Returns: `"Goodbye."` then closes the connection.

> On the data port, your program should accept new connections whenever the control port is not busy and delegate them to one of the free worker threads - or a queue that is watched by the worker threads.
> In the worker threads, process the following commands.
> Each command is a single line and you may assume that no line is longer than the constant `LINE=255`.
> Use the parser provided, but remember that the provided key/value store is not thread-safe so you have to synchronise all access to it.


### Error handling

An invalid command, overlong line or broken connection from a client must not crash your server. Proper error handling on the socket and connection is therefore essential and you cannot `abort()` or exit the program if something goes wrong that is the client’s fault.

You must also remember to free resources properly when a client connection closes.

