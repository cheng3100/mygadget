import struct

def parse_mbr(filename):
    with open(filename, 'rb') as f:
        mbr = f.read(512)

    if len(mbr) != 512:
        print("Invalid MBR size. Expected 512 bytes.")
        print("无效的MBR大小，应为512字节。")
        return

    # MBR signature is located at offset 510-511 and must be 0x55AA
    # MBR签名位于偏移量510-511，必须为0x55AA
    signature = mbr[510:512]
    if signature != b'\x55\xAA':
        print("Invalid MBR signature.")
        print("无效的MBR签名。")
        return

    print("Valid MBR detected.")
    print("检测到有效的MBR。")
    print("Partition Table Entries:")
    print("分区表条目：")

    for i in range(4):
        offset = 446 + i * 16  # Each partition entry is 16 bytes, starting at byte 446
        # 每个分区条目占16字节，从第446字节开始
        entry = mbr[offset:offset + 16]

        # Byte 0: Boot flag (0x80 = bootable, 0x00 = non-bootable)
        # 字节0：启动标志（0x80 = 可启动，0x00 = 不可启动）
        boot_flag = entry[0]

        # Bytes 1-3: Starting CHS address (deprecated)
        # 字节1-3：起始CHS地址（已废弃）
        start_chs = entry[1:4]

        # Byte 4: Partition type (e.g., 0x83 = Linux, 0x07 = NTFS)
        # 字节4：分区类型（例如，0x83 = Linux，0x07 = NTFS）
        partition_type = entry[4]

        # Bytes 5-7: Ending CHS address (deprecated)
        # 字节5-7：结束CHS地址（已废弃）
        end_chs = entry[5:8]

        # Bytes 8-11: Starting LBA (little-endian)
        # 字节8-11：起始LBA地址（小端格式）
        start_lba = struct.unpack('<I', entry[8:12])[0]

        # Bytes 12-15: Size in sectors (little-endian)
        # 字节12-15：分区大小（以扇区为单位，小端格式）
        size_in_sectors = struct.unpack('<I', entry[12:16])[0]

        print(f"\nPartition {i + 1}:")
        print(f"分区 {i + 1}：")
        print(f"  Boot Flag: {'Yes' if boot_flag == 0x80 else 'No'} (0x{boot_flag:02X})")
        print("  启动标志：{} (0x{:02X})".format('是' if boot_flag == 0x80 else '否', boot_flag))
        print("    - 0x80: Bootable / 可启动")
        print("    - 0x00: Non-bootable / 不可启动")

        print(f"  Partition Type: 0x{partition_type:02X}")
        print("  分区类型：0x{:02X}".format(partition_type))
        print("    - 常见类型 / Common types:")
        print("      * 0x07: NTFS/exFAT")
        print("      * 0x0B: FAT32 (CHS)")
        print("      * 0x0C: FAT32 (LBA)")
        print("      * 0x83: Linux native / Linux原生")
        print("      * 0x82: Linux swap / Linux交换分区")
        print("      * 0xA5: FreeBSD")

        print(f"  Starting LBA: {start_lba}")
        print(f"  起始LBA地址：{start_lba}")

        print(f"  Size (in sectors): {size_in_sectors}")
        print(f"  分区大小（以扇区为单位）：{size_in_sectors}")

        print(f"  Approx. size (in bytes): {size_in_sectors * 512}")
        print(f"  近似大小（以字节为单位）：{size_in_sectors * 512}")

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("Usage: python3 parse_mbr.py mbr_sectors_dump.bin")
        sys.exit(1)
    parse_mbr(sys.argv[1])

