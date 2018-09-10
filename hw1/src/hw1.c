#include <stdlib.h>

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
int recode(char **argv) {
    return 0;
}
