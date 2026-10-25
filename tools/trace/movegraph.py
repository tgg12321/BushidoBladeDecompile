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
motion_ResolveMoveData / fighter_StartMoveScript (src/main/9F9C.c). A node is a move
script, the file it was read from, the record executing it, D_800A38AE (the
record that started the last paired move: read, else 0) and whether a
paired start reset that record's unk_86 to unk_84:
  * move_kind is byte 0 of the move's motion metadata (fighter_StartMoveScript);
  * ids: < 0x8000 the common file; 0x8000 | n entry n of the executing
    record's file, or of the other record's file while borrow_other_motion
    is set: by a command with flag 0x1000, a tc[0] end transition with
    0x1000, a fallback of a move with unk_09 & 0x20 (fighter_StartMoveScript clears
    it on every install); 0x7FF0..0x7FFF the executing record's variant
    roots (unk_84 / unk_86 as read, or unk_84 after a paired start; 0x7FF1
    first advances unk_86). Not modelled: the variant skip on unk_88/8A/8E/90
    and the mode-3 CPU branch, 0x7FF0's lasting unk_86 = unk_84, unk_86
    changes along a path; move_kind is read from the script's file, while
    fighter_StartMoveScript reads the borrow-selected one (they differ only for a
    variant id under borrow or a hand-off without 0x1000);
  * input edges: transition_commands entries (button bit = flags & 0xF, with
    9 -> 3 and 0xA -> 5 under their conditions; flags & 0x30 held / pressed /
    unheld / released; entries with 0x8000 only when the mask in their extra
    halfwords holds the executing record's unk_0A; 0x2000 entries never fire;
    the loop takes the first entry that fires, so an entry is left out when
    an earlier unconditional entry on the same bit fires whenever it would
    (same type, held before pressed, unheld before released), and every
    printed input edge carries what the earlier live entries need to not
    fire ("[and none earlier fires: L1 held; RIGHT not held; ...]": an
    earlier "unheld X" means X must stay held);
    none fire for D_800A38AE in a kind-0x11 move; the other record's input
    there is handed off (D_800A376E) and installs the move on both);
  * the first 0x4000 entry is the paired move fight_UpdatePairedMoves
    starts by distance (its extra bytes are distance data; the label lists
    its gates): both records install it from the starter's file,
    D_800A38AE becomes the starter, the other runs it with
    borrow_other_motion and unk_86 = unk_84. The input loop also tests every
    0x4000 entry as an ordinary entry (with 0x8000, its bytes as the mask);
    those edges are labelled. Modes 2 / 5 never run fight_UpdatePairedMoves,
    so there neither paired starts nor kind-0x11 hand-offs happen;
  * move ended: the tc[0] 0x2000 end transition for a record that is not
    D_800A38AE, else the header fallback; a kind-0xA move also lands on its
    fallback (no borrow);
  * event-table edges only where fighter_Update takes them: 0xD (turn) from
    kinds 0x15/0x19/0x1A/0x13/0x30/0x31 with R1 and a direction held; 0x17 /
    0x18 on landing from kind 0xA. Not walked: landing with unk_72 set or
    after a 0x17/0x18 move, and pending_move_event codes set elsewhere;
  * the walk of record i follows the other record only while it runs the
    paired script (a node in record i's file; when both records share a
    motion slot it follows the other record everywhere); targets are listed for the
    nodes of either record, one shortest path per move, file and record;
    edges whose target does not read as a move script are dropped and listed;
  * the window: entries are read while move_frame <= last_command_frame
    (move_frame advances move_frame_step / 0x1000 per frame); in a move with
    unk_09 & 0x80 the chosen move is queued and starts after
    last_command_frame.
Button names are the physical buttons (fighter_BuildCpuOrMappedPadState and pad_Update in
src/main/87A0.c): game bits 0 L2 (never held for a human pad: pad_Update's
camera_UpdatePadOffset clears it in held before pressed / released / unheld
are derived, 9F9C.c:702, so for a record that is not cpu_controlled its
held / pressed / released entries are left out and an unheld one always
fires, ending the loop; bit 4 is cleared in all four words by
fighter_BuildCpuOrMappedPadState, so no entry on it fires),
1 R2, 2 L1, 3 R1, 5/6/7 the attack
buttons of the key config (D_80106A50.frec20, a nibble per player; in mode
D_800A38DC 6 the record that is not D_800A38A0 uses D_800A3912..14), 8
SELECT, 11 START, 12..15 UP RIGHT DOWN LEFT; for record 1 outside mode 6 the
directions are mirrored (game UP/RIGHT/DOWN/LEFT = pad DOWN/LEFT/UP/RIGHT).
L3 / R3 fold into bits 3 / 5 per word, after pressed / released are
computed per button (either button pressed / released fires; unheld fires
unless both are held). D_800A38BA's swap (fighter_Update) is not
read or applied: names assume it is 0. pressed SELECT is cleared in modes
2 / 3 / 5 and in mode 0 with D_800A385C. A cpu_controlled record's input
comes from fighter_BuildCpuPad: the button names do not apply to it.

In kinds 0x15/0x19/0x1A/0x13/0x30/0x31 the 0xD turn (held R1/L3 and a
direction) is tested before the loop and is listed as an earlier entry.

Static graph only: whether a branch fires at run time still depends on the
frame window, distance (paired), unk_26C and the other fighter's state, and
on timing: an input in the same game update as the previous edge's input
(e.g. CIRCLE 1 frame after R2, before the R2 move is installed) is not seen
by the new move.
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
PAIRED_STARTER = 0x800A38AE  # D_800A38AE (s16)

# motion_InitBuffers (9F9C.c): common file at 0x80120000, fighter files at
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
    {"at": "D_800A38AE", "len": 2},
    {"at": "0x80120000", "len": 0x801D8800 - 0x80120000},
]

TYPES = {0x00: "held", 0x10: "pressed", 0x20: "unheld", 0x30: "released"}
# what keeps an entry of each type from firing (pad_Update: pressed = held &
# ~old, unheld = ~held, released = ~held & old)
NOT_FIRING = {0x00: "{} not held", 0x10: "{} not newly pressed",
              0x20: "{} held", 0x30: "{} not just released"}
# bits 3 / 5 are each an OR of two pad buttons (R1|L3, CIRCLE|R3)
# (pressed / released are computed per pad button before the OR)
NOT_FIRING_FOLDED = {0x00: "neither of {} held",
                     0x10: "neither of {} newly pressed",
                     0x20: "both of {} held",
                     0x30: "neither of {} just released"}
# for a human pad: bit 0 is cleared in held before the other words are
# derived (camera_UpdatePadOffset masks 0xFFFEFFFE, 9F9C.c:702), so its
# unheld is always set; bit 4 is cleared in all four words (fighter_BuildCpuOrMappedPadState
# keeps none of 0xF0 but 0x20 / 0x40 / 0x80), so nothing on it fires
CLEARED_HELD_BIT = 0
CLEARED_BIT = 4
# kinds where fighter_Update tests event 0xD (turn) before the input loop
TURN_KINDS = (0x15, 0x19, 0x1A, 0x13, 0x30, 0x31)
NIBBLE_PAD = (("CIRCLE", 0x20), ("TRIANGLE", 0x10), ("CROSS", 0x40),
              ("SQUARE", 0x80))  # fighter_BuildCpuOrMappedPadState: nibble bit k tests these


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
        self.cpu = m.s16(b + 0x06) != 0  # cpu_controlled
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
        if m.covered(PAIRED_STARTER, 2):
            self.ae = m.s16(PAIRED_STARTER)
        else:
            self.ae = 0
            self.notes.append("D_800A38AE not read: assumed 0 (BSS; only "
                              "fight_UpdatePairedMoves writes it)")
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
            shift = 0 if mode == 6 else 4 * idx  # fighter_BuildCpuOrMappedPadState's temp
            nib = [(m.u8(KEY_CONFIG + i) >> shift) & 0xF for i in range(3)]
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

    def resolve(self, mid, ex, borrow=False, reset=False):
        """motion_ResolveMoveData for executing record `ex` ->
        (addr, file slot or None for common, note)"""
        m, r = self.m, self.recs[ex]
        buf, f00 = r.buf, self.sections(r.slot)[0]
        u86 = r.u84 if reset else r.u86  # paired start: other's unk_86 = unk_84
        if 0x7FF5 <= mid <= 0x7FFF:
            i = m.u16(buf + 0x66 + ((mid - 0x7FF5) * 3 + u86) * 2)
            return f00 + 2 * i, r.slot, f"f66[{mid - 0x7FF5}][{u86}]"
        f14 = m.s16(buf + 0x14)
        if mid in (0x7FF0, 0x7FF1, 0x7FF2, 0x7FF4):
            k = r.u84 if mid == 0x7FF0 else \
                (u86 + 1) % f14 if mid == 0x7FF1 else u86
            return f00 + 2 * m.u16(buf + 0x4E + 2 * k), r.slot, \
                f"variant f4E[{k}] (" + ("unk_84" if mid == 0x7FF0 else
                                         "unk_86 + 1" if mid == 0x7FF1 else
                                         "unk_86") + ")"
        if mid == 0x7FF3:
            t = (u86 + 1) % f14
            return f00 + 2 * m.u16(buf + 0x54 + (u86 * 3 + t) * 2), \
                r.slot, f"variant f54[{u86}][{t}]"
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
        """a reads as a move script: RAM that was read, first_move_frame <=
        last_move_frame, a command list that ends, a
        motion entry and metadata inside the read RAM, move_kind < 0x40
        (a heuristic: every kind seen in the walks so far is below 0x33)."""
        m = self.m
        if not (RAM0 <= a < RAM0 + RAMSZ - 0x400) or not m.covered(a, 12):
            return False
        h = self.header(a)
        if h["first"] > h["last"]:
            return False
        try:
            self.commands(a)
            _, s04, s08 = self.sections(slot)
            ent = s04 + m.u16(a + 4) * 4
            meta = s08 + m.u16(ent + 2)
            return m.covered(ent, 4) and m.covered(meta, 3) \
                and m.u8(meta) < 0x40
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
            cond.append("other fighter in kind 6 or at/after its unk_AB frame")
        elif bit == 0xA:
            bit = 5
            cond.append("needs unk_26C")
        s = f"{TYPES[f & 0x30]} {self.buttons[ex].get(bit, f'bit{bit}')}"
        if bit in (3, 5) and f & 0x30 == 0x20:
            # the game bit is an OR of two pad bits (fighter_BuildCpuOrMappedPadState)
            cond.append("fires unless both buttons are held")
        elif bit in (3, 5) and f & 0x30 in (0x10, 0x30):
            cond.append("fires when either button is "
                        + ("newly pressed" if f & 0x30 == 0x10 else "released"))
        if f & 0x8000:
            cond.append(f"char mask {extra:#x}")
        if f & 0x1000:
            cond.append("borrow other motion")
        return s + (f" [{'; '.join(cond)}]" if cond else "")

    def edges(self, node):
        """-> [(node, label)] for node (addr, file slot, executing record,
        D_800A38AE, unk_86 reset to unk_84 by a paired start)"""
        a, _, ex, ae, reset = node
        r, m = self.recs[ex], self.m
        h = self.header(a)
        kind = self.move_kind(a, node[1])
        out = []

        def target(mid, borrow=False):
            na, ns, note = self.resolve(mid, ex, borrow, reset)
            return (na, ns, note) if self.valid(na, ns) else None

        def add(mid, lab, borrow=False):
            t = target(mid, borrow)
            if t is None:
                self.dropped.append((node, lab, mid))
                return
            na, ns, note = t
            out.append(((na, ns, ex, ae, reset), f"{lab} -> id {mid:#06x}"
                        + (f" ({note})" if note else "")))

        paired_seen = stop = False
        earlier = []  # what the earlier live entries need to not fire
        if kind in TURN_KINDS:
            earlier.append("not (held R1/L3 + a direction with a turn: "
                           "event 0xD runs before the loop)")
        covered = set()  # (bit, input type) an earlier live entry takes
        # an entry of type t on bit b never fires once an unconditional
        # entry of a type implying t on b came first (the loop breaks)
        implied_by = {0x00: (0x00,), 0x10: (0x00, 0x10), 0x20: (0x20,),
                      0x30: (0x20, 0x30)}
        for f, mid, extra in self.commands(a):
            if f & 0x4000 and not paired_seen:
                paired_seen = True  # motion_FindPairedMoveEntry: the first
                b = extra.to_bytes(4, "little")
                lab = (f"paired move (fight_UpdatePairedMoves, not input): "
                       f"distance {b[0] * 20}..{b[1] * 20}, height gap < 100, "
                       f"other in kind 0x15/0x2C/0xE/0x19, func_8001FB34 "
                       f"(not modes 2/3/5, nor mode 0 with D_800A385C; the "
                       f"other's unk_0C not 0xD/0x1C; unk_0A 0xE needs a "
                       f"carried kind-0xA object"
                       + ("; needs unk_26C" if b[3] & 0x80 else "") + "); "
                       "tested only on the frame the move starts, with no "
                       "pending move event on either record")
                t = target(mid)
                if t is None:
                    self.dropped.append((node, lab, mid))
                else:
                    na, ns, _ = t
                    # both install it from the starter's file; D_800A38AE =
                    # ex; the other gets borrow_other_motion, unk_86 = unk_84
                    out.append(((na, ns, ex, ex, reset),
                                f"{lab} -> id {mid:#06x}"))
                    out.append(((na, ns, 1 - ex, ex, True),
                                f"{lab}, run by the other record -> id "
                                f"{mid:#06x}"))
            # the input loop (9F9C.c:4489-4552) also tests 0x4000 entries
            if stop:
                continue  # an always-firing entry came first
            if f & 0x2000:
                continue  # var_8 = 0: never fires on input
            if kind == 0x11 and ex == ae:
                continue  # var_8 = 0 for D_800A38AE in a kind-0x11 move
            if f & 0x8000 and not (extra >> r.unk_0A) & 1:
                continue  # mask (a paired entry's bytes) excludes the record
            bit, typ = f & 0xF, f & 0x30
            key_bit = {9: 3, 0xA: 5}.get(bit, bit)
            if any((key_bit, t) in covered for t in implied_by[typ]):
                continue  # an earlier entry takes this input first
            if not r.cpu and (key_bit == CLEARED_BIT or (
                    key_bit == CLEARED_HELD_BIT and typ != 0x20)):
                continue  # never fires for a human pad
            lab = self.label(f, extra, ex)
            if not r.cpu and key_bit == CLEARED_HELD_BIT:
                lab += " [always fires: L2 is never held for a human pad]"
                stop = True
            if earlier:
                # first match: every earlier live entry must not fire
                lab += " [and none earlier fires: " + "; ".join(earlier) + "]"
            name = self.buttons[ex].get(key_bit, f"bit{key_bit}")
            if bit in (9, 0xA):
                earlier.append(f"not ({TYPES[typ]} {name} with its condition)")
            else:
                covered.add((key_bit, typ))
                earlier.append((NOT_FIRING_FOLDED if key_bit in (3, 5)
                                else NOT_FIRING)[typ].format(name))
            if f & 0x4000:
                lab += " (paired entry tested as an input)"
            if kind == 0x11:
                # D_800A376E hand-off: fight_UpdatePairedMoves installs the
                # move on D_800A38AE and on the other (borrow if 0x1000)
                t = target(mid, bool(f & 0x1000))
                if t is None:
                    self.dropped.append((node, lab, mid))
                    continue
                na, ns, note = t
                lab += f" -> id {mid:#06x}" + (f" ({note})" if note else "")
                out.append(((na, ns, ex, ae, reset),
                            lab + " (paired hand-off, both records)"))
                # D_800A38AE's unk_86 is not reset (only the other's, 2720)
                out.append(((na, ns, ae, ae, False),
                            lab + " (paired hand-off, run by D_800A38AE)"))
                continue
            add(mid, lab, bool(f & 0x1000))
        tc0 = m.u16(a + 10)
        if tc0 & 0x2000 and ex != ae:
            add(m.u16(a + 12), "end transition (move ended, tc[0] & 0x2000)",
                bool(tc0 & 0x1000))
        else:
            add(h["fallback"], "fallback (move ended)",
                bool(h["unk_09"] & 0x20))
        if kind == 0xA:
            add(h["fallback"], "landing after a fall of 0xFA0 or less "
                "(fallback, no borrow)")
        tbl, _, _ = self.resolve(h["event_table"], ex, False, reset)
        if m.covered(tbl, 0x32):
            if kind in (0x15, 0x19, 0x1A, 0x13, 0x30, 0x31):
                add(m.u16(tbl + 2 * 0xD), "held R1/L3 + a direction (event "
                    "0xD, if fighter_ProbeInputDirectedSurfaceClearanceAndSurfaceBelow finds a turn)")
            if kind == 0xA:
                for code in (0x17, 0x18):
                    add(m.u16(tbl + 2 * code), f"landing after a fall of more "
                        f"than 0xFA0 (event {code:#x}, unk_94 "
                        f"{'set' if code == 0x17 else 'clear'})")
        return out

    def walk(self, idx):
        r = self.recs[idx]
        roots = [((r.cur, r.slot, idx, self.ae, False), "cur_move")]
        for k in range(3):
            a = self.sections(r.slot)[0] + 2 * self.m.u16(r.buf + 0x4E + 2 * k)
            roots.append(((a, r.slot, idx, self.ae, False), f"root f4E[{k}]"))
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
        a, s, ex, ae, reset = node
        h = self.header(a)
        return (f"{a:#010x} kind {self.move_kind(a, s):#04x} "
                f"({'common' if s is None else f'file slot {s}'}, run by "
                f"record {ex}, D_800A38AE {ae}"
                f"{', unk_86 = unk_84' if reset else ''}; frames "
                f"{h['first']}..{h['last']}, input to {h['last_cmd']}"
                f"{', queued' if h['unk_09'] & 0x80 else ''})")

def modifications(path: str) -> list[str]:
    """The capture's stub and poke records, one line each. gdbtrace writes
    them after a phase's value reads; an "entry" stub precedes every read."""
    out = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            e = json.loads(line)
            if e.get("ev") not in ("stub", "poke"):
                continue
            when = e.get("when")
            out.append(f"{e['ev']} {e.get('at')} " + (
                "at the EXE entry: every read is from a modified game"
                if when == "entry" else f"when phase {when} starts, after its "
                "value reads: later phases read a modified game"))
    return out


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
    for m in modifications(args.capture):
        print(f"note: modified run: {m}")
    for n in fight.notes:
        print(f"note: {n}")
    print(f"mode D_800A38DC {fight.mode}, D_800A38AE {fight.ae}")
    kinds = {int(k, 0) for k in args.kinds.split(",") if k}
    recs = (0, 1) if args.record is None else (args.record,)
    if args.move and args.record is None:
        recs = (0,)
    for idx in recs:
        r = fight.recs[idx]
        print(f"record {idx}: unk_0A {r.unk_0A:#x} unk_0C {r.unk_0C:#x} "
              f"unk_0E {r.unk_0E} slot {r.slot} unk_26C {r.unk_26C} "
              f"move_frame_step {r.step:#x} (base {r.base_step:#x}); "
              f"cur_move {fight.describe((r.cur, r.slot, idx, fight.ae, False))}")
        print("  buttons: " + ", ".join(
            f"bit {b} {fight.buttons[idx][b]}" for b in (5, 6, 7, 12, 13, 14, 15)))
        if args.move:
            addr, _, file = args.move.partition(":")
            node = (int(addr, 0), None if file == "common"
                    else int(file) if file else r.slot, idx, fight.ae, False)
            print(f"  move {fight.describe(node)}")
            fight.dropped = []
            for nn, lab in fight.edges(node):
                print(f"  {lab}\n      => {fight.describe(nn)}")
            for _, lab, mid in fight.dropped:
                print(f"  {lab} -> id {mid:#06x}\n      => not a move "
                      "script (dropped)")
            continue
        fight.dropped = []
        seen = fight.walk(idx)
        if fight.dropped:
            print(f"  {len(fight.dropped)} edges dropped (target does not read "
                  "as a move script): " + "; ".join(sorted({
                      f"from {n[0]:#x} (record {n[2]}) {lab} -> id {mid:#06x}"
                      for n, lab, mid in fight.dropped})))
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
