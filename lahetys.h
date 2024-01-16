#ifndef lahetysfile
#define lahetysfile

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "morse.h"
#include <assert.h>
#include <ctype.h>

// Lähetysprosessi
void sig_handler(int signo);
int lahetys(char kirjain, int ppid);

#endif 