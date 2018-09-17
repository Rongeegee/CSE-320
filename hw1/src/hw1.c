#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "hw1.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int verifyFactor(char** argv){
    int length = 0;
    while(**argv != '\0'){
        if(**argv < 48 || **argv > 57)
            return 0;
        length++;
        (*argv)++;
    }
    int factor_len = length;
    while(length != 0){
        (*argv)--;
        length--;
    }
    if(factor_len > 4){
        return 0;
    }

    else if(factor_len < 4){
        if(**argv == '0'){
            return 0;
        }
        else
            return 1;
    }
    else if(factor_len == 4){
        if(**argv != '1'){
            return 0;
         }
        (*argv)++;
        if(**argv != '0'){
            return 0;
        }
        (*argv)++;
        if(**argv != '0' && **argv != '1' && **argv != '2'){
            return 0;
        }
        (*argv)++;
        if(**argv != '0' && **argv != '1' && **argv != '2' && **argv != '3' && **argv != '4'){
            return 0;
        }
        return 1;
    }
        return 1;
}

int verifyKey(char** argv){
    int length = 0;
    while(**argv != '\0'){
        if(**argv < 48 || **argv >102)
            return 0;
        else if (**argv > 57 && **argv < 65)
            return 0;
        else if(**argv > 70 && **argv < 97)
            return 0;
        length++;
        (*argv)++;
    }
    if(length > 8)
        return 0;
    while(length != 0){
        (*argv)--;
        length--;
    }
    return 1;
}

/**
 * @brief return the int value of a digit in a character form.
 *
 */
int getIntFromChar(char digit){
    if(digit == '0')
        return 0;
    else if(digit == '1')
        return 1;
    else if(digit == '2')
        return 2;
    else if(digit == '3')
        return 3;
    else if(digit == '4')
        return 4;
    else if(digit == '5')
        return 5;
    else if(digit == '6')
        return 6;
    else if(digit == '7')
        return 7;
    else if(digit == '8')
        return 8;
    else
        return 9;
}

/**
 * @brief return the decimal digit of a hex number in a character form.
 *
 */

int getDigitFromHex(char digit){
    if(digit == '0')
        return 0;
    else if(digit == '1')
        return 1;
    else if(digit == '2')
        return 2;
    else if(digit == '3')
        return 3;
    else if(digit == '4')
        return 4;
    else if(digit == '5')
        return 5;
    else if(digit == '6')
        return 6;
    else if(digit == '7')
        return 7;
    else if(digit == '8')
        return 8;
    else if(digit == '9')
        return 9;
    else if(digit == 'A' || digit == 'a')
        return 10;
    else if(digit == 'B' || digit == 'b')
        return 11;
    else if(digit == 'C' || digit == 'c')
        return 12;
    else if(digit == 'D' || digit == 'd')
        return 13;
    else if(digit == 'E' || digit == 'e')
        return 14;
    else
        return 15;
}

/**
 * @brief return the decimal value of a hex number in a character form.
 *
 */
int getDecFromHex(char** argv){
    int length = 0;
    while(**argv != '\0'){
        (*argv)++;
        length++;
    }
    int current_bit = 0;
    int decValue = 0;
    (*argv)--;

    while(current_bit < length){
        if(current_bit == 0)
            decValue += getDigitFromHex(**argv);

        else if(current_bit == 1)
            decValue += (getDigitFromHex(**argv) * 16);

        else if(current_bit == 2)
            decValue += (getDigitFromHex(**argv) * 16 * 16);

        else if(current_bit == 3)
            decValue += (getDigitFromHex(**argv) * 16 * 16 * 16);

        else if(current_bit == 4)
            decValue += (getDigitFromHex(**argv) * 16 * 16 * 16 * 16);

        else if(current_bit == 5)
            decValue += (getDigitFromHex(**argv) * 16 * 16 * 16 * 16 * 16);

        else if(current_bit == 6)
            decValue += (getDigitFromHex(**argv) * 16 * 16 * 16 * 16 * 16 * 16);

        else
            decValue += (getDigitFromHex(**argv) * 16 * 16 * 16 * 16 * 16 * 16 * 16);
        (*argv)--;
        current_bit++;
    }
    return decValue;
}

/**
 * @brief set the seventh- through sixteenth-most-significant bits (bits 57 - 48) to the factor (minus one) if the -f option was specified
 *
 */
void setGlobalOptF(char** argv){
    int length = 0;
    while(**argv != '\0'){
        length++;
        (*argv)++;
    }
    int factor_len = length;
    while(length != 0){
        length--;
        (*argv)--;
    }
    int factor_minus_1;
    if(factor_len == 1)
        factor_minus_1 = getIntFromChar(**argv)-1;

    else if(factor_len == 2){
        int tens = getIntFromChar(**argv) * 10;
        (*argv)++;
        int ones = getIntFromChar(**argv);
        factor_minus_1 = tens + ones - 1;
        (*argv)--;
    }
    else if(factor_len == 3){
        int hundreds = getIntFromChar(**argv) * 100;
        (*argv)++;
        int tens = getIntFromChar(**argv) * 10;
        (*argv)++;
        int ones = getIntFromChar(**argv);
        factor_minus_1 = hundreds + tens + ones - 1;
        (*argv) = (*argv) - 2;
    }
    else if(factor_len == 4){
        int thousands = getIntFromChar(**argv) * 1000;
        (*argv)++;
        int hundreds = getIntFromChar(**argv) * 100;
        (*argv)++;
        int tens = getIntFromChar(**argv) * 10;
        (*argv)++;
        int ones = getIntFromChar(**argv);
        factor_minus_1 = thousands + hundreds + tens + ones - 1;
        (*argv) = (*argv) - 3;
    }
    global_options = global_options >> 48;
    global_options = global_options & 64512;
    global_options = global_options | factor_minus_1;
    global_options = global_options << 48;
}
/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the content of three frames of audio data and
 * two annotation fields have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 1 if validation succeeds and 0 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variables "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 1 if validation succeeds and 0 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char** argv)
{
    if(argc == 0){
        return 0;
    }
    argv++;
    int curr_pos = 1;
    int scenario;
    int curr_string = 2;
    int fComeFirst;

    while(curr_pos < argc){
        if(curr_pos == 1){
            if(**argv != '-'){
                return 0;
            }
            (*argv)++;

            if(**argv == 'h'){
               (*argv)++;
                if(**argv == '\0'){
                    scenario = 1;
                    global_options = global_options >> 63;
                    global_options = (global_options | 1);
                    global_options = global_options << 63;
                    
                    return 1;
                }
                else{
                    return 0;
                }
            }
            else if(**argv == 'u'){
                (*argv)++;
                if(**argv == '\0'){
                    scenario = 2;
                    global_options = global_options >> 62;
                    global_options = (global_options | 1);
                    global_options = global_options << 62;
                }
                else
                    return 0;
            }
            else if(**argv == 'd'){
                (*argv)++;
                if(**argv == '\0'){
                    scenario = 2;
                    global_options = global_options >> 61;
                    global_options = (global_options | 1);
                    global_options = global_options << 61;

                }
                else
                    return 0;
            }

            else if(**argv == 'c'){
                (*argv)++;
                if(**argv == '\0'){
                    scenario = 3;
                    if(argc < 3){
                        return 0;
                    }
                    global_options = global_options >> 60;
                    global_options = (global_options | 1);
                    global_options = global_options << 60;
                }
                else
                    return 0;
            }
            else{
                return 0;
            }
        }

        else if(curr_pos == 2){
            if(**argv != '-'){
                return 0;
            }
            if(scenario == 2){
               (*argv)++;
                if(**argv == 'f'){
                    (*argv)++;
                    if(**argv != '\0')
                        return 0;
                    if(argc < 4)
                        return 0;
                    fComeFirst = 1;
                    argv++;
                    setGlobalOptF(argv);
                    argv--;
                }
                else if(**argv == 'p'){
                    (*argv)++;
                    if(**argv != '\0')
                        return 0;
                    global_options = global_options >> 59;
                    global_options = (global_options | 1);
                    global_options = global_options << 59;
                    fComeFirst = 0;
                }
                else
                    return 0;
            }
            else if(scenario == 3){
                (*argv)++;
                if(**argv == 'k'){
                    (*argv)++;
                    if(**argv != '\0')
                        return 0;
                    if(argc < 4)
                        return 0;
               }
                else
                    return 0;
            }
        }
        else if(curr_pos == 3){
                if(scenario == 2){
                    if (fComeFirst == 1){
                        if(!verifyFactor(argv))
                           return 0;
                    }
                    else if(fComeFirst == 0){
                        if(**argv != '-'){
                            return 0;
                        }
                        (*argv)++;
                        if(**argv != 'f')
                            return 0;
                        (*argv)++;
                        if(**argv != '\0'){
                            return 0;
                        }
                        if(argc < 5)
                            return 0;
                    }
                }
                else if(scenario == 3){
                    if(!verifyKey(argv))
                        return 0;
                    unsigned long key = getDecFromHex(argv);
                    global_options = (global_options | key);

                }
        }
        else if(curr_pos == 4){
                if(scenario == 2){
                    if(fComeFirst == 1){
                        if(**argv != '-'){
                             return 0;
                        }
                        (*argv)++;
                        if(**argv != 'p')
                             return 0;
                        (*argv)++;
                        if(**argv != '\0')
                             return 0;
                        global_options = global_options | 576460752303423488;
                        return 1;
                    }
                    else if(fComeFirst == 0){
                        if(!verifyFactor(argv))
                            return 0;
                        else
                            return 1;
                    }
                }
                else if(scenario == 3){
                    if(**argv != '-'){
                         return 0;
                     }
                    (*argv)++;
                    if(**argv != 'p')
                         return 0;
                    (*argv)++;
                    if(**argv != '\0')
                         return 0;
                    global_options = global_options | 576460752303423488;
                    return 1;
                }

        }

        argv++;
        curr_pos++;
      }


    return 1;
}




/**
 * @brief  Recodes a Sun audio (.au) format audio stream, reading the stream
 * from standard input and writing the recoded stream to standard output.
 * @details  This function reads a sequence of bytes from the standard
 * input and interprets it as digital audio according to the Sun audio
 * (.au) format.  A selected transformation (determined by the global variable
 * "global_options") is applied to the audio stream and the transformed stream
 * is written to the standard output, again according to Sun audio format.
 *
 * @param  argv  Command-line arguments, for constructing modified annotation.
 * @return 1 if the recoding completed successfully, 0 otherwise.
 */
int recode(char **argv){
    return 0;
}

/**
 * @brief Read the header of a Sun audio file and check it for validity.
 * @details  This function reads 24 bytes of data from the standard input and
 * interprets it as the header of a Sun audio file.  The data is decoded into
 * six unsigned int values, assuming big-endian byte order.   The decoded values
 * are stored into the AUDIO_HEADER structure pointed at by hp.
 * The header is then checked for validity, which means:  no error occurred
 * while reading the header data, the magic number is valid, the data offset
 * is a multiple of 8, the value of encoding field is one of {2, 3, 4, 5},
 * and the value of the channels field is one of {1, 2}.
 *
 * @param hp  A pointer to the AUDIO_HEADER structure that is to receive
 * the data.
 * @return  1 if a valid header was read, otherwise 0.
 */
int read_header(AUDIO_HEADER *hp){

    char c;
    char* charPointer = &c;

    int i;
    for(i = 0; i < 24; i++){
       *charPointer = getchar();
       charPointer++;    
    }
    charPointer = &c;

    //check the validity of magic number
    //if the number is 0x2e736e64 in big endian format, the first byte will be equivalent to '.', the second byte will be equivalent to 's', the third byte will be 'n', the 4th byte will be 'd'
    //an integer is 4 byte.
    for(i = 0; i < 4;i++){
        if (i == 0 && *charPointer != '.')
            return 0;
        if (i == 1 && *charPointer != 's')
            return 0;
        if (i == 2 && *charPointer != 'n')
            return 0;
        if (i == 3 && *charPointer != 'd')
            return 0;
        charPointer++;
    }
        
    //checking the data offset
    //only need to check if it's divisible by 8
    //move the pointer to check the last byte of the "data offset" to see if it ends with three 0's in binary
    charPointer = charPointer + 3;
    char tempChar = *charPointer << 5;
    if(tempChar != 0){
        return 0;
    }
    
    //checking the encoding field's validity
    charPointer = charPointer + 5;
    if(*charPointer != '\0')
        return 0;
    charPointer++;
    if(*charPointer != '\0')
        return 0;
    charPointer++;
    if(*charPointer != '\0')
        return 0;
    charPointer++;
    if(*charPointer != 2 && *charPointer != 3 && *charPointer != 4 && *charPointer != 5)
        return 0;    
    
    //checking the sixth field's validity, it can be either 1 or 2.
    charPointer = charPointer + 5;
    if(*charPointer != '\0')
        return 0;
    charPointer++;

    if(*charPointer != '\0')
        return 0;
    charPointer++;

    if(*charPointer != '\0')
        return 0;
    charPointer++;

    if(*charPointer != 1 && *charPointer != 2)
        return 0;
    
    //now, the data is valid, and we're gonna read it
    charPointer = &c;
    (*hp).magic_number = ((*hp).magic_number | (*charPointer << 24));
    charPointer++;
    (*hp).magic_number = ((*hp).magic_number | (*charPointer << 16));
    charPointer++;
    (*hp).magic_number = ((*hp).magic_number | (*charPointer << 8));
    charPointer++;
    (*hp).magic_number = ((*hp).magic_number | *charPointer);
    charPointer++;

    (*hp).data_offset = ((*hp).data_offset | (*charPointer << 24));
    charPointer++;
    (*hp).data_offset = ((*hp).data_offset | (*charPointer << 16));
    charPointer++;
    (*hp).data_offset = ((*hp).data_offset | (*charPointer << 8));
    charPointer++;
    (*hp).data_offset = ((*hp).data_offset | *charPointer );
    charPointer++;

    (*hp).data_size = ((*hp).data_size | (*charPointer << 24));
    charPointer++;
    (*hp).data_size = ((*hp).data_size | (*charPointer << 16));
    charPointer++;
    (*hp).data_size = ((*hp).data_size | (*charPointer << 8));
    charPointer++;
    (*hp).data_size = ((*hp).data_size | *charPointer );
    charPointer++;

    (*hp).encoding = ((*hp).encoding | (*charPointer << 24));
    charPointer++;
    (*hp).encoding = ((*hp).encoding | (*charPointer << 16));
    charPointer++;
    (*hp).encoding = ((*hp).encoding | (*charPointer << 8));
    charPointer++;
    (*hp).encoding = ((*hp).encoding | *charPointer );
    charPointer++;
    
    (*hp).sample_rate = ((*hp).sample_rate | (*charPointer << 24));
    charPointer++;
    (*hp).sample_rate = ((*hp).sample_rate | (*charPointer << 16));
    charPointer++;
    (*hp).sample_rate = ((*hp).sample_rate | (*charPointer << 8));
    charPointer++;
    (*hp).sample_rate = ((*hp).sample_rate | *charPointer );
    charPointer++;

    (*hp).channels = ((*hp).channels | (*charPointer << 24));
    charPointer++;
    (*hp).channels = ((*hp).channels | (*charPointer << 16));
    charPointer++;
    (*hp).channels = ((*hp).channels | (*charPointer << 8));
    charPointer++;
    (*hp).channels = ((*hp).channels | *charPointer );
   
   return 1;
}

/**
 * @brief  Write the header of a Sun audio file to the standard output.
 * @details  This function takes the pointer to the AUDIO_HEADER structure passed
 * as an argument, encodes this header into 24 bytes of data according to the Sun
 * audio file format specifications, and writes this data to the standard output.
 *
 * @param  hp  A pointer to the AUDIO_HEADER structure that is to be output.
 * @return  1 if the function is successful at writing the data; otherwise 0.
 */
int write_header(AUDIO_HEADER *hp){
    if(putchar(((*hp).magic_number >> 24)) == EOF)
            return 0;
    if(putchar((((*hp).magic_number & 16711680) >> 16)) == EOF)
            return 0;
    if(putchar((((*hp).magic_number & 65280) >> 8)) == EOF)
            return 0;
    if(putchar(((*hp).magic_number & 255) == EOF))               
            return 0;

    if(putchar(((*hp).data_offset >> 24)) == EOF)
            return 0;
    if(putchar((((*hp).data_offset & 16711680) >> 16)) == EOF)
            return 0;
    if(putchar((((*hp).data_offset & 65280) >> 8)) == EOF)
            return 0;
    if(putchar(((*hp).data_offset & 255) == EOF)
            return 0;

    if(putchar(((*hp).data_size >> 24)) == EOF)
            return 0;
    if(putchar((((*hp).data_size & 16711680) >> 16)) == EOF)
            return 0;
    if(putchar((((*hp).data_size & 65280) >> 8)) == EOF)
            return 0;
    if(putchar(((*hp).data_size & 255) == EOF)
            return 0;

    if(putchar(((*hp).encoding >> 24)) == EOF)
            return 0;
    if(putchar((((*hp).encoding & 16711680) >> 16)) == EOF)
            return 0;
    if(putchar((((*hp).encoding & 65280) >> 8)) == EOF)
            return 0;
    if(putchar(((*hp).encoding & 255) == EOF)
            return 0;

    if(putchar(((*hp).sample_rate >> 24)) == EOF)
            return 0;
    if(putchar((((*hp).sample_rate & 16711680) >> 16)))
            return 0;
    if(putchar((((*hp).sample_rate & 65280) >> 8)))
            return 0;
    if(putchar(((*hp).sample_rate & 255) == EOF)
            return 0;

    if(putchar(((*hp).channels >> 24)) == EOF)
            return 0;
    if(putchar((((*hp).channels & 16711680) >> 16)))
            return 0;
    if(putchar((((*hp).channels & 65280) >> 8)))
            return 0;
    if(putchar(((*hp).channels & 255) == EOF)
            return 0;

    return 1;
}

/**
 * @brief  Read annotation data for a Sun audio file from the standard input,
 * storing the contents in a specified buffer.
 * @details  This function takes a pointer 'ap' to a buffer capable of holding at
 * least 'size' characters, and it reads 'size' characters from the standard input,
 * storing the characters read in the specified buffer.  It is checked that the
 * data read is terminated by at least one null ('\0') byte.
 *
 * @param  ap  A pointer to the buffer that is to receive the annotation data.
 * @param  size  The number of bytes of data to be read.
 * @return  1 if 'size' bytes of valid annotation data were successfully read;
 * otherwise 0.
 */
int read_annotation(char *ap, unsigned int size){
    while(*ap != '\0'){
        
    }
}


