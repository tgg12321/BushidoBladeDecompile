#!/usr/bin/env python3
"""mkpoc.py: turn the scratch copy into the proof-of-concept configuration:
  1. no per-file `.align 3 -> .align 2` sed (RODATA_ALIGN2_FILES emptied);
  2. ONE uniform rule for every C object: after `as`, the object's .rodata section alignment is set
     to 4 (Sony PSYLINK places objects on 4-byte boundaries; ASPSX pads .align relative to the
     object start, which GNU as already does inside a section);
  3. the five objects split at the evidence-derived TU boundaries (asm-level proxy of a .c split),
     each part listed consecutively in every section of bb2.ld."""
import re
P = "/tmp/claude-0/poc/"
SPLITS = {
    "code6cac":   ["D_800100A4"],
    "code6cac_b": ["D_80010478", "jtbl_8001084C"],
    "text1a_pre": ["func_80040D48"],
    "text1b":     ["func_800747D8"],
    "text1b_b":   ["prnt", "sprintf"],
}
mk = open(P + "Makefile").read()
mk = re.sub(r"^RODATA_ALIGN2_FILES :=.*$", "RODATA_ALIGN2_FILES :=", mk, flags=re.M)
old = "$(MULTU_PAD) | $(AS) $(AS_FLAGS) -o $@\n"
assert mk.count(old) == 1
mk = mk.replace(old, old + "\t$(OBJCOPY) --set-section-alignment .rodata=4 $@\n")
rules = []
for stem, cuts in SPLITS.items():
    n = len(cuts) + 1
    names = ["$(BUILD_DIR)/$(SRC_DIR)/%s.o" % stem] + ["$(BUILD_DIR)/$(SRC_DIR)/%s__%d.o" % (stem, k) for k in range(2, n + 1)]
    ss = [o[:-2] + ".part.s" for o in names]
    args = " ".join([ss[0]] + [x for c, s in zip(cuts, ss[1:]) for x in (c, s)])
    r = "$(BUILD_DIR)/$(SRC_DIR)/%s.o: $(SRC_DIR)/%s.c $(PIPELINE_DEPS)\n" % (stem, stem)
    r += "\t@mkdir -p $(dir $@)\n"
    r += "\t$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< | $(CC1) $(call cc_flags_for,%s) | $(PROLOGUE_FIX) | $(MASPSX) $(call maspsx_flags_for,%s) | $(MULTU_PAD) > $(BUILD_DIR)/$(SRC_DIR)/%s.full.s\n" % (stem, stem, stem)
    r += "\tpython3 /tmp/claude-0/splitasm.py $(BUILD_DIR)/$(SRC_DIR)/%s.full.s %s\n" % (stem, args)
    for o, s in zip(names, ss):
        r += "\t$(AS) $(AS_FLAGS) -o %s %s\n\t$(OBJCOPY) --set-section-alignment .rodata=4 %s\n" % (o, s, o)
    rules.append(r)
anchor = "# -- Assemble .s files (non-decompiled asm) --"
assert mk.count(anchor) == 1
mk = mk.replace(anchor, "\n".join(rules) + "\n" + anchor)
open(P + "Makefile", "w", newline="\n").write(mk)
ld = open(P + "bb2.ld").read()
for stem, cuts in SPLITS.items():
    for sec in ("rodata", "text", "data", "bss"):
        pat = "build/src/%s.o(.%s);" % (stem, sec)
        if pat not in ld:
            print("note: %s has no .%s line" % (stem, sec)); continue
        m = re.search(r"^(\s*)" + re.escape(pat), ld, re.M)
        ind = m.group(1)
        extra = "".join("\n%sbuild/src/%s__%d.o(.%s);" % (ind, stem, k, sec) for k in range(2, len(cuts) + 2))
        ld = ld.replace(pat, pat + extra, 1)
open(P + "bb2.ld", "w", newline="\n").write(ld)
print("ok")
