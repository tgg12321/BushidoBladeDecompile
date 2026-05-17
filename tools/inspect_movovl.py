#!/usr/bin/env python3
"""
inspect_movovl.py - Analyse disc/STR/MOVOVL.EXE (FMV/MDEC playback overlay).

The overlay is a standalone PS-X EXE that loads at 0x801D8800 and runs the
opening / title FMVs. It links statically against PsyQ libgpu (sys.c v1.129),
libpress (MDEC decode), and libapi — the same SDK 3.5 used by the main
binary — plus a thin BB2-specific frontend. See docs/formats/MOVOVL.md for
the byte-level header and the overlay's role at runtime.

Usage:
    inspect_movovl.py disc/STR/MOVOVL.EXE                summary report
    inspect_movovl.py disc/STR/MOVOVL.EXE --strings      every >=4-char ASCII string
    inspect_movovl.py disc/STR/MOVOVL.EXE --funcs        every detected function entry
    inspect_movovl.py disc/STR/MOVOVL.EXE --syscalls     BIOS-call trampolines
"""

import argparse
import struct
import sys
from collections import defaultdict


def parse_header(data):
    if data[:8] != b"PS-X EXE":
        raise ValueError("not a PS-X EXE")
    pc, gp, t_addr, t_size, d_addr, d_size, b_addr, b_size, sp_addr, sp_size = (
        struct.unpack_from("<IIIIIIIIII", data, 0x10)
    )
    return {
        "pc": pc, "gp": gp,
        "text_addr": t_addr, "text_size": t_size,
        "data_addr": d_addr, "data_size": d_size,
        "bss_addr": b_addr, "bss_size": b_size,
        "sp_addr": sp_addr, "sp_size": sp_size,
        "signature": data[0x4C:0x100].decode("ascii", "replace").rstrip("\x00"),
    }


def extract_strings(data, base, min_len=4):
    out = []
    i = 0
    while i < len(data):
        if 0x20 <= data[i] < 0x7F:
            j = i
            while j < len(data) and 0x20 <= data[j] < 0x7F:
                j += 1
            if j - i >= min_len:
                out.append((base + i, data[i:j].decode("ascii")))
            i = j
        else:
            i += 1
    return out


def find_func_prologues(data, base):
    """Find probable function entries by looking for the canonical MIPS prologue.

    The two canonical openers are:
      (a) addiu $sp, $sp, -N        immediately followed by sw $ra, off($sp)
      (b) tail-call (just sw $sp adjust + jr) — rare; we only catch (a).

    Plus we add the EXE entry point.
    """
    funcs = set()
    n = len(data) // 4
    for i in range(n - 1):
        w0 = struct.unpack_from("<I", data, i * 4)[0]
        # addiu $sp, $sp, -N  ->  27bd_xxxx where N = -(int16(xxxx))
        if (w0 & 0xFFFF0000) == 0x27BD0000:
            imm = w0 & 0xFFFF
            if imm & 0x8000:  # negative — i.e., stack-allocation
                # confirm: next ~6 instructions contain a sw $ra
                ok = False
                for j in range(1, 12):
                    if (i + j) * 4 + 4 > len(data):
                        break
                    w = struct.unpack_from("<I", data, (i + j) * 4)[0]
                    # sw $ra, off($sp)  ->  AFBF_xxxx
                    if (w & 0xFFFF0000) == 0xAFBF0000:
                        ok = True
                        break
                if ok:
                    funcs.add(base + i * 4)
    return sorted(funcs)


def find_bios_trampolines(data, base):
    """Find PSX BIOS call sites.

    The canonical PSX BIOS call pattern is:

       li   $t2, TBL    ; TBL in {0xa0, 0xb0, 0xc0} = BIOS table base
       jr   $t2         ; jump to table
       li   $t1, FN     ; delay slot: function index within the table

    The BIOS itself (built into the PSX kernel at 0xA0/0xB0/0xC0) dispatches
    on $t1. We detect each (li-t2, jr-t2, li-t1) triple as one BIOS call
    site and decode the (table, function) pair.

    Returns: list of {"addr", "table", "funcnum"}.
    """
    out = []
    n = len(data) // 4
    for i in range(n - 2):
        w0 = struct.unpack_from("<I", data, i * 4)[0]
        # li $t2, 0xa0/0xb0/0xc0
        if w0 not in (0x240A00A0, 0x240A00B0, 0x240A00C0):
            continue
        w1 = struct.unpack_from("<I", data, (i + 1) * 4)[0]
        if w1 != 0x01400008:  # jr $t2
            continue
        w2 = struct.unpack_from("<I", data, (i + 2) * 4)[0]
        # li $t1, FN (delay slot) — addiu $t1, $zero, N
        if (w2 & 0xFFFF0000) != 0x24090000:
            continue
        out.append(
            {
                "addr": base + i * 4,
                "table": w0 & 0xFF,
                "funcnum": w2 & 0xFFFF,
            }
        )
    return out


def find_mdec_refs(data, base):
    """Find code that references MDEC I/O registers.

    MDEC0 (cmd / DMA0)  = 0x1F801820
    MDEC1 (status/DMA1) = 0x1F801824

    Loads typically take the form:
        lui $reg, 0x1f80         3cXX_1f80
        lw / sw $reg2, 0x1820/24($reg)
    """
    out = []
    for i in range(len(data) // 4 - 1):
        w0 = struct.unpack_from("<I", data, i * 4)[0]
        # lui $X, 0x1f80
        if (w0 & 0xFFE0FFFF) == 0x3C001F80:
            w1 = struct.unpack_from("<I", data, (i + 1) * 4)[0]
            opc = (w1 >> 26) & 0x3F
            off = w1 & 0xFFFF
            base_reg = (w1 >> 21) & 0x1F
            lui_dst = (w0 >> 16) & 0x1F
            if opc in (0x23, 0x2B) and base_reg == lui_dst and off in (0x1820, 0x1824):
                kind = "lw" if opc == 0x23 else "sw"
                reg = ["MDEC0", "MDEC1"][off == 0x1824]
                out.append(
                    {
                        "addr": base + i * 4,
                        "op": kind,
                        "register": reg,
                        "offset": off,
                    }
                )
    return out


def summary(path):
    with open(path, "rb") as f:
        data = f.read()
    hdr = parse_header(data)
    text = data[0x800 : 0x800 + hdr["text_size"]]
    print(f"{path}  ({len(data)} bytes)")
    print("Header:")
    print(f"  PS-X signature : {hdr['signature']!r}")
    print(f"  entry point    : 0x{hdr['pc']:08x}")
    print(f"  gp init        : 0x{hdr['gp']:08x} (set by code, header is 0)")
    print(
        f"  text region    : 0x{hdr['text_addr']:08x} .. "
        f"0x{hdr['text_addr'] + hdr['text_size']:08x} ({hdr['text_size']} bytes)"
    )
    print(f"  stack          : 0x{hdr['sp_addr']:08x}")
    funcs = find_func_prologues(text, hdr["text_addr"])
    if hdr["pc"] not in funcs:
        funcs.append(hdr["pc"])
        funcs.sort()
    bios = find_bios_trampolines(text, hdr["text_addr"])
    mdec = find_mdec_refs(text, hdr["text_addr"])
    strings = extract_strings(text, hdr["text_addr"])
    print(f"Detected:")
    print(f"  functions       : {len(funcs)} (heuristic: prologues + entry point)")
    print(f"  BIOS call sites : {len(bios)}")
    print(f"  MDEC refs       : {len(mdec)}  (PsyQ libpress wrappers — MDEC0/MDEC1 register access is via library, not inlined)")
    print(f"  strings (>=4)   : {len(strings)}")
    # Library identification from strings
    lib_markers = []
    for _, s in strings:
        if s.startswith("$Id:"):
            lib_markers.append(s)
    if lib_markers:
        print(f"  PsyQ library tags:")
        for s in lib_markers:
            print(f"    {s}")
    # First / last bytes of the BSS region (post-text reserved)
    bss_start = hdr["text_addr"] + hdr["text_size"]
    print(f"  inferred BSS / runtime region begins at 0x{bss_start:08x}")
    if bios:
        tbl_counts = defaultdict(int)
        for b in bios:
            tbl_counts[b["table"]] += 1
        print(
            f"  BIOS by table   : "
            + ", ".join(
                f"0x{t:02x}({c})" for t, c in sorted(tbl_counts.items())
            )
        )


def cmd_strings(path):
    with open(path, "rb") as f:
        data = f.read()
    hdr = parse_header(data)
    text = data[0x800 : 0x800 + hdr["text_size"]]
    for vaddr, s in extract_strings(text, hdr["text_addr"]):
        print(f"0x{vaddr:08x}  {s}")


def cmd_funcs(path):
    with open(path, "rb") as f:
        data = f.read()
    hdr = parse_header(data)
    text = data[0x800 : 0x800 + hdr["text_size"]]
    funcs = find_func_prologues(text, hdr["text_addr"])
    if hdr["pc"] not in funcs:
        funcs.append(hdr["pc"])
        funcs.sort()
    print(f"# {len(funcs)} detected function entries")
    for f in funcs:
        marker = "  <- entry point" if f == hdr["pc"] else ""
        print(f"0x{f:08x}{marker}")


PSX_BIOS_NAMES = {
    # A-table (0xa0) — most common entries used in overlays
    (0xA0, 0x00): "FileOpen",
    (0xA0, 0x01): "FileSeek",
    (0xA0, 0x02): "FileRead",
    (0xA0, 0x03): "FileWrite",
    (0xA0, 0x04): "FileClose",
    (0xA0, 0x13): "SaveState",
    (0xA0, 0x18): "strcat",
    (0xA0, 0x19): "strncat",
    (0xA0, 0x1A): "strcmp",
    (0xA0, 0x2A): "memcpy",
    (0xA0, 0x2B): "memset",
    (0xA0, 0x33): "malloc",
    (0xA0, 0x34): "free",
    (0xA0, 0x39): "InitHeap",
    (0xA0, 0x3F): "printf",
    (0xA0, 0x44): "FlushCache",
    (0xA0, 0x49): "GPU_cw",
    (0xA0, 0x70): "_bu_init",
    (0xA0, 0xA0): "_get_errno",
    # B-table (0xb0) — pad, MC, event
    (0xB0, 0x07): "DeliverEvent",
    (0xB0, 0x08): "OpenEvent",
    (0xB0, 0x09): "CloseEvent",
    (0xB0, 0x0A): "WaitEvent",
    (0xB0, 0x0B): "TestEvent",
    (0xB0, 0x0C): "EnableEvent",
    (0xB0, 0x0D): "DisableEvent",
    (0xB0, 0x12): "InitPAD",
    (0xB0, 0x13): "StartPAD",
    (0xB0, 0x14): "StopPAD",
    (0xB0, 0x17): "ReturnFromException",
    (0xB0, 0x18): "SetDefaultExitFromException",
    (0xB0, 0x19): "SetCustomExitFromException",
    (0xB0, 0x3D): "putchar",
    # C-table (0xc0) — interrupt-related
    (0xC0, 0x00): "EnqueueTimerAndVblankIrqs",
    (0xC0, 0x01): "EnqueueSyscallHandler",
    (0xC0, 0x02): "SysEnqIntRP",
    (0xC0, 0x03): "SysDeqIntRP",
    (0xC0, 0x0A): "ChangeClearRCnt",
}


def cmd_syscalls(path):
    with open(path, "rb") as f:
        data = f.read()
    hdr = parse_header(data)
    text = data[0x800 : 0x800 + hdr["text_size"]]
    bios = find_bios_trampolines(text, hdr["text_addr"])
    print(f"# {len(bios)} BIOS call site(s)")
    for b in bios:
        name = PSX_BIOS_NAMES.get((b["table"], b["funcnum"]), "?")
        print(
            f"  0x{b['addr']:08x}  table=0x{b['table']:02x} func=0x{b['funcnum']:02x}  {name}"
        )


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("path", help="path to MOVOVL.EXE")
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--strings", action="store_true")
    g.add_argument("--funcs", action="store_true")
    g.add_argument("--syscalls", action="store_true")
    args = ap.parse_args()

    if args.strings:
        cmd_strings(args.path)
    elif args.funcs:
        cmd_funcs(args.path)
    elif args.syscalls:
        cmd_syscalls(args.path)
    else:
        summary(args.path)


if __name__ == "__main__":
    try:
        main()
    except (ValueError, FileNotFoundError) as e:
        print(f"error: {e}", file=sys.stderr)
        sys.exit(2)
