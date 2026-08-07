#!/usr/bin/env python3
"""Regenerate movovl/splat.movovl.yaml + movovl/symbol_addrs.movovl.txt from the
committed movovl_scan evidence (docs/naming/movovl_scan/).

The yaml/symbol files ARE committed — run this only if the scan evidence or the
game-function census changes. All output written with LF endings.

Usage (from the repo root, any Python 3):  python3 movovl/tools/gen_splat_config.py
"""
import os, json, collections

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SCAN = os.path.join(REPO, "docs", "naming", "movovl_scan")
OUT = os.path.join(REPO, "movovl")

TADDR = 0x801D8800
def foff(va): return va - TADDR + 0x800

P = json.load(open(os.path.join(SCAN, "placements.json")))
mods = [(int(a, 16), int(b, 16), lib, mod) for a, b, lib, mod in P["modules"]]

def owner_mod(va):
    for a, b, lib, mod in mods:
        if a <= va < b:
            return mod
    return "X"

rows = []
used = collections.Counter()
for line in open(os.path.join(SCAN, "symbols.txt")):
    if line.startswith("#") or not line.strip():
        continue
    parts = line.split()
    va = int(parts[0], 16)
    name = parts[2]
    if used[name]:
        name = "%s_%s" % (name, owner_mod(va))   # e.g. memclr_INTR_VB
    used[parts[2]] += 1
    rows.append((va, name))

# Game-code function starts — independent census (movovl/tools/census_game_funcs.py):
# 11 prologue functions + 2 frameless leaves (0x801D9964, 0x801D9D2C) + 3 stubs
# (gp-getter 0x801DA070, empty 0x801DA07C, entry bootstrap 0x801DA084).
GAME_FUNCS = [
    0x801D919C, 0x801D91CC, 0x801D971C, 0x801D98DC, 0x801D9964, 0x801D99E4,
    0x801D9A6C, 0x801D9B74, 0x801D9C0C, 0x801D9D2C, 0x801D9DC4, 0x801D9E70,
    0x801D9F98, 0x801DA070, 0x801DA07C, 0x801DA084,
]
for va in GAME_FUNCS:
    rows.append((va, "func_%08X" % va))
rows.sort()

with open(os.path.join(OUT, "symbol_addrs.movovl.txt"), "w", newline="\n") as f:
    f.write("// MOVOVL.EXE symbol map — seeded from docs/naming/movovl_scan/symbols.txt\n")
    f.write("// (264 bit-verbatim PsyQ library symbols) + the 16 game-code function\n")
    f.write("// starts verified independently (see docs/movovl/ notes). Statics that\n")
    f.write("// collide across modules carry a _<MODULE> suffix (memclr x3).\n")
    for va, name in rows:
        f.write("%s = 0x%08X; // type:func\n" % (name, va))

Y = []
Y.append("# splat config for MOVOVL.EXE — the FMV/MDEC playback overlay (SLUS-00663).")
Y.append("# Parallel scaffolding to the main-EXE splat.yaml; nothing here touches the")
Y.append("# main build. Run from the repo root:  python -m splat split movovl/splat.movovl.yaml")
Y.append("# The binary movovl/MOVOVL.EXE is gitignored — copy it from disc/STR/MOVOVL.EXE.")
Y.append("name: Bushido Blade 2 (USA) MOVOVL.EXE overlay")
Y.append("sha1: a1307dbebefca0b057e02509207d00f6225e13e4")
Y.append("options:")
Y.append("  basename: movovl")
Y.append("  target_path: MOVOVL.EXE")
Y.append("  elf_path: build/movovl.elf")
Y.append("  base_path: .   # relative to this yaml -> movovl/")
Y.append("  platform: psx")
Y.append("  compiler: PSYQ")
Y.append("")
Y.append("  asm_path: asm")
Y.append("  src_path: src")
Y.append("  build_path: build")
Y.append("")
Y.append("  ld_script_path: movovl.ld")
Y.append("")
Y.append("  find_file_boundaries: False")
Y.append("  gp_value: 0x801F65D0")
Y.append("")
Y.append("  o_as_suffix: True")
Y.append("  use_legacy_include_asm: False")
Y.append("")
Y.append('  section_order: [".rodata", ".text", ".data", ".bss"]')
Y.append('  auto_link_sections: [".data", ".rodata", ".bss"]')
Y.append("")
Y.append("  symbol_addrs_path:")
Y.append("    - symbol_addrs.movovl.txt")
Y.append("")
Y.append("  undefined_funcs_auto_path: undefined_funcs_auto.movovl.txt")
Y.append("  undefined_syms_auto_path: undefined_syms_auto.movovl.txt")
Y.append("")
Y.append("  subalign: 2")
Y.append("")
Y.append("  string_encoding: ASCII")
Y.append("  data_string_encoding: ASCII")
Y.append("  rodata_string_guesser_level: 2")
Y.append("  data_string_guesser_level: 2")
Y.append("")
Y.append("segments:")
Y.append("  - name: header")
Y.append("    type: header")
Y.append("    start: 0x0")
Y.append("")
Y.append("  - name: movovl")
Y.append("    type: code")
Y.append("    start: 0x800")
Y.append("    vram: 0x801D8800")
Y.append("    subsegments:")
Y.append("      - [0x800, rodata, rodata_800]      # strings/constants before .text (PsyQ order)")
Y.append("      - [0x%X, asm, game]               # game code: 0x801D919C..0x801DA094 (16 funcs)" % foff(0x801D919C))
for a, b, lib, mod in mods:
    Y.append("      - [0x%X, asm, %s_%s]" % (foff(a), lib.lower(), mod.lower()))
Y.append("      - [0x%X, data, data_C268]         # lib .rodata/.data tail (VLC tables, strings, sdata)" % foff(0x801E3C68))
Y.append("")
Y.append("  - [0x1E800]")
Y.append("")

with open(os.path.join(OUT, "splat.movovl.yaml"), "w", newline="\n") as f:
    f.write("\n".join(Y))

print("wrote movovl/splat.movovl.yaml (%d module subsegments) and symbol_addrs (%d rows)"
      % (len(mods), len(rows)))
