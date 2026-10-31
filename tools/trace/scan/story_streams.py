#!/usr/bin/env python3
"""Decode the fight command streams fight_RunCommandStream walks.

fight_InitCommandStream (2B344.c:217) points D_800A3844 at
g_fight_command_streams[D_8008D538[P1's character id]], one of 32 pointers to the byte
streams D_8008F220 .. D_8009000C (2B344.c:2763-3118; the table is at
2B344.c:3119); fight_RunCommandStream (2B344.c:318) then reads one opcode
byte and its operands per command, until
a command ends the call. This lists every command of every stream, read from
the retail EXE's bytes, with the operand count and the opponent id each
fight-setting command leaves in D_80102778.unk_4[1] (P2's character id,
fighter_Init's roster_index for fighter 1, 9F9C.c:1244-1250):

  op 16 (5 operands): fight_ConfigureFighter1FromLookupAndOperandsAndLoad, unk_4[1] = 0xE or 0x1D (2B344.c:270:
        0xE when D_8008D9EC[P1's D_8008D538 entry] != 0 and D_800A37A0 != 1)
  op 17 (3): fight_ConfigureFighter1FromOperandsAndLoad, unk_4[1] = op0, unk_4[3] = op1,
        unk_A[1] = op2
  op 3  (2): unk_4[1] = op0, unk_4[3] = op1
  op 2  (29): D_800A3894 = the operands, fight_SaveFighter0SelectionAndStateSubstituteStreamSelectionClearState (unk_4[0] = op0,
        the old unk_4[0] kept in g_fight_saved_first_fighter_selection), fight_FillMissingConfigInitFighterAndLoadObject(0), fight_ConfigureFighter1FromLookupAndOperandsAndLoad(operands + 1)
        (0xE or 0x1D as op 16), then as op 18; after that fight
        fight_ConfigureFighter1FromOperandsAndLoad(D_800A3894 + 6) (28708.c:1511, 9F9C.c:1300) sets
        unk_4[1] = operand 6, shown as "0x0E|0x1D then 0xNN"
  op 20 (12): D_800A385C = the operands, fight_SetFixedFighterBytesAndLoadBothFighters: unk_4[1] = 0x1F,
        fight_QueueFixedFighterCommandDescriptorPlayback(1): D_800A3834 = 6, ends the call
  op 1  (1): D_800A36A4 = op0, D_800A3834 = 22, ends the call
  op 19 (4): g_fight_active_playback_descriptor = the operands, D_800A3834 = 6, ends the call
  op 0, op 18 (0): end the call (op 18: D_800A3834 = 0)
Any other byte is consumed with no operands (the switch has no default).
Nothing is inferred about what the commands mean.

  python tools/trace/scan/story_streams.py [--exe disc/SLUS_006.63]
      [--map build/bb2.map] [--stream N]
"""

import argparse
from pathlib import Path
import re
import sys

LOAD = 0x80010000
HEADER_SIZE = 0x800
OPERANDS = {16: 5, 17: 3, 3: 2, 2: 29, 20: 12, 1: 1, 19: 4, 0: 0, 18: 0}
SIDE = "0x0E|0x1D"
COLUMNS = ["entry", "stream", "addr", "op", "operands", "unk_4_1"]


def map_symbols(map_path, names):
    """Address of each name in a GNU ld map ('0x... name' and
    '0x... name = value' lines)."""
    found = {}
    pat = re.compile(r"^\s+0x([0-9a-fA-F]{8,16})\s+(\S+)(\s+=.*)?\s*$")
    with open(map_path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = pat.match(line)
            if m and m.group(2) in names:
                found.setdefault(m.group(2), int(m.group(1), 16))
    missing = sorted(set(names) - set(found))
    if missing:
        raise SystemExit(f"story_streams: {map_path}: no {', '.join(missing)}")
    return found


def read(exe, addr, size):
    off = addr - LOAD + HEADER_SIZE
    if off < HEADER_SIZE or off + size > len(exe):
        raise ValueError(f"0x{addr:08X}+{size} is outside the EXE")
    return exe[off:off + size]


def decode(buf, base):
    """Yields (addr, op, operands) up to and including the first op 0;
    raises ValueError if a command runs past buf."""
    p = 0
    while p < len(buf):
        op = buf[p]
        n = OPERANDS.get(op, 0)
        if p + 1 + n > len(buf):
            raise ValueError(f"command at 0x{base + p:08X} runs past the "
                             f"stream end")
        yield base + p, op, buf[p + 1:p + 1 + n]
        p += 1 + n
        if op == 0:
            return
    raise ValueError(f"stream at 0x{base:08X} has no op 0")


def opponent(op, args):
    if op == 16:
        return SIDE
    if op == 2:
        return f"{SIDE} then 0x{args[6]:02X}"
    if op in (17, 3):
        return f"0x{args[0]:02X}"
    if op == 20:
        return "0x1F"
    return ""


def rows(exe, table_addr, stream_bound):
    """One row per command of the stream behind each of the 32 table
    entries (entry index, stream address, ...); stream_bound(addr) is the
    end of the bytes the stream at addr may occupy (its .data object)."""
    table = read(exe, table_addr, 32 * 4)
    for i in range(32):
        addr = int.from_bytes(table[i * 4:i * 4 + 4], "little")
        buf = read(exe, addr, stream_bound(addr) - addr)
        for a, op, args in decode(buf, addr):
            yield [str(i), f"0x{addr:08X}", f"0x{a:08X}", str(op),
                   " ".join(f"{b:02X}" for b in args), opponent(op, args)]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--exe", default="disc/SLUS_006.63")
    ap.add_argument("--map", default="build/bb2.map")
    ap.add_argument("--stream", type=int)
    a = ap.parse_args(argv)
    exe = Path(a.exe).read_bytes()
    syms = map_symbols(a.map, {"g_fight_command_streams"})
    table_addr = syms["g_fight_command_streams"]
    table = read(exe, table_addr, 32 * 4)
    starts = sorted({int.from_bytes(table[i * 4:i * 4 + 4], "little")
                     for i in range(32)})
    # The streams are contiguous and end at the table (2B344.c:2763).
    ends = dict(zip(starts, starts[1:] + [table_addr]))
    out = sys.stdout
    out.write("\t".join(COLUMNS) + "\n")
    for r in rows(exe, table_addr, ends.__getitem__):
        if a.stream is None or int(r[0]) == a.stream:
            out.write("\t".join(r) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
