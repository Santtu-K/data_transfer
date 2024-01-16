#ifndef morse
#define morse

//#define _POSIX_C_SOURCE 202009L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

char morse_to_char(char* merkit);

char *char_to_morse(char merkki);

#endif