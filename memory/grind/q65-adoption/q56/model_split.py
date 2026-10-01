#!/usr/bin/env python3
"""model_split.py — step 2 of the Q56 per-file-model POC, on /tmp/q56/model only.
 (a) maspsx fix: `_uses_gp` (the load-delay-nop helper) ignores an INDEXED operand `sym($reg)`, which the
     expansion code itself never makes gp-relative (it only mattered once a file defines a symbol that
     another function of the file reaches indexed: func_8003047C's spurious nop).
 (b) the two TUs whose accesses contradict the model are split at a function boundary, as the
     original files evidently were: each part defines only the small data its own functions reach gp.
"""
import json, re, subprocess
Q = "/tmp/q56"; M = Q + "/model"
SPLITS = {"text1a_c": "func_80044800", "code6cac_b_tu2": "func_800343F0"}
A = json.load(open(Q + "/model_analysis.json"))
E = json.load(open(Q + "/model_edits.json"))

# (a)
p = M + "/tools/maspsx/maspsx/__init__.py"
s = open(p).read()
old = """            if op in load_mnemonics or op in store_mnemonics:
                (
                    _,
                    _,
                    operand,"""
assert s.count(old) == 1
s = s.replace(old, """            if op in load_mnemonics or op in store_mnemonics:
                (
                    r_source,
                    _,
                    operand,""")
old2 = """                if operand.count("+") == 1:
                    symbol, _ = operand.split("+")
                    gp_allowed = self.gp_allow_offset or not self._is_comm(symbol)"""
assert s.count(old2) == 1
s = s.replace(old2, """                if r_source is not None:
                    return False  # indexed `sym($reg)`: expanded via $at, never gp
                if operand.count("+") == 1:
                    symbol, _ = operand.split("+")
                    gp_allowed = self.gp_allow_offset or not self._is_comm(symbol)""")
open(p, "w", newline="\n").write(s)

# (b)
mk = open(M + "/Makefile").read()
ld = open(M + "/bb2.ld").read()
rules = []
for tu, cut in SPLITS.items():
    out = subprocess.run(f"mipsel-linux-gnu-objdump -d {Q}/refobj/{tu}.o", shell=True, capture_output=True, text=True).stdout
    order = re.findall(r"^[0-9a-f]+ <([^>]+)>:", out, re.M)
    side = {f: (0 if i < order.index(cut) else 1) for i, f in enumerate(order)}
    used = {}
    for S, v in A["objects"][tu]["gp"].items():
        for fn, mn, add in v:
            used.setdefault(S, set()).add(side[fn])
    added = set(E["edits"].get(tu, [])) | set(re.search(r"(\w+)(\[.*\])?;$", d).group(1) for d in E.get("block_scope", {}).get(tu, []))
    d1 = sorted(S for S in added if used.get(S) == {1})
    d2 = sorted(S for S in added if 1 not in used.get(S, set()))
    r = f"$(BUILD_DIR)/$(SRC_DIR)/{tu}.o: $(SRC_DIR)/{tu}.c $(PIPELINE_DEPS)\n\t@mkdir -p $(dir $@)\n"
    r += f"\t$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | $(CC1) $(call cc_flags_for,{tu}) | $(PROLOGUE_FIX) > $(BUILD_DIR)/$(SRC_DIR)/{tu}.cc1.s\n"
    r += (f"\tpython3 /mnt/c/Users/Trenton/Desktop/Bushido\\ Blade\\ 2\\ Decompile/tmp/q56/splitcc1.py $(BUILD_DIR)/$(SRC_DIR)/{tu}.cc1.s "
          f"$(BUILD_DIR)/$(SRC_DIR)/{tu}.p1.s $(BUILD_DIR)/$(SRC_DIR)/{tu}.p2.s {cut} {','.join(d1) or '-'} {','.join(d2) or '-'}\n")
    for part, obj in (("p1", f"{tu}.o"), ("p2", f"{tu}__2.o")):
        r += (f"\t$(MASPSX) $(call maspsx_flags_for,{tu}) < $(BUILD_DIR)/$(SRC_DIR)/{tu}.{part}.s | $(MULTU_PAD) | "
              f"$(AS) $(AS_FLAGS) -o $(BUILD_DIR)/$(SRC_DIR)/{obj}\n\t$(OBJCOPY) $(RODATA_OBJ_ALIGN) $(BUILD_DIR)/$(SRC_DIR)/{obj}\n")
    rules.append(r)
    for sec in ("rodata", "text", "data", "bss"):
        pat = f"build/src/{tu}.o(.{sec});"
        if pat in ld:
            m_ = re.search(r"^(\s*)" + re.escape(pat), ld, re.M)
            ld = ld.replace(pat, pat + f"\n{m_.group(1)}build/src/{tu}__2.o(.{sec});", 1)
    E.setdefault("splits", {})[tu] = {"cut": cut, "part1_drops": d1, "part2_drops": d2}
anchor = "# -- Assemble .s files (non-decompiled asm) --"
mk = mk.replace(anchor, "\n".join(rules) + "\n" + anchor)
# (bb2.ld names the __2 objects; they are produced by their TU's rule)
open(M + "/Makefile", "w", newline="\n").write(mk)
open(M + "/bb2.ld", "w", newline="\n").write(ld)
json.dump(E, open(Q + "/model_edits.json", "w"), indent=1)
print(json.dumps(E["splits"], indent=1))
