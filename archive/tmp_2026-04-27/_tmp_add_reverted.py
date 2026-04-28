#!/usr/bin/env python3
"""Add kengo:LOW comments for the 9 reverted PS2 UI stubs."""
import re, sys
from pathlib import Path

ROOT = Path("/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile")
DRY = "--apply" not in sys.argv

REVERTED = {
    "func_8001B748": ("su_menu_tuto/_DispPracticeMenuTex",  231, "PS2 UI — size coincidence, different stack frames"),
    "func_80020E74": ("su_menu_tuto/_DispPracticeMenuTex",  231, "PS2 UI — size coincidence, different stack frames"),
    "func_800290B8": ("su_menu_tuto/_DispPracticeMenuTex",  231, "PS2 UI — size coincidence, different stack frames"),
    "func_80083E9C": ("su_menu_ending/_DispStuff",          209, "PS2 UI — reverted"),
    "func_8002EBDC": ("su_menu_single/_DispSchoolBG",       188, "PS2 UI — reverted"),
    "func_8003D52C": ("su_menu_home/_DispSleepMenuTex",     146, "PS2 UI — reverted"),
    "func_800344B4": ("su_menu_vs/_DispSamnailWindow",      149, "PS2 UI — reverted"),
    "func_800889D4": ("su_menu_home/_DispUpdateStatusMessage", 206, "PS2 UI — reverted"),
    "func_8003C714": ("su_menu_edit/_SetCurrentCursor",     104, "PS2 UI — reverted"),
}

comments = {}
for func, (name, insns, reason) in REVERTED.items():
    comments[func] = f"/* kengo:LOW  |  {name}  |  {insns}i  |  {reason} */"

total = 0
for c_path in sorted((ROOT / "src").glob("*.c")):
    text = c_path.read_text(encoding="utf-8", errors="replace")
    new_lines = []
    changed = False
    for line in text.splitlines(keepends=True):
        new_lines.append(line)
        m = re.match(r'(\s*)INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*(\w+)\s*\)\s*;', line)
        if m:
            func_name = m.group(2)
            if func_name in comments:
                indent = m.group(1)
                new_lines.append(indent + comments[func_name] + "\n")
                total += 1
                changed = True
    if changed:
        new_text = "".join(new_lines)
        print(f"  {c_path.relative_to(ROOT)}")
        if not DRY:
            c_path.write_text(new_text, encoding="utf-8")

mode = "DRY RUN" if DRY else "APPLIED"
print(f"\n{mode}: {total} LOW comments added")
if DRY:
    print("Pass --apply to write.")
