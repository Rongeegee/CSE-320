#include "helper.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "imprimer.h"
#include "graph.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

void addJob(JOB* job){
    jobNode* newJobNode;
    if(jobHead == NULL){
       jobHead = malloc(sizeof(job));
       jobHead->job = job;
       jobHead->nextJob = NULL;
    }
    else{
        jobNode* jobNode = jobHead;
        while(jobNode->nextJob != NULL){
            jobNode = jobNode->nextJob;
        }
        newJobNode = malloc(sizeof(jobNode));
        newJobNode->job = job;
        newJobNode->nextJob = NULL;
        jobNode->nextJob = newJobNode;
    }
}

JOB* getJob(char* fileName, char* fileType,PRINTER* printer){
    if(jobHead == NULL){
        JOB* job = malloc(sizeof(JOB));
        job->jobid = 0;
        job->status = QUEUED;
        int pgid = getpid();
        job->pgid = getpgid(pgid);
        char* fn = malloc(strlen(fileName)*sizeof(char));
        char* ft = malloc(strlen(fileType)*sizeof(char));
        strcpy(fn,fileName);
        strcpy(ft,fileType);
        job->file_name = fn;
        job->file_type =ft;
        job->eligible_printers = *eligible_printers;

        job->chosen_printer = printer;
        job->eligible_printers = printer_set;
        gettimeofday(&job->creation_time,NULL);
        gettimeofday(&job->change_time ,NULL);
        job->other_info = NULL;
        return job;
    }
    else{
        jobNode* jobNode = jobHead;
        while(jobNode->nextJob != NULL){
            jobNode = jobNode->nextJob;
        }
        JOB* job = malloc(sizeof(JOB));
        job->jobid = (job->jobid) + 1 ;
        job->status = QUEUED;
        int pgid = getpid();
        job->pgid = getpgid(pgid);
        char* fn = malloc(strlen(fileName)*sizeof(char));
        char* ft = malloc(strlen(fileType)*sizeof(char));
        strcpy(fn,fileName);
        strcpy(ft,fileType);
        job->file_name = fn;
        job->file_type =ft;
        job->eligible_printers = *eligible_printers;

        job->chosen_printer = printer;
        job->eligible_printers = printer_set;
        gettimeofday(&job->creation_time,NULL);
        gettimeofday(&job->change_time ,NULL);
        job->other_info = NULL;
        return job;
    }
}
int printerAvailable(char* printerName,char* fileType){
    printer_address* current_printer_address = printer_head;
    while(current_printer_address != NULL){
        if(strcmp(printerName,current_printer_address->printer->name) == 0){
            if(strcmp(current_printer_address->printer->type,fileType) == 0){
                return 1;
            }
        }
        current_printer_address = current_printer_address->next;
    }
    return 0;
}

int printerExisted(char* printerName){
    printer_address* current_printer_address = printer_head;
    while(current_printer_address != NULL){
        if(strcmp(printerName,current_printer_address->printer->name) == 0){
            return 1;
        }
        current_printer_address = current_printer_address->next;
    }
    return 0;
}

PRINTER* getPrinter(char* printerName){
    printer_address* current_printer_address = printer_head;
    while(current_printer_address != NULL){
        if(strcmp(printerName,current_printer_address->printer->name) == 0){
            return current_printer_address->printer;
        }
        current_printer_address = current_printer_address->next;
    }
    return NULL;
}
void printSameFileType(char* fileName, char* extension,PRINTER* eligible_printer){
    pid_t childPid  = fork();
    eligible_printer->busy = 1;
    JOB* job = getJob(fileName, extension,eligible_printer);
    addJob(job);
    rmFromPrinterSet(eligible_printer->id);
    job->status = RUNNING;
    gettimeofday(&job->change_time,NULL);
    if(childPid  == 0){
        int fdOfFile = open(fileName,O_RDONLY);
        dup2(fdOfFile,0);
        char* rv[] = {"/bin/cat",NULL};
        int fd = imp_connect_to_printer(eligible_printer,PRINTER_NORMAL);
        dup2(fd,1);
        execve(rv[0],rv,NULL);
        exit(EXIT_FAILURE);
    }
    else if(childPid < 0){
        printErrorMsg("An error has occur");
        return;
    }
    else{
        int returnStatus;
        waitpid(childPid, &returnStatus, 0);
        gettimeofday(&job->change_time,NULL);
        addToPrinterSet(eligible_printer->id);
        job->status = COMPLETED;
    }
}

void printOldJob(JOB* job, char* fileName, char* extension,PRINTER* eligible_printer){
    pid_t childPid  = fork();
    eligible_printer->busy = 1;
    rmFromPrinterSet(eligible_printer->id);
    job->status = RUNNING;
    gettimeofday(&job->change_time,NULL);
    if(childPid  == 0){
        int fdOfFile = open(fileName,O_RDONLY);
        dup2(fdOfFile,0);
        char* rv[] = {"/bin/cat",NULL};
        int fd = imp_connect_to_printer(eligible_printer,PRINTER_NORMAL);
        dup2(fd,1);
        execve(rv[0],rv,NULL);
        exit(EXIT_FAILURE);
    }
    else if(childPid < 0){
        printErrorMsg("An error has occur");
        return;
    }
    else{
        int returnStatus;
        waitpid(childPid, &returnStatus, 0);
        gettimeofday(&job->change_time,NULL);
        addToPrinterSet(eligible_printer->id);
        job->status = COMPLETED;
    }
}

int getNumOfWords(char* line){
    int length = 0;
    int numOfwords = 0;
    if(*line == '\0')
        return 0;
    while(*line != '\0'){
        if(*line == ' '){
            numOfwords++;
        }
        line++;
        length++;
    }
    numOfwords++;
    while(length != 0){
        line--;
        length--;
    }
    return numOfwords;
}

int typeExisted(char* fileType){
    file_type* type = type_head;
    while(type != NULL){
        if(strcmp(type->name, fileType) == 0){
            return 1;
        }
        type = type->next;
    }
    return 0;
}

void rmFromPrinterSet(int id){
    int number = 4294967295;
    *eligible_printers = (~((1 << id) & number)) & *eligible_printers;
}


void addToPrinterSet(int id){
    *eligible_printers |= (1 << id);
}

void addConvertFile(char* type1, char* type2,char* cp){
    if(convert_head == NULL){
        convert_head = (convertible*)malloc(sizeof(convertible));
        convert_head->original_type = type1;
        convert_head->new_type = type2;
        convert_head->conversion_program = cp;
        convert_head->next = NULL;
    }
    else{
        convertible* conversion = convert_head;
        while(conversion->next != NULL){
            if((strcmp(conversion->original_type,type1) == 0) && (strcmp(conversion->new_type, type2) == 0) && (strcmp(conversion->conversion_program,cp) == 0)){
                free(type1);
                free(type2);
                free(cp);
                return;
            }
            conversion = conversion->next;
        }
        if((strcmp(conversion->original_type,type1) == 0) && (strcmp(conversion->new_type, type2) == 0) && (strcmp(conversion->conversion_program,cp) == 0)){
            free(type1);
            free(type2);
            free(cp);
            return;
        }
        convertible* convertTypes = (convertible*)malloc(sizeof(convertible));
        convertTypes->original_type = type1;
        convertTypes->new_type = type2;
        convertTypes->conversion_program = cp;
        convertTypes->next = NULL;
        conversion->next = convertTypes;
    }
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

void printErrorMsg(char* msg){
    printf("%s\n",imp_format_error_message(msg,errorMsg,60));
}

void addPrinter(char* name, char* type){
    if(type_head == NULL){
        printErrorMsg("File type has not been declared.");
        free(name);
        free(type);
        return;
    }
    file_type* FT = type_head;
    while(FT != NULL){
        if(strcmp(FT->name,type) == 0)
            break;
        if(FT->next == NULL){
            printErrorMsg("File type has not been declared");
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
        printer->enabled = 0;
        printer->busy = 0;
        printer->other_info = NULL;
        printer_head->next = NULL;
    }
    else{
        printer_address* current_printer = printer_head;
        while(current_printer->next != NULL){
            if(current_printer->printer->id == 31){
                printErrorMsg("Number of Printer is already maximum");
                free(name);
                free(type);
                return;
            }
            else if(strcmp(current_printer->printer->name, name) == 0){
                printErrorMsg("Printer name already exists");
                free(name);
                free(type);
                return;
            }
            current_printer = current_printer->next;
        }
        if(strcmp(current_printer->printer->name, name) == 0){
                printErrorMsg("Printer name already exists");
                free(name);
                free(type);
                return;
            }
        PRINTER* printer = (PRINTER*) malloc(sizeof(PRINTER));
        printer_address* printer_address = malloc(sizeof(printer_address));
        printer->id = current_printer->printer->id + 1;
        printer->name = name;
        printer->type = type;
        printer->enabled = 0;
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
            printErrorMsg("File type already exists");
            return;
        }
        file_type* current_type = type_head;
        while(current_type->next != NULL){
            if(strcmp(current_type->name,type) == 0){
                printErrorMsg("File type already exists");
                return;
            }
            current_type = current_type->next;
        }
        if(strcmp(current_type->name,type) == 0){
            printErrorMsg("File type already exists");
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