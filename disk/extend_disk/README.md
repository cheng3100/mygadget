resize_ext4 usage
=================

This is simple tool to extend the ext4 partition to max disk size on board.
Work both for EMMC and SD card

# compile
1. install aarch64-linux-gnu toolchain
2.  run `make`

The result is the resize_ext4 binary
Check with `file resize_ext4` to see whether it is crosscompiled.

# usage
copy the resize_ext4 to target board.
run `./resize_ext4 /dev/mmcblk0 /dev/mmcblk0p2`  it will extend the second ext4 partition to max

# Note
1. It only support MBR partition for now. GPT partition may add support later.
2. It only support extend the second ext4 to the end of disk! Means it is for MBR disk with only 2 partition. It can be extend to support more disk layout later.

# Dev
It mainly do 2 things:
1. modify MBR header to extend the partition table
2. extend the ext4 fs to max.



