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


## [s2 2026-08-24, structural] SOLVED - honest floor 32 -> 0 (93/93 insns), zero rules, zero FAKE

**Result:** `sandbox func_80040B44 --disable all` = **score 0**, `build_insns` 93 ==
`target_insns` 93, with the body in `memory/grind/func_80040B44/candidate.c` in place
in `src/text1a_pre.c`. No permuter was used. No inline asm (`grep -c __asm__
src/text1a_pre.c` = 0). No FAKE annotation, no sanctioned-family claim, no coercion
construct of any spelling - the diff is a net REMOVAL of two constructs the rule-era
chassis carried (`s32 one = 1;` and the explicit `s32 *t4`). The 15 regfix rules are
now pure debt; retiring them is the operator step, not a source change.

The 32-diff residual fell in four independent structural findings, measured one at a
time. Each is a general lever, not a func_80040B44 curiosity:

**(1) Addend order survives from source ONLY in the integer domain (killed H1 as
stated, confirmed its corrected form).** s1 predicted that writing
`a3 * 0x68 + (u8 *)t7` instead of `(u8 *)t7 + a3 * 0x68` would flip target's
commutative `addu` operand order. Measured: floor 32 -> 32, ZERO change, and the
emitted operand order was base-first at every site regardless of what the source
said. Mechanism (read, not guessed): the C front end's `pointer_int_sum()` in
c-typeck.c builds every pointer+integer PLUS_EXPR as `(ptr, int)` before `fold()` or
`expand_expr` ever runs, so source order is erased for pointer arithmetic. Redoing
the same five sites in the INTEGER domain - `a3 * 0x68 + (s32)t7`,
`a3 * 4 + (s32)&seen[0]`, `(t2 << 2) + (s32)arg0 + 0x1A34` - preserves source order
and reproduced target's index-first `addu` at all five, floor 32 -> **27**. The one
site target spells base-first (insn 77, the value stored into `*t4`) stays ordinary
pointer arithmetic, which is why s1 saw it already matching. This is plain C, no
family, no annotation.

**(2) The two init loops share ONE counter and use FRESH pointers (H2, corrected).**
s1's H2 proposed fresh locals for both counter and pointer per loop, from the m2c
partition. Measured floor 27 -> 24: that fixes loop 2's POINTER seat ($v0) but leaves
both counters wrong (ours a1 / v1, target holds $a0 in BOTH loops). Target's shared
$a0 is the tell that ONE counter variable spans both loops. Shared `i` + fresh
`p1`/`p2` measured 24 -> **15**. NOTE (cost a probe): writing `i = 0x11;` before
`s32 *p1;` inside the block is a C89 declaration-after-statement violation that this
cc1 accepts SILENTLY and miscompiles - the init-loop stores vanished and the score
"improved" to a meaningless 23. Declare first, assign after; distrust any score drop
that comes with a shrinking body.

**(3) The walking-pointer advance belongs at the TOP of the loop body - and that is
where the missing 93rd instruction comes from (H3 CLOSED, honestly).** s1 banked the
exit-path `addiu t3,t3,-2` as the one insn GCC would not produce and rule @175 faked
with the forbidden lost-codegen class. Dump attribution: the corresponding RTL insn
is present through `.cse2` and GONE in `.flow`, so flow.c's `insn_dead_p` deletion is
what kills a source-level `t3 = t3 - 2;` - no dead-store spelling can survive it, and
the whole H3(c) dead-store-FAKE branch is moot. The real answer is that the insn was
never a source statement: writing the advance at the TOP of the loop body (`t3 += 2;
a3 = *t3; t3 += 2; ...body...; a0_val = *t3;` with no preheader advance and no
post-loop compensation) makes reorg.c steal the loop-top advance into the `bne` delay
slot, replicate it in the preheader, and emit the exit-path `addiu t3,t3,-2`
compensation itself. Floor 15 -> **2**, and `build_insns` went 92 -> 93 - the shortfall
closed as compiler output. Generalizable: an unexplained exit-path insn that undoes a
delay-slot insn is a reorg.c loop-top steal, not a lost source statement.

**(4) `t4` is not a variable - it is a strength-reduction giv, and that is what fixes
the preheader order (the last 2 diffs).** With (1)-(3) in place the only residual was
preheader order: target `li t9,1 / li t8,0xffff / addiu t4,t5,88`, ours
`li t9,1 / addiu t4,t5,88 / li t8,0xffff`. Dump attribution: the `li 0xffff` is
loop.c insn uid 290, and `move_movables` emits hoisted invariants immediately before
`NOTE_INSN_LOOP_BEG` - i.e. always at the END of the preheader - so NO source
ordering of preheader statements can put it before an explicit `t4` initializer. The
resolution is that `t4` should not exist: it is invariantly `t5 + 0x58`, so writing
the else arm against `t5` (`*(s16 *)((u8 *)t5 + 2)`, `*(s32 *)((u8 *)t5 + 0x58)`,
dropping `t4 += 0x68` since `t5 += 0x68` already runs) lets loop.c create it as a
giv - and a giv initializer is emitted AFTER the movable hoist, which is exactly
target's order. Floor 2 -> **0**.

**(5) The `s32 one = 1;` holder is gone, measured.** s1 flagged it as the
forbidden-catalog opaque-variable spelling. Under the p1 chassis deleting it was a
REGRESSION (6 -> 7, because loop.c then hoists `li 1` to the end of the preheader
too), which is exactly the trap that keeps such holders alive. Once (4) landed,
deleting it is byte-NEUTRAL: `*a1 = 1;` measures 0. Re-test suspect holders after
every structural change; their apparent necessity is chassis-relative.

**Allocation model, banked for reuse (this is the transferable part).** GCC 2.7.2
`global.c allocno_compare` priority is
`floor_log2(reg_n_refs) * reg_n_refs / reg_live_length`, and with no `REG_ALLOC_ORDER`
in `config/mips/mips.h` the MIPS port assigns strictly first-fit by ascending hard
regno among non-conflicting registers. `reg_n_refs` is loop-depth weighted: refs
outside any loop count 1, refs inside a single loop count 2. Measured on this
function: walking pointer = 16 refs / 67 length -> 4*16/67 = .955; the
`a0_val & 0xFFFF` temp = 10 / 34 -> 3*10/34 = .882; pointer allocated first and
first-fit took $t2 while target wanted $t3. Because `floor_log2` is a step function,
16 sits exactly on a cliff: shedding ONE weighted ref (to 15) collapses the priority
to .67 and reverses the order. Finding (3) sheds two. Corollary proven the hard way
(rejected/p1): splitting the value across TWO pseudos also reverses the order but can
never make them share a hard reg - `global.c expand_preferences` only unions
preference SETS, both of which are empty when neither pseudo is copied to/from a hard
reg, so the short-lived one is allocated last and first-fit hands it the lowest free
regno ($a2). When target computes a value into one register and keeps it there, the C
must use ONE variable; tune the REF COUNT, not the variable count.

**Artifacts:** tmp/grind/func_80040B44/s2/{diff.sh, sidebyside.txt, target.dis,
build.dis, splice.py, base.c, h1.c, h1b.c, h2.c, h2b.c, h2c.c, p1.c, p2.c, p3.c,
p4.c, p5.c} and tmp/grind/func_80040B44/dumps/text1a_pre.{loop,flow,cse2,greg,lreg,
combine}.

- [s2] Honest floor 0 measured this session (93/93 insns) with candidate.c in src/; zero rules, zero asm, zero FAKE, zero sanctioned-family claim; permuter unused
- [s2] pointer_int_sum() erases source addend order for pointer+int; only integer-domain arithmetic preserves it (5 addu operand-order diffs, 32 -> 27)
- [s2] Target shares ONE init-loop counter ($a0 in both loops) with FRESH per-loop pointers (27 -> 24 -> 15)
- [s2] The exit-path addiu t3,t3,-2 is reorg.c delay-slot-steal compensation from a loop-TOP advance, not a source statement; flow.c deletes any source-level dead decrement (present in .cse2, gone in .flow) (15 -> 2, build_insns 92 -> 93)
- [s2] loop.c move_movables always emits hoisted invariants immediately before NOTE_INSN_LOOP_BEG, so a preheader statement can never precede them; target's t4 is a strength-reduction giv whose init is emitted after the hoist (2 -> 0)
- [s2] GCC 2.7.2 global.c priority = floor_log2(n_refs)*n_refs/live_length, loop-depth-weighted refs, first-fit by ascending regno (no REG_ALLOC_ORDER on MIPS); floor_log2 cliffs at powers of 2 make a ONE-ref change decisive
- [s2] Two pseudos can never share a hard reg via preference here: expand_preferences unions preference SETS, empty unless a pseudo touches a hard reg
- [s2] cc1 SILENTLY miscompiles C89 declaration-after-statement inside a block (init-loop stores vanished, bogus score improvement) - declare first, and distrust a score drop that shrinks the body
