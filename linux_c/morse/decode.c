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

/*
 * PARIS is 50 dot units long.  5 * PARIS is 5 WPM
 *
 * Two types of timing: Standard where all timing is derived from the speed
 * of a '.' and Farnsworth code where the letters are sent at 18 WPM but the
 * space between letters and words are stretched to the lower speed, above
 * 18 WPM Farnsworth and stadard are the same, no extra shifting.
 * 
 * This stretched code is called Farnsworth code.
 * 
 * 1) The length of a dot is 1 time unit.
 * 2) A dash is 3 time units.
 * 3) The space between symbols (dots and dashes) of the same letter is 1 time unit.
 * 4) The space between letters is 3 time units.
 * 5) The space between words is 7 time units.
 * 6) Words end with a space or /n
 * 
 * Unless using Farnsworth timing, where 
 */

/* For fast lookups, have made an array of strings that represent all the
 * morse char's by using the ascii letter to be sent as the array offset.
 * Then walk the string pointed to until hit the null byte.  That will send 
 * a single char in morse.  Put the same morse code in the upper and lower 
 * case of each letter since there is no case in morse code.
 *
 * A space or \n will trigger a word wait.
 *
 * Between each *- of a letter of morse code, send a single time unit of 
 * silence/no light.
 *
 * At the end of each letter send 3 time units of silence/no light to
 * signify end of char.
 * 
 * When the string is a space send 7 time units of silence/no light to show
 * end of word.
 *  
 */
#include "morse.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


/* WARNING WARNING This array of strings is position sensitive WARNING WARNING */
/* Do NOT make changes unless you fully understand what this table does.       */
char *morse_code[] = {
	"", "", "", "", "", "", "", "", "", "",
	" ",		/* use space instead of \n Use this for timing of words, this is not in Morse code */
	"", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "",
	" ",	 	/* space Use this for timing of words, this is not in Morse code */	
	"-.-.--",	/* !	Not in ITU-R recommendation */
	".-..-.",	/* " */	
	"",		/* #	Char not in Morse Code */
	"",		/* $	Char not in Morse Code */
	"",		/* %	Char not in Morse Code */
	".-...",	/* & */	
	".----.",	/* ' */	
	"-.--.",	/* ( */	
	"-.--.-",	/* ) */	
	"",		/* *	Char not in Morse Code */
	".-.-.",	/* + */	
	"--..--",	/* , */	
	"-....-",	/* - */	
	".-.-.-",	/* . */	
	"-..-.",	/* / */	
	"-----",	/* 0 */	
	".----",	/* 1 */	
	"..---",	/* 2 */	
	"...--",	/* 3 */	
	"....-",	/* 4 */	
	".....",	/* 5 */	
	"-....",	/* 6 */		
	"--...",	/* 7 */		
	"---..",	/* 8 */		
	"----.",	/* 9 */		
	"---...",	/* : */		
	"",		/* ;	Char not in Morse Code */
	"",		/* <	Char not in Morse Code */
	"-...-",	/* = */	
	"",		/* >	Char not in Morse Code */
	"..--..",	/* ? */	
	".--.-.",	/* @ */	
	".-",		/* A */	
	"-...",		/* B */	
	"-.-.",		/* C */	
	"-..",		/* D */	
	".",		/* E */	
	"..-.",		/* F */	
	"--.",		/* G */	
	"....",		/* H */	
	"..",		/* I */	
	".---",		/* J */	
	"-.-",		/* K */	
	".-..",		/* L */	
	"--",		/* M */	
	"-.",		/* N */	
	"---",		/* O */	
	".--.",		/* P */	
	"--.-",		/* Q */	
	".-.",		/* R */	
	"...",		/* S */	
	"-",		/* T */	
	"..-",		/* U */	
	"...-",		/* V */	
	".--",		/* W */	
	"-..-",		/* X */	
	"-.--",		/* Y */	
	"--..",		/* Z */	
	"",		/* [	Char not in Morse Code */
	"",		/* \	Char not in Morse Code */
	"",		/* ]	Char not in Morse Code */
	"",		/* ^	Char not in Morse Code */
	"",		/* _	Char not in Morse Code */
	"",		/* `	Char not in Morse Code */
	".-",		/* a */	
	"-...",		/* b */	
	"-.-.",		/* c */	
	"-..",		/* d */	
	".",		/* e */	
	"..-.",		/* f */	
	"--.",		/* g */	
	"....",		/* h */	
	"..",		/* i */	
	".---",		/* j */	
	"-.-",		/* k */	
	".-..",		/* l */	
	"--",		/* m */	
	"-.",		/* n */	
	"---",		/* o */	
	".--.",		/* p */	
	"--.-",		/* q */	
	".-.",		/* r */	
	"...",		/* s */	
	"-",		/* t */	
	"..-",		/* u */	
	"...-",		/* v */	
	".--",		/* w */	
	"-..-",		/* x */	
	"-.--",		/* y */	
	"--.."		/* z */	
	};// Nothing below in the ASCII table in Morse Code

int sizeof_morsecode() { return sizeof(morse_code)/sizeof(char *);};

#define HASHSIZE 256

// NOTE: should typedef before declaring the struct. 
// see https://stackoverflow.com/questions/18689345/assignment-from-incompatible-pointer-type-structs-linked-list
typedef struct _hash_item hash_item;
struct _hash_item {
	char * morse;
	char c;
	hash_item * nxt;
};


static hash_item hmorse[HASHSIZE];

static int hash_func(char * str) {
	long val =0;
	for (int i=0; i< strlen(str); i++) {
		val += (long)pow(137, strlen(str) - (i + 1)*str[i]);
		val %= HASHSIZE;
	}

	return (int)val;
}

static void hash_insert(char * k, char v) {
	hash_item *tp;
	for (tp=&hmorse[hash_func(k)]; tp->nxt; tp=tp->nxt) {
		if (strlen(tp->morse)==strlen(k) &&!strncmp(tp->morse, k, strlen(tp->morse))) {
			pr_dbg("conflict tab entry: k1: %s, v1: %c, \
					k2: %s, v2: %c\n", tp->morse, tp->c, k, v);
			return;
		}
	}
	tp->morse = k;
	tp->c = v;
	tp->nxt = calloc(1, sizeof(hash_item));
}

void hmorse_init() {
	char *ts;
	pr_dbg("tab size: %d\n", sizeof_morsecode());
	for (uint8_t i=0; i< sizeof_morsecode(); i++) {
		ts = morse_code[i];
		if(ts[0] == '\0')
			continue;
		pr_dbg("morse hash tab:%s\n", ts);
		hash_insert(ts, i);
	}
}

void hmorse_relase() {
	hash_item *tp, *tn=NULL;
	for (int i=0;i<HASHSIZE; i++) {
		tp=hmorse[i].nxt;
		if (!tp)
			continue;
		for (;tp->nxt; tp=tp->nxt) {
			tn = tp->nxt;
			free(tp);
			tp = tn;
		}
		if (tp)
			free(tp);
		if (tn)
			free(tn);
	}
}

char morse2char(char *s) {
	hash_item *tp;

	for (tp=&hmorse[hash_func(s)]; tp->nxt; tp=tp->nxt) {
		if (!strncmp(tp->morse, s, strlen(tp->morse)))
			return tp->c;
	}

	pr_err("unknown morse code pattern:%s\n", s?s:"");
	//TODO find a way to not produce multiple space
	return ' ';
}

void morse_decode(struct start_options options) {
// split by word space
    char *s=options.message, *sec;
    hmorse_init();
    sec = strtok(s, " ");

    while (sec) {
        pr_dbg("sec: %s\n", sec);
        printf("%c", morse2char(sec));
        sec = strtok(NULL, " ");
    }
}
