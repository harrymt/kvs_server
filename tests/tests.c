
#include "cheat.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../source/server.h"

CHEAT_SET_UP(
	fputs("Set up began.\n", stderr);

	// Start a test server
	system("build/server 5000 8000");
	system("telnet localhost 8000");
	fputs("Set up!\n", stdout);

	fputs("Set up ended.\n", stderr);
)

CHEAT_TEST(mathematics_still_work,
	fputs("Test Test began.\n", stderr);
    cheat_assert(2 + 2 == 4);
    cheat_assert_not(2 + 2 == 5);
	fputs("TestTest ended.\n", stderr);
)

CHEAT_TEAR_DOWN(
	fputs("Tear down began.\n", stderr);

	fputs("Tear down!\n", stdout);

	fputs("Tear down ended.\n", stderr);
)
