ifndef _ARCH
_ARCH := $(shell arch)
export _ARCH
# set to x86_64 on PC
# set to armv7l on Raspberry Pi 3B
endif

CC=/usr/bin/gcc
# Always be pedantic on errors
CFLAGS=-I. -Wall -pthread -pg
LDFLAGS=-L/usr/local/lib

# LIBS=-lm -lconfuse -lpigpiod_if2 -lrt
LIBS=-lm -lconfuse -lrt

#endif

DEPS = morse.h

#^TODO makefile build into standalone path
# see https://codereview.stackexchange.com/questions/74136/makefile-that-places-object-files-into-an-alternate-directory-bin for a good reference
.PHONY: clean morse install all
SRC= morse.c decode.c encode.c process_command_line.c process_file.c
BUILDDIR=build

OBJ = $(SRC:%.c=$(BUILDDIR)/%.o)
TARGET_NAME = morse
TARGET=$(TARGET_NAME:%=$(BUILDDIR)/%)

# by default makefile will build the first target
morse:$(TARGET)

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(BUILDDIR)/%.o: %.c $(DEPS) | $(BUILDDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)


clean:
	rm -rf $(BUILDDIR)

install:
	/bin/cp $(TARGET) /usr/local/bin/morse

all: clean morse
