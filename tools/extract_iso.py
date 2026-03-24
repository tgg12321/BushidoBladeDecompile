"""
Extract files from a PS1 MODE2/2352 BIN disc image.

PS1 discs use ISO 9660 filesystem with 2352-byte raw sectors.
Each sector: 12 sync + 4 header + 8 subheader + 2048 data + 280 ECC/EDC
"""

import os
import sys
import struct
from pathlib import Path

SECTOR_SIZE = 2352
DATA_OFFSET = 24  # 12 sync + 4 header + 8 subheader
DATA_SIZE = 2048
SYSTEM_AREA_SECTORS = 16  # First 16 sectors are system area

def read_sector(f, sector_num):
    """Read the 2048-byte data portion of a raw 2352-byte sector."""
    f.seek(sector_num * SECTOR_SIZE + DATA_OFFSET)
    return f.read(DATA_SIZE)

def decode_both_endian_16(data, offset):
    """ISO 9660 stores values in both little-endian and big-endian."""
    return struct.unpack_from('<H', data, offset)[0]

def decode_both_endian_32(data, offset):
    """ISO 9660 stores values in both little-endian and big-endian."""
    return struct.unpack_from('<I', data, offset)[0]

def parse_directory_record(data, offset):
    """Parse a single ISO 9660 directory record."""
    length = data[offset]
    if length == 0:
        return None

    ext_attr_length = data[offset + 1]
    extent_loc = decode_both_endian_32(data, offset + 2)
    data_length = decode_both_endian_32(data, offset + 10)
    flags = data[offset + 25]
    name_length = data[offset + 32]
    name = data[offset + 33:offset + 33 + name_length]

    is_directory = bool(flags & 0x02)

    # Decode name
    if name == b'\x00':
        name_str = '.'
    elif name == b'\x01':
        name_str = '..'
    else:
        name_str = name.decode('ascii', errors='replace')
        # Remove version number (;1)
        if ';' in name_str:
            name_str = name_str.split(';')[0]

    return {
        'length': length,
        'extent': extent_loc,
        'size': data_length,
        'flags': flags,
        'is_directory': is_directory,
        'name': name_str,
    }

def read_directory(f, extent, size):
    """Read all directory records from a directory extent."""
    records = []
    sectors_to_read = (size + DATA_SIZE - 1) // DATA_SIZE
    data = b''
    for i in range(sectors_to_read):
        data += read_sector(f, extent + i)

    offset = 0
    while offset < size:
        record = parse_directory_record(data, offset)
        if record is None:
            # Move to next sector boundary
            next_sector = ((offset // DATA_SIZE) + 1) * DATA_SIZE
            if next_sector >= size:
                break
            offset = next_sector
            continue
        if record['name'] not in ('.', '..'):
            records.append(record)
        offset += record['length']

    return records

def extract_file(f, extent, size, output_path):
    """Extract a file from the disc image."""
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    sectors_to_read = (size + DATA_SIZE - 1) // DATA_SIZE
    bytes_remaining = size

    with open(output_path, 'wb') as out:
        for i in range(sectors_to_read):
            sector_data = read_sector(f, extent + i)
            write_size = min(DATA_SIZE, bytes_remaining)
            out.write(sector_data[:write_size])
            bytes_remaining -= write_size

def extract_directory(f, extent, size, output_dir, depth=0):
    """Recursively extract a directory."""
    records = read_directory(f, extent, size)
    indent = "  " * depth

    for record in records:
        output_path = os.path.join(output_dir, record['name'])
        if record['is_directory']:
            print(f"{indent}[DIR]  {record['name']}/")
            os.makedirs(output_path, exist_ok=True)
            extract_directory(f, record['extent'], record['size'], output_path, depth + 1)
        else:
            size_kb = record['size'] / 1024
            if size_kb >= 1024:
                size_str = f"{size_kb/1024:.1f} MB"
            else:
                size_str = f"{size_kb:.1f} KB"
            print(f"{indent}[FILE] {record['name']} ({size_str})")
            extract_file(f, record['extent'], record['size'], output_path)

def main():
    if len(sys.argv) < 2:
        bin_path = None
        # Look for .bin file in current directory
        for f in os.listdir('.'):
            if f.lower().endswith('.bin'):
                bin_path = f
                break
        if not bin_path:
            print("Usage: python extract_iso.py <image.bin> [output_dir]")
            sys.exit(1)
    else:
        bin_path = sys.argv[1]

    output_dir = sys.argv[2] if len(sys.argv) > 2 else 'disc'

    print(f"Extracting: {bin_path}")
    print(f"Output dir: {output_dir}")
    print()

    with open(bin_path, 'rb') as f:
        # Read Primary Volume Descriptor (sector 16)
        pvd = read_sector(f, 16)

        # Verify PVD
        assert pvd[0] == 1, "Expected Primary Volume Descriptor"
        assert pvd[1:6] == b'CD001', "Not a valid ISO 9660 image"

        volume_id = pvd[40:72].decode('ascii').strip()
        print(f"Volume ID: {volume_id}")

        # Root directory record is at offset 156 in PVD
        root_extent = decode_both_endian_32(pvd, 156 + 2)
        root_size = decode_both_endian_32(pvd, 156 + 10)

        print(f"Root directory: sector {root_extent}, size {root_size}")
        print()
        print("=== Disc Contents ===")

        os.makedirs(output_dir, exist_ok=True)
        extract_directory(f, root_extent, root_size, output_dir)

    print()
    print("Extraction complete!")

if __name__ == '__main__':
    main()
