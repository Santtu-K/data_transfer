#ifndef vastaanottofile
#define vastaanottofile

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "morse.h"
#include <assert.h>

// vastaanotto prosessi
void sig_handler_vast(int signo);
int vastaanotto(int ppid, int output);

#endif 