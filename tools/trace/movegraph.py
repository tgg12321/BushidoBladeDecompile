#!/usr/bin/env python3
"""movegraph: walk both fighters' move-script graphs in a RAM dump.

Finds which input reaches a move_kind: the move scripts are data (motion
files loaded at fight start), so the input that leads to a move is read from
RAM, not guessed in sweeps. Tools only: nothing here is a build input.

Input: a gdbtrace capture whose spec reads, at one phase start in a fight
("phases": [<name>] on each value entry), the RAM the walk uses (SPEC_VALUES
below: the motion buffers 0x80120000..0x801D8800, the two fighter records,
the section pointers, g_motion_fighter_buffers and the key config); a read of
all of main RAM works too. `--spec-values <phase>` prints those entries.

The walk follows fighter_Update / fight_UpdatePairedMoves (src/main/9F9C.c):
  * a move is a MoveScript (include/game.h); its move_kind is byte 0 of its
    motion metadata (func_80021A98);
  * ids resolve as motion_ResolveMoveData does: < 0x8000 the common file,
    0x8000 | n entry n of a fighter file, 0x7FF0..0x7FFF the per-fighter
    variant roots (with the record's current unk_84 / unk_86). A 0x8000 id
    resolves in the file of the script that names it (a paired move runs
    the thrower's script on both fighters, the other one with
    borrow_other_motion);
  * edges: transition_commands entries (button bit = flags & 0xF, with 9 ->
    3 and 0xA -> 5 under their conditions; flags & 0x30 held / pressed /
    unheld / released; 0x8000 entries only when the character mask holds
    the record's unk_0A; 0x2000 entries never fire on input; 0x4000 is the
    paired entry fight_UpdatePairedMoves starts by distance), the header
    fallback (move ended), the tc[0] 0x2000 end transition, and event codes
    0xD / 0x17 / 0x18 of the event table.
  * the window: entries are read while move_frame <= last_command_frame
    (move_frame advances move_frame_step / 0x1000 per frame); in a move with
    unk_09 & 0x80 the chosen move is queued and starts after
    last_command_frame.
Button names come from the dump's key config (D_80106A50.frec20, nibble per
player): game bits 0 L2, 1 R2, 2 L1, 3 R1, 5/6/7 the three attack buttons,
8 SELECT, 11 START, 12 UP, 13 RIGHT, 14 DOWN, 15 LEFT (pad_Update and
func_8001BE20; D_800A38BA swaps the direction bits when set).

Static graph only: whether a branch fires at run time still depends on the
frame window, distance (paired), unk_26C and the other fighter's state.
Confirm a path with a capture before citing it.

  python tools/trace/movegraph.py <capture.jsonl> --phase fight --kinds 0xB,0x12
  python tools/trace/movegraph.py <capture.jsonl> --phase fight --move 0x8012020c:common
"""

from __future__ import annotations

import argparse
import collections
import json
import struct
import sys

RAM0, RAMSZ = 0x80000000, 0x200000
COMMON_SECTIONS = 0x80102760  # g_motion_common_sections
FIGHTER_SECTIONS = 0x801027B0  # g_motion_fighter_sections[slot]
FIGHTER_BUFFERS = 0x800A3860  # g_motion_fighter_buffers[slot]
RECORDS, REC_SIZE = 0x80101EC8, 0x44C  # D_80101EC8[2]
SECTIONS_SIZE = 0x14
KEY_CONFIG = 0x80106A70  # D_80106A50.frec20[3]

# motion_Init (9F9C.c): common file at 0x80120000, fighter files at
# 0x80148800 / 0x80190800, 0x48000 apart.
SPEC_VALUES = [
    {"at": "g_motion_fighter_buffers", "len": 8},
    {"at": "D_80101EC8", "len": 2 * REC_SIZE},
    {"at": "g_motion_common_sections", "len": FIGHTER_SECTIONS + 2 * SECTIONS_SIZE
     - COMMON_SECTIONS},
    {"at": "0x80106A70", "len": 4},
    {"at": "0x80120000", "len": 0x801D8800 - 0x80120000},
]

TYPES = {0x00: "held", 0x10: "pressed", 0x20: "unheld", 0x30: "released"}
PAD_BUTTONS = {0x10: "TRIANGLE", 0x20: "CIRCLE", 0x40: "CROSS", 0x80: "SQUARE"}


class Ram:
    def __init__(self, data: bytes):
        if len(data) != RAMSZ:
            raise SystemExit(f"movegraph: dump is {len(data)} bytes, want {RAMSZ}")
        self.d = data

    def u8(self, a):
        return self.d[a - RAM0]

    def u16(self, a):
        return struct.unpack_from("<H", self.d, a - RAM0)[0]

    def s16(self, a):
        return struct.unpack_from("<h", self.d, a - RAM0)[0]

    def u32(self, a):
        return struct.unpack_from("<I", self.d, a - RAM0)[0]


def load_dump(path: str, phase: str) -> Ram:
    """Main RAM as the value reads at the start of `phase` saw it (bytes no
    read covered are zero)."""
    ram, got, phases = bytearray(RAMSZ), 0, set()
    with open(path, encoding="utf-8") as f:
        for line in f:
            e = json.loads(line)
            if e.get("ev") != "value" or "hex" not in e or "ptr" in e:
                continue
            phases.add(e.get("phase"))
            a = e["addr"] - RAM0
            if e.get("phase") != phase or e.get("end") or not 0 <= a < RAMSZ:
                continue
            b = bytes.fromhex(e["hex"])[:RAMSZ - a]
            ram[a:a + len(b)] = b
            got += len(b)
    if not got:
        raise SystemExit(f"movegraph: no value reads at phase {phase!r} in "
                         f"{path} (reads at: {sorted(map(str, phases))})")
    return Ram(bytes(ram))


class Graph:
    def __init__(self, ram: Ram, idx: int):
        self.m = m = ram
        b = RECORDS + idx * REC_SIZE
        self.idx = idx
        self.unk_0A = m.s16(b + 0x0A)
        self.unk_0C = m.s16(b + 0x0C)
        self.unk_0E = m.s16(b + 0x0E)
        self.slot = m.s16(b + 0x4A)
        self.cur = m.u32(b + 0x50)
        self.kind = m.u16(b + 0x6A)
        self.u84 = m.s16(b + 0x84)
        self.u86 = m.s16(b + 0x86)
        self.unk_26C = m.s16(b + 0x26C)
        self.step = m.s16(b + 0x1C)
        self.buf = m.u32(FIGHTER_BUFFERS + 4 * self.slot)
        nib = [(m.u8(KEY_CONFIG + i) >> (4 * idx)) & 0xF for i in range(3)]
        self.buttons = {0: "L2", 1: "R2", 2: "L1", 3: "R1", 8: "SELECT",
                        11: "START", 12: "UP", 13: "RIGHT", 14: "DOWN",
                        15: "LEFT"}
        for bit, n in zip((5, 6, 7), nib):
            # nibble bit k tests pad bit (0x20, 0x10, 0x40, 0x80)[k]
            names = [PAD_BUTTONS[(0x20, 0x10, 0x40, 0x80)[k]]
                     for k in range(4) if n & (1 << k)]
            self.buttons[bit] = "/".join(names) or f"bit{bit}(unmapped)"

    def sections(self, slot):
        b = COMMON_SECTIONS if slot is None else FIGHTER_SECTIONS + slot * SECTIONS_SIZE
        return self.m.u32(b), self.m.u32(b + 4), self.m.u32(b + 8)

    def resolve(self, mid, file_slot):
        """-> (addr, slot or None for common, note)"""
        m, buf = self.m, self.buf
        f00 = self.sections(self.slot)[0]
        if 0x7FF5 <= mid <= 0x7FFF:
            i = m.u16(buf + 0x66 + ((mid - 0x7FF5) * 3 + self.u86) * 2)
            return f00 + 2 * i, self.slot, f"f66[{mid - 0x7FF5}][{self.u86}]"
        if mid in (0x7FF0, 0x7FF1, 0x7FF2, 0x7FF4):
            k = self.u84 if mid == 0x7FF0 else self.u86
            return f00 + 2 * m.u16(buf + 0x4E + 2 * k), self.slot, \
                f"variant f4E[{k}]"
        if mid == 0x7FF3:
            t = (self.u86 + 1) % m.s16(buf + 0x14)
            return f00 + 2 * m.u16(buf + 0x54 + (self.u86 * 3 + t) * 2), \
                self.slot, f"variant f54[{self.u86}][{t}]"
        if mid & 0x8000:
            slot = self.slot if file_slot is None else file_slot
            return self.sections(slot)[0] + 2 * (mid & 0x7FFF), slot, ""
        return self.sections(None)[0] + 2 * mid, None, "common"

    def move_kind(self, addr, slot):
        _, s04, s08 = self.sections(slot)
        ent = s04 + self.m.u16(addr + 4) * 4
        return self.m.u8(s08 + self.m.u16(ent + 2))

    def header(self, a):
        m = self.m
        return {"event_table": m.u16(a), "fallback": m.u16(a + 2),
                "first": m.u8(a + 6), "last": m.u8(a + 7),
                "last_cmd": m.u8(a + 8), "unk_09": m.u8(a + 9)}

    def commands(self, a):
        m, p, out = self.m, a + 10, []
        for _ in range(128):
            f = m.u16(p)
            if f == 0:
                return out
            if f & 0xC000:
                out.append((f, m.u16(p + 2), m.u16(p + 4) | m.u16(p + 6) << 16))
                p += 8
            else:
                out.append((f, m.u16(p + 2), None))
                p += 4
        raise ValueError(f"no command terminator at {a:#x}")

    def label(self, f, extra):
        bit, cond = f & 0xF, []
        if bit == 9:
            bit = 3
            cond.append("other fighter in kind 6 or before its unk_AB frame")
        elif bit == 0xA:
            bit = 5
            cond.append("needs unk_26C")
        s = f"{TYPES[f & 0x30]} {self.buttons.get(bit, f'bit{bit}')}"
        if f & 0x4000:
            b = extra.to_bytes(4, "little")
            cond.append(f"paired: distance {b[0] * 20}..{b[1] * 20}, "
                        f"other in kind 0x15/0x2C/0xE/0x19")
        elif f & 0x8000:
            cond.append(f"char mask {extra:#x}")
        if f & 0x1000:
            cond.append("borrow other motion")
        return s + (f" [{'; '.join(cond)}]" if cond else "")

    def edges(self, a, slot):
        fs = slot
        h = self.header(a)
        out = []
        for f, mid, extra in self.commands(a):
            if f & 0x2000:
                continue  # var_8 = 0: never fires on input
            if f & 0x8000 and not f & 0x4000 and not (extra >> self.unk_0A) & 1:
                continue  # character mask excludes this fighter
            out.append((mid, self.label(f, extra)))
        out.append((h["fallback"], "fallback (move ended)"))
        tc0 = self.m.u16(a + 10)
        if tc0 & 0x2000:
            out.append((self.m.u16(a + 12), "end transition (tc[0] & 0x2000, "
                        "the fighter that is not D_800A38AE)"))
        tbl, tslot, _ = self.resolve(h["event_table"], fs)
        for code in (0xD, 0x17, 0x18):
            out.append((self.m.u16(tbl + 2 * code), f"event {code:#x}"))
        res = []
        for mid, lab in out:
            na, ns, note = self.resolve(mid, fs)
            if RAM0 <= na < RAM0 + RAMSZ:
                res.append((na, ns, f"{lab} -> id {mid:#06x}"
                            + (f" ({note})" if note else "")))
        return res

    def walk(self):
        roots = [(self.cur, self.slot, "cur_move")]
        for k in range(3):
            a = self.sections(self.slot)[0] + 2 * self.m.u16(self.buf + 0x4E + 2 * k)
            roots.append((a, self.slot, f"root f4E[{k}]"))
        seen, q = {}, collections.deque()
        for a, s, lab in roots:
            if (a, s) not in seen:
                seen[(a, s)] = (None, lab)
                q.append((a, s))
        while q:
            a, s = q.popleft()
            for na, ns, lab in self.edges(a, s):
                if (na, ns) not in seen:
                    seen[(na, ns)] = ((a, s), lab)
                    q.append((na, ns))
        return seen


def describe(g, key):
    a, s = key
    h = g.header(a)
    return (f"{a:#010x} kind {g.move_kind(a, s):#04x} "
            f"({'common' if s is None else f'slot {s}'}; frames "
            f"{h['first']}..{h['last']}, input to {h['last_cmd']}"
            f"{', queued' if h['unk_09'] & 0x80 else ''})")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("capture", nargs="?")
    ap.add_argument("--phase", default="fight")
    ap.add_argument("--spec-values", metavar="PHASE",
                    help="print the spec \"values\" entries the walk needs, "
                    "read at PHASE, and exit")
    ap.add_argument("--kinds", default="0xB,0x12,0x10",
                    help="move kinds to find paths to (comma list)")
    ap.add_argument("--move", help="list one move's edges: ADDR[:SLOT|:common] "
                    "(the file its 0x8000 ids resolve in; default the "
                    "record's own slot)")
    ap.add_argument("--record", type=int, choices=(0, 1),
                    help="only this D_80101EC8 record (default both; "
                    "--move: 0)")
    args = ap.parse_args(argv)
    if args.spec_values:
        print(json.dumps([dict(v, phases=[args.spec_values])
                          for v in SPEC_VALUES], indent=1))
        return 0
    if not args.capture:
        ap.error("capture is required")
    ram = load_dump(args.capture, args.phase)
    kinds = {int(k, 0) for k in args.kinds.split(",") if k}
    recs = (0, 1) if args.record is None else (args.record,)
    if args.move and args.record is None:
        recs = (0,)
    for idx in recs:
        g = Graph(ram, idx)
        print(f"record {idx}: unk_0A {g.unk_0A:#x} unk_0C {g.unk_0C:#x} "
              f"unk_0E {g.unk_0E} slot {g.slot} unk_26C {g.unk_26C} "
              f"move_frame_step {g.step:#x}; cur_move {describe(g, (g.cur, g.slot))}")
        print("  attack buttons: " + ", ".join(
            f"bit {b} = {g.buttons[b]}" for b in (5, 6, 7)))
        if args.move:
            addr, _, file = args.move.partition(":")
            a = int(addr, 0)
            s = None if file == "common" else int(file) if file else g.slot
            print(f"  move {describe(g, (a, s))}")
            for na, ns, lab in g.edges(a, s):
                print(f"  {lab}\n      => {describe(g, (na, ns))}")
            continue
        seen = g.walk()
        hist = collections.Counter(g.move_kind(a, s) for a, s in seen)
        print(f"  {len(seen)} moves reachable; kinds: " + ", ".join(
            f"{k:#x} x{n}" for k, n in sorted(hist.items())))
        for key in sorted(seen, key=lambda k: (g.move_kind(*k), k[0])):
            if g.move_kind(*key) not in kinds:
                continue
            chain, k = [], key
            while k is not None:
                chain.append((k, seen[k][1]))
                k = seen[k][0]
            print(f"  path to {describe(g, key)}:")
            for k, lab in reversed(chain):
                print(f"    {lab}\n      => {describe(g, k)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
