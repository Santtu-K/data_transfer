#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "lahetys.h"
#include "morse.h"
#include <signal.h>
#include "vastaanotto.h"
#include "global.h"
#define BLOCKSIZE 4096
FILE *f;

int main(int argc,char **argv) {
    f = fopen("logfile.log", "w");
    if (f == NULL) {printf("Error opening log file"); return -1;}
    fprintf(f, "Opened the log file succesfully\n");
    fflush(f);
    int ifd, ofd;
    // Quick and dirty command line parsing
    if (argc == 2) { // Only input file, output stdout
        ofd = STDOUT_FILENO;
        fprintf(f, "Writing to STDOUT\n");
        fflush(f);
        if (strcmp(argv[1],"-") == 0) {
            ifd = STDIN_FILENO;
            fprintf(f, "Reading from STDIN\n");
            fflush(f);
        } else {
            ifd = open(argv[1],O_RDONLY);
            if (ifd < 0) {
                fprintf(stderr,"Opening input file (%s) failed\n",argv[1]);
                fprintf(f,"Opening input file (%s) failed\n",argv[1]);
                fflush(f);
                return -1;
            }
            fprintf(f,"Opened input file (%s) succesfully\n",argv[1]);
            fflush(f);
        }
    } else if (argc == 3) { // Both input and output file given
        if (strcmp(argv[1],"-") == 0) {
            ifd = STDIN_FILENO;
            fprintf(f, "Reading from STDIN\n");
            fflush(f);
        } else {
            ifd = open(argv[1],O_RDONLY);
            if (ifd < 0) {
                fprintf(stderr,"Opening input file (%s) failed\n",argv[1]);
                fprintf(f,"Opening input file (%s) failed\n",argv[1]);
                fflush(f);
                return -1;
            }
            fprintf(f,"Opened input file (%s) succesfully\n",argv[1]);
            fflush(f);
        }
        if (strcmp(argv[2],"-") == 0) {
            ofd = STDOUT_FILENO;
            fprintf(f, "Writing to STDOUT\n");
            fflush(f);
        } else {
            ofd = open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0644);
            if (ofd < 0) {
                fprintf(stderr,"Creating output file (%s) failed\n",argv[2]);
                fprintf(f,"Creating output file (%s) failed\n",argv[2]);
                fflush(f);
                return -1;
            }
            fprintf(f,"Created output file (%s) succesfully\n",argv[2]);
            fflush(f);
        }
    } else {
        fprintf(stderr,"Usage: %s [input|-] [output|-]\n",argv[0]);
        fprintf(f,"Incorrect usage. Usage: %s [input|-] [output|-]\n",argv[0]);
        return -1;
    }
    //Forkkaus
    pid_t pid;
    pid = fork();
    if (pid == -1) {
		perror("Fork failed");
        fprintf(f,"Fork failed\n");
        fflush(f);
		return -1;
	}
    if (pid == 0) {
		// Lapsiprosessi, eli lähettäjä
        fprintf(f,"Fork succesful\n");
        fflush(f);
        // Parentin id
        int ppid = getppid();

        struct sigaction sig;
        assert(pipe(pipefd) == 0);
        sigpipe = pipefd[1];
        fcntl(sigpipe,F_SETFL,O_NONBLOCK);

        sigemptyset(&sig.sa_mask);
        sig.sa_flags= SA_RESTART;
        sig.sa_handler = sig_handler;

        assert((sigaction(SIGUSR1,&sig,NULL)) == 0); 
        // Allocate buffer
        char *buf = malloc(BLOCKSIZE);
        if (buf == NULL) return -1;
        
        fprintf(f,"Reading and sending data\n");
        fflush(f);
        // Lähetys loop
        while (1) {
            int s;
            s = read(ifd,buf,BLOCKSIZE);
            if (s < 0) return -1;
            if (s == 0) {
                break;
            }; // input closed
            int numBytes = s;
            int i = 0;
            while(i < numBytes) {
                char kirjain = toupper(buf[i]);
                int t = lahetys(kirjain, ppid);
                if (t == -1) {return -1;}
                i = i + 1;
            }
        }

        fprintf(f,"Data transmission succesful\nClosing the sender process\n");
        fflush(f);
        // Vastaanottimen sulkeminen
        kill(ppid, SIGINT);
        free(buf);
        exit(0);
	} else {
		// Parentprosessi, eli vastaanottaja
        fprintf(f,"Ready to receive and write data\n");
        fflush(f);
        int p = vastaanotto(pid, ofd);
        if (p == -1) {return -1;}
	}
    close(ifd);
    close(ofd);
    fprintf(f,"Program succesful\nClosing input, output and the receiver process\n");
    fflush(f);
    return 0;
}
