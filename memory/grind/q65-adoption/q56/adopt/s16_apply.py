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
# research tools that ran maspsx with the retired lists (layer-2 round 1 follow-up): drop the list flags; the
# stem-based ones take the Makefile's per-file -G8 (maspsx_flags_for: every file but PSYQ_LIBRARY_FILES)
LISTS3 = ("--sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt ")
G8_LINE = ("# owner rulings Q65/Q69: maspsx -G8 for every file except Sony library code (the Makefile's maspsx_flags_for)\n"
           "if python3 -c \"import sys; sys.path.insert(0, '.'); from engine import buildconfig as c; "
           "sys.exit(0 if '$STEM' in c.PSYQ_LIBRARY_FILES else 1)\"; then :; else MASPSX_FLAGS=\"$MASPSX_FLAGS -G8\"; fi\n")
for tp in ("tools/ra_solver/mkasm_honest.sh", "tools/sched_solver/mkasm.sh"):
    s = open(tp).read()
    assert s.count(LISTS3) == 1, tp
    s = s.replace(LISTS3, "")
    k = s.index('MASPSX_FLAGS="--expand-div')
    e = s.index("\n", k)
    s = s[:e + 1] + G8_LINE + s[e + 1:]
    open(tp, "w", newline=NL).write(s)
for tp in ("tools/mar_perm_compile.sh", "tools/mar_perm_workspace.sh"):
    s = open(tp).read()
    a_ = ("      --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt \\\n"
          "      --sdata-exclude=sdata_exclude.txt --expand-lb \\\n")
    assert s.count(a_) == 1, tp
    # marionation_Exec is in system.c, Sony library code (owner ruling Q69): maspsx without -G8
    s = s.replace(a_, "      --expand-lb \\\n")
    open(tp, "w", newline=NL).write(s)
# the files steps 2-11 split / merged / cut / moved
OLD = ["text1a_c", "code6cac_b_tu2", "text1b", "code6cac_b2_pre", "replay_camera_rob_back_loose2", "config",
       "text1a_c2", "text1a_b", "text1a_b_pre_rodata", "sound", "text1b_tu2"]
r = subprocess.run([sys.executable, f"{H}/relocate_records.py", R] + OLD, capture_output=True, text=True)
print(r.stdout.strip()); assert r.returncode == 0, r.stderr
# tools/sched_solver/validate.py's default stems name config.c, merged into code6cac_c2.c by step 07
edit("tools/sched_solver/validate.py", [('DEFAULT_STEMS = ["config", ', 'DEFAULT_STEMS = ["code6cac_c2", ')])
# dead small-data rows (layer-2 round 2): a symbol-file row in the small-data area that no C file, no header,
# no INCLUDE_ASM function's .s and no asm data file names any more is an alias left behind by the
# reconciliations (e.g. g_anim_hit_flags[1]'s old name): it goes
import glob
texts = [open(p).read() for p in glob.glob("src/*.c") + glob.glob("include/*.h") + glob.glob("asm/data/*.s")]
inc = set()
for p in glob.glob("src/*.c"):
    inc |= set(re.findall(r'INCLUDE_ASM\(\s*"[^"]*"\s*,\s*(\w+)\s*\)', open(p).read()))
texts += [open(f"asm/funcs/{fn}.s").read() for fn in sorted(inc) if os.path.exists(f"asm/funcs/{fn}.s")]
blob = "\n".join(texts)
DEAD = []
for sf in ("undefined_syms_auto.txt",):   # named_syms.txt is the naming record, not auto rows: kept
    L = open(sf).read().split(NL)
    keep = []
    for l in L:
        m = re.match(r"^\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+)\s*;", l)
        if m and 0x800A30CC <= int(m.group(2), 16) < 0x800A3800 and not re.search(r"\b%s\b" % m.group(1), blob):
            print(f"dead row removed: {sf} {m.group(1)}")
            DEAD.append(f"{sf} {m.group(1)}")
            continue
        keep.append(l)
    open(sf, "w", newline=NL).write(NL.join(keep))
msg = ["Tooling and records follow the retired lists and the moved functions (owner ruling Q65). Byte-neutral.",
       "- gate / key / root lists drop sdata_syms.txt / sdata_funcs.txt / sdata_exclude.txt: tools/grinder/grindlib.py,",
       "  tools/check_root_cleanliness.py, tools/naming_wave.py, tools/data_wave.py, tools/desync_audit.py; the research",
       "  scripts tools/ra_solver/mkasm_honest.sh, tools/sched_solver/mkasm.sh (per-file -G8 as the Makefile),",
       "  tools/mar_perm_compile.sh, tools/mar_perm_workspace.sh (marionation_Exec is Sony library code: no -G8);",
       "  tools/sched_solver/validate.py's default stems name code6cac_c2 for the merged config.",
       "- records: relocate_records.py over the files steps 02-11 split, merged or cut (" + ", ".join(OLD) + "):",
       "  " + (r.stdout.strip().replace(NL, "; ") or "no record moved") + ".",
       "- dead small-data rows (no C file, header, INCLUDE_ASM .s or asm data names them): " + (", ".join(DEAD) or "none") + "."]
open(f"{H}/s16_msg.txt", "w", newline=NL).write(NL.join(msg) + NL)
print("step 16 applied")
