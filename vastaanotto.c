#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "morse.h"
#include <assert.h>
#include "vastaanotto.h"
#include "global.h"

// signaalikäsittely vastaanottajalle
static int sigpipe_vast = 0;
FILE *f;

void sig_handler_vast(int signo) {
    char ret = signo;
	int t = write(sigpipe_vast,&ret,1);
    if (t < 0) {perror("\nwrite failed");}
}

void log_err_vast(){
    fprintf(f,"Error receiving signals\n");fflush(f);
}

// vastaanotto prosessi
int vastaanotto(int ppid, int output) {
    struct sigaction sig;
	int pipefd[2];

	assert(pipe(pipefd) == 0);
	sigpipe_vast = pipefd[1];
	fcntl(sigpipe_vast,F_SETFL,O_NONBLOCK);

	sigemptyset(&sig.sa_mask);
  	sig.sa_flags= SA_RESTART;  // restart interrupted system calls
	sig.sa_handler = sig_handler_vast;

	assert((sigaction(SIGUSR1,&sig,NULL)) == 0); 
	assert((sigaction(SIGUSR2,&sig,NULL)) == 0);
    assert((sigaction(SIGALRM,&sig,NULL)) == 0);
    assert((sigaction(SIGINT,&sig,NULL)) == 0);

    //Morsemerkkien alustus
    char morsejono[9];
    int w = 0;
    while (w < 8) {
        morsejono[w] = '\0';
        w++;
    }
    int i = 0;
    for ( ; ; ) {
		char mysignal;
		int res = read(pipefd[0],&mysignal,1);
		// When read is interrupted by a signal, it will return -1 and errno is EINTR.
		if (res < 0) {log_err_vast();perror("read failed");}
		if (res == 1) {
            if (mysignal == SIGUSR1){
                //morse merkki -
                morsejono[i] = '-';
                i = i + 1;
                //kuittaus
                int r = kill(ppid, SIGUSR1);
                if (r == -1) {fprintf(f,"Error sending ACK\n");fflush(f);printf("Signaali ei läpi");return -1;};
            }
            else if (mysignal == SIGUSR2){
                //morse merkki .
                morsejono[i] = '.';
                i = i + 1;
                int r = kill(ppid, SIGUSR1);
                if (r == -1) {fprintf(f,"Error sending ACK\n");fflush(f);printf("Signaali ei läpi");return -1;};
            }
            else if (mysignal == SIGALRM){
                //morse merkin loppu
                if (strlen(morsejono) >= 1) { 
                    char kirjain = morse_to_char(morsejono);
                    //morsemerkkijonon nollaus
                    int q = 0;
                    while (q < 8) {
                        morsejono[q] = '\0';
                        q++;
                    }
                    //edellisen merkin kirjoitus outputtiin
                    int t = write(output,&kirjain,1);
                    if (t < 0) {
                        fprintf(f,"Error writing to output\n");fflush(f);
                        return -1;
                    }
                }
                i = 0;
                int r = kill(ppid, SIGUSR1);
                if (r == -1) {fprintf(f,"Error sending ACK\n");fflush(f);printf("Signaali ei läpi");return -1;};
            }  
            else if (mysignal == SIGINT){
                //vastaanoton lopetus
                break;
            }  
        }
    }
    return 0;
}