#!/usr/bin/env python3
"""pool: headless PCSX-Redux instances, many at once, from many agents.

`gdbtrace.py run --headless` takes an instance slot here, so any number of
agents (in any worktree) can trace at the same time without windows and
without colliding on a GDB port or a settings directory:

  * a lock-file registry, one file per slot (slot-<n>.lock), created with
    O_CREAT|O_EXCL so two processes can never hold one slot; slot n owns
    GDB port PORT_BASE + n and its own profile dir slot-<n>/portable
    (PCSX-Redux -portable: settings, crash reports; a run's memory cards
    stay next to its capture, savestates are unavailable with -no-ui);
  * a machine-wide cap: BB2_TRACE_MAX, else <registry>/config.json
    {"max": N}, else min(cores // 2, running + free RAM // 512 MiB), at
    least 1;
  * crashed owners are reaped: a lock whose owner process is gone has its
    recorded emulator processes killed (only if they are still PCSX-Redux)
    and is removed, under a short reap mutex so a fresh lock is never taken
    for a stale one.

The registry is machine-wide: BB2_TRACE_INSTANCES, else tmp/trace-instances
in the main checkout (found through git's common dir, so every lane
worktree shares it). Profile dirs are reused by the next holder of the
slot, never deleted.

  python tools/trace/pool.py status     # cap, slots, owners
  python tools/trace/pool.py reap       # clear slots of dead owners
"""

from __future__ import annotations

import json
import os
import random
import socket
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
PORT_BASE = int(os.environ.get("BB2_TRACE_PORT_BASE", "2400"))
MAX_SLOTS = 64  # slot numbers scanned; the cap is the real limit
RAM_PER_INSTANCE_MB = 512  # emulator ~200 MB working set + harness, headroom
REAP_MUTEX_STALE_S = 60


def registry() -> str:
    env = os.environ.get("BB2_TRACE_INSTANCES")
    if env:
        return os.path.abspath(env)
    try:
        r = subprocess.run(["git", "-C", HERE, "rev-parse", "--path-format=absolute",
                            "--git-common-dir"], capture_output=True, text=True)
        common = r.stdout.strip() if r.returncode == 0 else ""
    except OSError:
        common = ""
    main = os.path.dirname(common) if common else \
        os.path.dirname(os.path.dirname(HERE))
    return os.path.join(main, "tmp", "trace-instances")


# ------------------------------------------------------------- processes

def image_name(pid: int) -> str | None:
    """Executable name of a live process, None if there is none."""
    if not pid:
        return None
    if os.name == "nt":
        try:
            r = subprocess.run(["tasklist", "/FI", f"PID eq {pid}", "/FO",
                                "CSV", "/NH"], capture_output=True, text=True)
        except OSError:
            return "?"  # cannot tell: never treat as dead
        for line in r.stdout.splitlines():
            f = [x.strip('"') for x in line.split('","')]
            if len(f) >= 2 and f[1].strip('"') == str(pid):
                return f[0].strip('"')
        if r.returncode == 0 and "No tasks" in r.stdout:
            return None
        return "?"  # tasklist failed or printed something else
    try:
        os.kill(pid, 0)
    except OSError:
        return None
    try:
        with open(f"/proc/{pid}/comm", encoding="utf-8") as f:
            return f.read().strip()
    except OSError:
        return "?"


def kill_tree(pid: int):
    if os.name == "nt":
        subprocess.run(["taskkill", "/T", "/F", "/PID", str(pid)],
                       capture_output=True)
    else:
        try:
            os.kill(pid, 9)
        except OSError:
            pass


def free_ram_mb() -> int:
    if os.name == "nt":
        import ctypes

        class MS(ctypes.Structure):
            _fields_ = [("len", ctypes.c_ulong), ("load", ctypes.c_ulong),
                        ("total", ctypes.c_ulonglong),
                        ("avail", ctypes.c_ulonglong),
                        ("tpage", ctypes.c_ulonglong),
                        ("apage", ctypes.c_ulonglong),
                        ("tvirt", ctypes.c_ulonglong),
                        ("avirt", ctypes.c_ulonglong),
                        ("aext", ctypes.c_ulonglong)]
        ms = MS()
        ms.len = ctypes.sizeof(MS)
        ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(ms))
        return int(ms.avail // (1 << 20))
    try:
        with open("/proc/meminfo", encoding="utf-8") as f:
            for line in f:
                if line.startswith("MemAvailable:"):
                    return int(line.split()[1]) // 1024
    except OSError:
        pass
    return RAM_PER_INSTANCE_MB


def port_in_use(port: int) -> bool:
    try:
        socket.create_connection(("127.0.0.1", port), 0.5).close()
        return True
    except OSError:
        return False


# --------------------------------------------------------------- registry

def lock_path(reg: str, n: int) -> str:
    return os.path.join(reg, f"slot-{n}.lock")


def read_lock(path: str) -> dict | None:
    try:
        with open(path, encoding="utf-8") as f:
            return json.load(f)
    except (OSError, ValueError):
        return None


def held(reg: str) -> dict[int, dict]:
    out = {}
    for n in range(MAX_SLOTS):
        rec = read_lock(lock_path(reg, n))
        if rec is not None or os.path.exists(lock_path(reg, n)):
            out[n] = rec or {}
    return out


def cap(reg: str, running: int) -> tuple[int, str]:
    """(cap, where it came from)."""
    env = os.environ.get("BB2_TRACE_MAX")
    if env:
        return max(1, int(env)), "BB2_TRACE_MAX"
    cfg = read_lock(os.path.join(reg, "config.json"))
    if cfg and isinstance(cfg.get("max"), int):
        return max(1, cfg["max"]), "config.json"
    cores = os.cpu_count() or 2
    by_ram = running + free_ram_mb() // RAM_PER_INSTANCE_MB
    return max(1, min(cores // 2, by_ram)), \
        f"default min({cores} cores // 2, {running} running + free RAM // " \
        f"{RAM_PER_INSTANCE_MB} MB)"


def owner_alive(rec: dict) -> bool:
    name = image_name(int(rec.get("pid") or 0))
    want = (rec.get("image") or "").lower()
    return name is not None and (name == "?" or not want
                                 or name.lower() == want)


def reap(reg: str, log=print) -> list[int]:
    """Remove the locks of dead owners (killing their emulators); returns
    the freed slot numbers."""
    stale = [n for n, rec in held(reg).items()
             if rec and not owner_alive(rec)
             or not rec and age(lock_path(reg, n)) > REAP_MUTEX_STALE_S]
    if not stale:
        return []
    mutex = os.path.join(reg, "reap.mutex")
    if os.path.exists(mutex) and age(mutex) > REAP_MUTEX_STALE_S:
        try:
            os.remove(mutex)
        except OSError:
            pass
    try:
        fd = os.open(mutex, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        os.close(fd)
    except FileExistsError:
        return []  # another process is reaping
    freed = []
    try:
        for n in stale:
            path = lock_path(reg, n)
            rec = read_lock(path)  # again, under the mutex
            if rec and owner_alive(rec):
                continue
            if not rec and age(path) <= REAP_MUTEX_STALE_S:
                continue  # being written right now
            for pid in (rec or {}).get("emulator_pids", []):
                name = image_name(pid)
                if name and name != "?" and (
                        name.lower().startswith("pcsx-redux")):
                    kill_tree(pid)
                    log(f"pool: slot {n}: killed {name} pid {pid} of dead "
                        f"owner pid {(rec or {}).get('pid')}")
            try:
                os.remove(path)
                freed.append(n)
                log(f"pool: slot {n}: stale lock removed")
            except OSError:
                pass
    finally:
        try:
            os.remove(mutex)
        except OSError:
            pass
    return freed


def age(path: str) -> float:
    try:
        return time.time() - os.path.getmtime(path)
    except OSError:
        return 0.0


def retry(fn, tries: int = 50):
    """Windows refuses to replace or remove a file another process has
    open (a reader of the lock): try again for a few seconds."""
    for i in range(tries):
        try:
            return fn()
        except PermissionError:
            if i == tries - 1:
                raise
            time.sleep(0.1)


class Slot:
    def __init__(self, reg: str, n: int, rec: dict):
        self.reg, self.n, self.rec = reg, n, rec
        self.port = PORT_BASE + n
        self.dir = os.path.join(reg, f"slot-{n}")
        self.profile = os.path.join(self.dir, "portable")
        self.lock = lock_path(reg, n)
        os.makedirs(self.profile, exist_ok=True)

    def record(self, **kw):
        """Add fields (e.g. emulator_pids) to the lock, atomically."""
        self.rec.update(kw)
        tmp = self.lock + f".{os.getpid()}.tmp"
        with open(tmp, "w", encoding="utf-8", newline="\n") as f:
            json.dump(self.rec, f)
        retry(lambda: os.replace(tmp, self.lock))

    def release(self):
        rec = read_lock(self.lock)
        if rec and rec.get("token") == self.rec.get("token"):
            retry(lambda: os.remove(self.lock))

    def describe(self) -> dict:
        return {"slot": self.n, "port": self.port, "registry": self.reg,
                "profile": self.profile}


def acquire(purpose: str, wait_s: float = 3600.0, log=print) -> Slot:
    """Take a free slot under the cap, waiting up to wait_s for one."""
    reg = registry()
    os.makedirs(reg, exist_ok=True)
    deadline = time.time() + wait_s
    me = image_name(os.getpid()) or ""
    said = False
    while True:
        reap(reg, log)
        taken = held(reg)
        limit, why = cap(reg, len(taken))
        if len(taken) < limit:
            for n in range(MAX_SLOTS):
                if n in taken:
                    continue
                rec = {"pid": os.getpid(), "image": me, "purpose": purpose,
                       "started": time.time(), "port": PORT_BASE + n,
                       "token": f"{os.getpid()}-{time.time_ns()}"}
                try:
                    fd = os.open(lock_path(reg, n),
                                 os.O_CREAT | os.O_EXCL | os.O_WRONLY)
                except FileExistsError:
                    continue
                with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as f:
                    json.dump(rec, f)
                slot = Slot(reg, n, rec)
                if len(held(reg)) > limit:  # raced other acquirers: back off
                    slot.release()
                    break
                if port_in_use(slot.port):  # not ours: try the next slot
                    log(f"pool: port {slot.port} has a foreign listener; "
                        f"skipping slot {n}")
                    slot.release()
                    continue
                return slot
        if time.time() > deadline:
            raise SystemExit(f"pool: no free instance slot within {wait_s:.0f}"
                             f" s ({len(taken)} held, cap {limit}: {why})")
        if not said:
            log(f"pool: {len(taken)} of {limit} instances busy; waiting")
            said = True
        time.sleep(1.0 + random.random() * 2)  # jitter: racers spread out


# -------------------------------------------------------------------- CLI

def main(argv=None) -> int:
    argv = sys.argv[1:] if argv is None else argv
    cmd = argv[0] if argv else "status"
    reg = registry()
    os.makedirs(reg, exist_ok=True)
    if cmd == "reap":
        freed = reap(reg)
        print(f"pool: {len(freed)} stale slot(s) freed")
        return 0
    if cmd != "status":
        print(__doc__)
        return 2
    taken = held(reg)
    limit, why = cap(reg, len(taken))
    print(f"pool: registry {reg}")
    print(f"pool: cap {limit} ({why}); {len(taken)} held; ports from "
          f"{PORT_BASE}")
    for n, rec in sorted(taken.items()):
        alive = "alive" if rec and owner_alive(rec) else "DEAD (reap)"
        print(f"  slot {n} port {PORT_BASE + n} owner pid {rec.get('pid')} "
              f"{alive} emulator {rec.get('emulator_pids', [])} "
              f"{rec.get('purpose', '')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
