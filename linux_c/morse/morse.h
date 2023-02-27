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

#ifndef MORSE_C
#define MORSE_C

// #define RASPBERRY_PI
#define CONSOLE_MORSE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#define MORSE_DEBUG 0

#if (MORSE_DEBUG == 1)
#define pr_dbg(fmt, ...) \
    printf("[debug]:"fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...) \
    printf("[error]:"fmt, ##__VA_ARGS__)
#else
#define pr_dbg(fmt, ...)
#define pr_err(fmt, ...)
#endif

enum {
    MORS_ENCO,
    MORS_DECO
};

void hmorse_init();
void hmorse_relase();
char morse2char(char *s);

extern char *morse_code[];

struct start_options {
    int fd;
    struct stat fileInfo;
    char *filename;			// Text file to open
    char * message;			// Pointer to the text to send
    int mode;
    };

int sizeof_morsecode();

extern void display_message(struct start_options options);
extern void process_config_file(struct start_options *options);
extern void process_command_line(int argc, char *argv[], struct start_options *options);
extern void open_text_file(struct start_options *options);
extern void close_text_file(struct start_options options);

// encode/decode
extern void morse_decode(struct start_options options);

#define DOT_FILE_NAME ".morsecode.cfg"
#define ETC_FILE_PATH_AND_NAME "/etc/morsecode.cfg"

#endif
