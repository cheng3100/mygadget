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
#include <time.h>
#include <unistd.h>

#include "morse.h"
#include <string.h>

#ifdef RASPBERRY_PI
#include <pigpiod_if2.h>
#define ON 1
#define OFF 0
#endif

void encode_char(char letter)
{
    char *morse_code_string;
    
    morse_code_string = morse_code[(int)letter];

    while (*morse_code_string){
        printf("%c", *morse_code_string++);
    }
    return;
}

void display_message(struct start_options options) {

    char *pstring = options.message;

    while (*pstring) {
        encode_char(*pstring++);
        printf(" ");
    }
    printf("\n");
    return;
}


