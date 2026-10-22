#!/usr/bin/env python3
"""
build_disc.py - Build a modded Bushido Blade 2 disc image from the retail one,
replacing files of any size. Notes in docs/formats/README.md ("Disc image
and rebuild").

    build_disc.py --image "Bushido Blade 2 (USA).bin" --out mod.bin \\
        [--exe build/bb2.exe] [--dir MODDIR] [--replace DISC/PATH=FILE ...]
    build_disc.py --image "Bushido Blade 2 (USA).bin" --out copy.bin --verify

The image is one MODE2/2352 data track (ISO 9660 with XA extensions).
`--dir` replaces every disc file whose path (e.g. TIM2D/GAME.BNK) exists
under MODDIR; `--replace` names single files; `--exe` replaces
SLUS_006.63. Only plain Form 1 data files can be replaced (not the XA audio or
STR video streams), and directories stay where they are.

A replacement that still fits the file's sectors is written in place (the
sectors it no longer needs keep their old bytes); a bigger one moves to the
end of the files, before the image's 150 trailing empty sectors. For every
file that moves or changes size the tool rewrites:

* its ISO 9660 directory record (extent and size, both byte orders), and
  the volume space size when the image grows;
* its entry in the EXE's CD file table (`g_cd_file_table`, CdFileEntry
  {CdlLOC, u32 size}): the old entry's 8 bytes are found in the EXE that
  goes on the disc and must occur there exactly once, so a shifted or
  rebuilt EXE works as long as its table still describes the input image.
  A file with no entry in the input image's own EXE (SLUS_006.63,
  SYSTEM.CNF, TIM2D/LAS_DD.TIM on the retail disc) only gets a note.

Every written sector gets its MSF address, subheader (data 0x08, last
sector of a file 0x89) and EDC/ECC (the CD-ROM Mode 2 Form 1 code of
ECMA-130 / CD-ROM XA, re-implemented). With no replacements the output equals the input byte for byte;
`--verify` checks that. Writes OUT plus OUT's .cue.
"""

import argparse
import hashlib
import os
import shutil
import struct
import sys
from pathlib import Path

SECTOR = 2352
DATA = 2048
SUB_DATA = bytes([0, 0, 0x08, 0])
SUB_LAST = bytes([0, 0, 0x89, 0])
SYNC = b"\x00" + b"\xff" * 10 + b"\x00"
INF_MAX = 2 * DATA  # ndata_StartReadInf reads NDATA.INF as 2 sectors

# ---------------------------------------------------------------------------
# EDC / ECC (ECMA-130 / CD-ROM XA Mode 2 Form 1)
# ---------------------------------------------------------------------------
EDC_TABLE = [0] * 256
ECC_F = bytearray(256)
ECC_B = bytearray(256)
for _i in range(256):
    _e = _i
    for _ in range(8):
        _e = (_e >> 1) ^ (0xD8018001 if _e & 1 else 0)
    EDC_TABLE[_i] = _e
    _j = ((_i << 1) ^ (0x11D if _i & 0x80 else 0)) & 0xFF
    ECC_F[_i] = _j
    ECC_B[_i ^ _j] = _i


def edc(data):
    e = 0
    for b in data:
        e = (e >> 8) ^ EDC_TABLE[(e ^ b) & 0xFF]
    return e


def ecc_block(src, major_count, minor_count, major_mult, minor_inc, out):
    size = major_count * minor_count
    parity = bytearray(2 * major_count)
    for major in range(major_count):
        index = (major >> 1) * major_mult + (major & 1)
        a = b = 0
        for _ in range(minor_count):
            t = src[index]
            index += minor_inc
            if index >= size:
                index -= size
            a ^= t
            b ^= t
            a = ECC_F[a]
        a = ECC_B[ECC_F[a] ^ b]
        parity[major] = a
        parity[major + major_count] = a ^ b
    out += parity


def form1_sector(lba, sub, payload):
    """A complete MODE2 Form 1 sector at lba holding payload (<= 2048 B)."""
    s = bytearray(SYNC + msf(lba + 150) + b"\x02" + sub + sub)
    s += payload.ljust(DATA, b"\0")
    s += struct.pack("<I", edc(s[0x10:0x818]))
    # ECC is computed with the 4 header bytes taken as zero.
    src = bytearray(4) + s[0x10:]
    ecc_block(src, 86, 24, 2, 86, src)  # P parity, appended
    ecc_block(src, 52, 43, 86, 88, src)  # Q parity over data + P
    s += src[0x81C - 0x0C:]
    assert len(s) == SECTOR
    return bytes(s)


def bcd(n):
    return (n // 10) << 4 | n % 10


def msf(lba150):
    m, rest = divmod(lba150, 75 * 60)
    s, f = divmod(rest, 75)
    return bytes([bcd(m), bcd(s), bcd(f)])


def cdloc(lba):
    return msf(lba + 150) + b"\0"  # CdlLOC {minute, second, sector, track}


# ---------------------------------------------------------------------------
# ISO 9660
# ---------------------------------------------------------------------------
class Image:
    def __init__(self, path):
        self.f = open(path, "rb")
        self.f.seek(0, 2)
        size = self.f.tell()
        if size % SECTOR:
            raise ValueError(f"{path}: size {size} is not whole 2352-byte sectors")
        self.nsec = size // SECTOR

    def raw(self, lba):
        self.f.seek(lba * SECTOR)
        return self.f.read(SECTOR)

    def data(self, lba, n=1):
        return b"".join(self.raw(lba + i)[24:24 + DATA] for i in range(n))


def walk(img):
    """Returns (pvd, files): files = {path: dict(lba, size, rec=(sector, off))}."""
    pvd = img.data(16)
    if pvd[:6] != b"\x01CD001":
        raise ValueError("no ISO 9660 primary volume descriptor at sector 16")
    files = {}

    def rec_dir(extent, size, prefix):
        n = (size + DATA - 1) // DATA
        d = img.data(extent, n)
        off = 0
        while off < size:
            ln = d[off]
            if ln == 0:
                off = (off // DATA + 1) * DATA
                continue
            lba, sz = struct.unpack_from("<I", d, off + 2)[0], \
                struct.unpack_from("<I", d, off + 10)[0]
            flags, nl = d[off + 25], d[off + 32]
            name = d[off + 33:off + 33 + nl]
            if name not in (b"\0", b"\1"):
                name = name.decode("ascii").split(";")[0]
                if flags & 2:
                    rec_dir(lba, sz, prefix + name + "/")
                else:
                    files[prefix + name] = {
                        "lba": lba, "size": sz,
                        "rec": (extent + off // DATA, off % DATA)}
            off += ln

    rec_dir(struct.unpack_from("<I", pvd, 158)[0],
            struct.unpack_from("<I", pvd, 166)[0], "")
    return pvd, files


def nsectors(size):
    return max(1, (size + DATA - 1) // DATA)


def both32(v):
    return struct.pack("<I", v) + struct.pack(">I", v)


# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------
def plan(img, files, repl):
    """Returns {path: (new_lba, new_size)} for the replaced files and the
    first free sector after all files."""
    end = max(e["lba"] + nsectors(e["size"]) for e in files.values())
    tail = img.nsec - end
    if any(img.raw(end + i)[0x10:] != bytes(SECTOR - 0x10) for i in range(tail)):
        raise ValueError("the sectors after the last file are not empty")
    layout = {}
    for path in sorted(repl, key=lambda p: files[p]["lba"]):
        e, new = files[path], len(repl[path])
        if img.raw(e["lba"])[0x12] not in (0x08, 0x89):
            raise ValueError(f"{path}: not a plain data file (XA / STR "
                             "stream sectors), not replaceable")
        if nsectors(new) <= nsectors(e["size"]):
            layout[path] = (e["lba"], new)
        else:
            layout[path] = (end, new)
            end += nsectors(new)
    return layout, end, tail


def patch_exe(exe, disc_exe, files, layout):
    """Patch exe's table entries; disc_exe is the input image's own EXE,
    which tells which files have an entry at all."""
    exe = bytearray(exe)
    for path, (lba, size) in layout.items():
        e = files[path]
        if (lba, size) == (e["lba"], e["size"]):
            continue
        old = cdloc(e["lba"]) + struct.pack("<I", e["size"])
        hits = []
        i = exe.find(old)
        while i >= 0:
            hits.append(i)
            i = exe.find(old, i + 1)
        if not hits:
            if old in disc_exe:
                raise ValueError(
                    f"{path}: the EXE has no table entry {old.hex()} for it "
                    "(the input image's EXE does); the EXE's table does not "
                    "describe this image, so the game would read the old "
                    "location")
            print(f"note: {path} has no CD file table entry (the game does "
                  "not read it by table)")
            continue
        if len(hits) > 1:
            raise ValueError(f"{path}: its table entry {old.hex()} occurs "
                             f"{len(hits)} times in the EXE")
        exe[hits[0]:hits[0] + 8] = cdloc(lba) + struct.pack("<I", size)
    return bytes(exe)


def build(image, out, repl):
    img = Image(image)
    pvd, files = walk(img)
    for path in repl:
        if path not in files:
            raise ValueError(f"{path}: not a file on the disc")
    inf = repl.get("NDATA/NDATA.INF")
    if inf is not None and len(inf) > INF_MAX:
        raise ValueError(f"NDATA.INF is {len(inf)} bytes; the game reads only "
                         f"{INF_MAX}")
    exe_path = "SLUS_006.63"
    layout, end, tail = plan(img, files, repl)
    moved = [p for p, v in layout.items()
             if v != (files[p]["lba"], files[p]["size"])]
    if moved:
        disc_exe = img.data(
            files[exe_path]["lba"], nsectors(files[exe_path]["size"])
        )[:files[exe_path]["size"]]
        exe = repl[exe_path] if exe_path in repl else disc_exe
        patched = patch_exe(exe, disc_exe, files, layout)
        if patched != exe:
            repl[exe_path] = patched
            if exe_path not in layout:
                layout[exe_path] = (files[exe_path]["lba"], len(patched))

    Path(out).parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(image, out)
    total = end + tail
    if total + 150 > 99 * 60 * 75:
        raise ValueError(f"image would be {total} sectors, past the 99:59:74 "
                         "that a BCD CdlLOC can address")
    with open(out, "r+b") as o:
        def put(lba, sector):
            o.seek(lba * SECTOR)
            o.write(sector)

        for path, (lba, size) in layout.items():
            body, n = repl[path], nsectors(size)
            for i in range(n):
                put(lba + i, form1_sector(
                    lba + i, SUB_LAST if i == n - 1 else SUB_DATA,
                    body[i * DATA:(i + 1) * DATA]))
        # trailing empty sectors (header + zeros), moved past new files
        for i in range(tail):
            lba = end + i
            put(lba, SYNC + msf(lba + 150) + b"\x02" + bytes(SECTOR - 16))
        o.truncate(total * SECTOR)

        # directory records and the volume size
        dirty = {}

        def sector_data(lba):
            if lba not in dirty:
                dirty[lba] = bytearray(img.data(lba))
            return dirty[lba]

        for path, (lba, size) in layout.items():
            if (lba, size) == (files[path]["lba"], files[path]["size"]):
                continue
            sec, off = files[path]["rec"]
            d = sector_data(sec)
            d[off + 2:off + 10] = both32(lba)
            d[off + 10:off + 18] = both32(size)
        if total != img.nsec:
            d = sector_data(16)
            d[80:88] = both32(total)
        for lba, d in dirty.items():
            sub = img.raw(lba)[0x10:0x14]
            put(lba, form1_sector(lba, sub, bytes(d)))

    cue = Path(out).with_suffix(".cue")
    cue.write_text(f'FILE "{Path(out).name}" BINARY\n  TRACK 01 MODE2/2352\n'
                   "    INDEX 01 00:00:00\n", newline="\r\n")
    for path, (lba, size) in sorted(layout.items(), key=lambda x: x[1]):
        e = files[path]
        how = "in place" if lba == e["lba"] else f"moved {e['lba']} -> {lba}"
        print(f"{path}: {e['size']} -> {size} bytes, {how}")
    return total


def sha1(path):
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def main():
    ap = argparse.ArgumentParser(description="BB2 disc image rebuild")
    ap.add_argument("--image", required=True, help="retail MODE2/2352 .bin")
    ap.add_argument("--out", required=True, help="output .bin (a .cue is "
                    "written next to it)")
    ap.add_argument("--exe", help="EXE to use as SLUS_006.63")
    ap.add_argument("--dir", help="directory mirroring disc paths")
    ap.add_argument("--replace", action="append", default=[],
                    metavar="DISC/PATH=FILE")
    ap.add_argument("--verify", action="store_true",
                    help="no replacements: check OUT is identical to IMAGE")
    args = ap.parse_args()
    if os.path.abspath(args.image) == os.path.abspath(args.out):
        sys.exit("error: --out must differ from --image")
    repl = {}
    try:
        if args.dir:
            root = Path(args.dir)
            for p in sorted(root.rglob("*")):
                if p.is_file():
                    repl[p.relative_to(root).as_posix().upper()] = p.read_bytes()
        for r in args.replace:
            disc, sep, local = r.partition("=")
            if not sep:
                sys.exit(f"error: --replace {r}: expected DISC/PATH=FILE")
            repl[disc.replace("\\", "/").upper()] = Path(local).read_bytes()
        if args.exe:
            repl["SLUS_006.63"] = Path(args.exe).read_bytes()
        if args.verify and repl:
            sys.exit("error: --verify takes no replacements")
        build(args.image, args.out, repl)
    except (OSError, ValueError) as e:
        sys.exit(f"error: {e}")
    if args.verify:
        ok = sha1(args.image) == sha1(args.out)
        print("verify:", "identical" if ok else "DIFFERS")
        sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
