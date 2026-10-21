"""
Build a PS-X EXE from the linked ELF and its raw .main binary.

Usage: python3 make_psexe.py <elf> <raw_binary> <output_exe> [--entry SYMBOL]

The 0x800-byte header template is the ELF's .header section (asm/header.s:
magic, stack, region string). The fields that depend on the link are computed:
  0x10 pc0     address of the entry symbol (default __SN_ENTRY_POINT)
  0x18 t_addr  load address of .main
  0x1C t_size  raw binary size rounded up to 0x800 (the binary is zero-padded
               to that size; the loader reads whole sectors)
For the matching link these equal the retail header, so the EXE is unchanged;
a build that moves code or data gets a header that describes it.
"""

import argparse
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from elf32 import Elf  # noqa: E402

HEADER_SIZE = 0x800
SECTOR = 0x800


def main():
    ap = argparse.ArgumentParser(description="Build a PS-X EXE from the linked ELF.")
    ap.add_argument("elf")
    ap.add_argument("binary")
    ap.add_argument("output")
    ap.add_argument("--entry", default="__SN_ENTRY_POINT")
    args = ap.parse_args()

    elf = Elf(args.elf)
    header_sec = elf.section(".header")
    main_sec = elf.section(".main")
    if header_sec is None or main_sec is None:
        sys.exit("make_psexe: %s has no .header or .main section" % args.elf)
    header = bytearray(elf.sec_bytes(header_sec))
    if len(header) != HEADER_SIZE or header[:8] != b"PS-X EXE":
        sys.exit("make_psexe: .header is not a 0x800-byte PS-X EXE header")
    entry = elf.symbol(args.entry)
    if entry is None:
        sys.exit("make_psexe: entry symbol %s not in %s" % (args.entry, args.elf))

    with open(args.binary, "rb") as f:
        binary = f.read()
    size = (len(binary) + SECTOR - 1) // SECTOR * SECTOR
    binary += b"\0" * (size - len(binary))

    struct.pack_into("<I", header, 0x10, entry)
    struct.pack_into("<I", header, 0x18, main_sec["addr"])
    struct.pack_into("<I", header, 0x1C, size)

    os.makedirs(os.path.dirname(args.output) or ".", exist_ok=True)
    with open(args.output, "wb") as f:
        f.write(header)
        f.write(binary)
    print("OK: %s (%d bytes; pc0 0x%08X, t_addr 0x%08X, t_size 0x%X)"
          % (args.output, HEADER_SIZE + size, entry, main_sec["addr"], size))


if __name__ == "__main__":
    main()
