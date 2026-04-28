#!/usr/bin/env python3
"""Add saTan0KiWareMoveB subst rules to regfix.txt"""
import pathlib

p = pathlib.Path("/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/regfix.txt")
text = p.read_text()

# Find the comment line to insert after
marker = "# saTan0KiWareMoveB: dist register a1->a0 (6 positions)"
if marker not in text:
    print("ERROR: marker not found")
    exit(1)

D = "$"
rules = []
for idx in [100, 101, 105, 109, 111, 121]:
    rules.append(f'saTan0KiWareMoveB: subst "\\{D}5" "{D}4" @ {idx}')

insert_text = "\n".join(rules) + "\n"
text = text.replace(marker + "\n", marker + "\n" + insert_text)

p.write_text(text)
print("OK: added 6 subst rules")
