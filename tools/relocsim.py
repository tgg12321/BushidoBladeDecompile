#!/usr/bin/env python3
"""Buildless link simulation for ONE function's HI16/LO16 data relocations.

Applies the MIPS REL addend rule -- AHL = (AHI << 16) + (short)ALO, then
HI16 field = %high(S+AHL), LO16 field = (S+AHL) & 0xffff -- to our sandbox
object, and compares the resulting words against the target's words from
asm/funcs/<func>.s.

Purpose: prove (or disprove) that a scored .o-vs-.o difference in relocated
operands is LINK-NEUTRAL, without needing the build lock.

Usage: relocsim.py <stem> <func>
"""
import re, subprocess, sys
from pathlib import Path

stem, func = sys.argv[1], sys.argv[2]
OBJ = Path(f"tmp/sandbox/{func}/{stem}.o")
TGT = Path(f"asm/funcs/{func}.s")

# ---- symbol values (absolutes defined in the splat symbol files) ----------
syms = {}
for f in ("undefined_syms_auto.txt", "named_syms.txt", "symbol_addrs.txt"):
    p = Path(f)
    if not p.exists():
        continue
    for line in p.read_text(errors="replace").splitlines():
        m = re.match(r"^\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;", line)
        if m:
            syms.setdefault(m.group(1), int(m.group(2), 16))

# ---- target words --------------------------------------------------------
tw, base = [], None
for line in TGT.read_text().splitlines():
    m = re.match(r"^\s*/\*\s*\S+\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s*\*/", line)
    if m:
        if base is None:
            base = int(m.group(1), 16)
        tw.append(int.from_bytes(bytes.fromhex(m.group(2)), "little"))

# ---- our words + the function's offset in the object ---------------------
dis = subprocess.run(["mipsel-linux-gnu-objdump", "-d", str(OBJ)],
                     capture_output=True, text=True).stdout
ow, offs, inside = [], [], False
for line in dis.splitlines():
    if line.endswith(">:") and "<" in line:
        inside = line.split("<")[1].rstrip(">:") == func
        continue
    if not inside:
        continue
    m = re.match(r"^\s*([0-9a-f]+):\s+([0-9a-f]{8})\s", line)
    if m:
        offs.append(int(m.group(1), 16))
        ow.append(int(m.group(2), 16))

# ---- relocations, keyed by section offset --------------------------------
rel = {}
sec = None
for line in subprocess.run(["mipsel-linux-gnu-objdump", "-r", str(OBJ)],
                           capture_output=True, text=True).stdout.splitlines():
    m = re.match(r"^RELOCATION RECORDS FOR \[(\S+)\]", line)
    if m:
        sec = m.group(1)
        continue
    m = re.match(r"^([0-9a-f]+)\s+(\S+)\s+(\S+)", line)
    if m and sec == ".text":
        rel[int(m.group(1), 16)] = (m.group(2), m.group(3))

# ---- apply HI16/LO16 pairs ----------------------------------------------
out = list(ow)
unresolved, applied = [], 0
for i, off in enumerate(offs):
    if off not in rel:
        continue
    kind, sym = rel[off]
    if kind == "R_MIPS_26":
        if sym == ".text":
            # section-relative: the addend IS the .text offset. Our function
            # starts at object offset offs[0] and links at `base`.
            tgt = base - offs[0]
        else:
            m = re.fullmatch(r"(?:func_)?([0-9A-F]{8})", sym)
            tgt = syms.get(sym) or (int(m.group(1), 16) if m else None)
        if tgt is None:
            unresolved.append(sym)
            continue
        A = (ow[i] & 0x03FFFFFF) << 2
        out[i] = (ow[i] & 0xFC000000) | (((tgt + A) >> 2) & 0x03FFFFFF)
        applied += 1
        continue
    if kind not in ("R_MIPS_HI16", "R_MIPS_LO16"):
        continue
    if sym not in syms:
        # C-defined data: splat names it D_<hex address>, and the linked build
        # is SHA1-identical, so the name's address is the placed address.
        m = re.fullmatch(r"D_([0-9A-F]{8})", sym)
        if not m:
            unresolved.append(sym)
            continue
        syms[sym] = int(m.group(1), 16)
    S = syms[sym]
    if kind == "R_MIPS_LO16":
        AHL = ow[i] & 0xFFFF
        if AHL >= 0x8000:
            AHL -= 0x10000
        out[i] = (ow[i] & 0xFFFF0000) | ((S + AHL) & 0xFFFF)
    else:
        # AHL needs the paired LO16's addend; find the next LO16 on same sym
        AHL = 0
        for j in range(i + 1, len(offs)):
            r2 = rel.get(offs[j])
            if r2 and r2[0] == "R_MIPS_LO16" and r2[1] == sym:
                lo = ow[j] & 0xFFFF
                AHL = lo - 0x10000 if lo >= 0x8000 else lo
                break
        v = S + AHL
        hi = ((v - ((v & 0xFFFF) - (0x10000 if (v & 0xFFFF) >= 0x8000 else 0))) >> 16) & 0xFFFF
        out[i] = (ow[i] & 0xFFFF0000) | hi
    applied += 1

print(f"{func}: {len(ow)} ours / {len(tw)} target insns, {applied} HI16/LO16 relocs applied")
if unresolved:
    print(f"  unresolved symbols (skipped): {sorted(set(unresolved))}")

pre = sum(1 for a, b in zip(ow, tw) if a != b)
post = [i for i, (a, b) in enumerate(zip(out, tw)) if a != b]
print(f"  raw word mismatches BEFORE relocation : {pre}")
print(f"  raw word mismatches AFTER  relocation : {len(post)}")
for i in post[:25]:
    print(f"    [{i:3d}] ours {out[i]:08x}  target {tw[i]:08x}")
