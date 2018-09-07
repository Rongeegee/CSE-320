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
    return 1;
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

    while(**argv != '\0' ){
        if(curr_pos == 1){
            if(**argv != '-'){
                return 0;
            }
            (*argv)++;

            if(**argv == 'h'){
               (*argv)++;
                if(**argv == '\0'){
                    scenario = 1;
                    return 1;
                }
                else{
                    return 0;
                }
            }
            else if(**argv == 'u' || **argv == 'd'){
                (*argv)++;
                if(**argv == '\0'){
                    scenario = 2;
                }
                else
                    return 0;
            }
            else if(**argv == 'c'){
                debug("%s","1");
                (*argv)++;
                if(**argv == '\0'){
                    scenario = 3;
                    if(argc < 3){
                        return 0;
                    }
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
                    else if(argc < 4)
                        return 0;
                }
                else
                    return 0;
            }
            else if(scenario == 3){
                (*argv)++;
                if(**argv == 'k'){
                    debug("%s","2");
                    (*argv)++;
                    if(**argv != '\0')
                        return 0;
               }
                else
                    return 0;
            }
        }
        else if(curr_pos == 3){
                if(scenario == 2){
                    //call the verifyFactor method here.
                    if(!verifyFactor(argv))
                        return 0;
                }
                else if(scenario == 3){
                    debug("%s","3");
                    if(!verifyKey(argv))
                        return 0;
                }
        }
        else if(curr_pos == 4){
                if(scenario == 2 || scenario == 3){
                    debug("%s","4");
                    if(**argv != '-'){
                         return 0;
                     }
                    (*argv)++;
                    if(**argv != 'p')
                         return 0;
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
