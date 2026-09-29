import re

s = open("tmp/f187/proof_v1.md", encoding="utf-8").read()


def rep(a, b):
    global s
    assert s.count(a) == 1, (a[:80], s.count(a))
    s = s.replace(a, b)


rep("# func_800187F4 — Ruling 11 (D) proof and landing record (manual, 2026-09-28)",
    """# func_800187F4 — Ruling 11 (D) proof and landing record, v3 (manual, 2026-09-28)

History. v1 (this text's base) FAILed a fresh layer-2 on `work`/`delta`
(rejected/r11-work-delta-reuse-layer2-fail-0.md); v2 split them with two FAKE dead stores and FAILed a
fresh layer-2 on `nforce`, which six FAKE do-while(0) wraps close as a per-value spelling
(rejected/r11-nforce-dowhile-layer2-fail-0.md). The owner then ruled (Q30, seventeenth batch; rule text
Ruling 11 (D) § "FAKE-construct spellings are set aside") that one-variable-per-value spellings needing a
FAKE-annotated construct do not count against (D)(3). v3 lands the v1 code (seven reused locals, no FAKE
construct for any of them) with the v1 reviewer's other corrections (mechanism wording, lz[6] census,
naming, header text). Every FAKE-construct spelling measured is banked in §5.""")

rep("- **Landing template** `r11/c5.c` (the `@gte_*` markers are expanded by `gen.py`, verbatim\n  inline_o.h statements) -> the landing body `candidate.c` (= `tmp/func_800187F4/out_c5.c`).\n  `r11/c5a.c` is the same code without comments (codegen identical).",
    "- **Landing template** `template.c` (= r11/variants_v3/c7.c; the `@gte_*` markers are expanded by\n  `gen.py`, verbatim inline_o.h statements) -> the landing body `candidate.c`. c7 is v1's code (c5, c5a)\n  with comment and type-name changes only (Scr1F800000); its objdump is identical to c5's (0 lines\n  each), so every v1 measurement below applies to it unchanged (variants in r11/variants/,\n  measurements_v1.md, sandbox_sweep.txt).")

# work: corrected property wording
rep("""`temp = work;` makes the two registers equivalent in cse; the class head (the register cse
substitutes into every equivalent use) is the one whose last use is later and beyond the block.""",
    """`temp = work;` makes the two registers equivalent in cse; the class head (the register cse
substitutes into every equivalent use) is the one whose LAST REFERENCE (reg_scan's regno_last_uid, which
counts reads AND sets: regclass.c:1763-1764) is later and beyond the block.""")
rep("""- Necessity: in every one-variable-per-value spelling the squared distance's reads are the compare,
  the small-arm index, the copy and the LZC-1 index (fixed statements), so its last use precedes
  temp's; the head is `temp` whatever the declaration (function scope 26). Only a variable whose last
  use is later than temp's (here: the same variable carrying the root/scale to the end of the body)
  keeps the head, and with it the target's `$a1` compare and the copy.""",
    """- Necessity: in every one-variable-per-value spelling that is not set aside, the squared distance's
  variable is referenced only by its own statements — its sum write, the compare, the small-arm index,
  the copy and the LZC-1 index (fixed by (C)(2)) — so its last reference precedes temp's; the head is
  `temp` whatever the declaration (function scope 26, chained `temp = sq1 = ...` 26). A later reference
  would be an extra statement: either a FAKE-annotated one (a dead store or self-assignment after
  temp's last use does reach 0: set aside under Q30, §5), or a real statement the reuse spelling lacks,
  which fails (C)(2). Only a variable that itself carries a later value (here: the root/scale to the end
  of the body) keeps the head, and with it the target's `$a1` compare and the copy.""")
rep("""- Necessity: in every one-variable-per-value spelling the ground depth's reads are those three in the
  ground block (fixed statements), so its last use is the division and the division's temp always
  outlives it; declaration scope does not change last-use order (function scope 4, inline 4). Only a
  variable that is read again later (here: carrying a sphere delta) stays the head.""",
    """- Necessity: in every one-variable-per-value spelling that is not set aside, the ground depth's
  variable is referenced only by its write and the three reads in the ground block (fixed statements),
  so its last reference is the division and the division's temp always outlives it; declaration scope
  does not change that (function scope 4, inline 4). A later reference would be a FAKE-annotated
  statement (a dead store after the ellipsoid loop reaches 0: set aside under Q30, §5) or a real statement
  the reuse spelling lacks ((C)(2)). Only a variable that carries a later value (here: a sphere delta)
  stays the head.""")
rep("""- Disclosure: a FAKE dead store `dg = 0;` after the ellipsoid loop also reaches 0 (fam_delta_deadstore_end:
  reg_scan counts the dead store as a later use before cse, and flow deletes it afterwards). It is not
  landed: it adds a no-semantic-purpose construct (dead-store-fake-exception) where the reuse adds none
  (Ruling 1(4)); the same choice the func_8008B488 Ruling 11 layer-2 accepted (its SR value closed with
  a FAKE chain-extender; the FAKE-free reuse landed, 0313b22b6).""",
    """- Set aside (Q30): the per-value spelling plus a FAKE dead store `dg = 0;` after the ellipsoid loop
  reaches 0 (reg_scan counts the dead set as dg's last reference before cse; flow deletes it). §5.""")
# idx / nforce: loop-depth weighting under FAKE-free spellings
rep("""  fixed by the statement list, whatever its declaration order, scope, name or type (the RTL is SImode
  for every 32-bit integer type; narrower types add extensions): priority >= 25600 > 10000.""",
    """  fixed by the statement list and its loop nesting (flow.c weights each reference by loop depth,
  reg_n_refs += loop_depth at :2081; the nesting is the program's own loops, which a FAKE-free spelling
  cannot change), whatever its declaration order, scope, name or type (the RTL is SImode for every
  32-bit integer type; narrower types add extensions): priority >= 25600 > 10000.""")
rep("""- Necessity: a force count's refs are its load and the loop test (weighted by loop depth): 7, over the
  loop (21 insns), in every one-variable-per-value spelling (the statements are fixed); floor_log2(7)
  = 2, so its priority is 6666 < 9183 whatever its declaration.""",
    """- Necessity: a force count's refs are its load and the loop test, weighted by loop depth (flow.c
  :2081): 7, over the loop (21 insns), in every one-variable-per-value spelling that is not set aside
  (the statements and their loop nesting are fixed); floor_log2(7) = 2, so its priority is 6666 < 9183
  whatever its declaration. Raising the weights of it or of competing allocnos needs added loop
  nesting: six FAKE do-while(0) wraps reach 0 (the v2 layer-2's find), set aside under Q30, §5.""")
# §5 replacement
i = s.index("## 5. Sanctioned-family (FAKE) probes")
j = s.index("## 6. Permuter campaigns")
s = s[:i] + """## 5. FAKE-construct spellings, set aside under Q30 (Ruling 11 (D) § "FAKE-construct spellings are set aside")

Every such spelling measured is banked here with its score, family and the sentence of its rule that
requires the annotation. None of them is used in the landing body.

Families and their annotation requirements, quoted:
- dead store / self-assignment / combine-foldable chain-extender — dead-store-fake-exception.md,
  prerequisite 3: "**Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` ON the
  statement."
- do-while(0) wrap — do-while-zero-exception.md, prerequisite 1: "**Inline `/* FAKE: ... */` or `// FAKE`
  annotation at the construct site** (not file-header prose), naming the observed effect".

Closing spellings (all one-variable-per-value for the variable named, each reaching 0):
| variable | spelling | lines | where |
|---|---|---|---|
| work | per-value + `sq1 = 0;` dead store after temp's last use (end of the ellipsoid body / before `tot`) | 0 / 0 | tmp/func_800187F4/rv_wd_end.c, rv_wd_after2.c (v1 layer-2); v2 chassis c6 (variants_v2/c6.c) |
| delta | per-value + `dg = 0;` / `depth = 0;` dead store after the ellipsoid loop | 0 | variants/fam_delta_deadstore_end.c; c6 |
| work + delta | both | 0 | rv_both.c; c6 |
| nforce | per-value + six single-level do-while(0) wraps (loads of vx, vy, vz, nforce_add, bits, bits2) | 0 | rejected/nforce-split-six-dowhile-wraps-closes-0.template.c (v2 layer-2) |

Non-closing FAKE sweeps (for the record): v2 chassis, on idx / nforce / temp / nbits / nbits2 per-value
spellings: 77 dead stores at six anchors (r11/ds.log, best 2) and 132 self-assigns / chain-extenders
(r11/ds2.log, best 2); the v2 layer-2's 500 single do-while(0) wraps over the five bodies (bests idx 62,
nforce 21, temp 42, nbits 2; rejected/v2_review/); v1 single probes (measurements_v1.md fam_* rows:
delta self-assign 4, chain-extenders 99 / 6; temp 40; nforce 21 / 54; work chain 123, dead store placed
before temp's last use 26; idx 67-103; nbits 2 / 9).

The reuse form carries no FAKE-annotated construct for any of the seven variables (the only FAKE
construct in the body is lz[6]'s oversized-locals annotation, unrelated to them), so the Q30 condition
"the reuse spelling itself needs no FAKE-annotated construct" holds.

""" + s[j:]
rep("## 6. Permuter campaigns (tools/permuter_campaign.py, `--stack-diffs`, standalone workspace)",
    "## 6. Permuter campaigns (tools/permuter_campaign.py, `--stack-diffs`, standalone workspace)\n"
    "Campaign 2 is the FAKE-free all-split body of this landing's code; campaign 3 (v2) ran from a body\n"
    "carrying the two FAKE dead stores (14,250 iterations, best 490 from 625; its best finds re-create\n"
    "reuses of `dist1`), recorded in rejected/ and the v2 ledger.")
# §7 updates
s = s.replace("RopeScratch", "Scr1F800000")
rep("- **`lz[6]`**: OVERSIZED-LOCALS carve-out, frame proof in the declaration comment; measured on c5:\n  lz[2]/lz[4] frame 0x68 (24 lines), lz[5] 0x78 (0), lz[7]/lz[8] 0x80 (24). lz[0]/lz[1] are the live\n  LZC outputs (stored by gte_stlzc, read back).",
    "- **`lz[6]`**: OVERSIZED-LOCALS carve-out, frame proof in the declaration comment (0x40 locals = 8 spill\n  + 32 orphan-USE phantom slots + 16 of the object); measured on c5: lz[2]/lz[4] frame 0x68 (24 lines),\n  lz[5] 0x78 (0), lz[7]/lz[8] 0x80 (24). lz[0]/lz[1] are the live LZC outputs. Phantom-slot producer\n  census: r11/frame_census.md (14 ordinary spellings of the lz[2] form, none gives 0x78 at zero cost;\n  measured on the v2 chassis, whose frame is the same).")
s = s.replace("constant-pointer scratchpad view as the\n  landed func_80018094", "constant-pointer scratchpad view as the\n  landed func_80018094 (offset-derived type name)")
s += """- **Header comment**: the state -0xFF..-1 path pulls toward the anchor and then integrates like state
  < -0xFF (no `continue`); the unevidenced "rope/cloth" naming is dropped.
"""
open("tmp/f187/proof_v3.md", "w", encoding="utf-8", newline="\n").write(s)
print("ok", len(s))
