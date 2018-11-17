#ifndef HELPER_H
#define HELPER_H


#include "imprimer.h"

int isHelp(char* line);
int isQuit(char* line);
int isTypeCommand(char* line);
int isPrinter(char* line);
int isConversion(char* line);
int getNumOfWords(char* line);
void addJob();

typedef struct file_type
{
  char *name;
  struct file_type *next;
} file_type;

typedef struct printer_address
{
    PRINTER* printer;
    struct printer_address *next;
} printer_address;

typedef struct convertible
{
    char* original_type;
    char* new_type;
    char* conversion_program;
    struct convertible *next;
}convertible;

typedef struct jobNode{
    JOB* job;
    struct jobNode* nextJob;
}jobNode;

void addFileType(char* type);
void addPrinter(char* name, char* type);
void addConvertFile(char* type1, char* type2, char* cp);
int getFTIndex(char* line);
int getPNIndex(char* line);
int getLen(char* line);
int typeExisted(char* fileType);
void addToPrinterSet(int id);
void rmFromPrinterSet(int id);
int printerExisted(char* printerName);
int printerAvailable(char* printerName,char* fileType);
PRINTER* getPrinter(char* printerName);
JOB* getJob(char* fileName, char* fileType, PRINTER* printer);
void printSameFileType(char* fileName, char* extension,PRINTER* eligible_printer);

file_type* type_head;
printer_address* printer_head;
convertible* convert_head;
PRINTER_SET printer_set;
jobNode* jobHead;
PRINTER_SET printer_set;
PRINTER_SET* eligible_printers;

char buff[20000];
char batchBuff[1024];
#endif
