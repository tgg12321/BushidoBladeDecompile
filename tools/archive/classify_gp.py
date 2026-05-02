#!/usr/bin/env python3
"""Classify remaining text1a.c stubs by GP-relative usage"""
import os

funcs = [
    "func_800401CC","func_80040304","func_80040400","func_80040594","func_800408F8",
    "func_80040A78","func_80040B44","func_80040CB8","func_80040D48","func_80041188",
    "func_80041398","func_80041430","func_80041688","func_800417D0",
    "func_80041988","func_80041AC8","func_80041BF4","func_80041E10","func_80041EB0",
    "func_800422BC","func_80042504","func_80042684","func_80042874","func_80042A88",
    "func_80042C80","func_80042FA0","func_800430E4","func_800432A0","func_800433E4",
    "func_80043454","func_80043BD0","func_80043C7C","func_80043D34","func_80043DE0",
    "func_80044010","func_80044098","func_80044100","func_80044170","func_8004428C",
    "func_80044378","func_80044504","func_80044670","func_8004473C","func_80044800",
    "func_80044B30","func_80044CCC","func_80044FA0","func_800450F4","func_80045294",
    "func_800453E0","func_80045510","func_800455AC","func_80045600","func_8004574C",
    "func_80045878","func_80045AA4","func_80045B68","func_80046048","func_800460E4",
    "func_800464C4","func_8004659C","func_800466C0"
]

basedir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
no_gp = []
has_gp = []

for f in funcs:
    path = os.path.join(basedir, "asm", "funcs", f + ".s")
    with open(path) as fh:
        text = fh.read()
    lines = text.count("\n")
    if "gp_rel" in text:
        has_gp.append((f, lines))
    else:
        no_gp.append((f, lines))

print("No GP-relative (%d functions) - CAN decompile with -G0:" % len(no_gp))
for f, lines in sorted(no_gp, key=lambda x: x[1]):
    print("  %s: %d lines" % (f, lines))

print("\nHas GP-relative (%d functions) - NEED -G8 to match:" % len(has_gp))
for f, lines in sorted(has_gp, key=lambda x: x[1]):
    print("  %s: %d lines" % (f, lines))
