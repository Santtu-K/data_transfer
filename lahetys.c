#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "morse.h"
#include <assert.h>
#include <ctype.h>
#include "lahetys.h"
#include "global.h"

int sigpipe = 0;
int pipefd[2];
FILE *f;

// Lähetys signalhandler
void sig_handler(int signo) {
    char ret = signo;
	int t = write(sigpipe,&ret,1);
    if (t < 0) {perror("\nwrite failed");}
}
void log_error(char kir){
    fprintf(f,"Error sending character: '%c'", kir);fflush(f);
}
// 1 ASCII merkin lähetys
int lahetys(char kirjain, int ppid) {
    char *morse_koodi = char_to_morse(kirjain);
    int i = 0;
    while(morse_koodi[i] != '\0' && morse_koodi[i] != 'X'){ 
        char merkki = (morse_koodi[i]);
        if (merkki == '-'){
            //Lähetä user signal 1, eli -
            int r = kill(ppid, SIGUSR1);
            if (r == -1) {log_error(kirjain);printf("\nSignaali ei läpi1");return -1;}
            i++;
        } else if (merkki == '.') {
            //Lähetä user signal 2, eli .
            int r = kill(ppid, SIGUSR2);
            if (r == -1) {log_error(kirjain);printf("\nSignaali ei läpi2");return -1;}
            i++;
        } 
        // Odota kuittausta
        for ( ; ; ) {
            char mysignal;
		    int res = read(pipefd[0],&mysignal,1);
            // When read is interrupted by a signal, it will return -1 and errno is EINTR.
            if (res < 0) {log_error(kirjain);perror("\nread failed");}
            if (res == 1) {
                    if (mysignal == SIGUSR1) {
                        break;
                    }
            }
        }
    }
    //Lähetä SIGALRM, eli uusi merkki
    int r = kill(ppid, SIGALRM);
    if (r == -1) {log_error(kirjain);printf("\nSignaali ei läpi3");return -1;}
    for ( ; ; ) {
        char mysignal;
        int res = read(pipefd[0],&mysignal,1);
        if (res < 0) {log_error(kirjain);perror("\nread failed");}
        if (res == 1) {
                if (mysignal == SIGUSR1) {
                    break;
                }
        }
    }
    
    return 0;
}
