CROSS_COMPILE ?= aarch64-linux-gnu
CC := $(CROSS_COMPILE)-gcc
AR := $(CROSS_COMPILE)-ar
STRIP := $(CROSS_COMPILE)-strip

TARGET = resize_ext4

SRC = resize_ext4.c

CFLAGS = -Wall -Wextra -static

all: $(TARGET)

clone_e2fsprogs:
	git clone --depth=1 https://github.com/tytso/e2fsprogs || true

build_e2fsprogs: clone_e2fsprogs
	cd e2fsprogs && \
	CFLAGS="-static" LDFLAGS="-static" \
		   ./configure --host=$(CROSS_COMPILE) && \
	make libs && make -C resize -j$(nproc)

resize2fs_bin.h: build_e2fsprogs
	cp e2fsprogs/resize/resize2fs resize2fs
	xxd -i resize2fs > $@

$(TARGET): $(SRC) resize2fs_bin.h
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<
		
clean: 
	rm -f $(TARGET)

purge: clean
	rm -rf e2fsprogs resize2fs_bin.h resize2fs

.PHONY: all clean purge
