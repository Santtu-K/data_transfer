#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "morse.h"
#include "global.h"

//static char *morse_ascii_charset = "ABCDEFGHIJLKMNOPQRSTUVWXYZ";
static char *morse_asciiplus_charset = "ABCDEFGHIJLKMNOPQRSTUVWXYZ1234567890&'@():,=!.-+\"?/";
FILE *f;

static char *morse_merkit[] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-",
    ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "-----", ".----", "..---", "...--", "....-", ".....",
    "-....", "--...", "---..", "----.",".-...",".----.",".--.-.", "-.--.","-.--.-","---...","--..--","-...-","-.-.--",".-.-.-",
    "-....-",".-.-.",".-..-.","..--..","-..-."
    };

// Koodaus ja dekoodaus tapahtuu etsimällä indeksin ja ottamalla vastaavalta paikalta arvon toisesta kokoelmasta
char morse_to_char(char* merkit) {
    int i = 0;
    while (strcmp(merkit, morse_merkit[i]) != 0)
    {
        i = i + 1;
        if (i>100) {break; fprintf(f, "Unknown morse-code: '%s'", merkit);}
    }
    return morse_asciiplus_charset[i];
}

char *char_to_morse(char merkki) {
    int i = 0;
    while (strcmp((char[2]) { (char) merkki, '\0'}, (char[2]) { (char) morse_asciiplus_charset[i], '\0'}) != 0)
    {
        i = i + 1;
        // Ei tunnettu merkki
        if (i>100) {break;}
    }
    char *res = morse_merkit[i];
    if (i>100) {
        res = "XXX";
        if (merkki == '\n') {
            fprintf(f, "Read a new-line of input\n"); fflush(f); 
        } else {
            fprintf(f, "Unknown char: '%c'\n", merkki); fflush(f); 
        }
    } // Ei tunnettu merkki
    return res;
}
