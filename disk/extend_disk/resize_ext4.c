#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <endian.h> 

#include "resize2fs_bin.h" // raw resize2fs bytes

int write_temp_binary(const char *path, const unsigned char *data, size_t len) {
    int fd = open(path, O_CREAT | O_WRONLY, 0755);
    if (fd < 0) return -1;
    write(fd, data, len);
    close(fd);
    return 0;
}

#pragma pack(push, 1)
typedef struct {
    uint8_t  boot_flag;       // 0x80 = bootable, 0x00 = non-bootable
    uint8_t  start_chs[3];    // CHS start (legacy)
    uint8_t  partition_type;  // 0x83 = Linux
    uint8_t  end_chs[3];      // CHS end (legacy)
    uint32_t start_lba;       // LBA start (little-endian)
    uint32_t size_sectors;    // Partition size in sectors (little-endian)
} MbrPartitionEntry;

#pragma pack(pop)


// modify MBR header , extend the second partition to max
// TODO:support multiple partition extend include the first one
void extend_second_partition(const char *device) {
    int fd = open(device, O_RDWR);
    if (fd == -1) {
        perror("Failed to open device");
        exit(1);
    }

    uint8_t mbr[512];
    if (read(fd, mbr, sizeof(mbr)) != sizeof(mbr)) {
        perror("Failed to read MBR");
        close(fd);
        exit(1);
    }

    if (mbr[510] != 0x55 || mbr[511] != 0xAA) {
        fprintf(stderr, "Invalid MBR signature\n");
        close(fd);
        exit(1);
    }

    MbrPartitionEntry *part2 = (MbrPartitionEntry *)&mbr[0x1CE];
    if (part2->partition_type != 0x83) {
        fprintf(stderr, "Second partition is not Linux (ext4)\n");
        close(fd);
        exit(1);
    }

    uint64_t disk_sectors;
    if (ioctl(fd, BLKGETSIZE64, &disk_sectors) == -1) {
        perror("Failed to get disk size");
        close(fd);
        exit(1);
    }
    disk_sectors /= 512;

    printf("disk sector number:%ld\n", disk_sectors);

    uint32_t new_size = disk_sectors - part2->start_lba;
    part2->size_sectors = new_size;
    printf("new  sector number:%u\n", new_size);

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("Failed to seek to MBR");
        close(fd);
        exit(1);
    }

    if (write(fd, mbr, sizeof(mbr)) != sizeof(mbr)) {
        perror("Failed to write MBR");
        close(fd);
        exit(1);
    }

    close(fd);

    printf("MBR updated: Partition 2 extended to %u sectors (%u MiB)\n",
           new_size, (new_size * 512) / (1024 * 1024));
}


int main(int argc, char *argv[]) {
    const char *tmp_bin = "/tmp/resize2fs";

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <mmc_device> <ext4_partition>\n", argv[0]);
        fprintf(stderr, "Example: %s /dev/mmcblk0 /dev/mmcblk0p2\n", argv[0]);
        return 1;
    }

    const char *device = argv[1];       // e.g., /dev/mmcblk0
    const char *partition = argv[2];    // e.g., /dev/mmcblk0p2

    extend_second_partition(device);

    if (write_temp_binary(tmp_bin, resize2fs, resize2fs_len) != 0) {
        perror("write_temp_binary");
        return 1;
    }

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s %s", tmp_bin, partition);
    if(system(cmd))
        printf("resize2fs %s fail\n", partition);
    else
        printf("resize2fs %s done\n", partition);
    unlink(tmp_bin);

    return 0;
}
