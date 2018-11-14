#ifndef HELPER_H
#define HELPER_H


#include "imprimer.h"

int isHelp(char* line);
int isQuit(char* line);
int isTypeCommand(char* line);
int isPrinter(char* line);
int isConversion(char* line);
int getNumOfWords(char* line);


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

void addFileType(char* type);
void addPrinter(char* name, char* type);
void addConvertFile(char* type1, char* type2, char* cp);
int getFTIndex(char* line);
int getPNIndex(char* line);
int getLen(char* line);
int typeExisted(char* fileType);
void addToPrinterSet(int id);

file_type* type_head;
printer_address* printer_head;
convertible* convert_head;
PRINTER_SET printer_set;


#endif
