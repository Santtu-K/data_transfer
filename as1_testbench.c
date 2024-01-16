/*
 * ELEC-C7310 Assignment #1: Morse transmission using signals
 * Testbench
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>



const char *morse_ascii_charset = "ABCDEFGHIJLKMNOPQRSTUVWXYZ";
const char *morse_asciiplus_charset = "ABCDEFGHIJLKMNOPQRSTUVWXYZ1234567890&'@():,=!.-+\"?/";
//const char *morse_finnish_charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZÅÄÖ";

/**
 * Generates a randomized input file using Morse alphabet.
 * \param name filename to create.
 * \param len length of randomized file.
 * \return 0 on success, -1 on failure. */
int generate_random_file(const char *name,int len) {
    char *data = malloc(len);
    if (data == NULL) { printf("Memory allocation failed\n"); return -1; }
    
    int i;
    int maxval = strlen(morse_asciiplus_charset);
    for (i = 0; i<len; i++) {
        data[i] = morse_asciiplus_charset[random() % maxval];
    }

    int fd = open(name,O_WRONLY|O_CREAT|O_TRUNC,0644);
    if (fd < 0) { printf("Failed to create file %s\n",name); free(data); return -1; }
    int r = write(fd,data,len);
    if (r == -1) {return -1;}
    close(fd);
    free(data);

    return 0;
}
/**
 * Compare two files together.
 * \param n1,n2 filenames to compare
 * \return 0 if files match, -1 if not. */
int compare_file(const char *n1, const char *n2) {
    FILE *a = fopen(n1,"r");
    FILE *b = fopen(n2,"r");
    int ofs = 0;
    int retval = 0;
    while (1) {
        int ca = fgetc(a);
        int cb = fgetc(b);
        if (ca != cb) { printf("%s != %s, mismatch at %d\n",n1,n2,ofs); retval = -1; break; }
        if (ca == EOF) break;
        ofs++;
    }
    fclose(a);
    fclose(b);
    return retval;
}
/**
 * Helper function to verbosely print out exit status of a child process.
 * \param retval Exit status of a child process. */
void print_retval(int retval) {
    if (WIFEXITED(retval)) {
        if (WEXITSTATUS(retval) == 0) return; // Normal exit without problems.
        printf("Program exited with error value %d\n",WEXITSTATUS(retval));
    } else if (WIFSIGNALED(retval)) {
        printf("Program terminated with signal %d\n",WTERMSIG(retval));
    }
}

/**
 * Run the test program with input piped into stdin/stdout.
 * \param bin Test program to exec().
 * \param size Size of the input file to generate.
 * \return 0 on success, -1 if the test failed. */
int test_pipe(const char *bin,int size) {
    const char *inputfile = "testfile1234";
    const char *outputfile = "testfile2345";
    int ret;
    ret = generate_random_file(inputfile,size);
    if (ret < 0) return -1;

    int pid = fork();
    if (pid < 0) { printf("Fork() failed\n"); return -1; }
    if (pid == 0) { // Child process
        close(STDIN_FILENO);
        open(inputfile,O_RDONLY);
        close(STDOUT_FILENO);
        open(outputfile,O_WRONLY|O_CREAT|O_TRUNC,0644);
        ret = execl(bin,bin,"-","-",NULL);
        fprintf(stderr,"Exec() failed (%d)\n",ret);
        exit(1);
    }
    int cret = 0;
    ret = waitpid(pid,&cret,0);
    if (ret < 0) { printf("waitpid() failed\n"); return -1; }
    if (cret != 0) { print_retval(cret); return -1; }

    // compare files
    ret = compare_file(inputfile,outputfile);
    if (ret < 0) { printf("Transmitted file not identical\n"); return -1; }

    // delete the files on success
    unlink(inputfile);
    unlink(outputfile);
    return 0;
}

/**
 * Helper function to run the pipe-test multiple times.
 * \param num How many times to run the test.
 * \param bin Test program to exec().
 * \param size Size of the input file to generate.
 * \return 0 on success, -1 if the test failed. */
int test_pipes(int num,const char *bin,int size) {
    int i;
    for (i=0; i<num;i++) {
        int retval = test_pipe(bin,size);
        if (retval != 0) return retval;
    }        
    return 0;
}


/**
 * Run the test program with input and output files.
 * \param bin Test program to exec().
 * \param size Size of the input file to generate.
 * \return 0 on success, -1 if the test failed. */
int test_file(const char *bin,int size) {
    const char *inputfile = "testfile1234";
    const char *outputfile = "testfile2345";
    int ret;
    ret = generate_random_file(inputfile,size);
    if (ret < 0) return -1;

    int pid = fork();
    if (pid < 0) { printf("Fork() failed\n"); return -1; }
    if (pid == 0) { // Child process
        ret = execl(bin,bin,inputfile,outputfile,NULL);
        printf("Exec() failed (%d)\n",ret);
        exit(1);
    }
    int cret = 0;
    ret = waitpid(pid,&cret,0);
    if (ret < 0) { printf("waitpid() failed\n"); return -1; }
    if (cret != 0) { print_retval(cret); return -1; }

    // compare files
    ret = compare_file(inputfile,outputfile);
    if (ret < 0) { printf("Transmitted file not identical\n"); return -1; }

    // delete the files on success
    unlink(inputfile);
    unlink(outputfile);

    return 0;
}


/**
 * Helper function to run the file-test multiple times.
 * \param num How many times to run the test.
 * \param bin Test program to exec().
 * \param size Size of the input file to generate.
 * \return 0 on success, -1 if the test failed. */
int test_files(int num,const char *bin,int size) {
    int i;
    for (i=0; i<num;i++) {
        int retval = test_file(bin,size);
        if (retval != 0) return retval;
    }        
    return 0;
}



/**
 * Main function
 * \param argc Argument count.
 * \param argv Argument table.
 * \return 0 if all tests succeeded, -1 if something failed. */
int main(int argc,char **argv) {
    time_t seed = time(NULL);
    int numtests = 10;
    int longtests = 0;

    int opt;
    while ((opt = getopt(argc,argv,"ln:s:")) != -1) {
        switch (opt) {
        case 'n': numtests = atoi(optarg); break;
        case 's': seed = atoi(optarg); break;
        case 'l': longtests = 1; break;
        default: printf("Usage: %s [-n numtests] [-s seedval] binary\n",argv[0]);
            return -1;
        }
    }
    if (optind >= argc) {
        printf("Missing executable to test\n");
        return -1;
    }
    // Initialize random number generator with seed value
    printf("Random seed = %ld\n",seed);
    srandom(seed);

    int ret;
    // Test short files
    
    printf("Short files test\n");
    ret = test_files(numtests,argv[optind],16);
    if (ret < 0) return -1;

    printf("Short files test\n");
    ret = test_files(numtests,argv[optind],16);
    if (ret < 0) return -1;
    
    // Test longer files
    printf("Longer files test\n");
    ret = test_files(numtests,argv[optind],1024);
    if (ret < 0) return -1;

    // Test short files
    printf("Short pipes test\n");
    ret = test_pipes(numtests,argv[optind],16);
    if (ret < 0) return -1;

    // Test longer files
    printf("Longer pipes test\n");
    ret = test_pipes(numtests,argv[optind],1024);
    if (ret < 0) return -1;

    if (longtests) {
        // Test very files
        printf("Very long files test\n");
        ret = test_files(numtests,argv[optind],1024*1024);
        if (ret < 0) return -1;

        // Test very files
        printf("Very long pipes test\n");
        ret = test_pipes(numtests,argv[optind],1024*1024);
        if (ret < 0) return -1;
    }

    printf("Tests passed successfully\n");
    return 0;
}


