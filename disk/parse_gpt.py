import struct
import os

# the gpt head size = 512 (LBA0 for adapt to MBR) + 128x128 = 16K
PARTITION_ENTRY_SIZE = 128 # each partition entry present one partition, size 128 byte
NUM_ENTRIES = 128  # Standard GPT has up to 128 entries

UTF16_NAME_OFFSET = 56
UTF16_NAME_LENGTH = 72
GPT_HEADER_SIGNATURE = b'EFI PART'


def format_guid(data):
    """Convert 16-byte GUID to string format."""
    d1, d2, d3 = struct.unpack("<IHH", data[:8])
    d4 = data[8:10].hex()
    d5 = data[10:].hex()
    return f"{d1:08x}-{d2:04x}-{d3:04x}-{d4}-{d5}"


def detect_sector_size(file_path):
    with open(file_path, "rb") as f:
        f.seek(512)  # LBA 1 if 512-byte sectors
        if f.read(8) == GPT_HEADER_SIGNATURE:
            return 512

        f.seek(4096)  # LBA 1 if 4096-byte sectors
        if f.read(8) == GPT_HEADER_SIGNATURE:
            return 4096

    raise ValueError("Unable to detect sector size: GPT header signature not found at expected locations.")


def parse_gpt_entries(file_path):
    sector_size = detect_sector_size(file_path)
    print(f"Detected sector size: {sector_size} bytes\n")

    with open(file_path, "rb") as f:
        f.seek(2 * sector_size)  # Skip Protective MBR (LBA0) and GPT Header (LBA1)

        for i in range(NUM_ENTRIES):
            entry = f.read(PARTITION_ENTRY_SIZE)
            if entry == b"\x00" * PARTITION_ENTRY_SIZE:
                continue  # skip empty slot

            type_guid = entry[0:16]
            unique_guid = entry[16:32]
            first_lba = struct.unpack("<Q", entry[32:40])[0]
            last_lba = struct.unpack("<Q", entry[40:48])[0]
            name_raw = entry[UTF16_NAME_OFFSET:UTF16_NAME_OFFSET + UTF16_NAME_LENGTH]
            name = name_raw.decode("utf-16le").rstrip("\x00")

            print(f"Partition {i}")
            print(f"  Name        : {name}")
            print(f"  Type GUID   : {format_guid(type_guid)}")
            print(f"  Unique GUID : {format_guid(unique_guid)}")
            print(f"  First LBA   : {first_lba}")
            print(f"  Last LBA    : {last_lba}")
            print("")


if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("Usage: python3 parse_gpt.py gpt_sectors_dump.bin")
        sys.exit(1)

    parse_gpt_entries(sys.argv[1])

