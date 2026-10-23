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
  * DuckStation: execution breakpoints (Z0) only. `--launch duckstation`
    sets [Debug] EnableGDBServer in its settings.ini only until the stub
    listens. On the owner's machine (0.1.11295) its socket layer fails
    (`WSAStartup() failed` in duckstation.log), so the stub never opens.

Subcommands
  run     connect (or launch an emulator), arm the spec, record events as JSONL
  report  summarize a trace into an evidence note (markdown) naming lanes cite

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
    ]
  }
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
caller. Watch hits name the load/store at the stop pc (or pc-4); a store in
a branch delay slot may be missed and then shows as an `unmatched-stop` event.
A trace covers one scenario, never everything a function can do.
"""

from __future__ import annotations

import argparse
import bisect
import collections
import configparser
import json
import os
import re
import shutil
import socket
import subprocess
import sys
import time

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
MAX_SYM_OFF = 0x4000  # larger offsets print raw: no symbol spans that far
DEBUG = bool(os.environ.get("GDBTRACE_DEBUG"))  # log every packet to stderr


# ----------------------------------------------------------------- symbols

class Symbols:
    """Address <-> name from a GNU ld map (`0xADDR   name` lines)."""

    LINE = re.compile(r"^\s+0x([0-9a-fA-F]{8,16})\s+([A-Za-z_.$][\w.$]*)\s*$")

    def __init__(self, path: str | None):
        self.by_name: dict[str, int] = {}
        pairs = []
        if path and os.path.exists(path):
            with open(path, encoding="utf-8", errors="replace") as f:
                for line in f:
                    m = self.LINE.match(line)
                    if not m:
                        continue
                    addr = int(m.group(1), 16) & 0xFFFFFFFF
                    name = m.group(2)
                    if not RAM_LO <= addr < RAM_HI:
                        continue
                    # INCLUDE_ASM bodies also emit a `<name>.NON_MATCHING`
                    # alias at the same address; keep the plain name.
                    name = name.removesuffix(".NON_MATCHING")
                    if name in self.by_name:
                        continue
                    self.by_name[name] = addr
                    pairs.append((addr, name))
        pairs.sort()
        self.addrs = [a for a, _ in pairs]
        self.names = [n for _, n in pairs]
        self.path = os.path.abspath(path) if self.addrs else None

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
        if 0xA0000000 <= addr < 0xA0200000 or addr < 0x00200000:
            addr = RAM_LO | (addr & 0x1FFFFFFF)  # KSEG1 / physical alias
        if not RAM_LO <= addr < RAM_HI:
            return f"0x{addr:08X}"
        i = bisect.bisect_right(self.addrs, addr) - 1
        if i < 0:
            return f"0x{addr:08X}"
        off = addr - self.addrs[i]
        if off > MAX_SYM_OFF:  # past the last object: heap, stack, overlay
            return f"0x{addr:08X}"
        return self.names[i] if off == 0 else f"{self.names[i]}+0x{off:X}"


# ------------------------------------------------------------ gdb protocol

class GdbError(Exception):
    pass


class Gdb:
    """Minimal GDB remote serial protocol client (ack mode, all-stop)."""

    def __init__(self, host: str, port: int, timeout: float = 10.0):
        self.sock = socket.create_connection((host, port), timeout=timeout)
        self.buf = b""
        self.console: list[str] = []
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
            if payload != "?" and re.match(r"[ST][0-9a-fA-F]{2}", reply):
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
            if reply.startswith("E") or len(reply) != 2 * n:
                raise GdbError(f"read 0x{addr:08X},{n}: {reply[:16]}")
            out += bytes.fromhex(reply)
            addr += n
            length -= n
        return out

    def set_bp(self, kind: int, addr: int, length: int = 4) -> bool:
        return self.cmd(f"Z{kind},{addr:x},{length:x}") == "OK"

    def clear_bp(self, kind: int, addr: int, length: int = 4) -> bool:
        return self.cmd(f"z{kind},{addr:x},{length:x}") == "OK"

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


def decode_access(gdb: Gdb, regs: list[int], pc: int):
    """The load/store a watchpoint stop was raised for. Some stubs
    (PCSX-Redux) send a bare T05 with no `watch:` address, so look at the
    instruction at pc, then pc-4 (stubs differ on stopping before or after
    it). Returns (insn_pc, addr, width, is_store, stored_value) or None."""
    for at in (pc, pc - 4):
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
        if store and op != 0x3A:
            value = regs[rt] & ((1 << (8 * width)) - 1)
        return at, addr, width, store, value
    return None


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
    syms = Symbols(args.map)
    with open(args.spec, encoding="utf-8") as f:
        spec = json.load(f)

    emu = None
    if args.launch == "duckstation":
        emu = launch_duckstation(args)
    elif args.launch == "pcsx-redux":
        emu = launch_redux(args)
    try:
        gdb = connect(args.host, args.port, args.connect_timeout)
        return trace(gdb, syms, spec, args)
    finally:
        if emu is not None:
            proc, restore = emu
            if args.keep_emulator:
                print("gdbtrace: emulator left running; DuckStation may save "
                      "EnableGDBServer = true when you close it")
            else:
                stop_emulator(proc, args.port)
                restore()


def connect(host: str, port: int, wait: float) -> Gdb:
    deadline = time.time() + wait
    while True:
        try:
            return Gdb(host, port)
        except OSError:
            if time.time() > deadline:
                raise SystemExit(f"gdbtrace: no GDB stub on {host}:{port}")
            time.sleep(0.5)


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
    for w in spec.get("watch", []):
        kind = WATCH_KIND[w.get("kind", "write")]
        addr, n = syms.addr(w["at"]), int(w.get("len", 4))
        if not gdb.set_bp(kind, addr, n):
            raise SystemExit(f"gdbtrace: stub has no Z{kind} watchpoints "
                             "(DuckStation has none; use PCSX-Redux)")
        armed[(kind, addr)] = {"spec": w, "hits": 0, "kind": kind,
                               "addr": addr, "len": n}

    polls = [{"spec": p, "addr": syms.addr(p["at"]), "len": int(p.get("len", 4)),
              "every": float(p.get("every_s", 1.0)), "next": 0.0}
             for p in spec.get("polls", [])]

    duration = float(spec.get("duration_s", args.duration))
    max_events = int(spec.get("max_events", 5000))
    t0 = time.time()
    events = 0
    totals = {}

    def entry_for_stop(regs, reply, interrupted):
        pc = regs[37] if len(regs) > 37 else 0
        if (0, pc) in armed:
            return armed[(0, pc)], None
        watches = [e for (k, _), e in armed.items() if k]
        if not watches or interrupted:
            # Our own ^C can land on any load/store; never a watch hit.
            return None, None
        m = re.search(r"(?:watch|rwatch|awatch):([0-9a-fA-F]+)", reply)
        acc = decode_access(gdb, regs, pc)
        a = int(m.group(1), 16) if m else (acc[1] if acc else None)
        if a is None:
            return None, acc
        for e in watches:
            n = acc[2] if acc else 1
            if acc and e["kind"] != 4 and acc[3] != (e["kind"] == 2):
                continue  # a load under a write watch, or vice versa
            if (phys(a) < phys(e["addr"]) + e["len"]
                    and phys(e["addr"]) < phys(a) + n):
                return e, acc
        return None, acc

    try:
        while events < max_events and time.time() - t0 < duration:
            now = time.time()
            for p in polls:
                if now >= p["next"]:
                    p["next"] = now + p["every"]
                    emit({"ev": "poll", "t": round(now - t0, 3),
                          "at": p["spec"]["at"], "addr": p["addr"],
                          "hex": gdb.mem(p["addr"], p["len"]).hex()})
                    events += 1
            gdb.cont()
            until = min([p["next"] for p in polls] + [t0 + duration])
            reply = gdb.wait_stop(max(0.05, until - time.time()))
            if gdb.console:
                emit({"ev": "tty", "t": round(time.time() - t0, 3),
                      "text": "".join(gdb.console)})
                gdb.console.clear()
            interrupted = reply is None
            if interrupted:
                # Poll or deadline due: stop the target. A breakpoint may
                # have raced the interrupt; its stop reply is handled below.
                reply = gdb.interrupt()
            if reply.startswith(("W", "X")):
                emit({"ev": "exit", "reply": reply})
                break
            regs = gdb.regs()
            e, acc = entry_for_stop(regs, reply, interrupted)
            if e is None:
                if not interrupted:
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
            if acc:
                rec["access"] = {"pc": acc[0], "addr": acc[1], "width": acc[2],
                                 "store": acc[3], "value": acc[4]}
            if sp.get("regs"):
                rec["regs"] = {r: regs[REG_INDEX[r]] for r in sp["regs"]}
            if sp.get("dump"):
                rec["dump"] = snapshot(gdb, syms, regs, sp["dump"])
            emit(rec)
            events += 1
            if e["hits"] >= int(sp.get("max_hits", 1 << 30)):
                gdb.clear_bp(e["kind"], e["addr"], e["len"])
                del armed[(e["kind"], e["addr"])]
                if not armed and not polls:
                    break
    finally:
        for (kind, addr), e in list(armed.items()):
            try:
                gdb.clear_bp(kind, addr, e["len"])
            except (GdbError, OSError):
                pass
        try:
            gdb.cont()
        except (GdbError, OSError):
            pass
        emit({"ev": "end", "events": events,
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


def stop_emulator(proc, port: int):
    """PCSX-Redux's pcsx-redux.exe is a launcher for pcsx-redux.main, so
    killing the Popen child is not enough: kill whoever owns the stub port,
    then the launcher's tree."""
    pids = [port_owner(port), proc.pid]
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
           "-fastboot", "-run", "-noupdate"]  # dynarec skips breakpoints
    if args.bios:
        cmd += ["-bios", os.path.abspath(args.bios)]
    # cwd: its crash reporter writes .sentry-native/ into the working dir.
    proc = subprocess.Popen(cmd, cwd=portable)
    wait_for_stub(proc, args.host, args.port, args.connect_timeout, "PCSX-Redux")
    if args.boot_wait:
        time.sleep(args.boot_wait)
    return proc, lambda: None


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
    try:
        with open(ini, "wb") as f:
            f.write(patched.encode("utf-8"))
        proc = subprocess.Popen([exe, "-batch", "-fastboot", "--",
                                 os.path.abspath(args.cue)])
        wait_for_stub(proc, args.host, args.port, args.connect_timeout,
                      "DuckStation")
    finally:
        with open(ini, "wb") as f:
            f.write(original)

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

    if args.boot_wait:
        time.sleep(args.boot_wait)
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


def report(args) -> int:
    syms = Symbols(args.map)
    hits = collections.defaultdict(list)
    polls = collections.defaultdict(list)
    start = end = None
    with open(args.trace, encoding="utf-8") as f:
        for line in f:
            rec = json.loads(line)
            if rec["ev"] == "hit":
                hits[(rec["at"], rec["kind"])].append(rec)
            elif rec["ev"] == "poll":
                polls[rec["at"]].append(rec)
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
    w.append("")
    for (at, kind), recs in hits.items():
        label = at if kind == 0 else f"{at} (Z{kind} watch)"
        w.append(f"## {label}: {len(recs)} hits")
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
        for r in recs:
            for d in r.get("dump", []):
                if "hex" in d:
                    dumps[json.dumps(d["spec"], sort_keys=True)].append(
                        u32s(d["hex"]))
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
                at0 = (syms.addr(base) + off) & 0xFFFFFFFF
                label = lambda o, a=at0: syms.name(a + o)
            w += describe_words(syms, samples, label)
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
    if start:
        seen = {at for at, _ in hits}
        spec = start.get("spec", {})
        idle = [b["at"] for b in spec.get("breakpoints", []) + spec.get("watch", [])
                if b["at"] not in seen]
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


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    default_map = os.path.join(REPO, "build", "bb2.map")

    r = sub.add_parser("run", help="arm a spec and record a JSONL trace")
    r.add_argument("--spec", required=True)
    r.add_argument("--out", required=True)
    r.add_argument("--map", default=default_map)
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
                   default=os.path.join(REPO, "tmp", "redux-portable"),
                   help="pcsx-redux settings/saves directory")
    r.add_argument("--duckstation-ini", help="DuckStation settings.ini path")
    r.add_argument("--boot-wait", type=float, default=0.0,
                   help="seconds to let the game run before arming")
    r.add_argument("--keep-emulator", action="store_true")

    p = sub.add_parser("report", help="summarize a trace as markdown evidence")
    p.add_argument("trace")
    p.add_argument("--map", default=default_map)
    p.add_argument("--context", help="one line: what the run exercised")
    p.add_argument("--out")

    args = ap.parse_args(argv)
    if args.cmd == "run":
        if args.launch and not args.cue:
            ap.error("--launch needs --cue")
        return run(args)
    return report(args)


if __name__ == "__main__":
    sys.exit(main())
