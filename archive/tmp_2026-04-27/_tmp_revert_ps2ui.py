#!/usr/bin/env python3
"""Revert 9 low-confidence PS2 UI renames back to func_XXXXXXXX."""
import re, sys, glob
from pathlib import Path

ROOT = Path("/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile")
DRY = "--apply" not in sys.argv

# new_name -> old_name  (reverse of what we applied)
REVERT = {
    "DispPracticeMenuTex_A":  "func_8001B748",
    "DispPracticeMenuTex_B":  "func_80020E74",
    "DispPracticeMenuTex_C":  "func_800290B8",
    "DispStuff":              "func_80083E9C",
    "DispSchoolBG":           "func_8002EBDC",
    "DispSleepMenuTex":       "func_8003D52C",
    "DispSamnailWindow":      "func_800344B4",
    "DispUpdateStatusMessage":"func_800889D4",
    "SetCurrentCursor":       "func_8003C714",
}

new_names = sorted(REVERT.keys(), key=len, reverse=True)
pattern = re.compile(r"\b(" + "|".join(re.escape(k) for k in new_names) + r")\b")

def replace_all(text):
    return pattern.sub(lambda m: REVERT[m.group(0)], text)

def replace_asm(text):
    lines = []
    for line in text.splitlines(keepends=True):
        s = line.lstrip()
        if s.startswith("glabel ") or s.startswith("endlabel "):
            lines.append(line)  # skip — handled separately for stub renames
        else:
            lines.append(pattern.sub(lambda m: REVERT[m.group(0)], line))
    return "".join(lines)

total = 0
c_files  = list((ROOT / "src").glob("*.c"))
asm_files = list((ROOT / "asm" / "funcs").glob("*.s")) + list((ROOT / "asm").glob("*.s"))
data_files = list((ROOT / "asm" / "data").glob("*.s"))
sdata_files = [p for p in [ROOT / "sdata_funcs.txt", ROOT / "sdata_exclude.txt"] if p.exists()]

for path in sorted(c_files + asm_files + data_files + sdata_files):
    text = path.read_text(encoding="utf-8", errors="replace")
    new_text = replace_all(text) if (path.suffix in (".c", ".txt") or path.parent.name == "data") else replace_asm(text)
    if new_text != text:
        n = sum(1 for _ in pattern.finditer(text))
        print(f"  [{n:3d}] {path.relative_to(ROOT)}")
        total += n
        if not DRY:
            path.write_text(new_text, encoding="utf-8")

# Rename .s stub files back (new_name.s -> old_name.s, update glabel)
funcs_dir = ROOT / "asm" / "funcs"
glabel_re = re.compile(r"^(glabel|endlabel) (\w+)\b", re.MULTILINE)

for new_name, old_name in REVERT.items():
    new_path = funcs_dir / f"{new_name}.s"
    old_path = funcs_dir / f"{old_name}.s"
    if not new_path.exists():
        continue
    text = new_path.read_text(encoding="utf-8", errors="replace")
    new_text = glabel_re.sub(
        lambda m: f"{m.group(1)} {REVERT.get(m.group(2), m.group(2))}", text)
    print(f"  [stub revert] {new_path.name} -> {old_path.name}")
    if not DRY:
        new_path.write_text(new_text, encoding="utf-8")
        new_path.rename(old_path)

mode = "DRY RUN" if DRY else "APPLIED"
print(f"\n{mode}: {total} text replacements + {len(REVERT)} stub file reverts")
if DRY:
    print("Pass --apply to write.")
