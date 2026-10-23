"""runkit: the reproducible-run side of gdbtrace (owner ruling Q124).

A trace counts as naming evidence only with a record of how it was made
(.claude/rules/naming-runtime-evidence.md): emulator and version, BIOS,
disc SHA1, start state, the scripted inputs and the captured output with
its SHA1. This module provides
  * the scripted run: a spec's "script" block becomes a PCSX-Redux Lua file
    (loaded with -dofile) that counts emulated frames (GPU::Vsync), presses
    pad buttons through pad overrides, takes screenshots, labels phases,
    and pauses the emulator at each phase change and at the end so the
    harness sees the boundary as a stop;
  * the manifest: <out>.manifest.json with the SHA1 of every input and
    output of the run.

Script block (frames count GPU::Vsync events from power-on; actions run
on frames 1..end_frame, a phase may also start at frame 0):
  "script": {
    "end_frame": 3600,                       # the run ends here
    "phases": [{"frame": 0, "name": "boot"},  # label for hits from here on
               {"frame": 1500, "name": "title"}],
    "inputs": [{"frame": 1200, "press": ["START"], "hold": 6}],
    "screenshots": [{"frame": 1400, "name": "title"}]
  }
Buttons: SELECT START UP RIGHT DOWN LEFT L1 R1 L2 R2 TRIANGLE CIRCLE CROSS
SQUARE (pad 1 of slot 1). A press holds for `hold` frames (default 4,
at least 1); holds of one button need a released frame between them.

The scripted emulator runs from PCSX-Redux defaults (-safe) plus the
command line, starts held at the reset vector and only runs once the
harness has armed, so coverage starts at frame 0 (recorded as
armed_frame). The manifest records the outcome and, taken before launch,
the SHA1 of the tools (with their git status), the build, the spec, the
emulator executables and version, the BIOS, the disc tracks and both
memory cards; after the run, the SHA1 of every output.
"""

from __future__ import annotations

import hashlib
import json
import os
import re
import struct
import subprocess
import sys
import zlib

ORACLE_SHA1 = "62efab4f73f992798c43e8c730aa43baa10bb4fa"
BUTTONS = {"SELECT", "START", "UP", "RIGHT", "DOWN", "LEFT", "L1", "R1",
           "L2", "R2", "TRIANGLE", "CIRCLE", "CROSS", "SQUARE"}
NAME_RE = re.compile(r"[A-Za-z0-9_.-]+")


def sha1_file(path: str) -> str:
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def file_rec(path: str, base: str | None = None) -> dict:
    rec = {"path": os.path.relpath(path, base) if base else path}
    if os.path.isfile(path):
        rec["sha1"] = sha1_file(path)
        rec["size"] = os.path.getsize(path)
    else:
        rec["missing"] = True
    return rec


def companion(out: str, suffix: str) -> str:
    """<out without .jsonl><suffix>: the files that travel with a capture."""
    stem = out[:-6] if out.endswith(".jsonl") else out
    return stem + suffix


# ------------------------------------------------------------- the script

def check_script(script: dict):
    def bad(msg):
        raise SystemExit(f"gdbtrace: script: {msg}")

    end = script.get("end_frame")
    if not isinstance(end, int) or end < 1:
        bad("end_frame must be an integer >= 1")

    def frame_of(item, lo=1):
        f = item.get("frame")
        if not isinstance(f, int) or not lo <= f <= end:
            bad(f"{item}: frame must be an integer in {lo}..end_frame")
        return f

    held = {}
    for i in script.get("inputs", []):
        f = frame_of(i)
        hold = i.get("hold", 4)
        press = i.get("press")
        if not press or set(press) - BUTTONS:
            bad(f"{i}: press needs buttons from {sorted(BUTTONS)}")
        if not isinstance(hold, int) or hold < 1:
            bad(f"{i}: hold must be an integer >= 1")
        if f + hold > end:
            bad(f"{i}: the release (frame {f + hold}) is after end_frame")
        for b in press:
            for f0, f1 in held.get(b, []):
                if f <= f1 and f0 <= f + hold:  # needs a released frame
                    bad(f"{i}: overlaps or touches another hold of {b}")
            held.setdefault(b, []).append((f, f + hold))
    for item in script.get("phases", []) + script.get("screenshots", []):
        if not NAME_RE.fullmatch(str(item.get("name", ""))):
            bad(f"name {item.get('name')!r}: use letters, digits, _ . -")
    for sh in script.get("screenshots", []):
        frame_of(sh)
    pf = [frame_of(ph, 0) for ph in script.get("phases", [])]
    if len(pf) != len(set(pf)):
        bad("two phases start on one frame")
    pn = [ph["name"] for ph in script.get("phases", [])]
    if 0 not in pf:
        pn.append(first_phase(script))  # the implicit "start" phase
    if len(pn) != len(set(pn)):
        bad("phase names must be unique (hit maps count per name)")
    names = [sh["name"] for sh in script.get("screenshots", [])]
    if len(names) != len(set(names)):
        bad("screenshot names must be unique")


def clean_outputs(out: str, script: dict | None):
    """Remove what an earlier run with the same --out left, so a failed run
    can never be recorded around an old capture."""
    stale = [out] + [companion(out, x) for x in (
        ".state", ".frame", ".script.lua", ".script.log", ".manifest.json")]
    for sh in (script or {}).get("screenshots", []):
        stale += [companion(out, f".{sh['name']}.raw"),
                  companion(out, f".{sh['name']}.png")]
    for path in stale:
        if os.path.isfile(path):
            os.remove(path)


def write_blank_card(path: str):
    """A formatted, empty 128 KiB PS1 memory card: header frame, 15 free
    directory frames, 20 unused broken-sector frames, header copy in frame
    63; each frame's byte 127 is the XOR of bytes 0..126."""
    card = bytearray(128 * 1024)

    def frame(i, body: bytes):
        f = bytearray(128)
        f[:len(body)] = body
        x = 0
        for b in f[:127]:
            x ^= b
        f[127] = x
        card[i * 128:(i + 1) * 128] = f

    frame(0, b"MC")
    for i in range(1, 16):
        frame(i, b"\xA0\0\0\0\0\0\0\0\xFF\xFF")
    for i in range(16, 36):
        frame(i, b"\xFF\xFF\xFF\xFF\0\0\0\0\xFF\xFF")
    frame(63, b"MC")
    with open(path, "wb") as f:
        f.write(card)


def lua_str(s: str) -> str:
    return "[==[" + s + "]==]"


ACTION_ORDER = {"release": 0, "press": 1, "shot": 2, "phase": 3,
                "finish": 4}


def planned_actions(script: dict) -> list[tuple[int, str, str]]:
    """(frame, action, argument) in execution order. Within a frame:
    releases, presses, screenshots, the phase change, the end."""
    acts = []
    for i in script.get("inputs", []):
        for b in i["press"]:
            acts.append((int(i["frame"]), "press", b))
            acts.append((int(i["frame"]) + int(i.get("hold", 4)), "release", b))
    for sh in script.get("screenshots", []):
        acts.append((int(sh["frame"]), "shot", sh["name"]))
    for ph in script.get("phases", []):
        if int(ph["frame"]) > 0:
            acts.append((int(ph["frame"]), "phase", ph["name"]))
    acts.append((int(script["end_frame"]), "finish", "-"))
    return sorted(acts, key=lambda a: (a[0], ACTION_ORDER[a[1]], a[2]))


def first_phase(script: dict) -> str:
    for ph in script.get("phases", []):
        if int(ph["frame"]) == 0:
            return ph["name"]
    return "start"


def write_lua(script: dict, out: str) -> str:
    """Generate the Redux Lua for a script block; returns its path.

    Frame actions are keyed by frame number; each runs under pcall and
    logs `frame action argument` when done (`frame error ...` when not),
    so check_log can prove the script ran as written. Phase changes and
    the end rewrite the state file (`seq frame phase ended`) *before*
    pausing, so the harness reads the new state at the stop the pause
    causes."""
    check_script(script)
    by_frame: dict[int, list[str]] = {}
    for frame, act, arg in planned_actions(script):
        by_frame.setdefault(frame, []).append(
            f"{{{lua_str(act)}, {lua_str(arg)}}}")

    lines = [
        "-- generated by tools/trace/runkit.py; do not edit",
        f"local LOG = {lua_str(os.path.abspath(companion(out, '.script.log')))}",
        f"local STATE = {lua_str(os.path.abspath(companion(out, '.state')))}",
        f"local PAUSED = {lua_str(os.path.abspath(companion(out, '.frame')))}",
        # abspath() would drop a trailing '.' on Windows: add it after.
        f"local SHOT = {lua_str(os.path.abspath(companion(out, '')) + '.')}",
        "local log = io.open(LOG, 'w')",
        f"local frame, seq, cur = 0, 0, {lua_str(first_phase(script))}",
        "local pad = PCSX.SIO0.slots[1].pads[1]",
        "local B = PCSX.CONSTS.PAD.BUTTON",
        "local function L(s) log:write(frame .. ' ' .. s .. '\\n'); log:flush() end",
        "local function state(ended)",
        "  seq = seq + 1",
        "  local f = io.open(STATE, 'w')",
        "  f:write(seq .. ' ' .. frame .. ' ' .. cur .. ' ' .. ended .. '\\n')",
        "  f:close()",
        "end",
        "local F = {}",
        "function F.press(b) pad.setOverride(B[b]); L('press ' .. b) end",
        "function F.release(b) pad.clearOverride(B[b]); L('release ' .. b) end",
        "function F.shot(name)",
        "  local s = PCSX.GPU.takeScreenShot()",
        "  local bpp = (tonumber(s.bpp) == 0) and 16 or 24",
        "  local f = Support.File.open(SHOT .. name .. '.raw', 'TRUNCATE')",
        "  f:writeMoveSlice(s.data); f:close()",
        "  L('shot ' .. name .. ' ' .. s.width .. ' ' .. s.height .. ' ' .. bpp)",
        "end",
        "function F.phase(name)",
        "  cur = name; state(0); L('phase ' .. name); PCSX.pauseEmulator()",
        "end",
        "function F.finish()",
        "  state(1); L('finish -'); PCSX.pauseEmulator()",
        "end",
        "local acts = {",
    ]
    for frame in sorted(by_frame):
        lines.append(f"  [{frame}] = {{{', '.join(by_frame[frame])}}},")
    lines += [
        "}",
        "state(0); L('phase ' .. cur)",
        "-- the latest pause (a breakpoint, ^C or ours): `count frame cycle`;",
        "-- an unchanged CPU cycle count means no instruction ran in between",
        "local npause = 0",
        "local function cycle()",
        "  local ok, c = pcall(PCSX.getCPUCycles)",
        "  local s = ok and c ~= nil and tostring(c):gsub('%D', '') or ''",
        "  return s ~= '' and s or '-'",
        "end",
        "_G.gdbtrace_pause = PCSX.Events.createEventListener(",
        "  'ExecutionFlow::Pause', function()",
        "    npause = npause + 1",
        "    local f = io.open(PAUSED, 'w')",
        "    f:write(npause .. ' ' .. frame .. ' ' .. cycle() .. '\\n'); f:close()",
        "  end)",
        "_G.gdbtrace_vsync = PCSX.Events.createEventListener('GPU::Vsync',",
        "  function()",
        "    frame = frame + 1",
        "    for _, a in ipairs(acts[frame] or {}) do",
        "      local ok, err = pcall(F[a[1]], a[2])",
        "      if not ok then",
        "        L('error ' .. a[1] .. ' ' .. a[2] .. ' ' .. tostring(err))",
        "      end",
        "    end",
        "  end)",
        "",
    ]
    path = companion(out, ".script.lua")
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))
    return path


def check_log(script: dict, out: str) -> list[str]:
    """Problems between the script and what its log says ran: missing or
    failed actions, screenshots without a PNG. Empty when it ran as
    written."""
    try:
        with open(companion(out, ".script.log"), encoding="utf-8") as f:
            log = [line.split() for line in f]
    except OSError:
        return ["no script log"]
    done = {(int(p[0]), p[1], p[2]) for p in log
            if len(p) >= 3 and p[1] in ACTION_ORDER}
    problems = [" ".join(p) for p in log if len(p) > 1 and p[1] == "error"]
    for frame, act, arg in planned_actions(script):
        if (frame, act, arg) not in done:
            problems.append(f"not run: frame {frame} {act} {arg}")
        if act == "shot" and not os.path.isfile(
                companion(out, f".{arg}.png")):
            problems.append(f"no PNG for screenshot {arg}")
    if (0, "phase", first_phase(script)) not in done:
        problems.append("the script never loaded")
    return problems


def read_pause(out: str):
    """(count, frame, cycle) of the emulator's latest pause, or None if no
    pause was seen (the caller decides what that means). cycle is the CPU
    cycle counter as a decimal string, None when Redux did not expose it."""
    try:
        with open(companion(out, ".frame"), encoding="utf-8") as f:
            count, frame, cyc = f.read().split()
        return int(count), int(frame), (cyc if cyc.isdigit() else None)
    except (OSError, ValueError):
        return None


def read_pause_frame(out: str) -> int | None:
    p = read_pause(out)
    return p[1] if p else None


def read_state(out: str):
    """(seq, frame, phase, ended) from the Lua state file, or None."""
    try:
        with open(companion(out, ".state"), encoding="utf-8") as f:
            seq, frame, phase, ended = f.read().split()
        return int(seq), int(frame), phase, ended == "1"
    except (OSError, ValueError):
        return None


def png(path: str, width: int, height: int, rgb: bytes):
    def chunk(tag, data):
        c = struct.pack(">I", len(data)) + tag + data
        return c + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
    rows = b"".join(b"\0" + rgb[y * width * 3:(y + 1) * width * 3]
                    for y in range(height))
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2,
                                           0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(rows, 9)))
        f.write(chunk(b"IEND", b""))


def convert_shots(out: str) -> list[str]:
    """Turn the Lua's raw screenshots into PNGs (raw files removed)."""
    done = []
    try:
        with open(companion(out, ".script.log"), encoding="utf-8") as f:
            log = f.read().splitlines()
    except OSError:
        return done
    for line in log:
        p = line.split()
        if len(p) != 6 or p[1] != "shot":
            continue
        name, w, h, bpp = p[2], int(p[3]), int(p[4]), int(p[5])
        raw = companion(out, f".{name}.raw")
        if not os.path.isfile(raw):
            continue
        with open(raw, "rb") as f:
            data = f.read()
        if len(data) < w * h * bpp // 8:
            raise ValueError(f"screenshot {name}: {len(data)} "
                             f"bytes, {w}x{h}x{bpp} needs {w * h * bpp // 8}")
        if bpp == 16:  # PS1 15-bit: R bits 0-4, G 5-9, B 10-14
            rgb = bytearray()
            for (v,) in struct.iter_unpack("<H", data[:w * h * 2]):
                rgb += bytes(((v & 31) * 255 // 31, (v >> 5 & 31) * 255 // 31,
                              (v >> 10 & 31) * 255 // 31))
        else:
            rgb = data[:w * h * 3]
        dst = companion(out, f".{name}.png")
        png(dst, w, h, bytes(rgb))
        os.remove(raw)
        done.append(dst)
    return done


# ------------------------------------------------------------ the manifest

def cue_tracks(cue: str) -> list[str]:
    base = os.path.dirname(os.path.abspath(cue))
    files = []
    with open(cue, encoding="latin-1") as f:
        for line in f:
            m = re.match(r'\s*FILE\s+"([^"]+)"', line) or \
                re.match(r"\s*FILE\s+(\S+)", line)
            if m:
                files.append(os.path.join(base, m.group(1)))
    return files


def redux_version(exe: str) -> dict:
    vj = os.path.join(os.path.dirname(os.path.abspath(exe)), "version.json")
    try:
        with open(vj, encoding="utf-8") as f:
            v = json.load(f)
        return {k: v.get(k) for k in ("version", "changeset", "buildId",
                                      "channel")}
    except (OSError, ValueError):
        return {"version": "unknown (no version.json next to the exe)"}


def git_state(repo: str) -> dict:
    def git(*a):
        try:
            r = subprocess.run(["git", "-C", repo, *a], capture_output=True,
                               text=True)
        except OSError as e:
            return None, f"git unavailable: {e}"
        if r.returncode:
            return None, f"git failed: {r.stderr.strip()[:200]}"
        return r.stdout.strip(), None
    head, err = git("rev-parse", "HEAD")
    status, err2 = git("status", "--porcelain", "--", "tools/trace")
    rec = {"head": head, "tools_trace_status": status or "clean"}
    if err or err2:
        rec = {"head": head, "error": err or err2}
    return rec


def build_rec(map_path: str, elf: str | None) -> dict:
    d = os.path.dirname(os.path.abspath(map_path))
    exe = os.path.join(d, "bb2.exe")
    rec = {"map": file_rec(map_path),
           "elf": file_rec(elf or os.path.join(d, "bb2.elf")),
           "exe": file_rec(exe)}
    rec["oracle_match"] = rec["exe"].get("sha1") == ORACLE_SHA1
    return rec


def inputs_record(args, start: dict, repo: str) -> dict:
    """Everything the run depends on, hashed before launch."""
    here = os.path.dirname(os.path.abspath(__file__))
    m = {
        "tool": {f: file_rec(os.path.join(here, f), repo)
                 for f in ("gdbtrace.py", "runkit.py")},
        "git": git_state(repo),
        "python": {"version": sys.version, "executable": sys.executable},
        "argv": sys.argv,
        "build": build_rec(args.map, args.elf),
        "spec": file_rec(args.spec),
        "start": start,
    }
    for n in (1, 2):
        card = start.get(f"memcard{n}")
        if card:
            card["sha1_at_start"] = sha1_file(card["card"])
    missing = []
    if args.launch == "pcsx-redux":
        d = os.path.dirname(os.path.abspath(args.emulator))
        m["emulator"] = {"name": "pcsx-redux", **redux_version(args.emulator),
                         "exe": file_rec(os.path.abspath(args.emulator)),
                         "main": file_rec(os.path.join(d, "pcsx-redux.main")),
                         "dlls": [file_rec(os.path.join(d, n)) for n in
                                  (sorted(os.listdir(d)) if os.path.isdir(d)
                                   else [])
                                  if n.lower().endswith(".dll")],
                         "settings": "defaults (-safe) + command line"
                         if args.lua else "the portable dir's pcsx.json"}
        if args.bios:
            m["bios"] = file_rec(args.bios)
        else:
            m["bios"] = {"builtin": "emulator default (OpenBIOS); which "
                         "image is not verified"}
            missing.append("an explicit --bios")
        if not args.lua:
            missing.append("script (start state and inputs)")
    else:
        m["emulator"] = {"name": args.launch or "attached, not launched by "
                         "gdbtrace", "exe": args.emulator}
        missing += ["emulator version", "BIOS", "start state and inputs"]
    if args.cue:
        m["disc"] = {"cue": file_rec(args.cue),
                     "tracks": [file_rec(t) for t in cue_tracks(args.cue)]}
    else:
        missing.append("disc")
    if args.lua:
        m["script"] = {"lua": file_rec(args.lua)}
    if "error" in m["git"]:
        missing.append("tool version (git)")
    elif m["git"]["tools_trace_status"] != "clean":
        missing.append("committed trace tools (tools/trace is dirty)")
    if not m["build"]["oracle_match"]:
        missing.append("a build that is the oracle")
    elif not elf_matches_exe(m["build"]["elf"]["path"],
                             m["build"]["exe"]["path"]):
        missing.append("an ELF (symbols) built with this EXE")
    if m.get("disc") and not m["disc"]["tracks"]:
        missing.append("disc tracks (the cue names no FILE)")
    if args.launch == "pcsx-redux" and str(
            m["emulator"].get("version", "")).startswith("unknown"):
        missing.append("emulator version (version.json)")
    missing += [f"input file {r['path']}" for r in missing_files(m)]
    m["q124_complete"] = not missing
    if missing:
        m["q124_missing"] = missing
    return m


def missing_files(node):
    """Every file record in a manifest subtree marked missing."""
    if isinstance(node, dict):
        if node.get("missing"):
            yield node
        for v in node.values():
            yield from missing_files(v)
    elif isinstance(node, list):
        for v in node:
            yield from missing_files(v)


def read_head(path: str, n: int, offset: int = 0) -> bytes | None:
    try:
        with open(path, "rb") as f:
            f.seek(offset)
            return f.read(n)
    except OSError:
        return None


def elf_sections(path: str):
    """(addr, size, flags, file_offset, type) of each ELF32 LE section."""
    with open(path, "rb") as f:
        data = f.read()
    if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 1:
        return data, []
    shoff, = struct.unpack_from("<I", data, 0x20)
    shentsize, shnum = struct.unpack_from("<HH", data, 0x2E)
    out = []
    for i in range(shnum):
        sh = struct.unpack_from("<10I", data, shoff + i * shentsize)
        out.append((sh[3], sh[5], sh[2], sh[4], sh[1]))
    return data, out


EXE_LOAD, EXE_HEADER = 0x80010000, 0x800


MAP_TEXT = re.compile(r"^\s*\.text\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+\S")


def map_text_end(map_path: str) -> int | None:
    """End of the last object's .text in the link map (the ELF has one
    .main section for .rodata + .text + .data, so it cannot say)."""
    end = None
    with open(map_path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = MAP_TEXT.match(line)
            if m and int(m.group(2), 16):
                e = int(m.group(1), 16) + int(m.group(2), 16)
                end = e if end is None else max(end, e)
    return end


def expected_memory(args) -> dict:
    """Bytes to find in emulated memory, read before launch: the BIOS ROM
    head, and the EXE image from its load address to the end of .text
    (.rodata then .text), compared at the EXE entry before any game code
    runs (the game later patches some of its own code)."""
    exp = {}
    if args.bios:
        exp["bios"] = read_head(args.bios, 0x80000)  # the whole 512 KiB ROM
    d = os.path.dirname(os.path.abspath(args.map))
    try:
        end = map_text_end(args.map)
    except OSError:
        return exp
    if end and end > EXE_LOAD:
        exp["exe"] = read_head(os.path.join(d, "bb2.exe"), end - EXE_LOAD,
                               EXE_HEADER)
    return exp


def elf_matches_exe(elf: str, exe: str) -> bool:
    """The ELF that names symbols is the build of this EXE: every loaded
    section's bytes sit at its address in the EXE image."""
    try:
        data, secs = elf_sections(elf)
        with open(exe, "rb") as f:
            image = f.read()[EXE_HEADER:]
    except OSError:
        return False
    loaded = [(a, n, off) for a, n, fl, off, t in secs
              if fl & 2 and t != 8 and n and a >= EXE_LOAD]  # ALLOC, !NOBITS
    if not loaded:
        return False
    return all(data[off:off + n] == image[a - EXE_LOAD:a - EXE_LOAD + n]
               for a, n, off in loaded)


def finish_manifest(args, inputs: dict, emu_cmd, result: dict) -> str:
    """Write <out>.manifest.json: inputs (hashed before launch), the
    emulator command, the outcome, and the SHA1 of every output."""
    m = dict(inputs)
    if emu_cmd:
        m["emulator"] = {**m["emulator"], "command": emu_cmd}
    m["result"] = result
    missing = []
    if result.get("outcome") != "ok" or (args.lua and
                                         not result.get("script_ended")):
        missing.append("a run that reached its end")
    outputs = [args.out] + convert_shots(args.out)
    if args.lua:
        if result.get("armed_frame") != 0:
            missing.append("breakpoints armed at frame 0")
        if result.get("bios_in_memory") != "matches":
            missing.append("the BIOS in memory is the recorded file")
        if result.get("exe_in_memory") != "matches":
            missing.append("the disc's EXE in RAM (.rodata+.text) at its "
                           "entry is the oracle build")
        if result.get("ambiguous_stops"):
            missing.append("no watch access ambiguous with a script pause")
        if result.get("dedupe_without_cycle"):
            missing.append("duplicate stops told apart by the CPU cycle count")
        if not result.get("canary_hit"):
            missing.append("the canary breakpoint (EXE entry) fired")
        problems = check_log(args.spec_obj["script"], args.out)
        m["script"] = {**m["script"], "problems": problems}
        if problems:
            missing.append("a script that ran as written")
    if missing:
        m["q124_complete"] = False
        m["q124_missing"] = m.get("q124_missing", []) + missing
    for x in (".state", ".frame"):
        if os.path.exists(companion(args.out, x)):
            os.remove(companion(args.out, x))
    if args.lua:
        outputs.append(companion(args.out, ".script.log"))
    for n in (1, 2):
        mc = companion(args.out, f".mc{n}.mcd")
        if os.path.exists(mc):
            outputs.append(mc)
    m["outputs_note"] = ("cards are hashed as on disk after the run; the "
                         "emulator was " + ("left running" if
                                            args.keep_emulator else
                                            "stopped first"))
    m["outputs"] = [file_rec(p) for p in outputs]
    path = companion(args.out, ".manifest.json")
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        json.dump(m, f, indent=2)
        f.write("\n")
    return path
