# Evidence bank — func_80040B44

## [s1 2026-08-24, recon] Baseline + coupling characterization + full diff map

**Honest floor (measured this session, HEAD chassis):** `sandbox func_80040B44
--disable all` = **32** (target 93 insns, honest build 92; rules_dropped 15,
cheat_asm_stripped 4). `canonical` verdict C (pure-C target). Queue distance 32
agrees.

**Owner-directive coupling characterization — EXECUTED and CONFIRMED (s1).**
Replaced the C body with `INCLUDE_ASM("asm/funcs", func_80040B44);` (rules left
in place), full build → sha1 `e29f91cc…` — byte-identical reproduction of the
migration's measured failure SHA, so this IS the recorded coupling state. Image
diff vs oracle (script tmp/grind/func_80040B44/s1/imgdiff.py):
- ONE dominant range: file 0x0309cc–0x0314b0, vaddr 0x800401CC–0x80040CB0,
  2788 bytes — the text1a_pre TU's .text start through B44's end.
- Build has func_80040B44's bytes AT THE TU START (build@0x800401CC begins
  `addiu sp,sp,-72` = B44's prologue; oracle's B44 64-byte signature found at
  build offset 0x0309cc), and the oracle's func_800401CC signature found at
  build offset 0x030b40 = +0x174 = +372 bytes = exactly B44's size.
- CONCLUSION: the INCLUDE_ASM (macro-inc) emission lands at the TU's .text
  START, not at the function's source position; func_80040B44 is mid-TU (10th
  of 15 functions), so all 9 earlier siblings shift +372. The 11 scattered
  1–2-byte diffs elsewhere in the image (0x80010d9c, 0x8001cafc, …) are
  %hi/%lo addend bytes in cross-TU references to the shifted sibling symbols.
- IMPLICATION (the useful part): the coupling is PURE EMISSION-POSITION of the
  INCLUDE_ASM representation. It does not constrain the C at all: at
  COMPLETED-C the body is compiled C in source position, no INCLUDE_ASM, no
  coupling. Nothing about this function's bytes/rodata/data is special. The
  clean exit is simply solving it. (Tree restored after the probe; full build
  re-verified MATCH 62efab4f… this session.)

**Full 93-insn side-by-side diff** (tmp/grind/func_80040B44/s1/sidebyside.txt,
regenerable via s1/diff.sh). 32 scoring diffs decompose into exactly these
classes (line numbers = target insn index, 1-based):

1. **a0<->a1 seat swap, init loops** (insns 6–19): target holds the counter in
   $a0 and the loop-1 pointer in $a1; loop-2 pointer in $v0. Ours: counter $a1,
   loop-1 ptr $a0, loop-2 ptr $a0. Covered today by rules @157/159/161–167.
2. **t2<->t3 seat swap, main loop** (insns 9, 20, 27–30, 36, 43, 45, 78, 81,
   85–88): target: $t3 = walking u16 pointer (seated at prologue insn 9
   `addu t3,v1,v0`), $t2 = `a0_val & 0xFFFF` temp. Ours swapped. Rule @157.
3. **Commutative addu operand order** (insns 32, 42, 44, 55, 82): target is
   index-first at 4 sites (`addu a1,v0,sp`; `addu v1,v1,t7`; `addu v0,v0,t6`
   ×2; `addu a2,v0,t7`) but base-first at ONE site (insn 77 `addu v0,t7,v0`,
   the value stored into *t4). Our build canonicalizes base-first everywhere
   except insn 77 (which matches). Rules @169–173.
4. **3-insn scheduling order at loop entry** (insns 26–28): target order
   `li t8,0xffff; addiu t4,t5,88; addiu t3,t3,2`; ours `addiu t4; addiu t3;
   li t8`. Rule @177 (reorder 24,27,25,26).
5. **Missing dead decrement** (insn 89 `addiu t3,t3,-2`): the single
   insn-count shortfall (92 vs 93). The committed C literally writes
   `t3 = t3 - 2;` post-loop (src line ~480) but GCC DCEs it ($t3 dead after).
   Currently faked by rule @175 `insert_after "addu $11,$11,-2"` — the
   FORBIDDEN lost-codegen-insert class. Honest producer unknown; see
   hypotheses.

**m2c reference shape** (run: `python3 tools/m2c/m2c.py --valid-syntax
asm/funcs/func_80040B44.s`; full output not banked, key structure):
- Fresh variables per init loop: loop1 uses (var_a0=0x11, var_a1=&sp44);
  loop2 uses fresh (var_a0_2=0x13, var_v0=arg0+0x4C). Our committed body
  REUSES `i`/`ptr` for both loops — a rule-era shape, likely wrong.
- Address arithmetic order in m2c matches target's mixed orders EXACTLY:
  `(temp_a3 * 4) + sp`, `(temp_a3 * 0x68) + temp_t7` (both index-first),
  `((temp_t2 * 4) + arg0)` (index-first), but `temp_t7 + (temp_a3 * 0x68)`
  (base-first) at the *t4 store site only. GCC 2.7.2 preserves source addend
  order here — this is plain-C spelling, not a coercion.
- The 0x60-byte copy loop: m2c shows four word-copies per iteration; our
  Copy16/Copy8 struct-copy typedefs already produce byte-matching insns
  (insns 57–71 all match) — either spelling is fine, keep the matching one.
- Main-loop read shape in m2c: `temp_a3 = *var_t3; … var_a0_3 =
  *(u16*)(var_t3+2); var_t3 += 4;` — one advance of +4 per iteration with the
  second read at offset +2, vs our two separate `t3 += 2` advances. Both
  compile to the same two lhu/addiu pairs today, but the m2c shape may matter
  for H-DEADDEC (the post-loop -2 compensation would be -4+2 …) — untested.

**Committed chassis warnings:** body carries 4 cheat-asm blocks (stripped by
sandbox) and `s32 one = 1;` (the forbidden-catalog spelling "opaque variable to
defeat single-bit transform" — vs the sanctioned opaque-arithmetic family;
treat as rule-era suspect, re-derive rather than defend). Rule-era named
intermediates (`t2`, `one`, split statements) are calibrated to the 15 rules,
not to honesty.

**TU context:** src/text1a_pre.c holds 15 functions; campaign siblings in the
same TU: func_80040D48 (34 rules), func_80041188 (16 rules + the last live
prologue_config entry). All sibling rules are function-relative @N indices
(no $L label references) — no rule-index coupling to B44's representation.

**No duplicate/sibling analog:** tmp/duplicates_leads.txt has no entry for
func_80040B44 or neighbor func_80040A78. Kengo annotation on neighbor
func_80040CB8: `my_rob/rob_calc_2d_position` (this TU is robot/2D-position
prep code).

**Artifacts:** tmp/grind/func_80040B44/s1/{diff.sh, sidebyside.txt, target.dis,
build.dis, imgdiff.py}.

- [s1] Honest floor 32/93 measured this session (rules_dropped 15, cheat_asm_stripped 4); canonical verdict C

- [s1] Migration failure sha e29f91cc reproduced byte-exactly; coupling = INCLUDE_ASM emission position only; tree restored and full build re-verified MATCH 62efab4f this session

- [s1] m2c shows original used fresh locals per init loop (our chassis reuses i/ptr) and mixed per-site addend orders exactly matching target's addu operand orders

- [s1] Missing insn (92 vs 93) is the dead compensation decrement t3-=2 currently faked by forbidden lost-codegen rule @175; committed C writes it but GCC DCEs it

- [s1] s32 one=1 in committed body is the forbidden-catalog opaque-variable spelling - rule-era suspect, re-derive not defend

- [s1] No duplicate/sibling analog in tmp/duplicates_leads.txt; TU siblings func_80040D48/func_80041188 rules are function-relative (no label coupling)
