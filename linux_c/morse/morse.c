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
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include<signal.h>

#include "morse.h"

static struct start_options options;

#define OFF 0

void sig_handler(int signo)
{
    if (signo == SIGINT){
        close_text_file(options);
#ifdef RASPBERRY_PI
    // All done, close the gpio code
        gpio_write(options.gpio_pi, (unsigned)options.port, OFF);
        pigpio_stop(options.gpio_pi);
#endif
        hmorse_relase();
        exit(0);
  }
}

int main(int argc, char* argv[]) {
//    struct start_options options;
    // Set some sane values to the options struct.
    pr_dbg("argc:%d\n", argc);
    memset(&options, 0, sizeof(options));
    pr_dbg("test\n");

    if (signal(SIGINT, sig_handler) == SIG_ERR){
        printf("\ncan't catch SIGINT, exiting\n");
        exit(-3);
    }
    // now process any command line arguments
    process_command_line(argc, argv, &options);

    if (options.filename)
        printf("Options: filename = %s, ready to encode morse code...\n", 
                       options.filename);

    open_text_file(&options);
    if (options.mode == MORS_ENCO)
        display_message(options);
    else
        morse_decode(options);
    close_text_file(options);

    return(0);
}
