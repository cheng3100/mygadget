ifndef _ARCH
_ARCH := $(shell arch)
export _ARCH
# set to x86_64 on PC
# set to armv7l on Raspberry Pi 3B
endif

CC=/usr/bin/gcc
# Always be pedantic on errors
CFLAGS=-I. -Wall -pthread
LDFLAGS=-L/usr/local/lib

# LIBS=-lm -lconfuse -lpigpiod_if2 -lrt
LIBS=-lm -lconfuse -lrt

#endif

DEPS = morse.h

#TODO makefile build into standalone path

# OBJ = morse.o display_morse.o process_command_line.o process_config_file.o process_file.o
OBJ = morse.o decode.o encode.o process_command_line.o process_file.o


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

morse: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)

.PHONY: clean

clean:
	/bin/rm -f *.o *~ core morse

.PHONY: install

install:
	/bin/cp morse /usr/local/bin/morse

all: clean morse 
