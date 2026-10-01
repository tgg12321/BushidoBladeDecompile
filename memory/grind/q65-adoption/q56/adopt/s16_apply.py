#!/usr/bin/env python3
"""Step 16 (Q65 adoption): tooling and records follow the retired lists and the moved functions.
Byte-neutral (no build input changes).
  - tools that listed sdata_syms.txt / sdata_funcs.txt / sdata_exclude.txt as gate / key / root files
    drop them (grindlib GATE_FILES, check_root_cleanliness, naming_wave, data_wave, desync_audit; the
    exists-guarded legacy tools rename_funcs / apply_kengo_names / kengo_globals are left as they are);
  - records that name a function's source file follow steps 2-11's moves (queue.json items, grind
    state.json, tools/canonical_asm_regions.json, tools/grinder/scope_allow.txt src/ tokens): run
    relocate_records.py, which derives every function's file from the tree itself.
usage: s16_apply.py <tree>"""
import os, re, subprocess, sys
R = sys.argv[1]
os.chdir(R)
H = os.path.dirname(os.path.abspath(__file__))
NL = "\n"


def edit(p, pairs):
    t = open(p).read()
    for a, b in pairs:
        if t.count(a) != 1:
            print(f"NOTE {p}: {t.count(a)} matches for {a!r}")
            continue
        t = t.replace(a, b)
    open(p, "w", newline=NL).write(t)


edit("tools/grinder/grindlib.py", [('    "sdata_funcs.txt",\n', "")])
edit("tools/check_root_cleanliness.py", [('    "sdata_exclude.txt", "sdata_funcs.txt", "sdata_syms.txt",\n', "")])
edit("tools/naming_wave.py", [('    "sdata_funcs.txt",\n    "sdata_exclude.txt",\n', "")])
edit("tools/data_wave.py", [
    ("# sdata_syms.txt is load-bearing: maspsx (--sdata-syms) emits %gp_rel only for symbols listed\n"
     "# there, so a C-side rename that leaves the old spelling in it silently turns a gp-relative\n"
     "# access into lui+lw and shifts every later byte (2026-09-24 sweep verifier finding).\n",
     "# (The sdata_syms / sdata_funcs / sdata_exclude lists were retired by owner ruling Q65: gp now\n"
     "# follows each file's own definitions, which a rename changes together with every use.)\n"),
    ('LIST_FILES = ["volatile_extern_allowlist.txt", "sdata_funcs.txt", "sdata_exclude.txt", "sdata_syms.txt"]',
     'LIST_FILES = ["volatile_extern_allowlist.txt"]')])
edit("tools/desync_audit.py", [('    "sdata_funcs.txt": r"^\\s*([A-Za-z_]\\w*)\\s*$",\n'
                                '    "sdata_exclude.txt": r"^\\s*([A-Za-z_]\\w*)\\s*$",\n', "")])
# the files steps 2-11 split / merged / cut / moved
OLD = ["text1a_c", "code6cac_b_tu2", "text1b", "code6cac_b2_pre", "replay_camera_rob_back_loose2", "config",
       "text1a_c2", "text1a_b", "text1a_b_pre_rodata", "sound", "text1b_tu2"]
r = subprocess.run([sys.executable, f"{H}/relocate_records.py", R] + OLD, capture_output=True, text=True)
print(r.stdout.strip()); assert r.returncode == 0, r.stderr
print("step 16 applied")
