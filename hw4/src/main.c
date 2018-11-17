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
#include <unistd.h>
#include "graph.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

/*
 * "Imprimer" printer spooler.
 */

int main(int argc, char *argv[])
{
    type_head = NULL;
    printer_head = NULL;
    convert_head = NULL;
    VHead = NULL;
    printer_set = 0;
    eligible_printers = &printer_set;
    char *line_read;

    char optval;
    FILE* input_file;
    FILE* output_file;
    int batch_mode = 0;
    int output_mode = 0;
    if(argc > 1){
        while(optind < argc) {
        if((optval = getopt(argc, argv, "i:o:")) != -1) {
            if(strcmp(argv[1],"-i") != 0 && strcmp(argv[1],"-o") != 0){
                fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
            }
            switch(optval) {
                case 'i':
                    input_file = fopen(optarg,"r");
                    batch_mode = 1;
                    if(input_file == NULL){
                        fprintf(stderr, "%s\n", "File does NOT exit");
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'o':
                    output_file = fopen(optarg,"w");
                    output_mode = 1;
                    dup2(fileno(output_file),1);
                    break;
               case '?':
                  fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
                  exit(EXIT_FAILURE);
                  break;
            default:
                break;
            }
        }
        }
    }

while(1){
    if(batch_mode == 1){
        char* c;
        if((c = fgets(batchBuff,1024,input_file)) == NULL){
            break;
        }
        else{
            line_read = malloc(strlen(c)*sizeof(char));
            strcpy(line_read,strtok(c,"\n"));

        }
    }
    else if(output_mode == 1){

        exit(EXIT_SUCCESS);
    }
    else{
        line_read = readline ("imp> ");
    }

    if(strcmp(line_read,"") == 0){
        continue;
    }
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
    else if(strcmp(strtok(line_read," "),"conversion") == 0){
        char* string1 = strtok(line_read + 11," ");
        char* string2 = strtok(line_read + 12 + getLen(line_read + 11), " ");
        char* string3 = line_read + 11 + getLen(string1) + getLen(string2) + 2;


        char* fileType1 = malloc(strlen(string1)*sizeof(char));
        char* fileType2 = malloc(strlen(string2)*sizeof(char));
        strcpy(fileType1, string1);
        strcpy(fileType2,string2);
        if(typeExisted(fileType1) == 0){
            free(fileType1);
            free(fileType2);
            fprintf(stderr, "%s\n", "file_type1 has not been declared");
            continue;
        }
        else if(typeExisted(fileType2) == 0){
            free(fileType1);
            free(fileType2);
            fprintf(stderr, "%s\n", "file_type2 has not been declared");
            continue;
        }
        addVertex(fileType1);
        addVertex(fileType2);
        addAjdVertex(fileType1,fileType2);

        char* conversion_program = malloc(strlen(string3)*sizeof(char));
        strcpy(conversion_program,string3);
        addConvertFile(fileType1,fileType2,conversion_program);
    }
    else if(strcmp(strtok(line_read, " "),"printers") == 0){
        printer_address* printer = printer_head;
        while(printer != NULL){
            fprintf(stdout, "%s", "PRINTER, ");
            fprintf(stdout, "%d, ", printer->printer->id);
            fprintf(stdout, "%s, ", printer->printer->name);
            fprintf(stdout, "%s, ", printer->printer->type);
            if(printer->printer->enabled == 1){
                fprintf(stdout, "%s, ", "enabled");
            }
            else{
                fprintf(stdout, "%s, ", "disabled");
            }
            if(printer->printer->busy == 1){
                fprintf(stdout, "%s, ", "busy");
            }
            else{
                fprintf(stdout, "%s\n", "idle");
            }
            printer = printer->next;
        }
    }
    else if(strcmp(strtok(line_read, " "),"jobs") == 0){
        char* s;
        jobNode* job = jobHead;
        while(job != NULL){
            s = imp_format_job_status(job->job,buff,20000);
            fprintf(stdout, "%s\n", s);
            job = job->nextJob;
        }
        free(s);
    }
    else if(strcmp(strtok(line_read, " "),"print") == 0){
        char* fileName = strtok(line_read + 6, " ");
        char *extension = strrchr(fileName, '.') + 1;
        if(typeExisted(extension) == 0){
            fprintf(stderr, "%s\n", "File Type has not been declared.");
            continue;
        }
        char* printers = fileName+strlen(fileName)+1;
        int printers_length = strlen(printers);
        if(printers_length > 0){
            int printedWithOptional = 0;
            while(printers_length > 0){
                char* printerName = strtok(printers," ");
                if(printerExisted(printerName) == 0){
                    fprintf(stderr, "%s ", printerName);
                    fprintf(stderr, "%s\n","has not been defined");
                    break;
                }
                else{
                    if(printerAvailable(printerName,extension) == 1){
                        PRINTER* printer = getPrinter(printerName);
                        printSameFileType(fileName,extension,printer);
                        printedWithOptional = 1;
                        break;
                    }
                }
                printers = printers + strlen(printerName) + 1;
                printers_length = printers_length - strlen(printerName) - 1;
            }
            if(printedWithOptional == 1){
                line_read = NULL;
                free(line_read);
                continue;
            }
        }
        PRINTER* eligible_printer = NULL;
        printer_address* printer_address = printer_head;
        while(printer_address != NULL){
            if(strcmp(printer_address->printer->type,extension) == 0 && printer_address->printer->busy == 0 && printer_address->printer->enabled == 1){
                eligible_printer = printer_address->printer;
            }
            printer_address = printer_address->next;
        }
        if(eligible_printer != NULL){
            printSameFileType(fileName, extension, eligible_printer);
        }
        else{
            //now, we need to do conversion pipeline
        }

    }
    else if(strcmp(strtok(line_read, " "),"enable") == 0){
        char* printerName = strtok(line_read + 7, " ");
        printer_address* printer_address = printer_head;
        while(printer_address != NULL){
            if(strcmp(printer_address->printer->name,printerName) == 0){
                printer_address->printer->enabled = 1;
                addToPrinterSet(printer_address->printer->id);
                break;
            }
            printer_address = printer_address->next;
        }
    }
    else if(strcmp(strtok(line_read, " "),"disable") == 0){
        char* printerName = strtok(line_read + 8, " ");
        printer_address* printer_address = printer_head;
        while(printer_address != NULL){
            if(strcmp(printer_address->printer->name,printerName) == 0){
                printer_address->printer->enabled = 0;
                rmFromPrinterSet(printer_address->printer->id);
                break;
            }
            printer_address = printer_address->next;
        }
    }
    line_read = NULL;
    free(line_read);
}
exit(EXIT_SUCCESS);
}


