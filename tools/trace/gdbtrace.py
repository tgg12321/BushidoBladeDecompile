#!/usr/bin/env python3
"""gdbtrace: scripted runtime tracing of the running game over the GDB remote protocol.

Collects runtime evidence (who calls a function, with which arguments, what a
struct holds when it is used, who writes a global) for naming placeholder
members and data. Tools only: nothing here is a build input.

Backends: any PS1 emulator with a GDB stub.
  * PCSX-Redux (the tested backend): breakpoints (Z0) and read/write
    watchpoints (Z2/Z3). `run --launch pcsx-redux --emulator <pcsx-redux.exe>`
    starts it in portable mode (settings in --portable-dir, never the
    owner's) with the GDB server and debugger on. Windows nightlies:
    https://github.com/grumpycoders/pcsx-redux#where
    `--headless` runs it with no window (-no-ui; pcsx-redux.main started
    directly, no console) on an instance slot of tools/trace/pool.py: its
    own GDB port and profile dir, under a machine-wide cap, so many runs
    can go at once from any worktree (`python tools/trace/pool.py status`).
  * DuckStation: execution breakpoints (Z0) only. `--launch duckstation`
    sets [Debug] EnableGDBServer in its settings.ini only until the stub
    listens. On the owner's machine (0.1.11295) its socket layer fails
    (`WSAStartup() failed` in duckstation.log), so the stub never opens.

Subcommands
  run     connect (or launch an emulator), arm the spec, record events as JSONL
  report  summarize a trace into an evidence note (markdown) naming lanes cite
  compare stub-and-observe: diff a stubbed run's screenshots against a
          baseline run of the same script (same tools, build, BIOS, disc)

Spec (JSON):
  {
    "duration_s": 60,                # wall-clock budget for the run
    "max_events": 5000,              # stop after this many recorded events
    "breakpoints": [                 # execution breakpoints (function entry)
      {"at": "softbody_SetObjectSnap", "max_hits": 200,
       "dump": [{"base": "a0", "off": 0, "len": 64},     # struct at a0
                {"base": "D_800A32B4", "len": 4}]}        # a global
    ],
    "watch": [                       # data watchpoints (backend must support)
      {"at": "D_800A32B4", "len": 4, "kind": "write", "max_hits": 100}
    ],
    "polls": [                       # pause, read, resume on a period
      {"at": "D_800A32B4", "len": 16, "every_s": 0.5}
    ],
    "values": [                      # value watch (needs a script): read
      {"at": "D_800A38DC", "len": 8},  # at every phase start after frame
      {"at": "D_800A3910", "len": 32,  # 0 and at the end, while the
       "ptr_off": 0}                   # script holds the emulator paused;
    ],                               # ptr_off: read the u32 at `at`, then
                                     # len (default 4) bytes at that
                                     # pointer + ptr_off; "phases": [names]
                                     # reads only at those phase starts
                                     # (not at the end): RAM dumps for
                                     # tools/trace/movegraph.py
    "hitmap": {"tus": ["main/51268"],     # per-mode hit map (needs a script):
               "exclude": ["main/psxsdk/*"],  # Z0 on every function of the
               "per_phase": 1},               # TUs (fnmatch on the TU id),
                                     # credited per_phase times per phase,
                                     # then off until the next phase.
                                     # ("per_phase" also works on one
                                     # breakpoint instead of max_hits.)
    "stubs": [{"at": "func_80051234",  # stub-and-observe (needs a script):
               "phase": "fight",      # patched to `jr ra` when this phase
               "ret": 0}],            # starts (no phase: at the EXE entry);
                                     # ret sets v0 (none: v0 unchanged)
    "pokes": [{"at": "D_80101EC8+0xE",  # set-and-observe (needs a script):
               "hex": "0600",         # these bytes written once when the
               "phase": "fight"}],    # phase starts (after its value reads)
    "script": {...}                  # PCSX-Redux scripted run: frame-timed
  }                                  # inputs, phases, screenshots (runkit.py)
`at`/`base` take a symbol from the link map, a 0x address, `sym+0xNN`, or (for
`base` only) a register name (a0..a3, sp, gp, s0..s7, v0, v1, ra).

Symbols come from the GNU ld map of the matching build (default
<repo>/build/bb2.map; run `make` or `verify-oracle --rebuild` first). Only
addresses from a build whose SHA1 equals the oracle describe the original game.

Examples
  python tools/trace/gdbtrace.py run --spec tmp/spec.json --out tmp/t.jsonl \\
      --launch pcsx-redux --emulator tmp/pcsx-redux/pcsx-redux.exe \\
      --bios scph1001.bin --cue "Bushido Blade 2 (USA).cue" --boot-wait 20
  python tools/trace/gdbtrace.py report tmp/t.jsonl --out tmp/evidence.md

Reading a report: call sites are ra-8, i.e. the `jal`/`jalr` that entered the
function; a function reached by a tail-call `j` is credited to its caller's
caller. Watch hits name the load/store at the stop pc (pc-4 too when the stub reports
the address); a store in a branch delay slot may be missed and then shows as an
`unmatched-stop` event.
A trace covers one scenario, never everything a function can do.

Reproducible runs (owner ruling Q124): with a "script" block the run starts
at power-on, runs to the script's end_frame, and every hit carries the
phase it fell in, and a canary breakpoint on the EXE entry (hit once per
boot, never reported as a hit) proves Z0 fires on game code. Each run writes <out>.manifest.json next to the capture
(see runkit.py): the run's outcome, SHA1 of every input taken before
launch, and of every output after it. Only a scripted PCSX-Redux run can
meet Q124 (manifest `q124_complete`). Cite a capture by its manifest.
"""

from __future__ import annotations

import argparse
import bisect
import collections
import configparser
import hashlib
import json
import os
import re
import shutil
import socket
import subprocess
import sys
import time

import pool
import runkit

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# MIPS gdb 'g' order: r0..r31, sr, lo, hi, bad, cause, pc (then fp regs).
GPR_NAMES = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
             "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
             "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
             "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]
REG_INDEX = {n: i for i, n in enumerate(GPR_NAMES)}
REG_INDEX.update({"s8": 30, "sr": 32, "lo": 33, "hi": 34, "bad": 35,
                  "cause": 36, "pc": 37})

RAM_LO, RAM_HI = 0x80000000, 0x80200000
RESET_VECTOR = 0xBFC00000
EXE_LOAD = 0x80010000  # PS-X EXE load address (header: asm/header.s)
MAX_SYM_OFF = 0x4000  # larger offsets print raw: no symbol spans that far
DUMP_SHOWN = 0x1000  # value reads longer than this are dumps: report a SHA1
DEBUG = bool(os.environ.get("GDBTRACE_DEBUG"))  # log every packet to stderr


# ----------------------------------------------------------------- symbols

class Symbols:
    """Address <-> name for the matching build.

    Sources, all merged: the GNU ld map's symbol lines (`0xADDR  name`) and
    linker assignments (`0xADDR  name = ...`, e.g. pinned names), plus the
    ELF symbol table next to it (`bb2.elf`), which alone has `static`
    functions/objects and symbol sizes. With a size, an address past the
    end of a symbol prints raw instead of being credited to it."""

    LINE = re.compile(r"^\s+0x([0-9a-fA-F]{8,16})\s+([A-Za-z_.$][\w.$]*)"
                      r"\s*(?:=.*)?$")

    def __init__(self, path: str | None, elf: str | None = None):
        self.by_name: dict[str, int] = {}
        self.size: dict[str, int] = {}
        pairs = []

        def add(addr, name, size=0):
            addr &= 0xFFFFFFFF
            if not RAM_LO <= addr < RAM_HI:
                return
            # INCLUDE_ASM bodies also emit a `<name>.NON_MATCHING` alias at
            # the same address; keep the plain name.
            name = name.removesuffix(".NON_MATCHING")
            if name.startswith("."):  # .L jump labels, section markers
                return
            if size:
                self.size[name] = max(size, self.size.get(name, 0))
            if name in self.by_name:
                return
            self.by_name[name] = addr
            pairs.append((addr, name))

        if path and os.path.exists(path):
            with open(path, encoding="utf-8", errors="replace") as f:
                for line in f:
                    m = self.LINE.match(line)
                    if m:
                        add(int(m.group(1), 16), m.group(2))
        if elf is None and path:
            elf = os.path.join(os.path.dirname(path), "bb2.elf")
        if elf and os.path.exists(elf):
            for addr, name, size in elf_symbols(elf):
                add(addr, name, size)
        pairs.sort()
        self.addrs = [a for a, _ in pairs]
        self.names = [n for _, n in pairs]
        self.sized = sorted((a, a + self.size[n], n) for a, n in pairs
                            if n in self.size)
        self.sized_starts = [a for a, _, _ in self.sized]
        self.max_size = max((e - a for a, e, _ in self.sized), default=0)
        self.path = os.path.abspath(path) if self.addrs else None

    def covering(self, addr: int):
        """Innermost sized symbol containing addr: (start, name) or None."""
        i = bisect.bisect_right(self.sized_starts, addr) - 1
        while i >= 0 and self.sized[i][0] > addr - self.max_size - 1:
            start, end, name = self.sized[i]
            if start <= addr < end:
                return start, name
            i -= 1
        return None

    def addr(self, text: str) -> int:
        text = text.strip()
        m = re.fullmatch(r"([^+]+?)\s*\+\s*(0x[0-9a-fA-F]+|\d+)", text)
        if m:
            return self.addr(m.group(1)) + int(m.group(2), 0)
        if re.fullmatch(r"0x[0-9a-fA-F]+", text):
            return int(text, 16)
        if text in self.by_name:
            return self.by_name[text]
        m = re.fullmatch(r"(?:func|D|jtbl)_([0-9A-Fa-f]{8})", text)
        if m:
            return int(m.group(1), 16)
        raise SystemExit(f"gdbtrace: unknown symbol {text!r} (map: {self.path})")

    def name(self, addr: int) -> str:
        if 0xA0000000 <= addr < 0xA0200000:  # KSEG1 alias (small ints stay raw)
            addr = RAM_LO | (addr & 0x1FFFFFFF)
        if not RAM_LO <= addr < RAM_HI:
            return f"0x{addr:08X}"
        i = bisect.bisect_right(self.addrs, addr) - 1
        if i < 0:
            return f"0x{addr:08X}"
        # A sized symbol that contains addr wins over unsized labels inside
        # it (jump-table / data labels placed on instructions).
        cov = self.covering(addr)
        if cov:
            off = addr - cov[0]
            return cov[1] if off == 0 else f"{cov[1]}+0x{off:X}"
        off = addr - self.addrs[i]
        size = self.size.get(self.names[i])
        if size and off >= size:  # inside no known object: say so
            return f"0x{addr:08X}"
        if off > MAX_SYM_OFF:  # past the last object: heap, stack, overlay
            return f"0x{addr:08X}"
        return self.names[i] if off == 0 else f"{self.names[i]}+0x{off:X}"


MAP_TU_TEXT = re.compile(r"^\s*\.text\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)"
                         r"\s+build/src/(\S+)\.o\s*$")


def hitmap_targets(map_path: str, elf: str | None, hm: dict):
    """[(name, addr)] for a spec's "hitmap" block: every function (ELF FUNC
    symbol, statics included) whose address lies in the .text of a TU
    matching one of hm["tus"] (fnmatch on the TU id, e.g. main/51268 or
    main/*), minus hm["exclude"] patterns."""
    import fnmatch
    ranges = []
    with open(map_path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = MAP_TU_TEXT.match(line)
            if not m or not int(m.group(2), 16):
                continue
            tu = m.group(3)
            if any(fnmatch.fnmatchcase(tu, p) for p in hm.get("tus", [])) \
                    and not any(fnmatch.fnmatchcase(tu, p)
                                for p in hm.get("exclude", [])):
                lo = int(m.group(1), 16)
                ranges.append((lo, lo + int(m.group(2), 16), tu))
    for p in hm.get("tus", []):
        if not any(fnmatch.fnmatchcase(tu, p) for _, _, tu in ranges):
            raise SystemExit(f"gdbtrace: hitmap: no linked TU matches {p!r}")
    elf = elf or os.path.join(os.path.dirname(os.path.abspath(map_path)),
                              "bb2.elf")
    funcs = {}
    for addr, name, _size in elf_symbols(elf, kinds=(2,)):
        addr &= 0xFFFFFFFF
        name = name.removesuffix(".NON_MATCHING")
        if any(lo <= addr < hi for lo, hi, _ in ranges):
            funcs.setdefault(addr, name)
    out = sorted((a, n) for a, n in funcs.items())
    return [(n, a) for a, n in out]


def elf_symbols(path: str, kinds=(1, 2)):
    """(value, name, size) of FUNC/OBJECT symbols (STT 2/1; `kinds`
    narrows), locals included, from a 32-bit little-endian ELF (stdlib
    only)."""
    import struct
    with open(path, "rb") as f:
        data = f.read()
    if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 1:
        return []
    shoff, = struct.unpack_from("<I", data, 0x20)
    shentsize, shnum = struct.unpack_from("<HH", data, 0x2E)
    secs = [struct.unpack_from("<10I", data, shoff + i * shentsize)
            for i in range(shnum)]
    out = []
    for sh in secs:
        if sh[1] != 2:  # SHT_SYMTAB
            continue
        strtab = secs[sh[6]]
        s_off = strtab[4]
        for k in range(sh[5] // 16):
            st_name, value, size, info, _, _ = struct.unpack_from(
                "<IIIBBH", data, sh[4] + 16 * k)
            if info & 0xF not in kinds or not st_name:  # OBJECT, FUNC
                continue
            end = data.index(b"\0", s_off + st_name)
            out.append((value, data[s_off + st_name:end].decode("latin-1"),
                        size))
    return out


# ------------------------------------------------------------ gdb protocol

class GdbError(Exception):
    pass


class Gdb:
    """Minimal GDB remote serial protocol client (ack mode, all-stop)."""

    def __init__(self, host: str, port: int, timeout: float = 10.0):
        self.sock = socket.create_connection((host, port), timeout=timeout)
        self.buf = b""
        self.console: list[str] = []
        self.late_stops: list[str] = []  # stop replies cmd() read past
        self.features = self.cmd("qSupported:swbreak+;hwbreak+")

    def close(self):
        try:
            self.sock.close()
        except OSError:
            pass

    def _send(self, payload: str):
        data = payload.encode("latin-1")
        frame = b"$" + data + b"#" + f"{sum(data) & 0xFF:02x}".encode()
        if DEBUG:
            print(f"-> {payload[:80]}", file=sys.stderr)
        for _ in range(3):
            self.sock.sendall(frame)
            ack = self._read_byte(10.0)
            if ack == b"+":
                return
            if ack != b"-":
                # Some stubs run without acks; keep the byte for the reader.
                self.buf = ack + self.buf
                return
        raise GdbError(f"stub kept NAKing {payload[:20]!r}")

    def _read_byte(self, timeout: float | None) -> bytes:
        if not self.buf:
            self.sock.settimeout(timeout)
            try:
                chunk = self.sock.recv(65536)
            finally:
                self.sock.settimeout(None)
            if not chunk:
                raise GdbError("stub closed the connection")
            self.buf += chunk
        b, self.buf = self.buf[:1], self.buf[1:]
        return b

    def recv(self, timeout: float | None = None) -> str | None:
        """One packet's payload, or None on timeout (stub still running)."""
        try:
            while True:
                b = self._read_byte(timeout)
                if b == b"$":
                    break
        except socket.timeout:
            return None
        body = b""
        while True:
            b = self._read_byte(10.0)
            if b == b"#":
                break
            body += b
        self._read_byte(10.0), self._read_byte(10.0)  # checksum: trust TCP
        self.sock.sendall(b"+")
        if DEBUG:
            print(f"<- {body[:80].decode('latin-1')}", file=sys.stderr)
        return body.decode("latin-1")

    def cmd(self, payload: str, timeout: float = 10.0) -> str:
        self._send(payload)
        while True:
            reply = self.recv(timeout)
            if reply is None:
                raise GdbError(f"no reply to {payload[:20]!r}")
            # A late stop reply (breakpoint racing an interrupt) can arrive
            # ahead of the answer; S/T are not hex, so it cannot be data.
            if payload != "?" and re.match(r"[STWX][0-9a-fA-F]{2}", reply):
                self.late_stops.append(reply)
                continue
            if self._console(reply):
                continue
            return reply

    def _console(self, reply: str) -> bool:
        """Stash an `O<hex>` console-output packet (the game's TTY)."""
        if re.fullmatch(r"O(?:[0-9a-fA-F]{2})+", reply):
            self.console.append(bytes.fromhex(reply[1:]).decode("latin-1"))
            return True
        return False

    def wait_stop(self, timeout: float) -> str | None:
        """Next stop reply within timeout, skipping console output."""
        deadline = time.time() + timeout
        while True:
            reply = self.recv(max(0.01, deadline - time.time()))
            if reply is None or not self._console(reply):
                return reply

    def interrupt(self, timeout: float = 5.0) -> str:
        if DEBUG:
            print("-> ^C", file=sys.stderr)
        self.sock.sendall(b"\x03")
        reply = self.wait_stop(timeout)
        if reply is None:
            raise GdbError("target did not stop on interrupt")
        return reply

    def regs(self) -> list[int]:
        raw = self.cmd("g")
        if not re.fullmatch(r"[0-9a-fA-Fx]{%d,}" % (38 * 8), raw):
            raise GdbError(f"bad register reply {raw[:24]!r}")
        out = []
        for i in range(0, min(len(raw), 38 * 8), 8):
            word = raw[i:i + 8]
            out.append(int.from_bytes(bytes.fromhex(word), "little")
                       if "x" not in word else 0)
        return out

    def mem(self, addr: int, length: int) -> bytes:
        out = b""
        while length > 0:
            n = min(length, 0x400)
            reply = self.cmd(f"m{addr:x},{n:x}")
            # An error is `Exx`; data may itself start with hex `E`.
            if len(reply) != 2 * n:
                raise GdbError(f"read 0x{addr:08X},{n}: {reply[:16]}")
            try:
                out += bytes.fromhex(reply)
            except ValueError:
                raise GdbError(f"read 0x{addr:08X},{n}: not hex "
                               f"{reply[:16]!r}") from None
            addr += n
            length -= n
        return out

    def set_bp(self, kind: int, addr: int, length: int = 4) -> bool:
        return self.cmd(f"Z{kind},{addr:x},{length:x}") == "OK"

    def clear_bp(self, kind: int, addr: int, length: int = 4) -> bool:
        return self.cmd(f"z{kind},{addr:x},{length:x}") == "OK"

    def write(self, addr: int, data: bytes) -> bool:
        return self.cmd(f"M{addr:x},{len(data):x}:{data.hex()}") == "OK"

    def cont(self):
        self._send("c")


# ---------------------------------------------------------------- runner

WATCH_KIND = {"write": 2, "read": 3, "access": 4}

# MIPS I load/store opcodes -> (width, is_store)
MEM_OPS = {0x20: (1, False), 0x21: (2, False), 0x22: (4, False),
           0x23: (4, False), 0x24: (1, False), 0x25: (2, False),
           0x26: (4, False), 0x28: (1, True), 0x29: (2, True),
           0x2A: (4, True), 0x2B: (4, True), 0x2E: (4, True),
           0x32: (4, False), 0x3A: (4, True)}  # lwc2 / swc2 (GTE)


def phys(addr: int) -> int:
    return addr & 0x1FFFFFFF


UNALIGNED_OPS = {0x22, 0x26, 0x2A, 0x2E}  # lwl lwr swl swr


def decode_access(gdb: Gdb, regs: list[int], pc: int, also_prev: bool):
    """Candidate load/stores a watchpoint stop was raised for. Some stubs
    (PCSX-Redux) send a bare T05 with no `watch:` address, so decode the
    instruction at pc (PCSX-Redux stops on the access itself). pc-4 is also
    tried only when the stub reported the address (`watch:`), which then
    decides between them. Yields (insn_pc, addr, width, is_store, value)."""
    for at in ((pc, pc - 4) if also_prev else (pc,)):
        try:
            insn = int.from_bytes(gdb.mem(at, 4), "little")
        except GdbError:
            continue
        op = insn >> 26
        if op not in MEM_OPS:
            continue
        width, store = MEM_OPS[op]
        base, rt = (insn >> 21) & 31, (insn >> 16) & 31
        imm = insn & 0xFFFF
        imm -= 0x10000 if imm & 0x8000 else 0
        addr = (regs[base] + imm) & 0xFFFFFFFF
        value = None  # swc2 stores a GTE register, not GPR rt
        if op in UNALIGNED_OPS:
            addr &= ~3  # touches part of the aligned word; value is partial
        elif store and op != 0x3A:
            value = regs[rt] & ((1 << (8 * width)) - 1)
        yield at, addr, width, store, value


def compare_memory(gdb: Gdb, addr: int, want: bytes | None) -> str:
    """"matches" when memory at addr holds exactly `want` (file bytes read
    before the emulator started)."""
    if not want:
        return "nothing to compare"
    n = len(want)
    try:
        got = gdb.mem(addr, n)
    except GdbError as ex:
        return f"not read: {ex}"
    if got == want:
        return "matches"
    first = next(i for i in range(n) if got[i] != want[i])
    return f"differs at 0x{addr + first:08X}"


def diff_ranges(gdb: Gdb, addr: int, want: bytes | None, cap: int = 64):
    """["0xSTART+LEN", ...] where memory at addr differs from `want`
    (at most `cap` ranges; "..." marks more)."""
    if not want:
        return []
    got = gdb.mem(addr, len(want))
    out, i, n = [], 0, len(want)
    while i < n:
        if got[i] == want[i]:
            i += 1
            continue
        j = i
        while j < n and got[j] != want[j]:
            j += 1
        if len(out) == cap:
            out.append("...")
            break
        out.append(f"0x{addr + i:08X}+{j - i}")
        i = j
    return out


JR_RA = 0x03E00008


def stub_words(st: dict) -> bytes:
    """`jr ra` + delay slot: `addiu v0, zero, ret` or (no ret) a nop."""
    ret = st.get("ret")
    if ret is None:
        slot = 0
    elif isinstance(ret, int) and -0x8000 <= ret <= 0x7FFF:
        slot = 0x24020000 | (ret & 0xFFFF)
    else:
        raise SystemExit(f"gdbtrace: stub {st.get('at')}: ret must be an "
                         "integer in -32768..32767 (or absent)")
    return JR_RA.to_bytes(4, "little") + slot.to_bytes(4, "little")


def parse_pokes(spec: dict, syms, phase_names: set, first: str,
                stubs: list[dict] = ()) -> list[dict]:
    """Validate spec["pokes"]: each writes `hex` at `at` when `phase` starts.
    A poke needs a script phase other than the first (at power-on the game
    has not set anything up yet), RAM or scratchpad bytes, and no overlap
    with another poke or a stub's two words."""
    pokes = []
    for pk in spec.get("pokes", []):
        if not spec.get("script"):
            raise SystemExit("gdbtrace: pokes need a script")
        at = pk.get("at")
        when = pk.get("phase")
        if when not in phase_names or when == first:
            raise SystemExit(f"gdbtrace: poke {at}: phase must be a script "
                             f"phase other than the first, not {when!r}")
        try:
            data = bytes.fromhex(pk.get("hex") or "")
        except (TypeError, ValueError):
            data = b""
        if not data:
            raise SystemExit(f"gdbtrace: poke {at}: hex must be one or more "
                             "bytes in hex (memory order)")
        addr = syms.addr(at)
        end = addr + len(data)
        if not ((RAM_LO <= addr and end <= RAM_HI)
                or (0x1F800000 <= addr and end <= 0x1F800400)):
            raise SystemExit(f"gdbtrace: poke {at}: 0x{addr:08X}+{len(data)} "
                             "is not RAM or scratchpad")
        for o in pokes:
            if addr < o["addr"] + len(o["data"]) and o["addr"] < end:
                raise SystemExit(f"gdbtrace: poke {at} overlaps poke "
                                 f"{o['spec']['at']}")
        for st in stubs:
            if addr < st["addr"] + 8 and st["addr"] < end:
                raise SystemExit(f"gdbtrace: poke {at} overlaps stub "
                                 f"{st['spec']['at']}")
        pokes.append({"spec": pk, "addr": addr, "when": when, "data": data})
    return pokes


def apply_pokes(gdb: Gdb, pokes: list[dict], when: str, regs: list[int],
                frame, emit) -> None:
    """Write every poke due when phase `when` starts; read back to prove it."""
    pc = regs[37] if len(regs) > 37 else None
    for pk in pokes:
        if pk["when"] != when or "applied" in pk:
            continue
        a, data = pk["addr"], pk["data"]
        pk["orig"] = gdb.mem(a, len(data)).hex()
        ok = gdb.write(a, data) and gdb.mem(a, len(data)) == data
        pk["applied"] = {"frame": frame, "pc": pc, "ok": ok}
        emit({"ev": "poke", "at": pk["spec"]["at"], "addr": a, "when": when,
              "frame": frame, "pc": pc, "orig": pk["orig"],
              "hex": data.hex(), "ok": ok})
        if not ok:
            raise SystemExit(f"gdbtrace: poke write at 0x{a:08X} refused")


def apply_stubs(gdb: Gdb, stubs: list[dict], when: str, regs: list[int],
                frame, emit, armed: dict, suspended: dict) -> None:
    """Patch every stub due at `when` ("entry" or a phase name) to return.
    A stop between the two patched words would run half a stub: flagged.
    Two breakpoints prove the stub works: one on the entry, credited once
    per phase (the stub was called), one on the word after the stub, which
    a working stub never reaches (a leak)."""
    pc = regs[37] if len(regs) > 37 else None
    for st in stubs:
        if st["when"] != when or "applied" in st:
            continue
        a = st["addr"]
        st["orig"] = gdb.mem(a, 8).hex()
        ok = gdb.write(a, st["words"]) and gdb.mem(a, 8) == st["words"]
        st["applied"] = {"frame": frame, "pc": pc, "ok": ok,
                         "mid_stub": pc is not None and a < pc < a + 8}
        emit({"ev": "stub", "at": st["spec"]["at"], "addr": a, "when": when,
              "frame": frame, "pc": pc, "orig": st["orig"],
              "words": st["words"].hex(), "ok": ok})
        if not ok:
            raise SystemExit(f"gdbtrace: stub write at 0x{a:08X} refused")
        name = st["spec"]["at"]
        if (0, a) in suspended:  # its per-phase credit is used up
            suspended[(0, a)]["stub_call"] = True
        elif (0, a) in armed:
            armed[(0, a)]["stub_call"] = True  # credited as its own hit
        elif gdb.set_bp(0, a):
            armed[(0, a)] = {"spec": {"at": name}, "hits": 0, "kind": 0,
                             "addr": a, "len": 4, "per_phase": 1,
                             "phase_hits": 0, "stub_call": True}
        else:
            raise SystemExit(f"gdbtrace: stub refused Z0 at 0x{a:08X}")
        if (0, a + 8) in armed or (0, a + 8) in suspended \
                or not gdb.set_bp(0, a + 8):
            raise SystemExit(f"gdbtrace: no leak breakpoint at 0x{a + 8:08X}")
        armed[(0, a + 8)] = {"spec": {"at": f"{name}+0x8"}, "hits": 0,
                             "kind": 0, "addr": a + 8, "len": 4,
                             "per_phase": 1, "phase_hits": 0,
                             "stub_leak": True}


def resolve_dump(syms: Symbols, regs: list[int], d: dict) -> int:
    base = d.get("base", "0")
    if base in REG_INDEX:
        addr = regs[REG_INDEX[base]]
    else:
        addr = syms.addr(base)
    return (addr + int(d.get("off", 0))) & 0xFFFFFFFF


def snapshot(gdb: Gdb, syms: Symbols, regs: list[int], dumps: list[dict]):
    out = []
    for d in dumps:
        addr = resolve_dump(syms, regs, d)
        n = int(d.get("len", 4))
        if not RAM_LO <= addr < RAM_HI and not 0x1F800000 <= addr < 0x1F800400:
            out.append({"spec": d, "addr": addr, "error": "not RAM"})
            continue
        try:
            out.append({"spec": d, "addr": addr, "hex": gdb.mem(addr, n).hex()})
        except GdbError as e:
            out.append({"spec": d, "addr": addr, "error": str(e)})
    return out


def run(args) -> int:
    syms = Symbols(args.map, args.elf)
    with open(args.spec, encoding="utf-8") as f:
        spec = json.load(f)

    args.lua = None
    args.spec_obj = spec
    script = spec.get("script")
    if script:
        if args.launch != "pcsx-redux":
            raise SystemExit("gdbtrace: a script needs --launch pcsx-redux")
        if args.boot_wait:
            raise SystemExit("gdbtrace: a script starts at power-on; drop "
                             "--boot-wait")
        runkit.check_script(script)
    if args.bios and (not os.path.isfile(args.bios)
                      or os.path.getsize(args.bios) != 0x80000):
        raise SystemExit(f"gdbtrace: --bios {args.bios} is not a 512 KiB "
                         "(0x80000-byte) PS1 BIOS ROM")
    runkit.clean_outputs(args.out, script)
    if script:
        args.lua = runkit.write_lua(script, args.out)
    if args.lua:
        kind = "power-on: emulator held at the reset vector until armed"
    elif args.launch:
        kind = f"attached after {args.boot_wait:g} s boot-wait"
    else:
        kind = "attached to a running emulator"
    start = {"kind": kind}
    start.update(prepare_memcards(args))
    args.slot = acquire_slot(args)
    try:
        return run_launched(args, syms, spec, start)
    finally:
        if args.slot is not None:
            args.slot.release()


def acquire_slot(args):
    """--headless: a pool slot (GDB port + profile dir), else None."""
    if not args.headless:
        return None
    if args.launch != "pcsx-redux":
        raise SystemExit("gdbtrace: --headless needs --launch pcsx-redux")
    if args.keep_emulator:
        raise SystemExit("gdbtrace: --headless releases its instance at the "
                         "end; drop --keep-emulator")
    slot = pool.acquire(f"gdbtrace run {os.path.abspath(args.out)}",
                        args.slot_wait)
    if args.portable_dir:  # settings to start from (runs without -safe)
        src = os.path.join(args.portable_dir, "pcsx.json")
        if os.path.isfile(src):
            shutil.copyfile(src, os.path.join(slot.profile, "pcsx.json"))
    args.port = slot.port
    args.portable_dir = slot.profile
    print(f"gdbtrace: headless instance slot {slot.n}, port {slot.port}")
    return slot


def run_launched(args, syms, spec, start) -> int:
    # Every input is hashed before the emulator can touch it, and the bytes
    # compared in memory later are read now.
    inputs = runkit.inputs_record(args, start, REPO)
    if args.slot is not None:
        inputs["instance"] = args.slot.describe()
    args.expect = runkit.expected_memory(args)
    inputs["build"]["map_matches_elf"] = map_matches_elf(syms, args)
    if not inputs["build"]["map_matches_elf"]:
        inputs["q124_complete"] = False
        inputs.setdefault("q124_missing", []).append(
            "a link map from the same link as the ELF")
    if not inputs["build"]["oracle_match"]:
        print("gdbtrace: WARNING build/bb2.exe is not the oracle; addresses "
              "may not describe the original game", file=sys.stderr)

    args.result = {}
    outcome = "error"
    emu = None
    rc = 1
    try:
        if args.launch == "duckstation":
            emu = launch_duckstation(args)
        elif args.launch == "pcsx-redux":
            emu = launch_redux(args)
        gdb = connect(args.host, args.port, args.connect_timeout)
        rc = trace(gdb, syms, spec, args)
        outcome = "ok"
    except BaseException as e:
        outcome = f"error: {type(e).__name__}: {e}"
        raise
    finally:
        if emu is not None:
            proc, restore = emu
            if args.keep_emulator:
                print("gdbtrace: emulator left running; DuckStation may save "
                      "EnableGDBServer = true when you close it")
            else:
                # Not None: the emulator exited (or crashed) before the end.
                args.result["emulator_exit_before_stop"] = proc.poll()
                stop_emulator(proc, getattr(proc, "stub_pid", None))
                restore()
            log = getattr(proc, "log", None)
            if log is not None:
                log.close()
                if outcome != "ok":
                    print(f"gdbtrace: emulator log {log.name}, last lines:\n"
                          + tail(log.name, 20), file=sys.stderr)
        # Errors here must not hide the run's own exception.
        try:
            man = runkit.finish_manifest(
                args, inputs, getattr(emu[0], "cmd", None) if emu else None,
                {"outcome": outcome, **args.result})
            print(f"gdbtrace: manifest -> {man}")
        except Exception as e:  # noqa: BLE001
            print(f"gdbtrace: manifest not written: {type(e).__name__}: {e}",
                  file=sys.stderr)
            rc = 1
    return rc


def map_matches_elf(syms: Symbols, args) -> bool:
    """The map (breakpoint addresses) and the ELF (checked against the
    EXE) come from one link: every global the ELF and the map both name
    has one address in both."""
    elf = args.elf or os.path.join(os.path.dirname(os.path.abspath(
        args.map)), "bb2.elf")
    try:
        in_elf = {n: a & 0xFFFFFFFF for a, n, _ in elf_symbols(elf)}
        with open(args.map, encoding="utf-8", errors="replace") as f:
            in_map = {}
            for line in f:
                m = Symbols.LINE.match(line)
                if m:
                    in_map.setdefault(m.group(2), int(m.group(1), 16))
    except OSError:
        return False
    common = [n for n in in_map if n in in_elf]
    return len(common) > 100 and all(in_map[n] == in_elf[n] for n in common)


def prepare_memcards(args) -> dict:
    """Memory cards are part of the start state. Each PCSX-Redux run gets
    its own pair next to the capture: a copy of --memcardN, or a freshly
    formatted blank card. Returns the start-state record (hashed later,
    before launch)."""
    rec = {}
    if args.launch != "pcsx-redux":
        return rec
    for n in (1, 2):
        dst = runkit.companion(args.out, f".mc{n}.mcd")
        src = getattr(args, f"memcard{n}")
        if src and os.path.normcase(os.path.abspath(src)) == \
                os.path.normcase(os.path.abspath(dst)):
            raise SystemExit(f"gdbtrace: --memcard{n} is this run's own card "
                             "copy; copy it elsewhere first")
        if os.path.exists(dst):
            os.remove(dst)
        if src:
            shutil.copyfile(src, dst)
            rec[f"memcard{n}"] = {"from": os.path.abspath(src)}
        else:
            runkit.write_blank_card(dst)
            rec[f"memcard{n}"] = {"from": "blank card (runkit.write_blank_card)"}
        rec[f"memcard{n}"]["card"] = dst
    return rec


def connect(host: str, port: int, wait: float) -> Gdb:
    deadline = time.time() + wait
    while True:
        try:
            return Gdb(host, port)
        except OSError:
            if time.time() > deadline:
                raise SystemExit(f"gdbtrace: no GDB stub on {host}:{port}")
            time.sleep(0.5)


def exe_entry(map_path: str) -> int:
    """Initial pc from the PS-X EXE header of the build next to the map."""
    exe = os.path.join(os.path.dirname(os.path.abspath(map_path)), "bb2.exe")
    head = runkit.read_head(exe, 4, 0x10)
    if not head or len(head) != 4:
        raise SystemExit(f"gdbtrace: cannot read the entry point of {exe}")
    return int.from_bytes(head, "little")


def wait_running(gdb: Gdb, until: float, out: str | None, stall: float):
    """wait_stop until `until`; with a script, give up when the emulated
    frame count (the Lua's .beat file) has not moved for `stall` s while
    the target is meant to run: a hung or wedged instance (seen with
    PCSX-Redux at a script pause) fails the run instead of idling to the
    deadline."""
    if out is None or stall <= 0:
        return gdb.wait_stop(max(0.05, until - time.time()))
    last, since = runkit.read_beat(out), time.time()
    while True:
        reply = gdb.wait_stop(max(0.05, min(until, time.time() + 10)
                                  - time.time()))
        if reply is not None or time.time() >= until:
            return reply
        beat = runkit.read_beat(out)
        if beat != last:
            last, since = beat, time.time()
        elif time.time() - since > stall:
            raise GdbError(f"stalled: no emulated frame for {stall:.0f} s "
                           f"while running (last frame {last})")


def pause_dup(now, at_stop) -> tuple[bool, bool]:
    """(duplicate, blind). Duplicate: the emulator has not run since the
    script stop: the latest pause record is the script stop's own, or a
    later one at the same CPU cycle. Blind: no cycle count to tell, so the
    caller's registers + Lua state test decided alone."""
    if now is not None and at_stop is not None and now[0] == at_stop[0]:
        return True, False
    if now is None or at_stop is None or now[2] is None or at_stop[2] is None:
        return True, True
    return now[2] == at_stop[2], False


def trace(gdb: Gdb, syms: Symbols, spec: dict, args) -> int:
    out = open(args.out, "w", encoding="utf-8", newline="\n")

    def emit(rec):
        out.write(json.dumps(rec, separators=(",", ":")) + "\n")
        out.flush()

    emit({"ev": "start", "time": time.time(), "map": syms.path,
          "features": gdb.features, "spec": spec})

    # Bring the target to a known stopped state before arming. A stub that
    # is already stopped may not answer the interrupt; that is fine.
    gdb.cmd("?")
    try:
        gdb.interrupt(2.0)
    except GdbError:
        pass

    armed = {}  # (kind, addr) -> entry
    for bp in spec.get("breakpoints", []):
        addr = syms.addr(bp["at"])
        if not gdb.set_bp(0, addr):
            raise SystemExit(f"gdbtrace: stub refused Z0 at 0x{addr:08X}")
        armed[(0, addr)] = {"spec": bp, "hits": 0, "kind": 0, "addr": addr,
                            "len": 4}
        if "per_phase" in bp:
            if not spec.get("script") or "max_hits" in bp:
                raise SystemExit(f"gdbtrace: {bp['at']}: per_phase needs a "
                                 "script and no max_hits")
            armed[(0, addr)].update(per_phase=int(bp["per_phase"]),
                                    phase_hits=0)
    hm = spec.get("hitmap")
    if hm:
        # Per-mode hit map: a breakpoint on every function of the chosen
        # TUs, credited at most per_phase times in each phase, then cleared
        # until the next phase starts (so every phase is fully covered).
        if not spec.get("script"):
            raise SystemExit("gdbtrace: a hitmap needs a script (phases)")
        per = int(hm.get("per_phase", 1))
        targets = hitmap_targets(args.map, args.elf, hm)
        emit({"ev": "hitmap", "per_phase": per,
              "targets": [[n, a] for n, a in targets]})
        for name, addr in targets:
            if (0, addr) in armed:
                raise SystemExit(f"gdbtrace: {name} is both a breakpoint and "
                                 "in the hitmap; drop one")
            if not gdb.set_bp(0, addr):
                raise SystemExit(f"gdbtrace: stub refused Z0 at 0x{addr:08X}")
            armed[(0, addr)] = {"spec": {"at": name}, "hits": 0, "kind": 0,
                                "addr": addr, "len": 4, "per_phase": per,
                                "phase_hits": 0, "hitmap": True}
    suspended = {}  # per-phase entries done for this phase
    for w in spec.get("watch", []):
        kind = WATCH_KIND[w.get("kind", "write")]
        addr, n = syms.addr(w["at"]), int(w.get("len", 4))
        if not gdb.set_bp(kind, addr, n):
            raise SystemExit(f"gdbtrace: stub has no Z{kind} watchpoints "
                             "(DuckStation has none; use PCSX-Redux)")
        armed[(kind, addr)] = {"spec": w, "hits": 0, "kind": kind,
                               "addr": addr, "len": n}

    # Stub-and-observe: patch a function to return at the EXE entry (before
    # any game code runs) or when a phase starts.
    stubs = []
    phase_names = {ph["name"] for ph in (spec.get("script") or {}).get(
        "phases", [])}
    for st in spec.get("stubs", []):
        if not spec.get("script"):
            raise SystemExit("gdbtrace: stubs need a script")
        when = st.get("phase", "entry")
        if when != "entry" and when not in phase_names:
            raise SystemExit(f"gdbtrace: stub {st['at']}: no phase {when!r}")
        if when == runkit.first_phase(spec["script"]):
            raise SystemExit(f"gdbtrace: stub {st['at']}: {when!r} is the "
                             "first phase; omit phase to stub at the EXE "
                             "entry")
        addr = syms.addr(st["at"])
        if any(x["addr"] == addr for x in stubs):
            raise SystemExit(f"gdbtrace: {st['at']} is stubbed twice")
        stubs.append({"spec": st, "addr": addr, "when": when,
                      "words": stub_words(st)})
    # Set-and-observe: write chosen bytes once when a phase starts.
    pokes = parse_pokes(spec, syms, phase_names,
                        runkit.first_phase(spec["script"])
                        if spec.get("script") else "start", stubs)

    if spec.get("script"):
        # Canary: the EXE entry runs exactly once per boot; its hit proves
        # Z0 breakpoints fire on game code in this emulator build.
        entry = exe_entry(args.map)
        if (0, entry) in armed:
            armed[(0, entry)]["canary"] = True
        elif gdb.set_bp(0, entry):
            armed[(0, entry)] = {"spec": {"at": f"0x{entry:08X}"}, "hits": 0,
                                 "kind": 0, "addr": entry, "len": 4,
                                 "canary_only": True}
        args.result["canary_at"] = entry

    polls = [{"spec": p, "addr": syms.addr(p["at"]), "len": int(p.get("len", 4)),
              "every": float(p.get("every_s", 1.0)), "next": 0.0}
             for p in spec.get("polls", [])]
    values = [{"spec": v, "addr": syms.addr(v["at"]),
               "len": int(v.get("len", 4))} for v in spec.get("values", [])]
    if values and not spec.get("script"):
        raise SystemExit("gdbtrace: values need a script (they are read at "
                         "its stops)")
    for v in values:
        off = v["spec"].get("ptr_off", 0)
        if type(off) is not int or off < 0:
            raise SystemExit(f"gdbtrace: value {v['spec']['at']}: ptr_off "
                             "must be an integer >= 0")
        if v["len"] < 1:
            raise SystemExit(f"gdbtrace: value {v['spec']['at']}: len must "
                             "be >= 1")
        only = v["spec"].get("phases")
        if only is not None:
            names = {p["name"] for p in
                     (spec.get("script") or {}).get("phases", [])}
            if (not isinstance(only, list) or not only
                    or not all(isinstance(p, str) for p in only)
                    or not set(only) <= names):
                raise SystemExit(f"gdbtrace: value {v['spec']['at']}: phases "
                                 f"must be a list of script phase names")

    def read_values(frame, phase_name, end, pause):
        """Value watch: every "values" entry, read while the script holds
        the emulator paused at a phase start or at its end. pause_frame is
        the frame the Lua's latest pause recorded (== frame unless that
        pause stop was missed)."""
        for i, v in enumerate(values):
            sp = v["spec"]
            only = sp.get("phases")
            if only is not None and (end or phase_name not in only):
                continue
            rec = {"ev": "value", "i": i, "frame": frame, "phase": phase_name,
                   "end": end, "at": sp["at"], "addr": v["addr"],
                   "pause_frame": pause[1] if pause else None}
            try:
                addr = v["addr"]
                if "ptr_off" in sp:
                    ptr = int.from_bytes(gdb.mem(addr, 4), "little")
                    rec["ptr"] = ptr
                    addr = (ptr + sp["ptr_off"]) & 0xFFFFFFFF
                    rec["read_at"] = addr
                    if not (RAM_LO <= addr < RAM_HI
                            or 0x1F800000 <= addr < 0x1F800400):
                        rec["error"] = "pointer is not RAM or scratchpad"
                        emit(rec)
                        continue
                rec["hex"] = gdb.mem(addr, v["len"]).hex()
            except GdbError as ex:
                rec["error"] = str(ex)
            emit(rec)

    script = spec.get("script")
    duration = float(spec.get("duration_s",
                              3600 if script else args.duration))
    max_events = int(spec.get("max_events", 5000))
    if hm and "max_events" not in spec:
        # Room for every hit-map credit in every phase, plus the rest.
        max_events = len(targets) * per * (
            len(spec["script"].get("phases", [])) + 1) + 5000
    t0 = time.time()
    events = 0
    totals = {}
    state = runkit.read_state(args.out) if script else None
    phase = state[2] if state else None
    if script:
        # Frames emulated before the breakpoints were armed are not covered.
        # Frame 0 is claimed only when the CPU is seen at the reset vector
        # and no pause has been recorded past frame 0.
        armed_pc = gdb.regs()[37]
        paused = runkit.read_pause_frame(args.out)
        loaded = state == (1, 0, runkit.first_phase(script), False)
        args.result["script_loaded_at_arming"] = loaded
        if armed_pc == RESET_VECTOR and paused in (None, 0) and loaded:
            armed_frame = 0
        else:
            armed_frame = paused  # None: unknown
        args.result.update({"armed_frame": armed_frame,
                            "armed_pc": armed_pc})
        # Redux falls back to OpenBIOS when it cannot load --bios: compare
        # the whole ROM it is running with the file read before launch.
        args.result["bios_in_memory"] = compare_memory(
            gdb, RESET_VECTOR, args.expect.get("bios"))
        emit({"ev": "phase", "t": 0.0, "frame": state[1] if state else 0,
              "phase": phase, "armed_frame": armed_frame})
    after_script_stop = None  # regs at the last script stop
    script_pause = None  # runkit.read_pause() at the last script stop
    ambiguous = 0

    def entry_for_stop(regs, reply, interrupted):
        pc = regs[37] if len(regs) > 37 else 0
        if (0, pc) in armed:
            return armed[(0, pc)], None
        watches = [e for (k, _), e in armed.items() if k]
        if not watches or interrupted:
            # Our own ^C can land on any load/store; never a watch hit.
            return None, None
        m = re.search(r"(?:watch|rwatch|awatch):([0-9a-fA-F]+)", reply)
        reported = int(m.group(1), 16) if m else None
        cands = list(decode_access(gdb, regs, pc, reported is not None))
        # The access is the candidate whose own address overlaps a watch of
        # its kind (and the stub's reported address, when it sends one).
        for acc in cands:
            a, n = acc[1], acc[2]
            if reported is not None and not (
                    phys(a) <= phys(reported) < phys(a) + n):
                continue
            for e in watches:
                if e["kind"] != 4 and acc[3] != (e["kind"] == 2):
                    continue  # a load under a write watch, or vice versa
                if (phys(a) < phys(e["addr"]) + e["len"]
                        and phys(e["addr"]) < phys(a) + n):
                    return e, acc
        return None, (cands[0] if cands else None)

    try:
        while events < max_events and time.time() - t0 < duration:
            now = time.time()
            for p in polls:
                if now >= p["next"]:
                    p["next"] = now + p["every"]
                    rec = {"ev": "poll", "t": round(now - t0, 3),
                           "at": p["spec"]["at"], "addr": p["addr"],
                           "hex": gdb.mem(p["addr"], p["len"]).hex()}
                    if script:
                        rec["phase"] = phase
                    emit(rec)
                    events += 1
            gdb.cont()
            until = min([p["next"] for p in polls] + [t0 + duration])
            reply = wait_running(gdb, until, args.out if script else None,
                                 args.stall)
            if gdb.console:
                emit({"ev": "tty", "t": round(time.time() - t0, 3),
                      "text": "".join(gdb.console)})
                gdb.console.clear()
            interrupted = reply is None
            extra = []
            gdb.late_stops.clear()
            if interrupted:
                # Poll or deadline due: stop the target. A breakpoint or
                # watch may have raced the interrupt: then the stub answers
                # twice. Read the second answer now rather than as the next
                # stop after `c`: wait briefly, then `g` (below), which the
                # stub answers only after any stop reply it sent first.
                reply = gdb.interrupt()
                late = gdb.wait_stop(0.2)
                if late is not None:
                    extra.append(late)
            ends = [s for s in [reply] + extra if s.startswith(("W", "X"))]
            if not ends:
                try:
                    regs = gdb.regs()
                except GdbError:
                    # The target exited while we asked: its W/X was read
                    # past as a late stop.
                    if not any(s.startswith(("W", "X"))
                               for s in gdb.late_stops):
                        raise
                extra += gdb.late_stops
                ends = [s for s in extra if s.startswith(("W", "X"))]
            if ends:
                emit({"ev": "exit", "reply": ends[0]})
                break
            # Two stop replies to one ^C: one was a real trap, so this stop
            # is decoded like any trap. (Redux answers a plain ^C with T05,
            # like a trap, so a lone reply cannot be told apart.)
            raced = interrupted and any(s.startswith(("S", "T"))
                                        for s in extra)
            new = runkit.read_state(args.out) if script else None
            pause = runkit.read_pause(args.out) if script else None
            if (after_script_stop is not None and regs == after_script_stop
                    and new == state):
                dup, blind = pause_dup(pause, script_pause)
                if blind:
                    # Without the cycle count a busy-loop re-hit looks
                    # the same as a duplicate: the run cannot meet Q124.
                    args.result["dedupe_without_cycle"] = \
                        args.result.get("dedupe_without_cycle", 0) + 1
                if dup:
                    # A breakpoint and the Lua pause on the same
                    # instruction can arrive as two stops; nothing ran in
                    # between (same registers, no new Lua state, same CPU
                    # cycle): skip it.
                    after_script_stop = None
                    continue
            after_script_stop = None
            script_stop = bool(new and new != state)
            if script_stop:
                # The Lua paused the emulator at a phase change or the end.
                # A breakpoint on the same instruction is still a hit, in
                # the phase that starts here.
                state, phase = new, new[2]
                after_script_stop = regs
                script_pause = pause
                if not new[3]:
                    # A new phase: per-phase entries start counting again.
                    for key, ent in list(suspended.items()):
                        if not gdb.set_bp(ent["kind"], ent["addr"], ent["len"]):
                            raise GdbError(f"re-arm 0x{ent['addr']:08X} "
                                           "refused")
                        armed[key] = ent
                    suspended.clear()
                    for ent in armed.values():
                        if "per_phase" in ent:
                            ent["phase_hits"] = 0
                emit({"ev": "phase", "t": round(time.time() - t0, 3),
                      "frame": new[1], "phase": phase, "end": new[3],
                      "pause": list(pause) if pause else None})
                # Read before this phase's stubs go in (stubs from earlier
                # phases already changed the game: the report says so).
                read_values(new[1], phase, new[3], pause)
                if not new[3]:
                    apply_stubs(gdb, stubs, phase, regs, new[1], emit,
                                armed, suspended)
                    apply_pokes(gdb, pokes, phase, regs, new[1], emit)
            # A Lua pause can land on any load/store, like our own ^C.
            e, acc = entry_for_stop(regs, reply,
                                    (interrupted and not raced) or script_stop)
            if e is not None and (e.get("canary_only") or e.get("canary")) \
                    and "canary_hit" not in args.result:
                # At the EXE entry no game code has run yet: the disc's EXE
                # must be in RAM as built. (Later the game patches its own
                # code, e.g. the sites listed at D_80015820.)
                args.result["exe_in_memory"] = compare_memory(
                    gdb, EXE_LOAD, args.expect.get("exe"))
                apply_stubs(gdb, stubs, "entry", regs,
                            state[1] if state else None, emit, armed,
                            suspended)
            if e is not None and e.get("canary_only"):
                # The canary proves Z0 fires on game code; it is no hit.
                args.result["canary_hit"] = {"pc": regs[37], "phase": phase}
                emit({"ev": "canary", "t": round(time.time() - t0, 3),
                      "pc": regs[37], "phase": phase})
                gdb.clear_bp(0, e["addr"], 4)
                del armed[(0, e["addr"])]
                if script_stop and state[3]:
                    break
                continue
            if e is not None and e.get("canary") and \
                    "canary_hit" not in args.result:
                args.result["canary_hit"] = {"pc": regs[37], "phase": phase}
                emit({"ev": "canary", "t": round(time.time() - t0, 3),
                      "pc": regs[37], "phase": phase})
            if e is None:
                if (script_stop or interrupted) and any(k for k, _ in armed):
                    # A watched access on the very instruction the Lua
                    # pause or our ^C stopped on cannot be told from the
                    # pause: say so (the run then cannot meet Q124).
                    e2, acc2 = entry_for_stop(regs, reply, False)
                    if e2 is not None:
                        # Redux may raise the watch again on resume: let
                        # that stop through as the hit.
                        after_script_stop = None
                        ambiguous += 1
                        emit({"ev": "ambiguous-stop", "pc": regs[37],
                              "at": e2["spec"]["at"], "phase": phase,
                              "access": list(acc2) if acc2 else None})
                if script_stop and state[3]:
                    break
                if (not interrupted or raced) and not script_stop:
                    emit({"ev": "unmatched-stop", "pc": regs[37],
                          "ra": regs[31], "stop": reply,
                          "access": list(acc) if acc else None})
                continue
            e["hits"] += 1
            sp = e["spec"]
            totals[f"Z{e['kind']}:{sp['at']}"] = e["hits"]
            rec = {"ev": "hit", "t": round(time.time() - t0, 3),
                   "at": sp["at"], "kind": e["kind"], "addr": e["addr"],
                   "n": e["hits"], "pc": regs[37], "ra": regs[31],
                   "args": regs[4:8], "v0": regs[2], "sp": regs[29],
                   "stop": reply}
            if script:
                rec["phase"] = phase
            if e.get("hitmap"):
                rec["hm"] = 1
            if e.get("stub_call"):
                rec["stub_call"] = 1
            if e.get("stub_leak"):
                rec["stub_leak"] = 1
                args.result["stub_leaks"] = \
                    args.result.get("stub_leaks", 0) + 1
            if acc:
                rec["access"] = {"pc": acc[0], "addr": acc[1], "width": acc[2],
                                 "store": acc[3], "value": acc[4]}
            if sp.get("regs"):
                rec["regs"] = {r: regs[REG_INDEX[r]] for r in sp["regs"]}
            if sp.get("dump"):
                rec["dump"] = snapshot(gdb, syms, regs, sp["dump"])
            emit(rec)
            events += 1
            if script_stop and state[3]:
                break
            if "per_phase" in e:
                e["phase_hits"] += 1
                if e["phase_hits"] >= e["per_phase"]:
                    # Enough for this phase: off until the next one.
                    gdb.clear_bp(e["kind"], e["addr"], e["len"])
                    key = (e["kind"], e["addr"])
                    del armed[key]
                    suspended[key] = e
                continue
            if e["hits"] >= int(sp.get("max_hits", 1 << 30)):
                gdb.clear_bp(e["kind"], e["addr"], e["len"])
                del armed[(e["kind"], e["addr"])]
                # From here on this entry covers nothing: a hit map must
                # not read later phases as "not reached".
                emit({"ev": "disarmed", "t": round(time.time() - t0, 3),
                      "at": sp["at"], "kind": e["kind"], "phase": phase,
                      "after_hits": e["hits"]})
                if not armed and not polls and not script:
                    break
    finally:
        for (kind, addr), e in list(armed.items()):
            try:
                gdb.clear_bp(kind, addr, e["len"])
            except (GdbError, OSError):
                pass
        ended = bool(state and state[3])
        args.result.update({"script_ended": ended, "events": events,
                            "ambiguous_stops": ambiguous})
        recs = []
        for st in stubs:
            rec = {"at": st["spec"]["at"], "addr": st["addr"],
                   "when": st["when"], "ret": st["spec"].get("ret"),
                   "words": st["words"].hex(), "orig": st.get("orig"),
                   "applied": st.get("applied")}
            if "applied" in st:
                # The game may rewrite its own code: still a stub at the end?
                try:
                    rec["at_end"] = ("intact" if gdb.mem(st["addr"], 8)
                                     == st["words"] else "overwritten")
                except (GdbError, OSError) as ex:
                    rec["at_end"] = f"not read: {ex}"
            recs.append(rec)
        if recs:
            args.result["stubs"] = recs
            args.result["modified_game"] = (
                "stubbed functions: hits and screenshots after a stub went "
                "in are from a modified game; cite with `compare` only")
        precs = []
        for pk in pokes:
            rec = {"at": pk["spec"]["at"], "addr": pk["addr"],
                   "when": pk["when"], "hex": pk["data"].hex(),
                   "orig": pk.get("orig"), "applied": pk.get("applied")}
            if "applied" in pk:
                # Information only: the game may legitimately rewrite data.
                try:
                    rec["at_end"] = gdb.mem(pk["addr"], len(pk["data"])).hex()
                except (GdbError, OSError) as ex:
                    rec["at_end"] = f"not read: {ex}"
            precs.append(rec)
        if precs:
            args.result["pokes"] = precs
            args.result["modified_game"] = (
                ("stubbed functions and poked memory" if recs else
                 "poked memory")
                + ": hits and screenshots after a stub or poke went in are "
                "from a modified game; cite with `compare` only")
        if ended:
            # Information only (the gate is the check at the EXE entry):
            # which .rodata + .text bytes the run itself changed.
            try:
                args.result["exe_changed_by_run"] = diff_ranges(
                    gdb, EXE_LOAD, args.expect.get("exe"))
            except (GdbError, OSError) as ex:
                args.result["exe_changed_by_run"] = f"not read: {ex}"
        if not ended:  # a finished script stays paused: cards stay as-is
            try:
                gdb.cont()
            except (GdbError, OSError):
                pass
        emit({"ev": "end", "events": events, "phase": phase,
              "script_ended": ended,
              "elapsed": round(time.time() - t0, 3), "hits": totals})
        out.close()
        gdb.close()
    print(f"gdbtrace: {events} events -> {args.out}")
    return 0


# ------------------------------------------------------- emulator launch

def port_owner(port: int) -> int | None:
    """PID listening on a local TCP port (Windows netstat), if any."""
    if os.name != "nt":
        return None
    out = subprocess.run(["netstat", "-ano", "-p", "tcp"], capture_output=True,
                         text=True).stdout
    for line in out.splitlines():
        f = line.split()
        if len(f) >= 5 and f[3] == "LISTENING" and f[1].endswith(f":{port}"):
            return int(f[4])
    return None


def check_port_free(host: str, port: int):
    try:
        socket.create_connection((host, port), 1).close()
    except OSError:
        return
    raise SystemExit(f"gdbtrace: {host}:{port} already has a listener "
                     f"(pid {port_owner(port)}); stop it or pick --port")


def stop_emulator(proc, stub_pid: int | None):
    """PCSX-Redux's pcsx-redux.exe is a launcher for pcsx-redux.main, so
    killing the Popen child is not enough: kill the process that opened the
    stub port (recorded at launch, when the port was known to be ours),
    then the launcher's tree."""
    pids = [stub_pid, proc.pid]
    for pid in pids:
        if not pid:
            continue
        if os.name == "nt":
            subprocess.run(["taskkill", "/T", "/F", "/PID", str(pid)],
                           capture_output=True)
        else:
            try:
                os.kill(pid, 15)
            except OSError:
                pass
    try:
        proc.wait(30)
    except subprocess.TimeoutExpired:
        pass


def wait_for_stub(proc, host: str, port: int, wait: float, name: str):
    deadline = time.time() + wait
    while time.time() < deadline:
        try:
            socket.create_connection((host, port), 1).close()
            return
        except OSError:
            if proc.poll() is not None:
                raise SystemExit(f"gdbtrace: {name} exited at boot")
            time.sleep(0.5)
    raise SystemExit(f"gdbtrace: {name} GDB stub did not open "
                     f"{host}:{port} within {wait:.0f} s")


def launch_redux(args):
    """Start PCSX-Redux in portable mode with the GDB server on."""
    if not args.emulator:
        raise SystemExit("gdbtrace: --launch pcsx-redux needs --emulator")
    portable = os.path.abspath(args.portable_dir)
    os.makedirs(portable, exist_ok=True)
    check_port_free(args.host, args.port)
    cmd = [os.path.abspath(args.emulator), "-portable", portable,
           "-iso", os.path.abspath(args.cue),
           "-gdb", "-gdb-port", str(args.port), "-debugger", "-interpreter",
           "-fastboot", "-noupdate"]  # dynarec skips breakpoints
    if getattr(args, "lua", None):
        # -safe: default settings (+ these flags) instead of whatever
        # pcsx.json holds, so the version + command line fix the setup.
        # No -run: the CPU waits at the reset vector for the first `c`,
        # after the breakpoints are armed.
        cmd += ["-safe"]
    else:
        cmd += ["-run"]
    if getattr(args, "headless", False):
        cmd += ["-no-ui"]
    if args.bios:
        cmd += ["-bios", os.path.abspath(args.bios)]
    for n in (1, 2):
        cmd += [f"-memcard{n}",
                os.path.abspath(runkit.companion(args.out, f".mc{n}.mcd"))]
    if getattr(args, "lua", None):
        cmd += ["-dofile", os.path.abspath(args.lua)]
    # cwd: its crash reporter writes .sentry-native/ into the working dir.
    slot = getattr(args, "slot", None)
    if slot is None:
        proc = subprocess.Popen(cmd, cwd=portable)
        proc.log = None
    else:
        # No console window either: pcsx-redux.exe is a launcher whose child
        # opens a visible console, so start pcsx-redux.main (the emulator,
        # hashed in the manifest) itself; its output goes to the slot's log.
        main = os.path.join(os.path.dirname(cmd[0]), "pcsx-redux.main")
        if os.path.isfile(main):
            cmd[0] = main
        log = open(os.path.join(slot.dir, "emulator.log"), "w",
                   encoding="utf-8", newline="\n")
        try:
            proc = subprocess.Popen(
                cmd, cwd=portable, stdin=subprocess.DEVNULL, stdout=log,
                stderr=subprocess.STDOUT,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        except BaseException:
            log.close()
            raise
        proc.log = log
    proc.cmd = cmd
    try:
        if slot is not None:
            slot.record(emulator_pids=[proc.pid])
        wait_for_stub(proc, args.host, args.port, args.connect_timeout,
                      "PCSX-Redux")
        proc.stub_pid = port_owner(args.port)
        if slot is not None:
            slot.record(emulator_pids=[p for p in dict.fromkeys(
                (proc.pid, proc.stub_pid)) if p])
        if args.boot_wait:
            time.sleep(args.boot_wait)
    except BaseException:
        stop_emulator(proc, port_owner(args.port))
        if proc.log is not None:
            proc.log.close()
        raise
    return proc, lambda: None


def tail(path: str, n: int) -> str:
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            return "".join(f.readlines()[-n:])
    except OSError as e:
        return f"(unreadable: {e})"


DUCK_PACKAGES = os.path.join(
    os.environ.get("LOCALAPPDATA", ""), "Microsoft", "WinGet", "Packages")


def find_duckstation(explicit: str | None) -> str:
    if explicit:
        return explicit
    found = shutil.which("duckstation-qt-x64-ReleaseLTCG")
    if found:
        return found
    if os.path.isdir(DUCK_PACKAGES):
        for d in os.listdir(DUCK_PACKAGES):
            if d.startswith("Stenzek.DuckStation"):
                exe = os.path.join(DUCK_PACKAGES, d,
                                   "duckstation-qt-x64-ReleaseLTCG.exe")
                if os.path.exists(exe):
                    return exe
    raise SystemExit("gdbtrace: DuckStation not found; pass --emulator")


def set_ini_key(text: str, section: str, key: str, value: str) -> str:
    eol = "\r\n" if "\r\n" in text else "\n"
    sec = re.search(rf"(?m)^\[{re.escape(section)}\][^\n]*\n", text)
    if not sec:
        return text + f"{eol}[{section}]{eol}{key} = {value}{eol}"
    nxt = re.search(r"(?m)^\[", text[sec.end():])
    stop = sec.end() + nxt.start() if nxt else len(text)
    body = text[sec.end():stop]
    kv = re.compile(rf"(?m)^{re.escape(key)}\s*=[^\r\n]*")
    if kv.search(body):
        body = kv.sub(f"{key} = {value}", body, 1)
    else:
        body = f"{key} = {value}{eol}" + body
    return text[:sec.end()] + body + text[stop:]


def launch_duckstation(args):
    """Start DuckStation with its GDB stub on, without leaving the owner's
    settings changed: the stub flag is set in settings.ini only until the
    stub is listening (DuckStation reads it at boot), then the file is put
    back byte for byte."""
    exe = find_duckstation(args.emulator)
    check_port_free(args.host, args.port)
    ini = args.duckstation_ini or os.path.join(
        os.path.expanduser("~"), "Documents", "DuckStation", "settings.ini")
    with open(ini, "rb") as f:
        original = f.read()
    text = original.decode("utf-8")
    cfg = configparser.ConfigParser(strict=False, interpolation=None)
    cfg.optionxform = str
    cfg.read_string(text)
    orig_debug = dict(cfg["Debug"]) if cfg.has_section("Debug") else {}

    patched = set_ini_key(text, "Debug", "EnableGDBServer", "true")
    patched = set_ini_key(patched, "Debug", "GDBServerPort", str(args.port))

    def restore():
        # DuckStation may rewrite settings.ini from memory (stub on); put the
        # keys this launch touched back to their original values.
        with open(ini, "rb") as f:
            now = f.read().decode("utf-8")
        fixed = now
        defaults = {"EnableGDBServer": "false"}  # absent key = off
        for key in ("EnableGDBServer", "GDBServerPort"):
            value = orig_debug.get(key, defaults.get(key))
            if value is not None:
                fixed = set_ini_key(fixed, "Debug", key, value)
        if fixed != now:
            with open(ini, "wb") as f:
                f.write(fixed.encode("utf-8"))

    proc = None
    try:
        try:
            with open(ini, "wb") as f:
                f.write(patched.encode("utf-8"))
            proc = subprocess.Popen([exe, "-batch", "-fastboot", "--",
                                     os.path.abspath(args.cue)])
            wait_for_stub(proc, args.host, args.port, args.connect_timeout,
                          "DuckStation")
            proc.stub_pid = port_owner(args.port)
        finally:
            with open(ini, "wb") as f:
                f.write(original)
        if args.boot_wait:
            time.sleep(args.boot_wait)
    except BaseException:
        # Never leave a stub-enabled DuckStation behind: it could write
        # EnableGDBServer = true back when it exits.
        if proc is not None:
            stop_emulator(proc, port_owner(args.port))
            restore()
        raise
    return proc, restore


# ---------------------------------------------------------------- report

def u32s(hexstr: str):
    b = bytes.fromhex(hexstr)
    return [int.from_bytes(b[i:i + 4], "little") for i in range(0, len(b) - 3, 4)]


def describe_words(syms: Symbols, samples: list[list[int]], label):
    """Per-u32 word: constant / varying, value range, pointer targets.
    label(byte_offset) names the word's row."""
    lines = []
    width = min(len(s) for s in samples) if samples else 0
    for i in range(width):
        vals = [s[i] for s in samples]
        distinct = collections.Counter(vals)
        ptrs = [v for v in distinct if RAM_LO <= v < RAM_HI]
        if len(distinct) == 1:
            v = vals[0]
            kind = "const"
            shown = f"0x{v:08X}" + (f" (`{syms.name(v)}`)" if ptrs else "")
        else:
            kind = f"varies ({len(distinct)} values)"
            top = ", ".join(f"0x{v:08X}" for v, _ in distinct.most_common(4))
            shown = f"min 0x{min(vals):08X} max 0x{max(vals):08X}; top {top}"
            if ptrs and len(ptrs) == len(distinct):
                shown += "; all RAM pointers -> " + ", ".join(
                    f"`{n}`" for n in
                    sorted({syms.name(p).split('+')[0] for p in ptrs})[:4])
        lines.append(f"| {label(4 * i)} | {kind} | {shown} |")
    return lines


def describe_fields(syms: Symbols, addr: int, samples: list[bytes]):
    """A value read split at every map/ELF symbol inside it (not at u32
    boundaries, which would credit a sub-word symbol's change to the
    symbol the word starts at). Each field runs to the next symbol or the
    end of the read and is shown as a little-endian integer."""
    n = min(len(s) for s in samples)
    starts = {}
    i = bisect.bisect_left(syms.addrs, addr)
    while i < len(syms.addrs) and syms.addrs[i] < addr + n:
        starts.setdefault(syms.addrs[i], []).append(syms.names[i])
        i += 1
    if addr not in starts:
        starts[addr] = [syms.name(addr)]
    order = sorted(starts)
    lines = ["| field | span (bytes to the next symbol) | behaviour | values |",
             "|---|---|---|---|"]
    for k, a in enumerate(order):
        end = order[k + 1] if k + 1 < len(order) else addr + n
        vals = [int.from_bytes(s[a - addr:end - addr], "little")
                for s in samples]
        width = end - a
        digits = 2 * min(width, 4)
        distinct = collections.Counter(vals)
        fmt = lambda v: f"0x{v:0{digits}X}"
        if len(distinct) == 1:
            kind, shown = "const", fmt(vals[0])
        else:
            kind = f"varies ({len(distinct)} values)"
            shown = ", ".join(f"{fmt(v)} x{c}"
                              for v, c in distinct.most_common(4))
        label = " = ".join(f"`{x}`" for x in dict.fromkeys(starts[a]))
        lines.append(f"| {label} (0x{a:08X}) | {width} | {kind} | {shown} |")
    return lines


def hitmap_section(hitmap: dict, hm_hits, phases: list, args) -> list[str]:
    """Function x phase table. A cell is the hit count, capped: `N+` means
    the per-phase limit was reached (the breakpoint was then off until the
    next phase); `.` means armed through the whole phase and never hit;
    `?` means not hit while the phase was only partly covered (armed after
    it began, or the run stopped inside it): no claim either way."""
    per = hitmap["per_phase"]
    armed = next((p["armed_frame"] for p in phases if "armed_frame" in p),
                 None)
    names, full = [], {}
    for i, ph in enumerate(phases):
        if ph.get("end"):
            continue
        name = str(ph["phase"])
        # Fully covered: armed by its first frame, and a later phase event
        # (the next phase or the script's end) closes it. A name seen twice
        # (older captures) merges two spans: never a full-coverage claim.
        ok = (armed is not None and armed <= ph["frame"]
              and i + 1 < len(phases) and name not in full)
        if name in full:
            full[name] = False
            continue
        full[name] = ok
        names.append(name)
    w = [f"## Hit map: {len(hitmap['targets'])} functions, "
         f"{per} hit(s) per phase recorded", ""]
    if args.hitmap_tsv and os.path.exists(args.hitmap_tsv):
        os.remove(args.hitmap_tsv)  # never leave an older TSV looking current
    if not names:
        return w + ["No phase recorded: no claims.", ""]
    if not any(full.values()):
        w += ["No phase was fully covered: no claims (`?` everywhere).", ""]

    def cell(n, p):
        if not n:
            return "." if full[p] else "?"
        return f"{n}+" if n >= per else str(n)

    def row(a):
        return [cell(hm_hits[a].get(p, 0), p) for p in names]

    partial = [p for p in names if not full[p]]
    if partial:
        w.append("Partly covered phases (`?` = no claim): " +
                 ", ".join(partial))
        w.append("")
    hit = [(n, a) for n, a in hitmap["targets"] if hm_hits.get(a)]
    never = [n for n, a in hitmap["targets"] if not hm_hits.get(a)]
    scope = ("in any phase" if not partial else
             "in the fully covered phases (no claim for the partly "
             "covered ones)" if any(full.values()) else
             "(no fully covered phase: no claim)")
    w.append(f"{len(hit)} reached, {len(never)} not hit {scope}.")
    w.append("")
    w.append("| function | " + " | ".join(names) + " |")
    w.append("|---|" + "---|" * len(names))
    for n, a in hit:
        w.append(f"| `{n}` | " + " | ".join(row(a)) + " |")
    w.append("")
    if never and any(full.values()):
        w.append(f"Not hit {scope}: " + ", ".join(f"`{n}`" for n in never))
        w.append("")
    if args.hitmap_tsv:
        with open(args.hitmap_tsv, "w", encoding="utf-8", newline="\n") as f:
            f.write("function\taddr\t" + "\t".join(names) + "\n")
            for n, a in hitmap["targets"]:
                f.write(f"{n}\t0x{a:08X}\t" + "\t".join(row(a)) + "\n")
    return w


def report(args) -> int:
    syms = Symbols(args.map, args.elf)
    hits = collections.defaultdict(list)
    polls = collections.defaultdict(list)
    values = collections.defaultdict(list)
    phases = []
    odd = collections.Counter()
    disarmed = {}
    start = end = canary = hitmap = None
    stubs = []
    pokes = []
    stub_calls = collections.defaultdict(list)
    hm_hits = collections.defaultdict(collections.Counter)
    with open(args.trace, encoding="utf-8") as f:
        for line in f:
            rec = json.loads(line)
            if rec["ev"] == "hit" and rec.get("stub_call"):
                stub_calls[rec["addr"]].append(rec.get("phase"))
            if rec["ev"] == "hit" and rec.get("hm"):
                hm_hits[rec["addr"]][str(rec.get("phase"))] += 1
            elif rec["ev"] == "hit":
                hits[(rec["at"], rec["kind"])].append(rec)
            elif rec["ev"] == "hitmap":
                hitmap = rec
            elif rec["ev"] == "poll":
                polls[rec["at"]].append(rec)
            elif rec["ev"] == "value":
                values[rec["i"]].append(rec)
            elif rec["ev"] == "phase":
                phases.append(rec)
            elif rec["ev"] in ("unmatched-stop", "ambiguous-stop"):
                odd[rec["ev"]] += 1
            elif rec["ev"] == "disarmed":
                disarmed[(rec["at"], rec["kind"])] = rec
            elif rec["ev"] == "canary":
                canary = rec
            elif rec["ev"] == "stub":
                stubs.append(rec)
            elif rec["ev"] == "poke":
                pokes.append(rec)
            elif rec["ev"] == "start":
                start = rec
            elif rec["ev"] == "end":
                end = rec

    w = [f"# Runtime trace evidence: {os.path.basename(args.trace)}", ""]
    if start:
        when = time.strftime("%Y-%m-%d %H:%M", time.localtime(start["time"]))
        mp = start.get("map")
        w.append(f"- recorded {when}, map `{mp or '-'}`"
                 f", {end['events'] if end else '?'} events in "
                 f"{end['elapsed'] if end else '?'} s")
    if args.context:
        w.append(f"- scenario: {args.context}")
    man = runkit.companion(args.trace, ".manifest.json")
    if os.path.isfile(man):
        w.append(f"- manifest `{os.path.basename(man)}` "
                 f"(SHA1 {runkit.sha1_file(man)}), capture SHA1 "
                 f"{runkit.sha1_file(args.trace)}")
    armed = next((p["armed_frame"] for p in phases if "armed_frame" in p),
                 None)
    if phases and armed != 0:
        w.append(f"- WARNING: breakpoints were armed at frame "
                 f"{armed if armed is not None else 'unknown'}; earlier "
                 "frames are not covered")
    if phases:
        w.append(f"- canary breakpoint (EXE entry) "
                 + (f"hit at 0x{canary['pc']:08X} in phase {canary['phase']}"
                    if canary else "not recorded in this capture"))
    for st in stubs:
        calls = stub_calls.get(st["addr"], [])
        leaks = hits.get((f"{st['at']}+0x8", 0), [])
        w.append(f"- STUBBED: `{st['at']}` (0x{st['addr']:08X}) returns at "
                 f"once from {st['when']} (frame {st['frame']}): every hit "
                 "after that is from a modified game. Called while stubbed "
                 "in phases: " + (", ".join(dict.fromkeys(
                     str(ph) for ph in calls)) or "none")
                 + "; " + (f"LEAKED past the stub in {len(leaks)} phase(s)"
                           if leaks else "never ran past the stub"))
    for pk in pokes:
        w.append(f"- POKED: `{pk['at']}` (0x{pk['addr']:08X}) set to "
                 f"{pk['hex']} (was {pk['orig']}) when {pk['when']} started "
                 f"(frame {pk['frame']}): every hit and read after that is "
                 "from a modified game")
    if phases:
        spans = []
        for i, ph in enumerate(phases):
            if ph.get("end"):
                continue
            nxt = phases[i + 1]["frame"] if i + 1 < len(phases) else "?"
            first = max(ph["frame"], armed or 0)
            if isinstance(nxt, int) and first >= nxt:
                spans.append(f"{ph['phase']} (not covered: armed later)")
            else:
                spans.append(f"{ph['phase']} (frames {first}-{nxt})")
        last = phases[-1]
        w.append("- phases covered: " + ", ".join(spans) +
                 ("; script reached its end" if last.get("end") else
                  "; run stopped before the script's end"))
    w.append("")
    if hitmap:
        w += hitmap_section(hitmap, hm_hits, phases, args)
    for (at, kind), recs in hits.items():
        label = at if kind == 0 else f"{at} (Z{kind} watch)"
        w.append(f"## {label}: {len(recs)} hits")
        w.append("")
        if any(r.get("phase") for r in recs):
            per = collections.Counter(r.get("phase") for r in recs)
            w.append("Hits per phase: " + ", ".join(
                f"{ph} {n}" for ph, n in per.items()))
        capped = [b.get("per_phase") for b in
                  (start or {}).get("spec", {}).get("breakpoints", [])
                  if b.get("at") == at and "per_phase" in b]
        if kind == 0 and capped:
            w.append(f"At most {capped[0]} hit(s) recorded per phase "
                     "(per_phase): counts and the tables below are capped.")
        d = disarmed.get((at, kind))
        if d:
            w.append(f"Disarmed after {d['after_hits']} hits (max_hits) in "
                     f"phase {d.get('phase')}: later phases are not covered.")
        if any(r.get("phase") for r in recs) or d:
            w.append("")
        if kind == 0:
            sites = collections.Counter(
                syms.name((r["ra"] - 8) & 0xFFFFFFFF) for r in recs)
            w.append("Call sites (ra-8):")
        else:
            sites = collections.Counter(
                syms.name(r["access"]["pc"]) if r.get("access")
                else f"stop pc {syms.name(r['pc'])}" for r in recs)
            w.append("Accessing instructions:")
        for c, n in sites.most_common(10):
            w.append(f"- `{c}` x{n}")
        w.append("")
        if kind:
            w.append("| access | width | value stored |")
            w.append("|---|---|---|")
            acc = collections.Counter(
                (r["access"]["addr"], r["access"]["width"], r["access"]["value"])
                for r in recs if r.get("access"))
            for (addr, width, value), n in sorted(acc.items())[:16]:
                shown = "-" if value is None else f"0x{value:X}"
                w.append(f"| `{syms.name(addr)}` | {width} | {shown} (x{n}) |")
            w.append("")
        if kind == 0:
            w.append("| arg | distinct | values (top 4) |")
            w.append("|---|---|---|")
            for i, name in enumerate(["a0", "a1", "a2", "a3"]):
                c = collections.Counter(r["args"][i] for r in recs)
                vals = ", ".join(
                    f"0x{v:08X}" + (f" `{syms.name(v)}`"
                                    if not syms.name(v).startswith("0x")
                                    else "") for v, _ in c.most_common(4))
                w.append(f"| {name} | {len(c)} | {vals} |")
            w.append("")
        dumps = collections.defaultdict(list)
        dump_addr = {}
        for r in recs:
            for d in r.get("dump", []):
                if "hex" in d:
                    key = json.dumps(d["spec"], sort_keys=True)
                    dumps[key].append(u32s(d["hex"]))
                    dump_addr.setdefault(key, d["addr"])
        for key, samples in dumps.items():
            d = json.loads(key)
            base = d.get("base", "?")
            w.append(f"Dump `{base}+0x{int(d.get('off', 0)):X}`, "
                     f"{d.get('len', 4)} bytes, {len(samples)} samples:")
            w.append("")
            w.append("| field | behaviour | values |")
            w.append("|---|---|---|")
            off = int(d.get("off", 0))
            if base in REG_INDEX:  # struct member offsets from the register
                label = lambda o, b=base, x=off: f"{b}+0x{x + o:02X}"
            else:  # a global: name each word's own address
                label = lambda o, a=dump_addr[key]: syms.name(a + o)
            w += describe_words(syms, samples, label)
            w.append("")
    if odd:
        w.append("## Stops not credited to any entry")
        w.append("")
        w += [f"- {k}: {n} (see the capture)" for k, n in odd.items()]
        w.append("")
    for at, recs in polls.items():
        w.append(f"## poll {at}: {len(recs)} samples over "
                 f"{recs[-1]['t'] - recs[0]['t']:.1f} s")
        w.append("")
        w.append("| field | behaviour | values |")
        w.append("|---|---|---|")
        at0 = recs[0]["addr"]
        w += describe_words(syms, [u32s(r["hex"]) for r in recs],
                            lambda o, a=at0: syms.name(a + o))
        w.append("")
    vspecs = (start or {}).get("spec", {}).get("values", [])
    for i, recs in sorted(values.items()):
        sp = vspecs[i] if i < len(vspecs) else {"at": recs[0]["at"]}
        what = (f"`{sp['at']}`" if "ptr_off" not in sp else
                f"*`{sp['at']}` + 0x{sp['ptr_off']:X}")
        w.append(f"## value {what}: {len(recs)} reads (at phase starts"
                 + (" and the end" if any(r.get("end") for r in recs)
                    else "; no end read") + ", emulator paused by the script)")
        w.append("")
        if stubs or pokes:
            w.append("Modified run: reads after a stub or poke went in are "
                     "from a modified game.")
            w.append("")
        if any(len(r.get("hex", "")) > 2 * DUMP_SHOWN for r in recs):
            # A RAM dump (movegraph.py reads it from the capture): one line
            # per read, no bytes or field table.
            for r in recs:
                n = len(r.get("hex", "")) // 2
                w.append(f"- {r['phase']} (frame {r['frame']}): "
                         + (f"{n} bytes, SHA1 {hashlib.sha1(bytes.fromhex(r['hex'])).hexdigest()}"
                            if r.get("hex") else f"not read: {r.get('error')}"))
            w.append("")
            continue
        w.append("| phase | frame | pointer | bytes (hex, memory order) |")
        w.append("|---|---|---|---|")
        good = []
        for r in recs:
            label = r["phase"] + (" (end)" if r.get("end") else " (start)")
            ptr = f"0x{r['ptr']:08X}" if "ptr" in r else "-"
            shown = r.get("hex") or f"not read: {r.get('error')}"
            if r.get("hex"):
                shown = " ".join(r["hex"][j:j + 8]
                                 for j in range(0, len(r["hex"]), 8))
                good.append(u32s(r["hex"]))
            fr = r["frame"]
            if r.get("pause_frame", fr) != fr:
                fr = f"{fr} (WARNING: paused at {r['pause_frame']})"
            w.append(f"| {label} | {fr} | {ptr} | `{shown}` |")
        w.append("")
        if good and "ptr_off" not in sp:
            w += describe_fields(syms, recs[0]["addr"],
                                 [bytes.fromhex(r["hex"]) for r in recs
                                  if r.get("hex")])
            w.append("")
    if start:
        seen = set(hits)
        spec = start.get("spec", {})
        armed = [(b["at"], 0) for b in spec.get("breakpoints", [])] + [
            (b["at"], WATCH_KIND[b.get("kind", "write")])
            for b in spec.get("watch", [])]
        idle = [at if k == 0 else f"{at} (Z{k} watch)"
                for at, k in armed if (at, k) not in seen]
        if idle:
            w.append("## Armed, never hit in this scenario")
            w.append("")
            w += [f"- `{a}`" for a in idle]
            w.append("")
    text = "\n".join(w) + "\n"
    if args.out:
        with open(args.out, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
    else:
        sys.stdout.write(text)
    return 0


# ------------------------------------------------------ stub-and-observe

def read_png(path: str):
    """(width, height, rgb) of a PNG written by runkit.png (8-bit RGB,
    filter 0 on every row)."""
    with open(path, "rb") as f:
        data = f.read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path}: not a PNG")
    pos, idat, w, h = 8, b"", None, None
    while pos < len(data):
        n, = runkit.struct.unpack(">I", data[pos:pos + 4])
        tag, body = data[pos + 4:pos + 8], data[pos + 8:pos + 8 + n]
        if tag == b"IHDR":
            w, h, depth, ctype = runkit.struct.unpack(">IIBB", body[:10])
            if (depth, ctype) != (8, 2):
                raise ValueError(f"{path}: not 8-bit RGB")
        elif tag == b"IDAT":
            idat += body
        pos += 12 + n
    raw = runkit.zlib.decompress(idat)
    stride = w * 3 + 1
    if len(raw) != stride * h or any(raw[y * stride] for y in range(h)):
        raise ValueError(f"{path}: not a runkit PNG (size or row filters)")
    return w, h, b"".join(raw[y * stride + 1:(y + 1) * stride]
                          for y in range(h))


def diff_shot(a: str, b: str, out_png: str | None):
    """Changed pixels between two screenshots: ((count, total, bbox), None)
    or (None, why not); with out_png, an image of `a` dimmed with the
    changed pixels in red."""
    wa, ha, pa = read_png(a)
    wb, hb, pb = read_png(b)
    if (wa, ha) != (wb, hb):
        return None, f"sizes differ: {wa}x{ha} vs {wb}x{hb}"
    n, box = 0, None
    img = bytearray(len(pa))
    for i in range(0, len(pa), 3):
        if pa[i:i + 3] != pb[i:i + 3]:
            n += 1
            x, y = (i // 3) % wa, (i // 3) // wa
            box = (x, y, x, y) if box is None else (
                min(box[0], x), min(box[1], y), max(box[2], x), max(box[3], y))
            img[i:i + 3] = b"\xff\x00\x00"
        else:
            img[i:i + 3] = bytes(v // 3 for v in pa[i:i + 3])
    if out_png and n:
        runkit.png(out_png, wa, ha, bytes(img))
    return (n, wa * ha, box), None


# Manifest parts two compared runs must share (SHA1s; lists compare whole).
SAME_INPUTS = [("tool", "gdbtrace.py"), ("tool", "runkit.py"),
               ("build", "exe"), ("build", "elf"), ("bios",),
               ("disc", "tracks"), ("emulator", "exe"), ("emulator", "main"),
               ("emulator", "version"), ("start", "memcard1", "sha1_at_start"),
               ("start", "memcard2", "sha1_at_start")]


def manifest_part(m: dict, path: tuple):
    for k in path:
        m = (m or {}).get(k)
    if isinstance(m, dict):
        return m.get("sha1")
    if isinstance(m, list):
        return [x.get("sha1") for x in m]
    return m


def compare(args) -> int:
    """Stub-and-observe evidence: a stubbed run against a baseline run of
    the same script on the same inputs; every screenshot both runs took
    is diffed. Exit 1 when the runs are not comparable."""
    mans = []
    for cap in (args.base, args.other):
        path = runkit.companion(cap, ".manifest.json")
        with open(path, encoding="utf-8") as f:
            mans.append((path, json.load(f)))
    (pa, ma), (pb, mb) = mans
    problems = []
    for path in SAME_INPUTS:
        if manifest_part(ma, path) != manifest_part(mb, path):
            problems.append("different " + "/".join(path))
    # Older manifests predate --headless: absent means windowed.
    if bool(manifest_part(ma, ("emulator", "headless"))) != \
            bool(manifest_part(mb, ("emulator", "headless"))):
        problems.append("different emulator/headless (one run windowed)")
    specs = []
    for m, given, label in ((ma, args.base_spec, "baseline"),
                            (mb, args.other_spec, "stubbed")):
        path = given or m["spec"]["path"]
        if runkit.sha1_file(path) != m["spec"].get("sha1"):
            problems.append(f"{label} spec {path} is not the one recorded")
        with open(path, encoding="utf-8") as f:
            specs.append(json.load(f))
    sa, sb = specs
    if sa.get("script") != sb.get("script"):
        problems.append("different scripts (inputs, phases or screenshots)")
    if sa.get("stubs") or sa.get("pokes"):
        problems.append("the baseline has stubs or pokes")
    if not sb.get("stubs") and not sb.get("pokes"):
        problems.append("the second run has no stubs or pokes")
    calls = collections.defaultdict(list)  # stub -> phases it was called in
    with open(args.other, encoding="utf-8") as f:
        for line in f:
            rec = json.loads(line)
            if rec.get("ev") == "hit" and rec.get("stub_call"):
                calls[rec["addr"]].append(rec.get("phase"))
    for st in mb.get("result", {}).get("stubs") or []:
        if not calls.get(st["addr"]):
            problems.append(f"stub {st['at']} was never called while "
                            "stubbed")
    for m, label in ((ma, "baseline"), (mb, "stubbed")):
        if not m.get("q124_complete"):
            problems.append(f"{label} is not q124_complete: "
                            f"{m.get('q124_missing')}")
    for m, cap, label in ((ma, args.base, "baseline"),
                          (mb, args.other, "stubbed")):
        # The outputs on disk are the ones the manifest recorded.
        d = os.path.dirname(os.path.abspath(cap))
        for rec in m.get("outputs", []):
            p = os.path.join(d, os.path.basename(rec["path"]))
            if not os.path.isfile(p) or runkit.sha1_file(p) != rec.get("sha1"):
                problems.append(f"{label} output {os.path.basename(p)} "
                                "differs from its manifest")

    base_dir = os.path.dirname(os.path.abspath(args.out)) if args.out \
        else os.getcwd()
    w = [f"# Stub-and-observe: {os.path.basename(os.path.dirname(os.path.abspath(args.other)))}"
         f" against {os.path.basename(os.path.dirname(os.path.abspath(args.base)))}",
         ""]
    if args.context:
        w.append(f"- scenario: {args.context}")
    for label, p, m in (("baseline", pa, ma), ("stubbed", pb, mb)):
        r = m.get("result", {})
        try:
            shown = os.path.relpath(p, base_dir)
        except ValueError:  # another drive
            shown = p
        w.append(f"- {label}: `{shown}` (SHA1 "
                 f"{runkit.sha1_file(p)}), outcome {r.get('outcome')}, script "
                 f"ended {r.get('script_ended')}, q124_complete "
                 f"{m.get('q124_complete')}")
    stubs = mb.get("result", {}).get("stubs") or []
    w.append("- stubs: " + (", ".join(
        f"`{s['at']}` (0x{s['addr']:08X}) from "
        + ("the EXE entry" if s["when"] == "entry" else f"phase {s['when']}")
        + f" (frame {(s.get('applied') or {}).get('frame')}), returns "
        + ("with v0 unchanged" if s.get("ret") is None else f"{s['ret']}")
        + f", {s.get('at_end')} at the end, called while stubbed in "
        + (", ".join(dict.fromkeys(str(x) for x in calls.get(s["addr"], [])))
           or "no phase") for s in stubs) or "none"))
    pokes = mb.get("result", {}).get("pokes") or []
    if pokes:
        w.append("- pokes: " + ", ".join(
            f"`{p['at']}` (0x{p['addr']:08X}) = {p['hex']} (was "
            f"{p.get('orig')}) from phase {p['when']} (frame "
            f"{(p.get('applied') or {}).get('frame')}), {p.get('at_end')} at "
            "the end" for p in pokes))
    w.append("- " + ("same inputs and script" if not problems else
                     "NOT COMPARABLE: " + "; ".join(problems)))
    w += ["", "| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |",
          "|---|---|---|"]
    stem_a, stem_b = (runkit.companion(os.path.abspath(c), "")
                      for c in (args.base, args.other))
    for sh in (sa.get("script") or {}).get("screenshots", []):
        name = sh["name"]
        fa, fb = f"{stem_a}.{name}.png", f"{stem_b}.{name}.png"
        if not (os.path.isfile(fa) and os.path.isfile(fb)):
            w.append(f"| {name} (frame {sh['frame']}) | missing | - |")
            continue
        out_png = (runkit.companion(args.out[:-3] if args.out.endswith(".md")
                                    else args.out, f".{name}.diff.png")
                   if args.out else None)
        res, err = diff_shot(fa, fb, out_png)
        if err:
            w.append(f"| {name} (frame {sh['frame']}) | {err} | - |")
            continue
        n, total, box = res
        w.append(f"| {name} (frame {sh['frame']}) | {n} of {total} "
                 f"({100 * n / total:.2f}%) | "
                 + (f"{box[0]},{box[1]}-{box[2]},{box[3]}" if box else "-")
                 + " |")
    w += ["", "A stub counts only if the game kept running (both runs reached "
          "the script's end) and the lost effect is the function's own output "
          "(.claude/rules/naming-runtime-evidence.md). Pixels that differ "
          "beyond the function's own drawing point at changed game state. "
          "A call already in progress when the stub went in finishes "
          "normally (a leak breakpoint catches only code reaching the word "
          "after the stub)."]
    text = "\n".join(w) + "\n"
    if args.out:
        with open(args.out, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
    else:
        sys.stdout.write(text)
    return 1 if problems else 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    default_map = os.path.join(REPO, "build", "bb2.map")

    r = sub.add_parser("run", help="arm a spec and record a JSONL trace")
    r.add_argument("--spec", required=True)
    r.add_argument("--out", required=True)
    r.add_argument("--map", default=default_map)
    r.add_argument("--elf", help="ELF for statics/sizes (default: bb2.elf "
                   "next to the map)")
    r.add_argument("--host", default="127.0.0.1")
    r.add_argument("--port", type=int, default=2345)
    r.add_argument("--duration", type=float, default=60.0,
                   help="seconds, when the spec has no duration_s")
    r.add_argument("--connect-timeout", type=float, default=60.0)
    r.add_argument("--launch", choices=["pcsx-redux", "duckstation"],
                   help="start this emulator with its GDB stub, then trace")
    r.add_argument("--emulator", help="emulator executable for --launch")
    r.add_argument("--cue", help="disc image for --launch")
    r.add_argument("--bios", help="BIOS image for pcsx-redux (default OpenBIOS)")
    r.add_argument("--portable-dir",
                   help="pcsx-redux settings/saves directory (default "
                   "tmp/redux-portable; with --headless, the instance's own "
                   "profile, seeded with this dir's pcsx.json)")
    r.add_argument("--duckstation-ini", help="DuckStation settings.ini path")
    r.add_argument("--boot-wait", type=float, default=0.0,
                   help="seconds to let the game run before arming")
    r.add_argument("--keep-emulator", action="store_true")
    r.add_argument("--headless", action="store_true",
                   help="pcsx-redux with no window (-no-ui) on a pool "
                   "instance slot (tools/trace/pool.py: its own GDB port and "
                   "profile; --port is ignored)")
    r.add_argument("--stall", type=float, default=120.0,
                   help="script runs: fail when no emulated frame passes "
                   "for this many seconds while the target runs (0: off)")
    r.add_argument("--slot-wait", type=float, default=3600.0,
                   help="--headless: seconds to wait for a free slot")
    for n in (1, 2):
        r.add_argument(f"--memcard{n}", help=f"pcsx-redux: start with a copy "
                       f"of this card in slot {n} (default: a blank card)")

    p = sub.add_parser("report", help="summarize a trace as markdown evidence")
    p.add_argument("trace")
    p.add_argument("--map", default=default_map)
    p.add_argument("--elf", help="ELF for statics/sizes (default: bb2.elf "
                   "next to the map)")
    p.add_argument("--context", help="one line: what the run exercised")
    p.add_argument("--hitmap-tsv", help="also write the hit map as TSV "
                   "(function, addr, one column per phase)")
    p.add_argument("--out")

    c = sub.add_parser("compare", help="stub-and-observe: diff a stubbed run's "
                       "screenshots against a baseline run")
    c.add_argument("base", help="baseline capture (.jsonl)")
    c.add_argument("other", help="stubbed capture (.jsonl)")
    c.add_argument("--base-spec", help="spec path if it moved since the run")
    c.add_argument("--other-spec", help="spec path if it moved since the run")
    c.add_argument("--context", help="one line: what the runs exercised")
    c.add_argument("--out", help="markdown; <out stem>.<shot>.diff.png too")

    args = ap.parse_args(argv)
    if args.cmd == "run":
        if args.launch and not args.cue:
            ap.error("--launch needs --cue")
        if not args.headless and not args.portable_dir:
            args.portable_dir = os.path.join(REPO, "tmp", "redux-portable")
        return run(args)
    if args.cmd == "compare":
        return compare(args)
    return report(args)


if __name__ == "__main__":
    sys.exit(main())
