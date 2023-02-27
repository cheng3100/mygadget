/*
 * morse, it will display text files via Morse Code
 *
 * Copyright (C) 2019  David I. S. Mandala
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * David I. S. Mandala davidm@them.com
 * 120 E. FM 544
 * Suite 72, BX 107
 * Murphy, TX 75094
 *
 */
  
#include <stdio.h>  
#include <stdlib.h>
#include <unistd.h>  
#include <string.h>

#include "morse.h"

#define TOTAL_WORD_BITS 50
#define STANDARD_WORD_BITS 31
#define FARNSWORTH_WORD_BITS 19
#define SECONDS 60.0

#define TEN_E6 1000000L

void display_help(void)
{
    printf("Morse may be called with command line options\n\n");
    printf("    -e encode morse code from ascii");
    printf("    -d deconde morse code to ascii");
    printf("    -f <file_name> Sets the text file. It could be normal ascii file(encode, with -e) or morse code text file(with -d)  File paths are allowed (expected).\n");
    printf("    -s <msg> Sets the input string to be encoded or decode with Morse code. \n");
    printf("      -h or -H displays this text.\n\n");
    printf(" \"$ morse -tS -f example.txt\"\n");
    printf("\n\n");
    return;
}

  
void process_command_line(int argc, char *argv[], struct start_options *options)
{
    int opt;

    // put ':' in the starting of the 
    // string so that program can  
    //distinguish between '?' and ':'  
    while((opt = getopt(argc, argv, ":def:hs:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'h':
                display_help();
                exit(-2);
                break;
            case 'd':
                options->mode = MORS_DECO;
                break;
            case 'e':
                options->mode = MORS_ENCO;
                break;
            case 'f':  
                options->filename = optarg;
                break;  
            case 's':
                options->message = optarg;
                break;
            case ':':  
                printf("option needs a value\n");
                display_help();
                exit(-1);  
                break;  
            case '?':  
                printf("unknown option: %c\n", optopt);
                display_help();
                exit(-2); 
                break;  
        }  
    }  

    // optind is for the extra arguments 
    // which are not parsed 
    for(; optind < argc; optind++){
        options->filename = argv[optind];
        printf("extra arguments: %s\n", options->filename);  
    }

    /* 
     * Some final checks, file name must be set or it's an error, 
     * Farnsworth timing only used below 18 WPM
     */
    if (options->filename == NULL && options->message == NULL){
        printf("A ascii file with -f or a string with -s is need\n");
        exit(-1);
    }      

    return;
}
