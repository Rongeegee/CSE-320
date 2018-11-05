#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>

#include "imprimer.h"
#include "debug.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "helper.h"
#include <string.h>

/*
 * "Imprimer" printer spooler.
 */

int main(int argc, char *argv[])
{
    type_head = NULL;
    printer_head = NULL;

    char *line_read;
    while(1){
        line_read = readline ("imp> ");
        if(isHelp(line_read) == 1){
            fprintf(stdout, "%s\n", "Miscallaneous Commands, Configuration Commands, Informational Commands, Spooling Commands.");
        }
        else if(isQuit(line_read) == 1){
            break;
        }
        else if(isTypeCommand(line_read) == 1){
            addFileType(line_read + 5);
        }
        else if(isPrinter(line_read) == 1){
            int printerIndex = getPNIndex(line_read);
            int typeIndex = getFTIndex(line_read);
            int printerLen = getLen(line_read + printerIndex);
            int fileLen = getLen(line_read + typeIndex);
            char* printerName = malloc(printerLen * sizeof(char) + 1);
            char* fileType = malloc(fileLen * sizeof(char) + 1);
            strncpy(printerName,line_read + printerIndex, printerLen);
            strncpy(fileType,line_read + typeIndex, fileLen);
            printerName[printerLen] = '\0';
            fileType[fileLen] = '\0';
            addPrinter(printerName, fileType);
        }
    }
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


int getLen(char* line){
    int len = 0;
    int length = 0;
    while (*line != ' ' && *line != '\0'){
        len++;
        length++;
        line++;
    }
    while(length != 0){
        line--;
        length--;
    }
    return len;
}

int getPNIndex(char* line){
    int incremented = 0;
    while(*line != ' '){
        line++;
        incremented++;
    }
    while(*line == ' '){
        line++;
        incremented++;
    }
    int index = incremented ;
    while(incremented != 0){
        line--;
        incremented--;
    }
    return index;
}

int getFTIndex(char* line){
    int incremented = 0;
    while(*line != ' '){
        line++;
        incremented++;
    }
    while(*line == ' '){
        line++;
        incremented++;
    }
    while(*line != ' '){
        line++;
        incremented++;
    }
    while(*line == ' '){
        line++;
        incremented++;
    }
    int index = incremented ;
    while(incremented != 0){
        line--;
        incremented--;
    }
    return index;
}

void addPrinter(char* name, char* type){
    if(type_head == NULL){
        fprintf(stderr, "%s\n", "File type has not been declared.");
        free(name);
        free(type);
        return;
    }
    file_type* FT = type_head;
    while(FT != NULL){
        if(strcmp(FT->name,type) == 0)
            break;
        if(FT->next == NULL){
            fprintf(stderr, "%s\n", "File type has not been declared.");
            free(name);
            free(type);
            return;
        }
        FT = FT->next;
    }
    if(printer_head == NULL){
        printer_head = (printer_address*)malloc(sizeof(printer_address));
        PRINTER* printer = (PRINTER*) malloc(sizeof(PRINTER));
        printer_head->printer = printer;
        printer->id = 0;
        printer->name = name;
        printer->type = type;
        printer->enabled = 1;
        printer->busy = 0;
        printer->other_info = NULL;
        printer_head->next = NULL;
    }
    else{
        printer_address* current_printer = printer_head;
        while(current_printer->next != NULL){
            if(current_printer->printer->id == 31){
                fprintf(stderr, "%s\n", "Number of Printer is already maximum");
                free(name);
                free(type);
                return;
            }
            else if(strcmp(current_printer->printer->name, name) == 0){
                fprintf(stderr, "%s\n", "Printer name already exists");
                free(name);
                free(type);
                return;
            }
            current_printer = current_printer->next;
        }
        if(strcmp(current_printer->printer->name, name) == 0){
                fprintf(stderr, "%s\n", "Printer name already exists");
                free(name);
                free(type);
                return;
            }
        PRINTER* printer = (PRINTER*) malloc(sizeof(PRINTER));
        printer_address* printer_address = malloc(sizeof(printer_address));
        printer->id = current_printer->printer->id + 1;
        printer->name = name;
        printer->type = type;
        printer->enabled = 1;
        printer->busy = 0;
        printer->other_info = NULL;
        current_printer->next = printer_address;
        printer_address->printer = printer;
        printer_address->next = NULL;
    }
}

int isPrinter(char* line){
    if(*line == 'p'){
        if(*(line + 1) == 'r'){
            if(*(line + 2) == 'i'){
                if(*(line + 3) == 'n'){
                    if(*(line + 4) == 't'){
                        if(*(line + 5) == 'e'){
                            if(*(line + 6) == 'r'){
                                if(*(line + 7) == ' ')
                                    return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

void addFileType(char* type){
    if(type_head == NULL){
        type_head = (file_type*)malloc(sizeof(file_type));
        char* name = (char*)malloc(strlen(type) * sizeof(char));
        strcpy(name,type);
        type_head->name = name;
        type_head->next = NULL;
    }
    else{
        if(strcmp(type_head->name,type) == 0){
            fprintf(stderr, "%s\n", "File type already exists");
            return;
        }
        file_type* current_type = type_head;
        while(current_type->next != NULL){
            if(strcmp(current_type->name,type) == 0){
                fprintf(stderr, "%s\n", "File type already exists");
                return;
            }
            current_type = current_type->next;
        }
        if(strcmp(current_type->name,type) == 0){
            fprintf(stderr, "%s\n", "File type already exists");
            return;
        }
        file_type* newType = (file_type*)malloc(sizeof(file_type));
        char* name = (char*)malloc(strlen(type) * sizeof(char));
        strcpy(name,type);
        newType->name = name;
        newType->next = NULL;
        current_type->next = newType;
    }
}

int isTypeCommand(char* line){
    if(*line == 't'){
        if(*(line + 1) == 'y'){
            if(*(line + 2) == 'p'){
                if(*(line + 3) == 'e'){
                    if(*(line + 4) == ' '){
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int isHelp(char* line){
    if(*line == 'h'){
        if(*(line + 1) == 'e'){
            if(*(line + 2) == 'l'){
                if(*(line + 3) == 'p'){
                    if(*(line + 4) == '\0'){
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int isQuit(char* line){
    if(*line == 'q'){
        if(*(line + 1) == 'u'){
            if(*(line + 2) == 'i'){
                if(*(line + 3) == 't'){
                    if(*(line + 4) == '\0'){
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}