#!/usr/bin/env python3
"""movegraph: walk both fighters' move-script graphs in a RAM dump.

Finds which input reaches a move_kind: the move scripts are data (motion
files loaded at fight start), so the input that leads to a move is read from
RAM, not guessed in sweeps. Tools only: nothing here is a build input.

Input: a gdbtrace capture whose spec reads, at one phase start in a fight
("phases": [<name>] on each value entry), the RAM the walk uses (SPEC_VALUES
below: the motion buffers 0x80120000..0x801D8800, the two fighter records,
the section pointers, g_motion_fighter_buffers, the key config and the mode
globals of the pad remap); a read of all of main RAM works too.
`--spec-values <phase>` prints those entries.

The walk follows fighter_Update / fight_UpdatePairedMoves /
motion_ResolveMoveData / func_80021A98 (src/main/9F9C.c). A node is a move
script, the file its 0x8000 ids came from, the record executing it and
whether that record started the paired move (D_800A38AE):
  * move_kind is byte 0 of the move's motion metadata (func_80021A98);
  * ids: < 0x8000 the common file; 0x8000 | n entry n of the executing
    record's file, or of the other record's file while borrow_other_motion
    is set: by a command with flag 0x1000, a tc[0] end transition with
    0x1000, a fallback of a move with unk_09 & 0x20 (func_80021A98 clears
    it on every install); 0x7FF0..0x7FFF the executing record's variant
    roots (unk_84 / unk_86 as read; 0x7FF1 first advances unk_86);
  * input edges: transition_commands entries (button bit = flags & 0xF, with
    9 -> 3 and 0xA -> 5 under their conditions; flags & 0x30 held / pressed /
    unheld / released; entries with 0x8000 only when the mask in their extra
    halfwords holds the executing record's unk_0A; 0x2000 entries never fire;
    none fire for the record that started a kind-0x11 paired move);
  * a 0x4000 entry is the paired move fight_UpdatePairedMoves starts by
    distance (not by input; its extra bytes are distance data): both records
    install it, from the starter's file; the other record runs it with
    borrow_other_motion. The input loop would also test its button bit (and,
    with 0x8000, its distance bytes as a mask); that use is not walked;
  * other edges: the header fallback (move ended), the tc[0] 0x2000 end
    transition (only the record that did not start a paired move), event
    codes 0xD / 0x17 / 0x18 of the event table.
  * the walk of record i follows the other record only while it runs the
    paired script (a node in record i's file); targets are listed for the
    nodes of either record, one shortest path per move and record (a node
    flagged "paired starter" is the same move after record i started a
    paired move: D_800A38AE stays set);
  * the window: entries are read while move_frame <= last_command_frame
    (move_frame advances move_frame_step / 0x1000 per frame); in a move with
    unk_09 & 0x80 the chosen move is queued and starts after
    last_command_frame.
Button names are the physical buttons (func_8001BE20 and pad_Update in
src/main/87A0.c): game bits 0 L2, 1 R2, 2 L1, 3 R1, 5/6/7 the attack
buttons of the key config (D_80106A50.frec20, a nibble per player; in mode
D_800A38DC 6 the record that is not D_800A38A0 uses D_800A3912..14), 8
SELECT, 11 START, 12..15 UP RIGHT DOWN LEFT; for record 1 outside mode 6 the
directions are mirrored (game UP/RIGHT/DOWN/LEFT = pad DOWN/LEFT/UP/RIGHT).
L3 / R3 fold into bits 3 / 5. D_800A38BA's swap (fighter_Update) is not
applied.

Static graph only: whether a branch fires at run time still depends on the
frame window, distance (paired), unk_26C and the other fighter's state.
Confirm a path with a capture before citing it.

  python tools/trace/movegraph.py <capture.jsonl> --phase fight --kinds 0xB,0x12
  python tools/trace/movegraph.py <capture.jsonl> --move 0x8012020c:common
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
MODE = 0x800A38DC  # D_800A38DC (s16)
MODE6_PLAYER = 0x800A38A0  # D_800A38A0 (s32)
MODE6_CONFIG = 0x800A3912  # D_800A3912..14

# motion_Init (9F9C.c): common file at 0x80120000, fighter files at
# 0x80148800 / 0x80190800, 0x48000 apart.
SPEC_VALUES = [
    {"at": "g_motion_fighter_buffers", "len": 8},
    {"at": "D_80101EC8", "len": 2 * REC_SIZE},
    {"at": "g_motion_common_sections", "len": FIGHTER_SECTIONS + 2 * SECTIONS_SIZE
     - COMMON_SECTIONS},
    {"at": "0x80106A70", "len": 4},
    {"at": "D_800A38DC", "len": 2},
    {"at": "D_800A38A0", "len": 4},
    {"at": "D_800A3912", "len": 3},
    {"at": "0x80120000", "len": 0x801D8800 - 0x80120000},
]

TYPES = {0x00: "held", 0x10: "pressed", 0x20: "unheld", 0x30: "released"}
NIBBLE_PAD = (("CIRCLE", 0x20), ("TRIANGLE", 0x10), ("CROSS", 0x40),
              ("SQUARE", 0x80))  # func_8001BE20: nibble bit k tests these


class Ram:
    def __init__(self, data: bytes, read: bytearray):
        self.d, self.read = data, read

    def covered(self, a, n):
        return all(self.read[a - RAM0:a - RAM0 + n])

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
    read covered are zero; Ram.covered tells)."""
    ram, read, phases = bytearray(RAMSZ), bytearray(RAMSZ), set()
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
            read[a:a + len(b)] = b"\1" * len(b)
    if not any(read):
        raise SystemExit(f"movegraph: no value reads at phase {phase!r} in "
                         f"{path} (reads at: {sorted(map(str, phases))})")
    return Ram(bytes(ram), read)


class Rec:
    """One D_80101EC8 record as read."""

    def __init__(self, m: Ram, idx: int):
        b = RECORDS + idx * REC_SIZE
        if not m.covered(b, REC_SIZE):
            raise SystemExit(f"movegraph: record {idx} was not read")
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
        self.base_step = m.s16(b + 0x1C)
        self.step = m.s16(b + 0x44)
        self.buf = m.u32(FIGHTER_BUFFERS + 4 * self.slot)


class Fight:
    def __init__(self, m: Ram):
        self.m = m
        self.recs = [Rec(m, 0), Rec(m, 1)]
        self.notes = []
        if m.covered(MODE, 2):
            self.mode = m.s16(MODE)
        else:
            self.mode = None
            self.notes.append("D_800A38DC not read: mode assumed not 6")
        self.buttons = [self._buttons(i) for i in (0, 1)]
        self.dropped = []  # edges whose target does not read as a script

    def _buttons(self, idx):
        m, mode = self.m, self.mode
        names = {0: "L2", 1: "R2", 2: "L1", 3: "R1", 8: "SELECT", 11: "START",
                 12: "UP", 13: "RIGHT", 14: "DOWN", 15: "LEFT"}
        if mode == 6 and (not m.covered(MODE6_PLAYER, 4)
                          or not m.covered(MODE6_CONFIG, 3)):
            raise SystemExit("movegraph: mode 6 needs D_800A38A0 and "
                             "D_800A3912..14 read")
        if mode == 6 and idx != m.u32(MODE6_PLAYER):
            nib = [m.u8(MODE6_CONFIG + i) & 0xF for i in range(3)]
        else:
            if not m.covered(KEY_CONFIG, 3):
                raise SystemExit("movegraph: key config 0x80106A70 not read")
            nib = [(m.u8(KEY_CONFIG + i) >> (4 * idx)) & 0xF for i in range(3)]
        for bit, n in zip((5, 6, 7), nib):
            pads = [name for k, (name, _) in enumerate(NIBBLE_PAD) if n & (1 << k)]
            names[bit] = "/".join(pads) or f"bit{bit}(no button)"
        names[3] += "/L3"
        names[5] += "/R3"
        if idx == 1 and mode != 6:
            names.update({12: "DOWN", 13: "LEFT", 14: "UP", 15: "RIGHT"})
        return names

    def sections(self, slot):
        b = COMMON_SECTIONS if slot is None else FIGHTER_SECTIONS + slot * SECTIONS_SIZE
        return self.m.u32(b), self.m.u32(b + 4), self.m.u32(b + 8)

    def resolve(self, mid, ex, borrow=False):
        """motion_ResolveMoveData for executing record `ex` ->
        (addr, file slot or None for common, note)"""
        m, r = self.m, self.recs[ex]
        buf, f00 = r.buf, self.sections(r.slot)[0]
        if 0x7FF5 <= mid <= 0x7FFF:
            i = m.u16(buf + 0x66 + ((mid - 0x7FF5) * 3 + r.u86) * 2)
            return f00 + 2 * i, r.slot, f"f66[{mid - 0x7FF5}][{r.u86}]"
        f14 = m.s16(buf + 0x14)
        if mid in (0x7FF0, 0x7FF1, 0x7FF2, 0x7FF4):
            k = r.u84 if mid == 0x7FF0 else \
                (r.u86 + 1) % f14 if mid == 0x7FF1 else r.u86
            return f00 + 2 * m.u16(buf + 0x4E + 2 * k), r.slot, \
                f"variant f4E[{k}] (unk_86 as read; may skip a variant)"
        if mid == 0x7FF3:
            t = (r.u86 + 1) % f14
            return f00 + 2 * m.u16(buf + 0x54 + (r.u86 * 3 + t) * 2), \
                r.slot, f"variant f54[{r.u86}][{t}]"
        if mid & 0x8000:
            slot = self.recs[1 - ex].slot if borrow else r.slot
            return self.sections(slot)[0] + 2 * (mid & 0x7FFF), slot, \
                "other's file" if borrow else ""
        return self.sections(None)[0] + 2 * mid, None, "common"

    def move_kind(self, addr, slot):
        _, s04, s08 = self.sections(slot)
        ent = s04 + self.m.u16(addr + 4) * 4
        return self.m.u8(s08 + self.m.u16(ent + 2))

    def valid(self, a, slot):
        """a reads as a move script: RAM that was read, a command list
        that ends, a motion entry and metadata inside the read RAM."""
        m = self.m
        if not (RAM0 <= a < RAM0 + RAMSZ - 0x400) or not m.covered(a, 12):
            return False
        try:
            self.commands(a)
            _, s04, s08 = self.sections(slot)
            ent = s04 + m.u16(a + 4) * 4
            return m.covered(ent, 4) and m.covered(s08 + m.u16(ent + 2), 3)
        except (ValueError, struct.error, IndexError):
            return False

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

    def label(self, f, extra, ex):
        bit, cond = f & 0xF, []
        if bit == 9:
            bit = 3
            cond.append("other fighter in kind 6 or before its unk_AB frame")
        elif bit == 0xA:
            bit = 5
            cond.append("needs unk_26C")
        s = f"{TYPES[f & 0x30]} {self.buttons[ex].get(bit, f'bit{bit}')}"
        if f & 0x8000:
            cond.append(f"char mask {extra:#x}")
        if f & 0x1000:
            cond.append("borrow other motion")
        return s + (f" [{'; '.join(cond)}]" if cond else "")

    def edges(self, node):
        """-> [(node, label)] for node (addr, file slot, executing record,
        started a paired move)"""
        a, _, ex, starter = node
        r, m = self.recs[ex], self.m
        h = self.header(a)
        kind = self.move_kind(a, node[1])
        out = []

        def add(mid, lab, borrow=False, who=ex, st=starter):
            na, ns, note = self.resolve(mid, who, borrow)
            if self.valid(na, ns):
                out.append(((na, ns, who, st), f"{lab} -> id {mid:#06x}"
                            + (f" ({note})" if note else "")))
            else:
                self.dropped.append((node, lab, mid, na))

        for f, mid, extra in self.commands(a):
            if f & 0x4000:
                b = extra.to_bytes(4, "little")
                lab = (f"paired move (fight_UpdatePairedMoves): distance "
                       f"{b[0] * 20}..{b[1] * 20}, other in kind "
                       f"0x15/0x2C/0xE/0x19")
                na, ns, note = self.resolve(mid, ex)
                if not self.valid(na, ns):
                    self.dropped.append((node, lab, mid, na))
                else:
                    out.append(((na, ns, ex, True), f"{lab} -> id {mid:#06x}"))
                    out.append(((na, ns, 1 - ex, False),
                                f"{lab}, run by the other record -> id {mid:#06x}"))
                continue
            if f & 0x2000:
                continue  # var_8 = 0: never fires on input
            if kind == 0x11 and starter:
                continue  # var_8 = 0 for D_800A38AE in a kind-0x11 move
            if f & 0x8000 and not (extra >> r.unk_0A) & 1:
                continue  # character mask excludes the executing record
            add(mid, self.label(f, extra, ex), bool(f & 0x1000))
        add(h["fallback"], "fallback (move ended)", bool(h["unk_09"] & 0x20))
        tc0 = m.u16(a + 10)
        if tc0 & 0x2000 and not starter:
            add(m.u16(a + 12), "end transition (tc[0] & 0x2000)",
                bool(tc0 & 0x1000))
        tbl, _, _ = self.resolve(h["event_table"], ex)
        if m.covered(tbl, 0x32):
            for code in (0xD, 0x17, 0x18):
                add(m.u16(tbl + 2 * code), f"event {code:#x}")
        return out

    def walk(self, idx):
        r = self.recs[idx]
        roots = [((r.cur, r.slot, idx, False), "cur_move")]
        for k in range(3):
            a = self.sections(r.slot)[0] + 2 * self.m.u16(r.buf + 0x4E + 2 * k)
            roots.append(((a, r.slot, idx, False), f"root f4E[{k}]"))
        seen, q = {}, collections.deque()
        for node, lab in roots:
            if node not in seen:
                seen[node] = (None, lab)
                q.append(node)
        while q:
            node = q.popleft()
            if node[2] != idx and node[1] != r.slot:
                continue  # the other record left the paired script
            for nn, lab in self.edges(node):
                if nn not in seen:
                    seen[nn] = (node, lab)
                    q.append(nn)
        return seen

    def describe(self, node):
        a, s, ex, starter = node
        h = self.header(a)
        return (f"{a:#010x} kind {self.move_kind(a, s):#04x} "
                f"({'common' if s is None else f'file slot {s}'}, run by "
                f"record {ex}{', paired starter' if starter else ''}; frames "
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
                    "(the file it was read from; default the record's own)")
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
    fight = Fight(load_dump(args.capture, args.phase))
    for n in fight.notes:
        print(f"note: {n}")
    print(f"mode D_800A38DC {fight.mode}")
    kinds = {int(k, 0) for k in args.kinds.split(",") if k}
    recs = (0, 1) if args.record is None else (args.record,)
    if args.move and args.record is None:
        recs = (0,)
    for idx in recs:
        r = fight.recs[idx]
        print(f"record {idx}: unk_0A {r.unk_0A:#x} unk_0C {r.unk_0C:#x} "
              f"unk_0E {r.unk_0E} slot {r.slot} unk_26C {r.unk_26C} "
              f"move_frame_step {r.step:#x} (base {r.base_step:#x}); "
              f"cur_move {fight.describe((r.cur, r.slot, idx, False))}")
        print("  buttons: " + ", ".join(
            f"bit {b} {fight.buttons[idx][b]}" for b in (5, 6, 7, 12, 13, 14, 15)))
        if args.move:
            addr, _, file = args.move.partition(":")
            node = (int(addr, 0), None if file == "common"
                    else int(file) if file else r.slot, idx, False)
            print(f"  move {fight.describe(node)}")
            fight.dropped = []
            for nn, lab in fight.edges(node):
                print(f"  {lab}\n      => {fight.describe(nn)}")
            for _, lab, mid, na in fight.dropped:
                print(f"  {lab} -> id {mid:#06x}\n      => {na:#010x}: not a "
                      "move script (dropped)")
            continue
        fight.dropped = []
        seen = fight.walk(idx)
        if fight.dropped:
            print(f"  {len(fight.dropped)} edges dropped (target does not read "
                  "as a move script): " + "; ".join(sorted({
                      f"from {n[0]:#x} (record {n[2]}) {lab} -> id {mid:#06x}"
                      for n, lab, mid, _ in fight.dropped})))
        hist = collections.Counter(fight.move_kind(n[0], n[1]) for n in seen)
        print(f"  {len(seen)} nodes reachable; kinds: " + ", ".join(
            f"{k:#x} x{n}" for k, n in sorted(hist.items())))
        shown = set()
        for node in seen:  # BFS order: the first path found is a shortest
            if fight.move_kind(node[0], node[1]) not in kinds \
                    or node[:3] in shown:
                continue
            shown.add(node[:3])
            chain, k = [], node
            while k is not None:
                chain.append((k, seen[k][1]))
                k = seen[k][0]
            print(f"  path to {fight.describe(node)}:")
            for k, lab in reversed(chain):
                print(f"    {lab}\n      => {fight.describe(k)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
