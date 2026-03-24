"""
Build a PS-X EXE from a raw binary by prepending the original 0x800-byte header.

Usage: python3 make_psexe.py <original_exe> <raw_binary> <output_exe>
"""

import sys
import os

def main():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <original_exe> <raw_binary> <output_exe>")
        sys.exit(1)

    original_path = sys.argv[1]
    binary_path = sys.argv[2]
    output_path = sys.argv[3]

    # Read the original 0x800-byte PS-X EXE header
    with open(original_path, 'rb') as f:
        header = f.read(0x800)

    assert header[:8] == b'PS-X EXE', "Original file is not a PS-X EXE"

    # Read the compiled binary
    with open(binary_path, 'rb') as f:
        binary = f.read()

    # Write output: header + binary
    os.makedirs(os.path.dirname(output_path) or '.', exist_ok=True)
    with open(output_path, 'wb') as f:
        f.write(header)
        f.write(binary)

    original_size = os.path.getsize(original_path)
    output_size = os.path.getsize(output_path)

    if output_size == original_size:
        print(f"OK: {output_path} ({output_size} bytes, matches original size)")
    else:
        print(f"WARNING: {output_path} ({output_size} bytes) != original ({original_size} bytes)")
        print(f"  Difference: {output_size - original_size:+d} bytes")

if __name__ == '__main__':
    main()
