#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>

#include "imprimer.h"
#include "debug.h"
#include <readline/readline.h>
#include <readline/history.h>

int isEmptyString(char* line);
/*
 * "Imprimer" printer spooler.
 */

int main(int argc, char *argv[])
{
    char *line_read;
    do{
         line_read = readline ("imp> ");
    }
    while((line_read != NULL && (line_read[0] == '\0')) || isEmptyString(line_read) == 0);
    //line_read needs to be freed when no longer in used


    char optval;
    while(optind < argc) {
	if((optval = getopt(argc, argv, "")) != -1) {
	    switch(optval) {
	       case '?':
		      fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
		      exit(EXIT_FAILURE);
		      break;
	    default:
            break;
	    }
	}
    }
    exit(EXIT_SUCCESS);
}

int isEmptyString(char* line){
    int addIncremented = 0;
    while (*line != '\0'){
        if(*line != 32){
            while (addIncremented != 0){
                line--;
                addIncremented--;
            }
            return 1;
        }
        addIncremented++;
        line++;
    }
    while(addIncremented != 0){
        line--;
        addIncremented--;
    }
    return 0;
}

