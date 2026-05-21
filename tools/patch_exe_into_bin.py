#!/usr/bin/env python3
"""
Patch a freshly-compiled PS-EXE into a copy of the original PS1 disc image,
producing a bootable .bin/.cue that runs YOUR build instead of the retail
executable.

Bushido Blade 2 streams all of its assets (NDATA, MOTION, stage geometry,
XA audio) from cdrom:, so a bare PS-EXE cannot run standalone -- the
executable has to live on a disc image alongside the rest of the filesystem.
This tool does an in-place sector patch:

  1. Locate SLUS_006.63 inside the MODE2/2352 image (ISO 9660 directory walk).
  2. Overwrite just that file's 2048-byte user-data regions with the new EXE.
  3. Recompute each touched sector's EDC + ECC (Reed-Solomon P/Q parity) so
     the disc is valid for strict emulators and real hardware.

Because step 3 regenerates the exact bytes the pressing plant produced,
patching a *byte-identical* EXE back in reproduces the original image
bit-for-bit -- which is the built-in correctness self-test (--verify).

The EDC/ECC implementation follows Neill Corlett's ECM reference (public
domain), the same algorithm used by cdrdao / mkpsxiso / ecm-tools.

Usage:
  python3 tools/patch_exe_into_bin.py                      # all defaults
  python3 tools/patch_exe_into_bin.py \\
      --image "Bushido Blade 2 (USA).bin" \\
      --exe   build/bb2.exe \\
      --out   tmp/bb2_patched.bin
  python3 tools/patch_exe_into_bin.py --exe disc/SLUS_006.63 --verify

Writes <out> plus a sibling <out>.cue. Exit status is non-zero on any
failure (file not found, size mismatch, or failed --verify).
"""

import argparse
import hashlib
import os
import shutil
import struct
import sys

SECTOR_SIZE = 2352
DATA_OFFSET = 24          # 12 sync + 3 addr + 1 mode + 8 subheader
DATA_SIZE = 2048          # MODE2 Form 1 user-data bytes
EDC_OFFSET = 0x818        # 4-byte EDC for Form 1
ECC_P_OFFSET = 0x81C      # 172-byte P parity
ECC_Q_OFFSET = 0x8C8      # 104-byte Q parity

# --------------------------------------------------------------------------
# EDC / ECC (after Neill Corlett's ECM, public domain)
# --------------------------------------------------------------------------
_EDC_TABLE = [0] * 256
_ECC_F_TABLE = bytearray(256)
_ECC_B_TABLE = bytearray(256)


def _init_eccedc_tables():
    for i in range(256):
        edc = i
        for _ in range(8):
            edc = (edc >> 1) ^ (0xD8018001 if (edc & 1) else 0)
        _EDC_TABLE[i] = edc & 0xFFFFFFFF
        j = ((i << 1) ^ (0x11D if (i & 0x80) else 0)) & 0xFF
        _ECC_F_TABLE[i] = j
        _ECC_B_TABLE[i ^ j] = i


def _edc_compute(data):
    edc = 0
    table = _EDC_TABLE
    for b in data:
        edc = (edc >> 8) ^ table[(edc ^ b) & 0xFF]
    return edc & 0xFFFFFFFF


def _ecc_compute_block(src, major_count, minor_count, major_mult, minor_inc,
                       sector, dest_off):
    """Compute one ECC parity block (P or Q) into sector[dest_off:].

    `src` is a memoryview of the sector starting at offset 0x0C; writing
    parity back into `sector` is visible through `src` (Q parity is computed
    over data that already includes the freshly written P parity)."""
    size = major_count * minor_count
    f = _ECC_F_TABLE
    b = _ECC_B_TABLE
    for major in range(major_count):
        index = (major >> 1) * major_mult + (major & 1)
        ecc_a = 0
        ecc_b = 0
        for _ in range(minor_count):
            temp = src[index]
            index += minor_inc
            if index >= size:
                index -= size
            ecc_a ^= temp
            ecc_b ^= temp
            ecc_a = f[ecc_a]
        ecc_a = b[f[ecc_a] ^ ecc_b]
        sector[dest_off + major] = ecc_a
        sector[dest_off + major + major_count] = (ecc_a ^ ecc_b) & 0xFF


def reconstruct_form1(sector):
    """Recompute EDC + ECC for a MODE2 Form 1 sector (bytearray, len 2352).

    Assumes sync (0x00..0x0B), address+mode (0x0C..0x0F), subheader
    (0x10..0x17) and user data (0x18..0x817) are already in place."""
    # EDC over subheader + user data (0x10 .. 0x817), stored little-endian.
    edc = _edc_compute(bytes(sector[0x10:0x10 + 0x808]))
    struct.pack_into("<I", sector, EDC_OFFSET, edc)
    # ECC: header address is treated as zero for Form 1, then restored.
    saved = bytes(sector[0x0C:0x10])
    sector[0x0C:0x10] = b"\x00\x00\x00\x00"
    view = memoryview(sector)[0x0C:]
    _ecc_compute_block(view, 86, 24, 2, 86, sector, ECC_P_OFFSET)
    _ecc_compute_block(view, 52, 43, 86, 88, sector, ECC_Q_OFFSET)
    sector[0x0C:0x10] = saved


# --------------------------------------------------------------------------
# ISO 9660 directory walk (MODE2/2352, Form 1 user data)
# --------------------------------------------------------------------------
def _read_sector_data(f, sector_num):
    f.seek(sector_num * SECTOR_SIZE + DATA_OFFSET)
    return f.read(DATA_SIZE)


def _both_endian_32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def _iter_dir_records(data, size):
    off = 0
    while off < size:
        length = data[off]
        if length == 0:
            nxt = ((off // DATA_SIZE) + 1) * DATA_SIZE
            if nxt >= size:
                break
            off = nxt
            continue
        extent = _both_endian_32(data, off + 2)
        data_len = _both_endian_32(data, off + 10)
        name_len = data[off + 32]
        name = data[off + 33:off + 33 + name_len]
        if name not in (b"\x00", b"\x01"):
            name_str = name.decode("ascii", "replace").split(";")[0]
            yield name_str, extent, data_len
        off += length


def _read_directory(f, extent, size):
    n = (size + DATA_SIZE - 1) // DATA_SIZE
    data = b"".join(_read_sector_data(f, extent + i) for i in range(n))
    return list(_iter_dir_records(data, size))


def find_file(image_path, target_name):
    """Return (extent_lba, size_bytes) for target_name in the root directory."""
    target = target_name.upper().split(";")[0]
    with open(image_path, "rb") as f:
        pvd = _read_sector_data(f, 16)
        if pvd[0] != 1 or pvd[1:6] != b"CD001":
            raise ValueError(f"{image_path}: not a valid ISO 9660 PVD at sector 16")
        root_extent = _both_endian_32(pvd, 156 + 2)
        root_size = _both_endian_32(pvd, 156 + 10)
        for name, extent, size in _read_directory(f, root_extent, root_size):
            if name.upper() == target:
                return extent, size
    raise FileNotFoundError(
        f"{target_name} not found in root directory of {image_path}")


# --------------------------------------------------------------------------
# Patch
# --------------------------------------------------------------------------
def patch(image_path, exe_path, out_path, target_name):
    extent, on_disc_size = find_file(image_path, target_name)
    exe = open(exe_path, "rb").read()
    n_sectors = (on_disc_size + DATA_SIZE - 1) // DATA_SIZE

    print(f"  image : {image_path}")
    print(f"  file  : {target_name}  @ LBA {extent}  "
          f"({on_disc_size} bytes, {n_sectors} sectors)")
    print(f"  new   : {exe_path}  ({len(exe)} bytes)")

    if len(exe) != on_disc_size:
        raise SystemExit(
            f"\nERROR: new EXE is {len(exe)} bytes but {target_name} occupies "
            f"{on_disc_size} bytes on disc.\n"
            "In-place patching only handles same-size replacement (which is the "
            "normal case -- a matching build, or a behavioral mod that doesn't "
            "change the PS-EXE layout). A size change requires a full disc "
            "relayout (dumpsxiso/mkpsxiso), not this tool.")

    if exe[:8] != b"PS-X EXE":
        print("  WARNING: new EXE does not start with 'PS-X EXE' magic "
              "(continuing anyway)")

    print(f"  copy  : -> {out_path} ...", flush=True)
    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    shutil.copyfile(image_path, out_path)

    print(f"  patch : {n_sectors} sectors (recomputing EDC + ECC) ...", flush=True)
    with open(out_path, "r+b") as f:
        for i in range(n_sectors):
            chunk = exe[i * DATA_SIZE:(i + 1) * DATA_SIZE]
            if len(chunk) < DATA_SIZE:
                chunk = chunk + b"\x00" * (DATA_SIZE - len(chunk))
            base = (extent + i) * SECTOR_SIZE
            f.seek(base)
            sector = bytearray(f.read(SECTOR_SIZE))
            mode = sector[0x0F]
            if mode != 0x02:
                raise SystemExit(
                    f"ERROR: sector {extent + i} is mode {mode}, expected "
                    "MODE2 (0x02). Unexpected image layout; aborting.")
            sector[DATA_OFFSET:DATA_OFFSET + DATA_SIZE] = chunk
            reconstruct_form1(sector)
            f.seek(base)
            f.write(sector)

    cue_path = os.path.splitext(out_path)[0] + ".cue"
    with open(cue_path, "w", newline="\r\n") as c:
        c.write(f'FILE "{os.path.basename(out_path)}" BINARY\n')
        c.write("  TRACK 01 MODE2/2352\n")
        c.write("    INDEX 01 00:00:00\n")
    print(f"  cue   : -> {cue_path}")
    return out_path, cue_path


def sha1_of(path):
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def _default_image():
    for name in sorted(os.listdir(".")):
        if name.lower().endswith(".bin"):
            return name
    return None


def main():
    ap = argparse.ArgumentParser(
        description="Patch a compiled PS-EXE into a copy of the BB2 disc image.")
    ap.add_argument("--image", default=None,
                    help="original .bin (default: first *.bin in cwd)")
    ap.add_argument("--exe", default="build/bb2.exe",
                    help="new executable to inject (default: build/bb2.exe)")
    ap.add_argument("--out", default="tmp/bb2_patched.bin",
                    help="output .bin path (default: tmp/bb2_patched.bin)")
    ap.add_argument("--file", default="SLUS_006.63",
                    help="on-disc filename to replace (default: SLUS_006.63)")
    ap.add_argument("--verify", action="store_true",
                    help="SHA1-compare output against the input image "
                         "(self-test; only matches when --exe is byte-identical "
                         "to the on-disc executable)")
    args = ap.parse_args()

    image = args.image or _default_image()
    if not image:
        ap.error("no --image given and no *.bin found in cwd")
    for p, what in ((image, "image"), (args.exe, "exe")):
        if not os.path.isfile(p):
            ap.error(f"{what} not found: {p}")

    _init_eccedc_tables()
    print("Patching EXE into disc image:")
    out_path, cue_path = patch(image, args.exe, args.out, args.file)

    if args.verify:
        print("  verify: hashing input + output ...", flush=True)
        src_sha = sha1_of(image)
        out_sha = sha1_of(out_path)
        print(f"          input  {src_sha}")
        print(f"          output {out_sha}")
        if src_sha == out_sha:
            print("  verify: OK -- byte-identical reproduction "
                  "(EDC/ECC pipeline confirmed correct)")
        else:
            print("  verify: DIFFER -- expected when --exe diverges from the "
                  "on-disc executable. If you passed a byte-identical EXE, "
                  "this indicates an EDC/ECC bug.")
            if args.exe in ("disc/" + args.file, args.file) or \
               os.path.basename(args.exe) == args.file:
                raise SystemExit("FAIL: byte-identical input did not reproduce "
                                 "the image -- patch pipeline is broken.")

    print(f"\nDone. Boot {cue_path} in a PS1 emulator (e.g. DuckStation).")


if __name__ == "__main__":
    main()
