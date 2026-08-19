# Evidence bank — func_80017848

## s1 (recon, 2026-08-18)
- Baseline re-confirmed: canonical = C (distance 16 / 127 insns); sandbox --disable all = 16,
  125/127 build insns, frame 0x40 both sides. The inherited ledger below is accurate as-is.
- Target preheader detail not previously recorded: the a3 copy is `addu a3,a0,zero` — a copy
  of the SLOTS POINTER (not the shift). My build's `move a3,a1` copies the shift instead.
  The loop base is then `addu a0,a1,a3` and the count reload goes through it
  (`lw v0,0x1C(a0)` loop 1 / `lw v0,0x20(a0)` loop 2). Guard count load in target is
  `lw v0,0x1C(v0)` — dest clobbers the address reg, so the count value cannot survive to
  the loop (structurally forced reload, matches the inherited root-cause analysis).
- GCC 2.7.2 loop.c read (tools/gcc-2.7.2/loop.c:2760ff): plain MEM loads ARE hoistable in a
  store-free loop (no RTX_UNCHANGING_P requirement; only volatile / unknown_address_altered /
  store-dependence reject), BUT the bound load sits after the conditional `beq` exit, i.e.
  conditionally executed (maybe_never), so loop.c will not move it in EITHER build. Therefore
  my build's count-caching (`move a1,v0` + `slt v0,v1,a1`) comes from a cse-family fold over
  the rotated-while's duplicated exit test, NOT from LICM — which pass exactly is hypothesis
  H2's probe. jump.c duplicate_loop_exit_test confirmed present at jump.c:2163.
- No sibling/duplicate lead: tmp/duplicates_leads.txt has no entry for this function.
- Artifact: tmp/grind/func_80017848/s1/build_disasm_floor16.txt (current 125-insn build body).
- Frontier for next session: hypotheses.md s1 section (H1 guarded do-while with distinct
  guard/bound lvalues + slots-copy local; H2 -da pass forensics; H3 coupled single-base
  prediction).

- == imported from memory/wip notes.md ==
# func_80017848 (src/ings.c) — blob strike

Whole-body asmfix blob masks this function; the oracle is unaffected until the blob retires,
so improved-but-incomplete C is safe to leave. Do NOT edit asmfix.txt.

## Current state: sandbox `--disable all` = **16** (was 84). 125/127 insns. Zero rules, zero cheat-asm.
Everything outside the two scan loops is byte-exact: prologue/epilogue (frame 0x40), the two
top guards, the `math_Distance3D` call, the four link-record stores, and both count-append blocks.

## Score ladder
| state | score | frame |
|---|---|---|
| inherited (register-asm pins, cached slot pointers) | 84 | 0x30 |
| drop pins; inline `*(u8**)(ctx+0xC)` everywhere (the call kills the memory CSE) | 63 | 0x30 |
| guarded do-while + integer-offset addressing + cached `link` pointer | 56 | 0x30 |
| tail: cache the record pointer across the count store | 52 | 0x30 |
| `slots` local inside the loop guard body | 46 | 0x30 |
| **`while` instead of `do-while`** — the rotated-while guard is the phantom-slot producer | 33 | **0x40** |
| drop the redundant explicit `if` guard (it emitted a second `blez`) | 28 | 0x40 |
| `links` inline in the loop + index expression written **i-first** | **16** | 0x40 |

## Levers that moved it (mechanism)
- **Never cache `*(u8**)(ctx+0xC)` across the `math_Distance3D` call** — the call clobbers memory so
  GCC reloads it; only the pure `slot<<6` shift survives (target keeps it in s0/s1).
- **`ptr + int` is not symmetric.** C's `pointer_int_sum` always emits the pointer operand first, so a
  target `addu v0,v0,v1` (shift-first) can only be spelled as integer arithmetic:
  `*(s32 *)((slot_a << 6) + (s32)base + FIELD)`. The `math_Distance3D` args and the `link` record
  address are ptr-first in target, so those stay pointer arithmetic.
- **A store kills memory CSE**, so the four post-call stores need one cached `link` pointer and each
  count-append block needs its own cached record pointer, or `lw 0xC(s2)` is re-emitted after every `sw`.
- **`while` vs `do-while` decides the frame.** The 16 bytes of phantom locals (`vars= 16`,
  frame 0x40 vs 0x30) come *only* from the rotated-`while` guard — producer #1 in
  [[phantom-slot-frame-lever]]. No named-local spelling produced them (swept 5 candidates, all
  `vars= 0`); every `do-while` spelling gives frame 0x30. This was the single biggest lever.
- **Index expression must be i-first**: `*(u8 *)(i + (slot_a << 6) + (s32)slots + 0x24)`. Any
  base-first or pointer-arithmetic spelling costs ~13 points (it lets CSE collapse the loop's base
  into the top guard's `slots` register and drops the `lw a0,0xC(s2)` reload).

## Remaining residual (16 pts, ~8 differing insns per scan loop, both loops identical)
Target hoists `(slot_a<<6) + slots` into ONE preheader register (`move a3,a0; addu a0,a1,a3`) used by
both the body index (`addu v0,a0,v1`) and the bound load, and **re-loads the count every iteration**
(`lw v0,0x1C(a0); slt v0,v1,v0`). Mine keeps `off` and `slots` in separate registers, adds both inside
the loop (one extra `addu`), and caches the count in `a1` (`move a1,v0` in the preheader), saving the
two bound-reload insns. Root cause: in target the guard's count load is `lw v0,0x1C(v0)` — dest ==
address reg, so the count dies at the `blez` and the loop must reload; in mine the loop's bound is
CSE-equal to the guard's, so GCC keeps the value live instead. Every spelling that gives the loop its
own base register also makes the guard's count survive, so the two effects have not been separable.

## Killed levers (measured, all with the current surrounding code)
- Register-asm pins — anti-cheat, and score-inert (the sandbox strips them).
- Explicit `if (n > 0)` guard around the `while` — emits a redundant second `blez` (33 vs 28).
- Any `do-while` shape — loses the 16 phantom frame bytes (37–42).
- `rec` pointer local before the loop (19), assigned inside the loop body (21), or with the bound
  read through it (19/23).
- `links` as a local before the loop (28) or inside the loop body (24) — inline is best (16).
- Base-first / pointer / slots-first index spellings (29 each); `for` instead of `while` (16, tie).
- Named locals for the guard counts, the slot offsets, the link index, separate `i`/`j`, or named
  top-guard fields — none change `vars` (all stayed `vars= 0` pre-`while`).

## Instruments (kept in tmp/, regenerate if lost)
- `tmp/frameprobe.sh` — cpp | cc1 on src/ings.c, prints the `.frame`/`vars=` line. Separates
  "wrong frame" from "wrong codegen" far faster than the sandbox.
- `tmp/dis17848.sh` + `tmp/norm17848.py` — normalized target-vs-build instruction diff.
- `tmp/score_sweep.py` + `tmp/run_sweep.sh <variants.py>` — scores a dict of body variants
  (score + frame), restoring src/ings.c afterwards.


- [s1] sandbox --disable all = 16 this session (125/127 insns, frame 0x40 both sides); canonical verdict C distance 16

- [s1] target a3 copy is the SLOTS pointer (addu a3,a0,zero), not the shift; loop base addu a0,a1,a3 feeds both the single index addu v0,a0,v1 and the per-iteration count reload lw 0x1C/0x20(a0)

- [s1] target guard count load is lw v0,0x1C(v0) (dest==addr) so the count dies at the blez — the reload is structurally forced in target

- [s1] GCC 2.7.2 loop.c hoists plain MEM loads in store-free loops, but NOT conditionally-executed ones (bound load sits after the beq exit), so my build's count cache is a cse-family fold over the duplicated exit test, not LICM

- [s1] residual is 8 pts per scan loop, loops identical in shape; everything outside the loops is byte-exact per inherited ledger, re-confirmed by 125-insn disasm

## s2 (structural, 2026-08-18)
- Floor unchanged at **16** (baseline form re-verified in place at end of session:
  score 16, 125/127 insns). No spelling measured this session went below 16 — but the
  residual MOVED: the best new form (17) has both scan loops byte-exact and confines
  the entire gap to the 2-insn preheader shape. Saved as
  `memory/grind/func_80017848/candidate_alt_dowhile_ivar_17.c` (start there, not at
  candidate.c).
- **The phantom-16 frame producer is now pinned exactly.** It is NOT the `while` vs
  `do-while` shape (the inherited belief) and NOT the guard/bound lvalue distinction
  (H1's claim). It is the guard's COMPARISON OPERAND: only an entry guard that compares
  the loop's own induction variable — `i = 0; if (i < count)` — yields `.frame vars=16`
  (frame 0x40). Measured on 9 guard spellings with the rest of the form held fixed:
  `i < count` -> 17 / frame 0x40; `count > 0` -> 35 / 0x30; `count != 0` -> 37 / 0x30;
  `count >= 1` -> 35 / 0x30; `0 < count` -> 35 / 0x30; named `n = count; n > 0` -> 36 / 0x30;
  `k = count - 1; k != -1` -> 37 / 0x30; `n = count; i = 0; i < n` -> 19 / 0x40;
  bound-through-named-`n` -> 41 / 0x30. This is [[phantom-slot-frame-lever]] producer #1
  (folded loop-guard compare) with a sharper predicate than the rule states.
- **A source-level do-while DOES give target's per-iteration bound reload** and its
  single hoisted base + one-addu index (`addu v0,a0,v1`), contradicting the inherited
  assumption that reload and phantom frame are mutually exclusive. Combined with the
  `i < count` guard the two coexist: variant `g_ivar` / `S1` = 17, frame 0x40, loops
  byte-exact.
- **Two rigid regimes, quantized at 17 and 34.** Whether a `u8 *` holding `*(u8**)(ctx+0xC)`
  is live across the entry guard decides everything downstream:
  * Regime A (pointer live across the guard): score **17** for all 20+ spellings measured
    — guard address form (int-cast vs pointer arithmetic), base source (slots local /
    fresh ctx read / copy local / two-step copy chain), operand order, base formed
    before vs inside the guard body, index `base + i` vs `i + (s32)base`. The preheader
    reuses the top guards' slots pseudo, so target's `lw a0,12(s2)` reload is absent.
  * Regime B (nothing live; every read fresh from ctx): score **34** for all 10+ spellings
    — the preheader DOES re-load ctx+0xC exactly like target, but cse then unifies the
    loop base with the guard's address pseudo, so target's uncoalesced `move a3,a0` copy
    and recomputed `addu a0,a1,a3` never appear (123 insns vs target 127) and the loop
    register assignment shifts off a0/a1/a2/a3.
  No spelling produced reload AND copy. That pair is the whole remaining residual.
- **KILLED: target's `move a3,a0` copy is a loop.c LICM artifact.** Wrote the record
  address fully inline inside the do-while body so loop.c must hoist it (variant T1):
  LICM does hoist, and cse still folds the hoisted base onto the guard's address —
  123 insns, byte-identical result to the copy-local spellings. Same for a two-step
  C-level copy chain (`p = read; q = p; base = q + off`).
- **Read-count forensics (H2, partially answered).** cc1 `-da` dumps of both regimes
  (tmp/grind/func_80017848/s2/da_S1, da_S3): the C-level number of `*(u8**)(ctx+0xC)`
  reads survives to the final asm — regime A carries 9 `const_int 12` refs at .rtl and 10
  at .greg, regime B carries 11 and 12. cse collapses both to 7 mid-pipeline and the
  difference re-emerges, i.e. cse is NOT what removes the preheader reload in regime A;
  the reload is simply never emitted because the C has one fewer read reaching that point.
  So the reload is a SOURCE-STRUCTURE lever (how many live reads), not a pass to defeat.
- Artifacts: variants{,2,3,4,5,6}.py (the full measured matrices), diff_*.txt (normalized
  target-vs-build diffs for the baseline, the 17-form and the 34-form), da_S1/, da_S3/.

- [s2] Floor unchanged at 16; baseline form restored and re-verified in place at end of session (score 16, 125/127 insns, src/ings.c clean vs HEAD).

- [s2] Best new form scores 17 (frame 0x40) with BOTH scan loops byte-exact — target's per-iteration bound reload, single hoisted base and one-addu index all reproduce. Saved as memory/grind/func_80017848/candidate_alt_dowhile_ivar_17.c; s3 should start there, not from candidate.c, because the floor-16 form's residual is still inside the loops while the 17-form's is confined to a 2-insn preheader.

- [s2] The whole search space collapses into two rigid regimes decided by ONE property: whether a u8* holding *(u8**)(ctx+0xC) is live across the entry guard. Regime A (live) = score 17 for all 20+ spellings measured — guard address form (int-cast vs pointer arithmetic), base source (slots local / fresh ctx read / copy local / two-step chain), operand order, base formed before vs inside the guard body, index base+i vs i+(s32)base. Regime B (nothing live, every read fresh) = score 34 for all 10+ spellings.

- [s2] Regime B reproduces target's preheader reload (lw a0,12(s2)) exactly, but cse then unifies the loop base with the guard's address pseudo, so target's uncoalesced move a3,a0 copy and its recomputed addu a0,a1,a3 never appear: 123 insns vs target 127, and the loop register assignment shifts off a0/a1/a2/a3. No spelling in 45 produced reload AND copy — that pair is the entire remaining residual.

- [s2] Guard spelling controls the frame independently of loop shape: i < count gives vars=16 / frame 0x40 in BOTH the do-while and the rotated-while families; every constant-comparison guard gives vars=0 / frame 0x30 and costs ~18 points of sp-offset cascade through the save/restore block.

- [s2] One variant in the sweep (S6, guard reading the slots local while the assignment sits inside the guard body) scored 16 but is SEMANTICALLY INVALID — loop 1 would read an uninitialized slots. Recorded here so a later session does not resurrect it from the raw sweep numbers.

- [s2] Instruments regenerated/extended and left in place: tmp/score_sweep.py + tmp/run_sweep.sh (score + frame per variant, restores src afterwards), tmp/apply_variant.py, tmp/diffvar.sh (apply + sandbox + normalized target diff), tmp/dadump.sh (cc1 -da dumps), tmp/count_ctx12.py (per-stage load-count census).

## s3 (structural, 2026-08-18)
- **FLOOR BROKEN: 16 -> 14.** The floor had been flat at 16 across s1/s2. The new form is
  in src/ings.c and saved as `memory/grind/func_80017848/candidate.c`; 125/127 build insns
  (was 125 at score 16 with the residual spread through both loops, and 122-123 for the
  17-form family). `candidate_alt_dowhile_ivar_17.c` is now SUPERSEDED - do not start there.
- **The new lever (the whole session's finding): the loop entry guard's count address must
  be written POINTER-FIRST.**  `if (i < *(s32 *)((s32)slots + (slot_a << 6) + CNT))` scores
  14; the identical value written shift-first `(slot_a << 6) + (s32)slots + CNT` scores 16;
  with no `slots` local at all it scores 34.  This is a THREE-regime axis, not the two
  regimes s2 recorded, and the third regime was invisible to s2 because every s2 spelling
  wrote the address shift-first (the shift-first rule inherited from the pre-do-while era,
  "index expression must be i-first", does NOT extend to the guard's count address).
- **Second new lever: WHERE the `slots` read is hoisted.**  Hoisting `slots = *(u8**)(ctx+0xC)`
  ABOVE the two >=0 top guards (so those guards consume it too) = 14.  Hoisting it only to
  just-before-loop-1 = 15.  Re-reading it before each loop = 17.  No hoist (inline in each
  loop guard) = 16.  Measured with everything else held fixed.
- **Byte-exact surface is now everything except two insns.**  Target insns 75-126 (the
  math_Distance3D call, all four link stores, both count-append blocks, the epilogue) and
  both scan-loop BODIES are byte-identical; the frame is 0x40 / vars=16 as target.  The
  entire remaining residual is one insn per scan-loop preheader:
    target: `lw a0,0xC(s2); sll a1,s4,6; addu v0,a1,a0; lw v0,0x1C(v0)` then, after the
            blez, `move a3,a0; lw a2,0x10(s2); addu a0,a1,a3`
    ours:   `sll a0,s4,6; addu v0,a1,a0; lw v0,0x1C(v0)` then `lw v0,0xC(s2);
            lw a2,0x10(s2); addu a0,a0,v0`
  i.e. we reuse the hoisted `slots` register for the guard address and read fresh only for
  the base; target reads fresh for the guard address and *copies* that read for the base.
- **KILLED: the top >=0 guards are NOT a lever.**  s2's frontier item 1 claimed the top
  guards' spelling controls whether the preheader reload appears.  Eight spellings measured
  with the loop held fixed - inline reads (baseline), per-slot record-pointer locals dead
  after the compare, a named `slots` local, `&&`-flattened, explicit `goto scan` inversion
  (which puts the return-0 on the fall-through exactly as target does), pointer-arithmetic
  address form, count-into-a-local, and goto+record-pointers - ALL scored 17 with the
  17-form loop, and the association order of the top guards is inert at the 14-form too
  (ts/tp x gs/gp x 3 base forms: only the LOOP guard's association moves the score).
- **KILLED: no C-level spelling produces target's `move a3,a0` copy.**  Measured across ~60
  variants this session: a second read before the guard, a second read inside the guard
  body, a third read, a two-step C copy chain, a fresh read for the base with the guard on
  the hoisted local and vice versa, base by int-cast / pointer-first / pointer arithmetic,
  base assigned inside the do-while body, and fully-inline addressing (loop.c LICM).  Every
  added read is either folded by cse or coalesced by local-alloc; base spelling is
  completely inert at 14 (int-cast, pointer-first and pointer-arithmetic all score 14).
- Artifacts: tmp/grind/func_80017848/s3/variants{,2..9}.py + final.py (the 60-variant
  matrices), diff_G0_base.txt / diff_P3.txt / diff_Q4+G0_base.txt / diff_T1.txt /
  diff_U8.txt / diff_V2.txt / diff_FINAL.txt (normalized target-vs-build diffs at each
  score step), asm_*.txt (raw build disassembly), dv.sh (apply + score + dump + diff).

- [s3] Floor 16 -> 14. src/ings.c carries the 14-form at session end; candidate.c updated; 125/127 insns; frame 0x40.

- [s3] NEW LEVER: the loop entry guard's count address association order is a three-way regime switch - pointer-first `(s32)slots + (slot_a << 6) + CNT` = 14, shift-first `(slot_a << 6) + (s32)slots + CNT` = 16, no slots local (inline read) = 34. s2 saw only two regimes because every s2 spelling was shift-first.

- [s3] NEW LEVER: hoisting the `slots = *(u8**)(ctx+0xC)` read ABOVE the two >=0 top guards (so they consume it) = 14; hoisting it only before loop 1 = 15; re-reading before each loop = 17; no hoist = 16.

- [s3] The loop BASE spelling is completely inert at the 14-form (int-cast, pointer-first, pointer-arithmetic, copy-chain, inline/LICM all identical), as is the top-guard spelling (8 forms) and the top-guard association order.

- [s3] Residual is now exactly 2 insns (1 per scan-loop preheader): target's fresh `lw a0,0xC(s2)` for the guard address plus its uncoalesced `move a3,a0` copy feeding `addu a0,a1,a3`. Everything else in the function - both loop bodies, all four link stores, both count-append blocks, the call, the prologue/epilogue and the frame - is byte-exact.

- [s3] Floor improved 16 -> 14 this session (flat at 16 across s1 and s2). The 14-form is in src/ings.c at session end and saved to memory/grind/func_80017848/candidate.c; sandbox --disable all re-verified = 14, 125/127 build insns, frame 0x40 / vars=16 as target.

- [s3] NEW LEVER: the loop entry guard's count address is a three-way regime switch on association order alone - pointer-first `(s32)slots + (slot_a << 6) + CNT` = 14, shift-first `(slot_a << 6) + (s32)slots + CNT` = 16, inline read with no local = 34. s2 saw only two regimes because every s2 spelling wrote that address shift-first.

- [s3] NEW LEVER: hoisting the slots read ABOVE the two >=0 top guards (so they consume it too) = 14; hoisting only to just-before-loop-1 = 15; re-reading before each loop = 17; no hoist = 16.

- [s3] The residual is now exactly 2 insns, one per scan-loop preheader. Target: `lw a0,0xC(s2); sll a1,s4,6; addu v0,a1,a0; lw v0,0x1C(v0)` then after the blez `move a3,a0; lw a2,0x10(s2); addu a0,a1,a3`. Ours: `sll a0,s4,6; addu v0,a1,a0; lw v0,0x1C(v0)` then `lw v0,0xC(s2); lw a2,0x10(s2); addu a0,a0,v0`. We reuse the hoisted slots register for the guard address and read fresh only for the base; target reads fresh for the guard address and copies that read for the base.

- [s3] Everything else in the function is byte-exact: target insns 75-126 (the math_Distance3D call, all four link-record stores, both count-append blocks, the prologue/epilogue) and both scan-loop BODIES, verified against the normalized diff (tmp/grind/func_80017848/s3/diff_FINAL.txt).

- [s3] The two >=0 top guards are inert as a lever: 8 spellings measured against the 17-form loop (including an explicit `goto scan` inversion that puts return-0 on the fall-through exactly as target lays it out) all scored 17/19, and their association order is inert at the 14-form too.

- [s3] The loop BASE spelling is completely inert at the 14-form - int-cast, pointer-first, pointer arithmetic, an explicit two-step copy chain, assignment inside the do-while body and fully-inline addressing (forcing loop.c LICM) all score 14 with identical asm.

- [s3] ~60 distinct forms were measured this session across nine variant matrices, all with the sandbox (score) and the cc1 frameprobe (.frame vars=) recorded per variant; every measured form kept frame 0x40, confirming s2's `i = 0; if (i < count)` guard-operand finding is orthogonal to the new association lever.

## s4 (2026-08-18) — permuter modality

Starting point: the s3 candidate re-applied to `src/ings.c`, sandbox `--disable all`
re-confirmed at **14** (127 target insns, 125 build insns). Floor at end of session: **14**
(unchanged — no form measured below it this session).

### E-s4-1. A validated permuter workspace for func_80017848 now exists and is reproducible.
`tmp/grind/func_80017848/s4/mkws.sh <dir>` builds it end to end and self-validates
(prints `base insns: 125  target: 127`). It mirrors the real build exactly:
full-TU `cpp` of `src/ings.c` → `tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char
-quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel` → `prologue_fix` →
`maspsx` (all the Makefile flags) → `multu_pad`, then extracts only
func_80017848's `.ent`…`.end` region and assembles it against a `.set noat / .set
noreorder` prelude. `target.o` is `prelude_r3k.inc + asm/funcs/func_80017848.s`.
Two gotchas cost real time and are baked into the script so the next session does not
repay them:
  (a) `-mel` MUST be on the cc1 line (Makefile `CC_FLAGS`, [[mel-endianness-adoption]]);
      `tools/mar_perm_workspace.sh` predates it and is therefore NOT a copyable template.
  (b) maspsx emits `.ent func_80017848` with NO leading tab, so the mar-workspace's
      `/^\t\.ent\t.../` awk extraction silently never triggers and swallows the rest
      of the TU (including the `INCLUDE_ASM` `.include` of `asm/funcs/func_80017A44.s`),
      producing a pile of assembler errors that look like a toolchain problem and are not.
  (c) `src/ings.c` contains a multi-LINE inline-asm string literal (in func_80017D84,
      around line 931). cpp passes it through with a warning, but pycparser rejects it and
      the permuter dies with "Syntax error in base.c". `tmp/grind/func_80017848/s4/fix_base.py`
      joins any unterminated string literal onto one line with an escaped `\n`, which is
      semantically identical and lets the permuter parse the TU.

### E-s4-2. THE HEADLINE: decomp-permuter's default scorer is ANTI-CORRELATED with the engine's honest distance on this function.
Measured, not inferred. The s3 candidate scores **855** in the permuter and **14** in
`sandbox --disable all`. Every permuter output was then re-scored with the engine
(`tmp/grind/func_80017848/s4/batch_score.ps1`, results in the s4 scratch dir):

| permuter score | engine distance |
|---|---|
| 405 | 18 |
| 480 | 16 |
| 575 | 14 |
| 585 | 18 |
| 650 | 18 |
| 660 | 19 |
| 680 | 15 |
| 720 | 15 |
| 730 | 16 |
| 770 | 16 |
| 775 | 39 |
| 785 | 15 |
| 797 | 31 |
| 815 | 14 (x2) |
| 820 | 15 |
| 825 | 16 |
| 835 | 37 |
| 840 | 17, 19 |
| 855 (base-tie) | 14 (x5) |

The permuter's single best find (405, a 53% "improvement") is engine distance **18** —
four instructions WORSE than where it started. Nothing the permuter produced beat 14.

Mechanism: the permuter's weighted metric charges reorderings at 60 and ins/del at 100.
Our build is 2 instructions SHORT of target, and those 2 missing instructions sit early
(in the first scan-loop preheader), so the diff aligner charges the entire 100-instruction
tail as displaced: 855 ≈ 14 x 60 + change. The cheapest way for the permuter to lower that
number is to change the SHAPE of the tail so the aligner re-syncs — i.e. to move away from
the byte-exact tail we already have, which is exactly the wrong direction. The engine's
metric (differing instructions, branch/jump targets masked) has no such displacement term.
**Consequence for the pipeline, not just this function: on any function whose residual is a
small ins/del near the TOP of the body, a permuter campaign's own score ranking is not usable
as the objective. The campaign is still a legitimate PROPOSAL GENERATOR — a true match still
scores 0 and is still saved — but every find must be re-scored with `sandbox --disable all`
before it means anything, and "the permuter improved the score by half" is not evidence of
progress.** Budget campaigns accordingly: the hill-climb (`--keep-prob 0.6`, the default and
not exposed by `permuter_campaign.py launch`) actively walks away from the basin here.

### E-s4-3. The loop-preheader spelling cross-product is EXHAUSTED at 14.
s1-s3 swept these axes one at a time (~105 hand forms). s4 enumerated the full symmetric
CROSS-PRODUCT and scored every cell with the engine
(`gen_variants.py` -> 72 variants, `sweep.sh` -> `sweep_results.txt`):
  A  guard-count read base : hoisted `slots` | fresh `*(u8**)(ctx+0xC)`
  B  preheader `p`         : fresh read | reuse `slots`
  C  base spelling         : int-cast `(u8*)((slot_a<<6)+(s32)X)` | ptr-arith `X + (slot_a<<6)`
  D  loop-body index base  : `base` | recompute from `p` | recompute from a fresh ctx read
  E  do-while bound base   : `base` | recompute from `p` | recompute from a fresh ctx read
Result over all 72: **minimum 14, maximum 19**; twelve distinct cells tie at 14 and none
goes below. The 14-cells are `a0_b0_{c0,c1}_{d0e0,d1e1,d2e2}`, `a0_b1_{c0,c1}_d2e2`, and —
new information — `a1_b0_{c0,c1}_{d0,d2}_e1`, i.e. a FRESH-read guard also reaches 14 when
the do-while bound is recomputed from `p`. So s3's "the guard must consume the hoisted
`slots`" is a sufficient route to 14, not a necessary one; 14 is a broad plateau over this
entire family rather than a knife-edge. Both facts point the same way: the last 2
instructions are NOT reachable by respelling the preheader, which corroborates s3's live
frontier hypothesis that `move a3,a0` is a register-ALLOCATION outcome.

### E-s4-4. cc1 `-da` RTL dumps for the 14-form are now on disk and cost nothing to regenerate.
`tmp/grind/func_80017848/s4/dump.sh` produces the full pass series for the preprocessed TU
(`ings_pp.c.rtl/.jump/.cse/.loop/.combine/.cse2/.flow/.lreg/.greg/.sched/.jump2/.dbr/.sched2`).
This is the probe s1 and s2 both banked and never spent; s4 produced the artifacts but did
NOT get to the analysis (the permuter work consumed the session). The .lreg/.greg pair is
the direct evidence for which pseudo pair local-alloc coalesces and what conflict or
register preference would keep target's copy alive — read those, do not re-derive by
spelling sweeps.

### E-s4-5. FLOOR 14 -> 12. The lever is a second live pointer variable feeding the two scan-loop entry guards.
Chassis B (`tmp/perm_ings_s4b`) is the same validated workspace with the residual region —
the two scan-loop preheaders and bodies — wrapped in `PERM_RANDOMIZE(...)` so the randomizer
cannot touch the 125 already-byte-exact instructions (`s4/mk_perm_b.py`). 733 s, 21911
iterations, 130 saved outputs, all re-scored with the engine (`s4/score_outputs.sh`,
`perm_ings_s4b_engine_scores.txt`). Engine-distance distribution over the 130:
2x12, 1x13, 12x14, 15x15, 19x16, 11x17, 17x18, 15x19, and a long tail to 34.

The two 12-cells are `output-845-1` and `output-845-3` — permuter score **845**, i.e. the
permuter ranked them in its WORST decile, ten points off base. They would have been thrown
away by any campaign that trusts the permuter's ranking. This is E-s4-2 paying for itself.

The delta is one line: **`base = slots;`** right after the slots read, so the two SCAN-LOOP
ENTRY GUARDS read their count through `base` (`*(s32 *)((s32)base + (slot_a << 6) + 0x1C)`
and `+ 0x20`) while the two `>= 0` top guards keep reading through `slots`. `base` is then
re-pointed at the per-slot record inside each preheader exactly as before. Cleanup sweep
(`s4/variants2`, engine-scored):
  - both top guards through `base`                 -> **12**
  - both top guards through `slots` (chosen form)  -> **12**
  - exactly as the permuter emitted it (mixed)     -> **12**
  - loop entry guards moved back onto `slots`      -> **19**
So the top guards are indifferent, and routing the two LOOP ENTRY GUARDS through the second
pointer variable is worth 7 points. It is load-bearing, not incidental. The chosen candidate
is the both-top-guards-through-`slots` spelling because it is the one a reader can follow:
`slots` is the array base used by the top guards, `base` is the walking record pointer that
starts at the array base and is re-pointed per loop.

Classification note for whoever closes this function: this is the frozen-list
**"variable reuse for codegen control"** family — `base` is a genuine live pointer that is
READ by both loop guards before it is reassigned, so it is not a dead store, not a
constant-holder, and not a coercion. It carries no annotation requirement. It has NOT been
through a cheat-reviewer, because the function is not at distance 0 and nothing was submitted.

### E-s4-6. What 12 leaves, and what NOT to spend the next session on.
Still 125 build insns vs 127 target: the two instructions target has and we do not are the
per-preheader uncoalesced copy of the reloaded slots pointer (`lw a0,0xC(s2)` + `move a3,a0`
feeding `addu a0,a1,a3`). s3 argued this is a register-ALLOCATION outcome; s4's two
exhaustions (the 72-cell preheader cross-product, and 21911 permuter iterations scoped to
exactly that region, neither of which produced a 127-insn build) are two more independent
measurements pointing the same way. Do NOT open another spelling sweep of the preheaders and
do NOT run another undirected permuter campaign here. The unspent probe is the one s1, s2 and
s3 all banked and never ran: read the `.lreg` / `.greg` dumps that s4 has already generated
for the 14-form in `tmp/grind/func_80017848/s4/` (regenerate for the 12-form with
`s4/dump.sh`, seconds) and identify the coalesced pseudo pair and the conflict or register
preference that would keep target's copy alive.

- [s4] FLOOR 14 -> 12 this session, verified with the form in place in src/ings.c: sandbox --disable all = 12, target_insns 127, build_insns 125. Second drop in four sessions and the first produced by the permuter modality.

- [s4] The lever is one line: `base = slots;` after the slots read, so the two SCAN-LOOP ENTRY GUARDS read their count through `base` while the two >=0 top guards read through `slots`; `base` is then re-pointed at the per-slot record inside each preheader as before. Control measurement: routing the loop entry guards back through `slots` costs 7 points (19), so the lever is load-bearing, not incidental. The top guards are indifferent (12 either way).

- [s4] Classification of that lever: frozen-list 'variable reuse for codegen control'. `base` is a genuine live pointer READ by both loop guards before its reassignment  -  not a dead store, not a constant-holder, not a coercion, no annotation requirement. It has NOT been through a cheat-reviewer because nothing was submitted (distance is 12, not 0).

- [s4] HEADLINE, transferable beyond this function: decomp-permuter's default scorer is anti-correlated with the engine's honest distance whenever the residual is a small instruction DELETION near the TOP of the body. Measured table (perm -> engine): 405->18, 480->16, 575->14, 585->18, 650->18, 660->19, 680->15, 720->15, 730->16, 770->16, 775->39, 785->15, 797->31, 815->14, 820->15, 825->16, 835->37, 840->17, 845->12, 855(base)->14. Mechanism: reorderings cost 60 and 2 early missing insns displace the whole tail, so base scores 855 ~= 14 x 60 despite 125/127 insns being byte-identical; the cheapest way to lower that is to destroy the correct tail.

- [s4] Operational corollary: a permuter campaign here is a legitimate PROPOSAL GENERATOR (a true match still scores 0 and is still saved) but its ranking must be discarded and replaced by a `sandbox --disable all` rescoring pass over every saved output. Note `permuter_campaign.py launch` does not expose --keep-prob, so campaigns always run decomp-permuter's default 0.6 hill-climb  -  precisely the behaviour that walks away from the basin on a deletion residual.

- [s4] Second operational lesson: SCOPE the randomization. Chassis A (whole-function randomization from the s3 candidate) produced nothing below 14 in ~8 min / ~25 outputs. Chassis B (identical workspace, residual region wrapped in PERM_RANDOMIZE so the 125 byte-exact insns are untouchable) produced two 12-cells in 733 s / 21911 iterations / 130 outputs.

- [s4] The loop-preheader SPELLING family is now closed by exhaustion rather than sampling: the full symmetric 72-cell cross-product of the five axes scores min 14 / max 19 with twelve cells tied at 14 and none below. Corollary that corrects an s3 conclusion: fresh-read-guard cells (a1_b0_*_{d0,d2}_e1) also reach 14, so s3's pointer-first hoisted-guard rule is sufficient for 14 but not necessary.

- [s4] A reproducible per-function permuter workspace now exists and self-validates: tmp/grind/func_80017848/s4/mkws.sh <dir> prints 'base insns: 125  target: 127'. Three gotchas are baked into it so they are never repaid: (a) -mel MUST be on the cc1 line (Makefile CC_FLAGS)  -  tools/mar_perm_workspace.sh predates the 2026-08-04 adoption and is NOT a copyable template; (b) maspsx emits '.ent func_80017848' with NO leading tab, so the mar-workspace's /^\t\.ent\t/ awk extraction silently never fires and swallows the rest of the TU including the INCLUDE_ASM .include of func_80017A44, producing assembler errors that look like a toolchain fault and are not; (c) src/ings.c has a multi-LINE inline-asm string literal near line 931 that cpp passes but pycparser rejects ('Syntax error in base.c')  -  s4/fix_base.py joins it onto one line with an escaped newline.

- [s4] Both campaigns were harvested with --stop before the session ended; no permuter process is left running.

- [s4] cc1 -da RTL dumps for the pre-s4 14-form are on disk (tmp/grind/func_80017848/s4/ings_pp.c.{rtl,jump,cse,loop,combine,cse2,flow,lreg,greg,sched,jump2,dbr,sched2}), regenerable in seconds via s4/dump.sh. This is the probe s1, s2 and s3 all banked and none spent; s4 produced the artifacts but the permuter work consumed the session before the analysis.

## s5 (2026-08-18) — permuter modality

Starting point: the s4 candidate re-applied to `src/ings.c`, sandbox `--disable all`
re-confirmed at **12** (127 target insns, 125 build insns). Floor at end of session: **10**.

### E-s5-1. FLOOR 12 -> 10. The lever is the ENTRY-GUARD ADDRESS OPERAND ORDER, and it is chassis-dependent.
The two scan-loop entry guards must spell their count address **SHIFT-FIRST**:

    if (i < *(s32 *)((slot_a << 6) + (s32)base + 0x1C))    /* s5  -> distance 10 */
    if (i < *(s32 *)((s32)base + (slot_a << 6) + 0x1C))    /* s4  -> distance 12 */

Shift-first makes GCC emit `addu v0,a1,a0` (the `sll` result in the LEFT operand),
which is target's operand order at both preheaders; pointer-first emits `addu v0,a0,a1`.
Two instructions, one per scan loop.

**This directly contradicts a banked s3 conclusion, and the contradiction is the
transferable lesson.** s3 banked "the loop entry guard's count address must be written
POINTER-FIRST" and even filed the shift-first form as a rejected form
(`rejected/loopguard_shiftfirst_costs_2pts.c`). That was measured correctly ON THE S3
CHASSIS. On the s4 `base = slots;` chassis the sign flips: the s5 sweep measured all four
combinations of {pointer-first, shift-first} x {guard through `base`, guard through
`slots`} and only (guard through `base`) x (shift-first) reaches 10. **A rejected form in
this ledger is rejected RELATIVE TO THE CHASSIS IT WAS MEASURED ON.** When a session lands
a structural lever (s4's `base = slots;`), the cheap per-axis spellings that were killed on
the previous chassis are NOT still dead and must be re-measured — this one was worth 2
instructions for the price of a 56-cell sweep.

### E-s5-2. The 10-residual, fully decomposed (10 differing insns, still 125 vs 127).
Per scan loop, TARGET's preheader is

    lw   a0,0xC(s2)      ; pointer loaded BEFORE the entry guard
    sll  a1,s4,6
    addu v0,a1,a0        ; guard count address
    lw   v0,0x1C(v0)
    blez v0,<skip>
    move a3,a0           ; pointer survives the branch and is COPIED
    lw   a2,0x10(s2)
    addu a0,a1,a3        ; loop base = ptr + shift, computed a SECOND time

ours is

    sll  a1,s4,6
    addu v0,a1,a0        ; guard count address, from the long-lived top-guard pointer
    lw   v0,0x1C(v0)
    blez v0,<skip>
    lw   v0,0xC(s2)      ; pointer loaded AFTER the branch
    lw   a2,0x10(s2)
    addu a0,a1,v0        ; ptr + shift computed ONCE

So the residual is: target computes `ptr + (slot_a<<6)` TWICE per loop with a register copy
between the two, we compute it once. The other 4 differing insns are the same property seen
upstream: target's top guards hold the pointer in v1 and let it DIE at the join, so the
second `bltz`'s delay slot is a `nop`; ours keeps it live into the loops (a0), so `reorg`
steals `sll a1,s4,6` into that delay slot and retargets the branch to label+4.
Both `lw ...,0xC(s2)` COUNTS are already equal (6 in the function, 6 in target) — this is a
PLACEMENT difference, not a missing-load difference. Measured, not inferred
(`s5/probe2.sh`).

### E-s5-3. KILLED — hoisting the pointer read ABOVE the entry guard (so one pointer feeds guard and base).
This is the obvious reading of E-s5-2 and it is measured dead. Sweep `variants/` axis G=1
and `variants3/` axis K=1 both place `p = *(u8 **)(ctx + 0xC);` immediately before
`i = 0; if (i < ...)` and read the guard count through `p`. Every such cell scores 14-35;
the best is 17, i.e. **7 instructions WORSE** than the 10-form. Reason, from the
disassembly of `variants3/k11_c0_cp0_s1.c` (122 build insns, 5 short of target): with one
C-level pointer feeding both, cse folds `p + (slot_a<<6)` into a single `addu a0,v1,v0` and
the loop base and the guard address become the SAME pseudo, so the second `addu` and the
copy both disappear. The form gets target's preheader ORDER right and its instruction COUNT
further wrong. Banked as `rejected/read_hoisted_above_entry_guard_costs_7.c`.

### E-s5-4. KILLED — an explicit C-level pointer copy does not survive to `move a3,a0`.
Axis `cp` in `variants3/` inserts a named intermediate (`q = p; base = q + (slot_a<<6);`)
in each preheader, on BOTH the read-after-guard and read-before-guard chassis, with and
without a distinct copy variable per loop. All 32 cells: **cp0 and cp1 score identically**
(10/10, 17/17, 23/23, 31/31 pairwise). GCC 2.7.2 coalesces the copy in every one. The
"reproduce target's uncoalesced `move a3,a0` by writing the copy in C" hypothesis is dead;
the copy in target is an allocator artifact of the pointer being live across the guard
branch, and there is no C-level handle on it via a copy statement. Banked as
`rejected/explicit_pointer_copy_var_is_coalesced_inert.c`.

### E-s5-5. KILLED — removing the `slots` variable from the top guards.
Hypothesis: target's top-guard pointer (v1) dies at the join because there is no C variable
holding it, so the loops must re-read. Sweep 2 (`variants2/`, 40 cells) spells the top
guards as two inline `*(u8 **)(ctx + 0xC)` reads with no `slots` variable at all. Best cells
tie at **10**, none beats it, and the 11-cells mirror the 10-cells exactly one operand-order
step away. Whether the top-guard pointer is a named variable or an inline expression is
codegen-inert here: the `base` seed keeps the value live either way. Banked as
`rejected/top_guards_inline_no_slots_var_ties_at_10.c`.

### E-s5-6. Two directed permuter campaigns on the 10-form: 26.7k iterations, 88 outputs, NOTHING below 10.
Both were built on the validated s4 workspace (`s4/mkws.sh`) from the current 10-form and
launched via `tools/permuter_campaign.py` (telemetry; owner directive 2026-07-07):
  - **chassis C** (`tmp/perm_ings_s5c`, label `s5c_loops`) — s4's `mk_perm_b.py` region:
    `PERM_RANDOMIZE` over the two scan-loop preheaders + bodies only.
    633 s, 13447 iterations, 43 outputs.
  - **chassis D** (`tmp/perm_ings_s5d`, label `s5d_topguards`) — new `s5/mk_perm_d.py`
    region: widened to include the TOP-GUARD block, because at floor 10 four of the ten
    differing instructions live there and chassis B/C froze it.
    633 s, 13293 iterations, 45 outputs.
Every output was re-scored with the engine (`s5/score_outputs.sh`; results in
`*_engine_scores.txt`). Engine-distance distributions:
  C: 3x10, 1x11, 4x12, 5x13, 5x14, 6x15, 7x16, 3x17, tail to 27 (+1 apply-fail)
  D: 4x10, 7x12, 6x13, 4x14, 6x15, 7x16, 2x17, 2x18, tail to 35 (+1 apply-fail)
**No output beat the base.** E-s4-2's anti-correlation reproduced exactly: the permuter's
best-ranked find in each campaign (620) is not among the engine-best cells, and chassis D's
`output-685-1` — a mid-ranked find — ties the base at 10 with a structurally different body.
Both campaigns were harvested with `--stop`. One output per campaign
(`s5c/output-695-1`, `s5d/output-620-1`) could not be scored: `s4/apply_find.py` failed to
locate the function in the permuter's reformatted source and the awk fallback found no
`^s32 func_80017848(` line either. That is 2 of 88 unscored — a known, bounded gap, not a
silent truncation.

### E-s5-7. Session bookkeeping.
- Every campaign launched this session was harvested with `--stop` before the session ended.
- Sweep harnesses are reusable and self-contained: `s5/gen.py`/`gen2.py`/`gen3.py` emit
  variant sets, `s5/sweep.sh`/`sweep2.sh`/`sweep3.sh` engine-score them (~50 cells per
  blocking call), `s5/apply.py` splices a variant into `src/ings.c`, `s5/dis.sh` prints the
  build-vs-target instruction diff, `s5/probe3.sh <variant>` does both for one cell.
  `s5/ings_10form.c.bak` is the restore point every sweep resets to.

### E-s5-8. Chassis E (the fresh-seed reseed) — 31600 iterations, 205 outputs, nothing below 10.
Per the fresh-seed discipline, after C and D plateaued the reseed was a STRUCTURALLY
DIFFERENT base rather than another window on the same one: `tmp/perm_ings_s5e`, label
`s5e_readbeforeguard`, built from `variants3/k11_c0_cp0_s1.c` — the read-BEFORE-guard form
(122 build insns, engine 17) whose preheader ORDER matches target even though its
instruction count is further off — with the widened `mk_perm_d.py` region. Every output
engine-scored (`perm_ings_s5e_engine_scores.txt`): 1x10, 7x11, 1x12, 15x13, 13x14, 15x15,
20x16, 22x17, and a long tail to 121; 4 apply-fails and 1 unscorable. **The single best cell
ties the 10-form and nothing beats it** — and note it came from permuter score 540, i.e.
from the middle of the ranking, not the top, which is E-s4-2 reproducing for the third time.
Harvested with `--stop`; `permuter_campaign.py status` reports 0 live campaigns.

Cumulative permuter evidence for this function across s4+s5: **4 chassis, ~80k iterations,
423 engine-scored outputs, zero finds below the hand-derived floor of the session that ran
them.** Both of this session's floor drops came from directed hand sweeps, not from the
randomizer. That is now a strong prior for s6+: on func_80017848 the permuter is a
proposal generator whose yield below the current floor has been measured at zero, and the
modality ladder should not spend another session on it.


### E-s5-9. CORRECTION, AND IT SUPERSEDES E-s5-1: the 12 -> 10 drop was an UNUSED LOCAL DECLARATION, not the guard operand order. HONEST FLOOR THIS SESSION IS 12, UNCHANGED.
Caught at the end of the session while re-applying the banked candidate: the form
saved from the sweep scored 10, and the same body typed out by hand scored 13. The
difference was a single line in the declaration block - `u8 *q;`, never written and
never read. `s5/gen.py`, `gen2.py` and `gen3.py` each emitted a FIXED declaration
block (`p`, `q`, and in gen3 also `r`) for every generated cell, so any cell that
did not reference `q`/`r` silently carried one or two dead declarations.

Clean re-measurement, declaration block pruned to exactly the variables used
(`s5/gen_clean.py`, each cell applied to `src/ings.c` from a `git checkout`d tree and
scored with `sandbox --disable all`):

| form | unused local | score |
|---|---|---|
| s4 body, guard address POINTER-FIRST | none | **12** |
| s4 body, guard address SHIFT-FIRST   | none | **13** |
| s4 body, SHIFT-FIRST                 | `u8 *q;` | **10** |
| s4 body, POINTER-FIRST               | `u8 *q;` | **12** |

So the dead declaration is worth 3 instructions on the shift-first chassis and 0 on
the pointer-first one, and the guard operand order is worth NOTHING on its own -
shift-first is a 1-instruction REGRESSION without the dead local. **s3's banked
conclusion "the loop entry guard's count address must be written POINTER-FIRST"
STANDS; E-s5-1's claim that it was chassis-relative is WITHDRAWN, as is the
`H-s5-G` frontier item built on top of it.** The candidate's body is therefore
byte-for-byte the s4 body and the floor for s5 is 12.

The dead declaration is NOT shippable: it has no semantic purpose (cheat-checklist
T1), no programmer writes an unreferenced local (T2), and its only appearance in the
function is the declaration itself (T6). It is the dead-local family, and first reach
of an unsanctioned family is a cheat regardless of spelling. It is banked as
`rejected/unused_local_decl_q_contaminates_sweeps.c` explicitly as a MEASUREMENT
ARTIFACT to be avoided, not as a lever to be spent.

**What this costs and what it does not.** Every s5 sweep number carries the same
contamination, so the RANKINGS inside sweeps 1-3 (which cell beats which) are not
trustworthy in absolute terms. What survives untouched is every KILL that was
measured as a matched pair or a large margin, because both sides of those
comparisons carried the identical declaration block:
  - E-s5-3 (hoisting the read above the entry guard costs 7): the whole family sits
    at 14-35 against a same-template base of 10, a margin no dead declaration
    explains.
  - E-s5-4 (an explicit C-level pointer copy is coalesced away): cp0 and cp1 are
    matched pairs from the same template and score IDENTICALLY in all 16 pairs.
  - E-s5-5 (top-guard `slots` variable vs inline expression is inert): same
    template on both sides, ties everywhere.
  - E-s5-6 / E-s5-8 (three permuter chassis, ~58k iterations, 336 engine-scored
    outputs, nothing below the base): the base and the outputs share the template.
  - E-s5-2 (the instruction-level decomposition of the residual) and the load-count
    equality in the s5 evidence are disassembly facts, independent of scoring.

**Two process rules fall out of this, and they generalize past this function.**
  1. A generated-variant sweep MUST prune its declaration block per cell.
     A fixed declaration block turns "which spelling is best" into "which spelling
     tolerates the most dead declarations", and on a function this tight that is a
     3-instruction lie. `s5/gen_clean.py` is the corrected template shape.
  2. ALWAYS re-measure the banked candidate end-to-end from a clean tree before
     writing the outcome. This was caught only because the final verification
     re-applied `candidate.c` from scratch and got a different number than the
     sweep reported. A session that trusts its own sweep log ships a floor that
     does not reproduce.

### E-s5-10. Tooling hazard worth one line: the sandboxed Bash tool writes to an OVERLAY, not the real tree.
Edits made with `python3`/`git checkout` through the Bash tool were visible to
subsequent Bash reads but NOT to the engine (which runs through PowerShell/WSL) -
`sandbox` kept scoring a stale `src/ings.c`, which is what produced two confusing
scores (19, 33) mid-session. Every mutation of `src/ings.c` in this repo must go
through `bash tools/wsl.sh '...'` or the PowerShell wrapper; the Bash tool is safe
for reading and for writing scratch under `tmp/` that is later consumed from WSL.

- [s5] [s5] HONEST FLOOR IS 12, UNCHANGED - and the reason is the session's most transferable finding. s5's sweeps reported a 12 -> 10 drop from spelling the scan-loop entry guards' count address SHIFT-FIRST. Re-measuring the banked candidate from a clean tree scored 13 instead of 10, which exposed that gen.py/gen2.py/gen3.py emitted a FIXED declaration block (`u8 *q;`, and in gen3 `u8 *r;`) for every cell, so cells that never referenced them carried dead declarations. Clean: pointer-first 12, shift-first 13, shift-first + `u8 *q;` 10, pointer-first + `u8 *q;` 12. The dead declaration is worth 3 instructions on one chassis and 0 on the other; the operand order is worth nothing. s3's banked pointer-first conclusion STANDS.

- [s5] [s5] The dead declaration is not shippable and was not shipped: no semantic purpose (T1), no programmer writes an unreferenced local (T2), only appearance is the declaration itself (T6) - dead-local family, first reach of an unsanctioned family. Banked as rejected/unused_local_decl_q_contaminates_sweeps.c as a measurement artifact.

- [s5] [s5] PROCESS RULE 1 (generalizes past this function): a generated-variant sweep MUST prune its declaration block per cell. A fixed declaration block turns 'which spelling is best' into 'which spelling tolerates the most dead declarations', worth up to 3 instructions of false signal here. s5/gen_clean.py is the corrected template shape.

- [s5] [s5] PROCESS RULE 2: always re-apply the banked candidate end-to-end from a clean tree and re-score it before writing the outcome. This contamination was caught only by that step; a session that trusts its own sweep log ships a floor that does not reproduce.

- [s5] [s5] TOOLING HAZARD: the sandboxed Bash tool writes to an overlay, not the real tree. Edits to src/ings.c made with python3/git through the Bash tool were visible to later Bash reads but NOT to the engine (PowerShell/WSL), which kept scoring a stale file and produced two spurious scores (19, 33) mid-session. Mutate src/ only through `bash tools/wsl.sh '...'` or the PowerShell wrapper.

- [s5] [s5] The 10-residual decomposes into ONE property plus its downstream shadow. Per scan loop, target does `lw a0,0xC(s2)` BEFORE the entry guard, `addu v0,a1,a0` (guard address), `blez`, `move a3,a0`, `addu a0,a1,a3` - computing `ptr + (slot_a<<6)` TWICE with the pointer live across the branch. Ours does `addu v0,a1,a0` from the long-lived top-guard pointer, `blez`, then `lw v0,0xC(s2)` and `addu a0,a1,v0` - computing it once. That is 3 differing insns per loop. The remaining 4 are upstream shadow: target's top guards hold the pointer in v1 and let it die, so the second `bltz` delay slot is a `nop`; ours keeps it live, so reorg steals `sll a1,s4,6` into that slot and retargets the branch to label+4.

- [s5] [s5] The `lw ...,0xC(s2)` COUNT is already equal - 6 in our build, 6 in target (measured via s5/probe2.sh over three different forms). The residual is a PLACEMENT difference, not a missing-load difference. Any future hypothesis phrased as 'target has an extra load we are missing' is factually wrong.

- [s5] [s5] KILLED: hoisting the ctx+0xC read above the entry guard so one pointer feeds guard and base. ~52 engine-scored cells, best 17 (7 worse). cse folds `p + (slot_a<<6)` to one pseudo, the second addu and the copy both vanish, and the build drops to 122 insns vs target's 127.

- [s5] [s5] KILLED: an explicit C-level pointer copy (`q = p; base = q + (slot_a<<6);`) as a way to materialize target's `move a3,a0`. 32 cells in matched with/without pairs: cp0 and cp1 score identically in EVERY pair. GCC 2.7.2 coalesces the copy unconditionally.

- [s5] [s5] KILLED: removing the `slots` variable so the top-guard pointer dies at the join. 40 cells with inline-expression top guards tie at 10, never beat it.

- [s5] [s5] KILLED: the s4 frontier's 'third live pointer / different guard-site-to-pointer assignment' lever. Covered as the pv2/share/cp axes across all three sweeps (128 cells); inert at every 10-cell and harmful elsewhere (31-34).

- [s5] [s5] Three directed permuter campaigns (chassis C: scan loops only; chassis D: widened to include the top-guard block, new this session via s5/mk_perm_d.py; chassis E: the structurally different read-before-guard base as the fresh-seed reseed) produced nothing below 10 across 26.7k+ iterations. All were harvested with --stop before the session ended; no permuter process was left running.

- [s5] [s5] E-s4-2's permuter/engine anti-correlation reproduced on the 10-form: each campaign's best permuter-ranked output (620) is not among the engine-best cells, while a mid-ranked output (D's 685) ties the base at 10. Re-scoring every output with `sandbox --disable all` remains mandatory on this function.

- [s5] [s5] Reusable harness banked in tmp/grind/func_80017848/s5/: gen.py/gen2.py/gen3.py emit variant sets, sweep*.sh engine-score ~50 cells per blocking call, apply.py splices a variant into src/ings.c, dis.sh prints the build-vs-target instruction diff, probe3.sh does both for one cell, score_outputs.sh engine-scores a whole permuter workspace, ings_10form.c.bak is the restore point.

- [s5] [s5] The cc1 -da dump probe that s1, s2, s3 and s4 all banked is STILL unspent - s5 spent its session on the sweeps and campaigns that dropped the floor. It is now the highest-value remaining probe and is regenerable for the 10-form in seconds via s4/dump.sh.

- [s5] [s5] Chassis E, the fresh-seed reseed onto a STRUCTURALLY DIFFERENT base (the read-before-guard form, 122 insns / engine 17, whose preheader order matches target even though its insn count is further off): 31600 iterations, 205 outputs, all engine-scored; best cell TIES 10, nothing below. Cumulative across s4+s5: 4 chassis, ~80k iterations, 423 engine-scored outputs, zero finds below the running floor - a strong prior that the ladder should not spend another session on permuter for this function.

## s6 (2026-08-18) — forensics modality (the cc1 -da probe, banked and unspent since s1)

Starting point: the s5 candidate re-applied to `src/ings.c` from a clean tree, sandbox
`--disable all` re-confirmed at **12**. Floor at end of session: **14**, and the
increase is the finding, not a regression — see E-s6-1.

### E-s6-1. CONFIRMED, AND IT INVALIDATES THE ENTIRE S3/S4/S5 CHASSIS: the 12-form is SEMANTICALLY DIVERGENT from the target and can never reach distance 0.
The inherited chassis writes `base = slots;` after the slots read, re-points `base` to
`(slot_a << 6) + (s32)p` inside **loop 1's preheader**, and then lets **loop 2's entry
guard** read its count through `base`:

    if (i < *(s32 *)((s32)base + (slot_a << 6) + 0x20))

On the path where loop 1's guard passed, `base` already holds `ptr + slot_a*64`, so that
guard addresses `ptr + 2*slot_a*64 + 0x20`. The target addresses `ptr + slot_a*64 + 0x20`
on the same path: at `0x80017914`, in loop 1's exit tail, it does `lw a0,0xC(s2)` followed
by `sll a1,s4,6`, and only then falls into `.L8001791C: addu v0,a1,a0 ; lw v0,0x20(v0)`.
That reload exists precisely so the second guard sees a **fresh** pointer.

Two independent proofs, both measurements:
  - **RTL** (`s6/dump/ings_pp.c.cse`, sliced to the function): insn 86 sets `reg/v 79`
    (the `base` user pseudo) to `(plus reg92 reg/v78)` in loop 1's preheader; insn 146,
    loop 2's guard address, is `(set reg111 (plus reg/v79 reg110))` — it consumes that
    same stale `reg/v 79`.
  - **Disassembly** (`s6/build.txt` vs `s6/target.txt`): our loop-2 guard is
    `sll a1,s4,6 ; addu v0,a0,a1 ; lw v0,32(v0)` with `a0` holding loop 1's base; target's
    is `lw a0,0xC(s2) ; sll a1,s4,6 ; addu v0,a1,a0 ; lw v0,0x20(v0)`.

Byte-identical code implies identical semantics. Our C is not semantically equal to the
target on that path, therefore **no compilation of it can produce the target's bytes**,
therefore 12 was never a real floor. This retro-explains the plateau that s3, s4 and s5
all hit: ~230 hand-swept cells and ~80k permuter iterations across four chassis were
searching a neighbourhood around a form that could not close, and the two "missing"
instructions (125 vs 127) were exactly the two pointer reloads the divergence deleted.
Banked as `rejected/base_reuse_loop2_guard_is_semantically_divergent.c`.

**Process lesson, and it generalizes past this function:** the engine's scorer compares
instructions, so it cannot see a semantic divergence that happens to produce the same
instruction *shapes* in different registers. A grind chassis that reuses one variable
across a control-flow join must be re-checked against the target's data flow, not just
against its instruction count. On this function the divergence was worth 2 instructions
of false credit and cost three sessions.

### E-s6-2. The honest, semantically-correct floor is 14, and it beats HEAD's 16.
Seven correct forms measured this session, each applied to a `git checkout`d `src/ings.c`
and scored with `sandbox --disable all` (`s6/score.sh`, `s6/scores.txt`):

| form | shape | score |
|---|---|---|
| **B** | both scan-loop guards read the count through `slots` (the single pre-join read, never re-pointed); each loop's base from its own fresh read after the branch | **14** |
| A | the 12-form plus a fresh `base = *(u8 **)(ctx + 0xC);` before loop 2's guard | 15 |
| D | fresh pre-guard read `p` per loop feeding that loop's guard; base through `slots` | 15 |
| G | D plus an explicit `q = slots;` copy feeding the base | 15 |
| C | re-read into `slots` before loop 2's guard (closest to the HEAD form) | 16 |
| I | D but the base's addend is a copy of the post-join read `p` | 17 |
| F | D but loop 2's base through `p` | 18 |
| — | the divergent 12-form, for reference | (12) |

B is banked as `candidate.c` and re-verified end-to-end from a clean tree at 14 (process
rule 2 from s5). HEAD's untouched while-loop form is 16, so 14 is still the best correct
floor this function has ever had; the inherited levers (1) source-level do-while, (2) the
`i = 0; if (i < count)` entry guard shape, (3) the `slots` read hoisted above the two `>=0`
top guards all survive intact. Only lever (4), `base = slots;`, is retired.

### E-s6-3. FORENSICS, PART 1 — the cse fold that kills the E-s5-3 family is named, and it IS defeatable.
`s6/hoist/ings_pp.c.cse` is the read-before-guard form (E-s5-3, 122 insns, engine 17).
Its preheader is:

    (insn 71  (set (reg 93)     (plus (reg 92) (reg/v 78))))   ; guard address = shift + p
    (jump_insn 78 ...)                                          ; the entry-guard branch
    (insn 83  (set (reg/v 79)   (reg 93)))                      ; base = COPY of the address

The second `(plus shift p)` never reaches the allocator: cse.c finds it already in the
equivalence hash table and rewrites the insn to a register copy, because the preheader
block is the fall-through successor of the guard block and therefore sits in the **same
extended basic block** (cse's table is only reset at a join). That is the exact mechanism
behind E-s5-3's "cse folds `p + (slot_a<<6)` into one pseudo" and it is now insn-level,
not inferred. Local-alloc then coalesces the copy, which is why the family lands 5
instructions short of target.

**The fold is defeatable, and the lever is a live-range one, not a spelling one.** If the
base's addend is a pseudo that was live *before* the join label — variant D/G, where the
guard reads the fresh post-join read `p` and the base reads `slots` — cse cannot prove the
two addends equal (its table was reset at the join and `slots`' value is unknown in the new
EBB). `s6/G/ings_pp.c.combine` shows the result: insn 71 `reg94 = reg/v78 + reg93` and
insn 86 `reg/v80 = reg93 + reg/v77`, two independent `addsi3_internal`s. The build then
contains **both** of target's structural features that the 12-form was faking — the
pre-guard `lw v0,12(s2)` and two separate addus (`s6/build.txt` for D). It costs 1 net
instruction because `slots` must stay live across both loops, hence D/G = 15 vs B = 14.

### E-s6-4. FORENSICS, PART 2 — target's `addu a3,a0,zero` cannot be written in C, and s5's attribution of why was wrong.
E-s5-4 concluded "GCC 2.7.2 coalesces the copy in every one" and attributed it to the
register allocator. The dumps say the copy never gets that far. Variant G writes
`q = slots; base = (u8 *)((slot_a << 6) + (s32)q);` on the fold-defeated chassis — i.e. in
the one regime where s5's kill does not apply by construction — and in `s6/G/ings_pp.c.combine`
the copy's insn is already `(note 82 ... NOTE_INSN_DELETED)`. **combine.c's `try_combine`
substituted the copy into its single use (insn 86) and the dead copy insn was deleted
before local-alloc ever ran.** G scores 15 with 125 build insns, unchanged from D: the copy
statement is completely inert. Two named passes now cover the whole "make a copy appear"
family — cse.c when the two expressions are equal, combine.c when they are not — and no
C-level copy statement in any spelling can survive either. Banked as
`rejected/copy_var_deleted_by_combine_not_allocator.c`.

Where target's copy actually comes from: in target, `a0` (the loop-1 pre-guard pointer) is
**live-out on the guard branch's TAKEN edge** — on the skip path the `blez` at `0x800178C8`
jumps straight to `.L8001791C`, which consumes `a0` as loop 2's guard operand. So the
preheader cannot overwrite `a0` with the base; the allocator emits `addu a3,a0,zero`, then
`addu a0,a1,a3`. That is a live-range property of the *whole function's* pointer flow, and
it is the only remaining source of the 2-instruction shortfall.

### E-s6-5. Session bookkeeping and reusable harness.
- No permuter campaign was launched this session; `permuter_campaign.py` was not used, so
  nothing was left running.
- `s6/dump.sh` regenerates the full `cc1 -da` dump set for whatever is currently in
  `src/ings.c` (seconds). `s6/probe.sh <body.c> <tag>` does the same for an arbitrary body
  without touching `src/ings.c`, dropping the dumps in `s6/<tag>/`. `s6/dump/slice.sh <pass>`
  slices a dump to the `func_80017848` region; `s6/dump/flat.py` collapses each RTL insn
  onto one line (a 640-line region becomes 156 readable lines — this is what made the
  dumps affordable to read at all, and every future forensics session should start there).
  `s6/dis.sh` writes `build.txt` / `target.txt` and diffs them. `s6/score.sh <bodies...>`
  scores a list of bodies from a clean tree.
- The instrumented-cc1 caveat in [[instrumented-cc1-location]] did not bite: plain `-da`
  dumps come from `tools/gcc-2.7.2/build/cc1`, which is the same binary the Makefile uses.

- [s6] [s6] CONFIRMED AND CHASSIS-RETIRING: the inherited 12-form is semantically divergent from the target. `base = slots;` is re-pointed to (slot_a<<6)+(s32)p inside loop 1's preheader while loop 2's entry guard still reads its count through `base`, so on the loop-1-ran path it addresses ptr + 2*slot_a*64 + 0x20 where target addresses ptr + slot_a*64 + 0x20. Target reloads the pointer at 0x80017914 (`lw a0,0xC(s2)` + `sll a1,s4,6` in loop 1's exit tail) precisely so the second guard sees a fresh pointer. Byte-identical code implies identical semantics, so no compilation of the 12-form can produce target's bytes: 12 was never a real floor.

- [s6] [s6] Proof is two independent measurements, not an argument. RTL: in tmp/grind/func_80017848/s6/dump/ings_pp.c.cse, insn 86 sets reg/v79 (the `base` user pseudo) to (plus reg92 reg/v78) in loop 1's preheader, and insn 146 — loop 2's guard address — is (set reg111 (plus reg/v79 reg110)), consuming that stale pseudo. Disassembly: our loop-2 guard is `sll a1,s4,6 ; addu v0,a0,a1 ; lw v0,32(v0)` with a0 holding loop 1's base, against target's `lw a0,0xC(s2) ; sll a1,s4,6 ; addu v0,a1,a0 ; lw v0,0x20(v0)`.

- [s6] [s6] This retro-explains the three-session plateau. ~230 hand-swept cells (s3/s4/s5) and ~80k permuter iterations across four chassis were all searching the neighbourhood of a form that could not close, and the two 'missing' instructions (125 build vs 127 target) were exactly the two pointer reloads the divergence deleted.

- [s6] [s6] PROCESS RULE 3, and it generalizes past this function: the engine's scorer compares instructions, so it cannot see a semantic divergence that produces the same instruction SHAPES in different registers. Any grind chassis that reuses one variable across a control-flow join must be re-checked against the target's DATA FLOW, not just its instruction count. Here the divergence was worth 2 instructions of false credit and cost three sessions.

- [s6] [s6] Honest semantically-correct floor is 14, from variant B: both scan-loop entry guards read their count through `slots` (the single pre-join read, never re-pointed), and each loop's base comes from its own fresh read after the branch. Re-verified end-to-end by re-applying candidate.c to a `git checkout`d src/ings.c and re-scoring (s5's process rule 2). HEAD's untouched while-loop form is 16, so 14 is still the best correct floor this function has ever had.

- [s6] [s6] Full clean-tree measurement of every correct form tried (s6/scores.txt): B (both guards through `slots`) 14; A (12-form + fresh read before loop 2's guard) 15; D (fresh pre-guard read per loop feeding the guard, base through `slots`) 15; G (D + explicit copy var) 15; C (re-read into `slots` before loop 2's guard) 16; I (D but base's addend is a copy of the post-join read) 17; F (D but loop 2's base through `p`) 18.

- [s6] [s6] The inherited levers SURVIVE except lever (4). Source-level do-while, the `i = 0; if (i < count)` entry-guard shape (the phantom-16 frame), and the `slots` read hoisted above the two >=0 top guards are all still in the candidate. Only `base = slots;` — s4's structural lever — is retired, because its entire codegen effect WAS the semantic divergence.

- [s6] [s6] FORENSICS: the cse fold that kills the E-s5-3 hoisted-read family is now insn-level. In s6/hoist/ings_pp.c.cse the preheader is (insn 71) guard address = (plus reg92 reg/v78), (jump_insn 78) the guard branch, (insn 83) `(set reg/v79 reg93)` — a COPY, not the second addu. cse.c found (plus shift p) already in its equivalence hash table because the preheader block is the fall-through successor of the guard block and therefore in the SAME extended basic block (the table is reset only at a join). That is why the family lands 5 instructions short of target.

- [s6] [s6] FORENSICS: the cse fold is DEFEATABLE, and the lever is a live-range one, not a spelling one. If the base's addend is a pseudo that was live BEFORE the join label — variants D/G, guard through the fresh post-join read `p`, base through `slots` — cse cannot prove the addends equal. s6/G/ings_pp.c.combine shows insn 71 `reg94 = reg/v78 + reg93` and insn 86 `reg/v80 = reg93 + reg/v77` as two independent addsi3_internal insns, and the build then contains BOTH of target's structural features the divergent form was faking: the pre-guard `lw v0,12(s2)` and two separate addus. Cost: `slots` must stay live, so D/G = 15 vs B = 14.

- [s6] [s6] FORENSICS, AND IT CORRECTS E-s5-4: target's `addu a3,a0,zero` cannot be written in C, and the pass responsible is combine.c, NOT the register allocator. Variant G writes `q = slots;` feeding the base on the fold-defeated chassis (the one regime where E-s5-4's kill does not apply by construction) and in s6/G/ings_pp.c.combine the copy's insn is already `(note 82 ... NOTE_INSN_DELETED)`: try_combine substituted it into its single use (insn 86) and the dead copy was removed BEFORE local-alloc ran. G scores 15 with 125 build insns, identical to D — completely inert.

- [s6] [s6] Where target's copy actually comes from: in target the loop-1 pre-guard pointer a0 is LIVE-OUT on the entry guard's TAKEN edge — the `blez` at 0x800178C8 jumps to .L8001791C, which consumes a0 as loop 2's guard operand on the skip path. The preheader therefore cannot overwrite a0 with the loop base, so the allocator emits `addu a3,a0,zero` and then `addu a0,a1,a3`. That is a whole-function pointer-live-range property, and it is the only remaining source of the 2-instruction shortfall.

- [s6] [s6] Two named passes now cover the entire 'make a register copy appear' family and both are closed to C: cse.c when the two expressions are provably equal (rewrites the second to a copy, which local-alloc then coalesces), combine.c when they are not (propagates the copy into its single use and deletes it). Do not re-propose a copy statement in any spelling.

- [s6] [s6] Reusable forensics harness banked in tmp/grind/func_80017848/s6/: dump.sh regenerates the full cc1 -da set for whatever is in src/ings.c; probe.sh <body.c> <tag> does the same for an arbitrary body WITHOUT touching src/ings.c; dump/slice.sh <pass> slices a dump to the func_80017848 region; dump/flat.py collapses each RTL insn onto one line (a 640-line region becomes 156 readable lines — this is what made the dumps affordable to read, and every future forensics session should start there); dis.sh writes build.txt/target.txt and diffs them; score.sh <bodies...> scores a list of bodies from a clean tree.

- [s6] [s6] No permuter campaign was launched this session and none was left running. The instrumented-cc1 caveat did not bite: plain -da dumps come from tools/gcc-2.7.2/build/cc1, the same binary the Makefile uses.

## s7 (2026-08-18) - forensics modality (the cse-fold lever, spent on the corrected chassis)

Starting point: s6's candidate B re-applied from a clean tree, sandbox `--disable all`
re-confirmed at **14** / 125 build insns.  Floor at end of session: **11** / 125 build
insns.  First floor movement in three sessions and the largest single-session drop this
function has had since s3.

### E-s7-1. CONFIRMED: the missing pointer reload is a SOURCE-LEVEL reload in loop 1's EXIT TAIL, and it reproduces target's skip-past-the-reload branch exactly.
Target's loop-1 entry guard is `blez v0,.L8001791C` at `0x800178C8`, and `.L8001791C`
(`0x8001791C`) is instruction index 53 of the function - i.e. the branch skips BOTH
`lw a0,0xC(s2)` and `sll a1,s4,6` (indices 51/52, loop 1's exit tail) and lands on
`addu v0,a1,a0`, the second insn of loop 2's guard.  That is only expressible in C if
ONE pointer variable feeds both entry guards and is re-read in loop 1's exit tail:

    p = *(u8 **)(ctx + 0xC);
    i = 0;
    if (i < *(s32 *)((slot_a << 6) + (s32)p + 0x1C)) {
        ... do/while ...
        p = *(u8 **)(ctx + 0xC);      /* <-- the exit-tail reload */
    }
    i = 0;
    if (i < *(s32 *)((slot_a << 6) + (s32)p + 0x20)) { ... }

Measured (variant J, `s7/v/J.c`): the build's `blez v0,1444` skips the `lw a0,12(s2)` at
`1440` and lands on the `sll` at `1444` - target's structure, one insn narrower because
target also carries the shift across the edge.  This is the frontier item s6 handed over
and it is now CONFIRMED as a C-expressible property.  Semantically the reload is a no-op
(the scan loops contain no stores), so this is plain equivalent C, not a coercion.

### E-s7-2. CONFIRMED, AND IT IS THE FLOOR-MOVING FACT: (1) alone is worth NOTHING and (2) alone is worth NOTHING; together they are worth 3 instructions.
Variant J (exit-tail reload, guards AND bases both through `p`) scores **14** at 124
insns - one insn WORSE in body count than s6's B - because both the guard address and the
loop base are `(plus shift p)` in the same cse extended basic block, so cse.c rewrites the
base into a copy of the guard address (loop 1) or drops it entirely and reuses the guard's
address register as the base (loop 2).  That is E-s6-3's fold, reproduced on the new
chassis.  s6's fold-defeat lever (base's addend = the pre-join `slots` read) cost 1 insn on
the OLD chassis (D/G = 15 vs B = 14) and was therefore banked as a loser.  On the corrected
chassis it is the opposite: **variant O = the exit-tail reload + the base's addend sourced
from `slots` scores 11 at 125 insns.**  s7/O/ings_pp.c.combine shows the fold defeated
insn-for-insn: insn 146 `reg111 = reg110 + reg/v78` (guard, `reg/v78` = the post-join `p`)
and insn 158 `reg/v79 = reg110 + reg/v77` (base, `reg/v77` = the pre-join `slots`), two
independent `addsi3_internal`s.  This is a direct instance of s6's process rule 3 in the
other direction: a lever measured on a divergent chassis was banked with the WRONG sign.

### E-s7-3. The whole remaining residual is ONE instruction, appearing once per scan loop, and it is a DEAD register copy.
Normalized target-vs-build diff (`s7/T.txt` vs `s7/B.txt`, produced by `s7/dis2.sh` +
`s7/norm.py`) has exactly two non-register hunks left, one per loop:

    target:  addu a3,a0,zero  /  lw a2,16(s2)  /  addu a0,a1,a3
    ours:    addu a0,a1,a2    /  lw a1,16(s2)

`a3` is written, read once by the very next `addu`, and never read again - in BOTH
preheaders.  Target therefore routes the loop base's addend through a register copy of the
pre-guard pointer that is dead immediately afterwards.  Everything else in the function is
instruction-identical; the only other differences are register NAMES that fall out of
keeping `slots` live (ours slots=a2 / links=a1; target slots=v1, dead after the two `>=0`
top guards, links=a2).  125 build insns vs 127 target.

### E-s7-4. KILLED: placement cannot defeat the cse fold - only the addend's live range can.
Variant M writes the base assignment INSIDE the do-loop body (so cse1 sees it in a
different extended basic block from the guard).  It builds byte-identically to J (124
insns, 14): loop.c hoists the invariant into the preheader and the second cse pass folds it
exactly as cse1 would have.  Variant N (no `base` local at all, the whole expression
inlined in the loop) is also 14.  The fold is a property of the two expressions' addends,
not of where the statement is written.

### E-s7-5. KILLED: an explicit `shift` local carried across loop 1's guard edge.
Target skips both the reload AND the `sll` on the skip edge, so making `slot_a << 6` an
explicit local assigned before loop 1's guard and recomputed in the exit tail looks like
the matching shape.  Measured: 16 on the J chassis (K), **17** on the O chassis (X).  The
extra live pseudo costs far more than the one `sll` it saves; letting GCC recompute the
shift at the join is correct.  The one-insn structural difference this leaves (our skip
edge lands on the `sll`, target's one insn later) is NOT worth attacking directly.

### E-s7-6. KILLED: source-level control of the preheader's lw/addu ORDER.
Target's preheader order is copy / `lw` links / `addu` base; ours is `addu` base / `lw`
links.  Hoisting the links read into an explicit per-loop local placed BEFORE the base
assignment (Y1) scores 15; the same local placed AFTER (Y2) scores 11, identical to O
without the local.  The order is a scheduling artifact, not a source order, and the local
is inert at best.

### E-s7-7. Session bookkeeping and harness.
- No permuter campaign was launched and none was left running.
- `s7/norm.py` + `s7/dis2.sh` replace s6's `dis.sh`: s6's sed pipeline stripped every hex
  offset from the target side and left objdump's alias mnemonics (`move`, `nop`, `bnez`) on
  the build side, so the two files never aligned and the diff was a whole-file rewrite.
  `norm.py` canonicalizes both sides (aliases expanded to their real ops, offsets to
  decimal, branch targets to `LBL`) and produces a diff that is directly readable.  Use it,
  not s6/dis.sh.
- `s7/score.sh <bodies...>` (inherited from s6) scores a list of bodies from a clean tree;
  `s7/ad.sh <body>` applies one body, scores it and prints the normalized diff;
  `s7/probe.sh <body> <tag>` drops a full cc1 `-da` dump set in `s7/<tag>/`.
- `src/ings.c` was left at HEAD (every measurement reverts it); the candidate lives only in
  `memory/grind/func_80017848/candidate.c`.

- [s7] Floor moved 14 -> 11, the first movement in three sessions and the largest single-session drop since s3. The winning form (variant O) is banked as memory/grind/func_80017848/candidate.c and was re-verified from a clean tree (git checkout src/ings.c, re-apply, re-score) at 11.

- [s7] The win required TWO facts at once, and each is worth nothing alone: (1) `p` read fresh before loop 1's entry guard and re-read ONLY in loop 1's exit tail, with both entry guards reading their count through it; (2) each loop's base built from the PRE-JOIN `slots` read instead of `p`. Fact (1) alone (variant J) is 14 at 124 insns; fact (2) alone was measured at 15 by s6 on the divergent chassis; together they are 11 at 125 insns.

- [s7] Fact (1) reproduces target's control flow exactly: target's `blez` at 0x800178C8 jumps to .L8001791C = insn index 53, skipping loop 1's exit-tail `lw a0,0xC(s2)` and `sll a1,s4,6`; our build's `blez v0,1444` skips the `lw a0,12(s2)` at 1440 the same way. The reload is semantically a no-op (the scan loops contain no stores), so this is plain equivalent C, not a coercion.

- [s7] Fact (2) is s6's E-s6-3 lever re-measured on the corrected chassis, where its SIGN flips: cse.c folds the loop base into a copy of the entry guard's address whenever both are `(plus shift p)` in the same extended basic block, and sourcing the base's addend from a pseudo live before the join defeats it. RTL proof in tmp/grind/func_80017848/s7/O/ings_pp.c.combine: insn 146 `reg111 = reg110 + reg/v78` (guard, post-join p) and insn 158 `reg/v79 = reg110 + reg/v77` (base, pre-join slots), two independent addsi3_internal insns.

- [s7] This is the second consecutive session in which a banked spelling conclusion flipped sign after the chassis changed (s6 retired s3/s4/s5's chassis as semantically divergent; s7 finds s6's own D/G verdict inverted). The standing process rule holds: re-run a banked conclusion on the CURRENT chassis before spending it.

- [s7] THE ENTIRE REMAINING RESIDUAL IS ONE INSTRUCTION APPEARING ONCE PER SCAN LOOP. Normalized target-vs-build diff (tmp/grind/func_80017848/s7/T.txt vs s7/B.txt) has exactly two non-register hunks: target `addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3` against ours `addu a0,a1,a2 / lw a1,16(s2)`. In BOTH of target's preheaders a3 is written, read once by the very next addu, and never read again - a dead register copy of the pre-guard pointer.

- [s7] The only other differences are register NAMES that fall out of keeping `slots` live: ours slots=a2 / links=a1; target's top-guard slots read dies after the two `>=0` guards (slots=v1) and links=a2. 125 build insns vs 127 target, frame 0x40 both sides.

- [s7] s6's two pass-level kills on C-level copies still stand and were not re-tested: cse.c folds a copy when the two expressions are equal, combine.c's try_combine propagates and deletes it when they are not. In target the copy is dead after its single use in both preheaders, so it is a pass artifact of a source shape not yet found, not a written copy.

- [s7] In our build the base's addend DIES at the base's own insn (REG_DEAD reg/v77 on insn 158 of s7/O/ings_pp.c.combine), which is exactly why no copy is needed. Any route to target's copy must make that addend NOT die there.

- [s7] Tool-health finding: s6/dis.sh was broken - its sed pipeline stripped every hex offset from the target side and left objdump's alias mnemonics (move / nop / bnez) on the build side, so target and build never aligned and every diff was a whole-file rewrite. s7/norm.py + s7/dis2.sh canonicalize both sides (aliases expanded, offsets to decimal, branch targets to LBL) and produce a directly readable diff. Future sessions should use s7/dis2.sh, not s6/dis.sh.

- [s7] No permuter campaign was launched and none was left running. src/ings.c was left at HEAD (every measurement reverts it); the candidate lives only in memory/grind/func_80017848/candidate.c.

- [s8] FLOOR 11 -> 5. Three stacked levers, each measured on a clean tree with s8/score.sh and each verified in the disassembly with s8/dis2.sh: (1) the loop base's addend is its OWN fresh read of ctx+0xC rather than the top-guard `slots` read (11 -> 9); (2) loop 1's guard address and the count it loads share ONE local, `t = sh + (s32)p; t = *(s32 *)(t + 0x1C); if (i < t)` (9 -> 6); (3) an explicit `sh = slot_a << 6;` local assigned once and shared by both loops (6 -> 5). The best form is memory/grind/func_80017848/candidate.c (s8 variant Z1); the full score table is tmp/grind/func_80017848/s8/scores.txt and the variant bodies are tmp/grind/func_80017848/s8/v/.

- [s8] TWO s7 CONCLUSIONS ARE RETIRED AS CHASSIS-RELATIVE, both by direct measurement, and both in the direction s7 explicitly ruled out. s7 banked "the base's addend must be the PRE-JOIN `slots` read, in BOTH loops" (variant O = 11); a per-loop FRESH read is worth 2 points on that same chassis (B_basefresh = 9). s7 banked "the explicit shift local costs far more than the one `sll` it saves" (K = 16, X = 17); on the s8 Q chassis it is worth 1 point (Z1 = 5 vs Q = 6). The frontier's own warning - every banked spelling conclusion is chassis-relative and must be re-run before it is spent - is now confirmed twice over, and it applies to s8's conclusions too.

- [s8] MECHANISM, EXACT: cse.c hashes a guard's address expression `(plus shift p)` and records the pseudo holding it. Overwriting THAT pseudo with the value loaded through it invalidates the hash entry (cse_insn invalidates on SET_DEST), so a later recomputation of `(plus shift p)` in the preheader can no longer be folded into a copy of the guard address. This is what the guard-address/count variable reuse buys: without it loop 1's base collapses to `addu a0,a1,zero` (a copy of the still-live guard address, one insn short of target); with it the base is genuinely recomputed as `addu a0,a1,a0`.

- [s8] THE LEVER IS LOOP-1-ONLY AND CHASSIS-SPECIFIC. The same guard-address/count reuse applied to loop 2 costs 26 points (P_treuse_both = 32); applied to the s7 O chassis it costs 31 (S_treuse_O = 37). Loop 2 does not need it because loop 2's preheader is a cse JOIN - target's loop-1 `blez` branches to .L8001791C, which sits AFTER both `lw a0,0xC(s2)` and `sll a1,s4,6` - so cse's table is already reset there and loop 2's redundant read survives as a real `lw` unaided.

- [s8] THE ENTIRE REMAINING RESIDUAL IS 5 INSTRUCTIONS, ALL IN THE TWO PREHEADERS (s8/T.txt vs s8/B.txt): loop 1 target `addu a3,a0,zero` + `addu a0,a1,a3` vs ours `addu a0,a1,a0`; loop 2 target `sll a1,s4,6` + `addu a3,a0,zero` + `addu a0,a1,a3` vs ours `lw v0,12(s2)` + `addu a0,a1,v0`. Every other instruction in the function, including all register names, now matches. Loop 2's `sll` is the price of sharing `sh` across both loops - recomputing it (Z5 = 6) or using two shift locals (Z7 = 11) both cost more than the one insn they buy.

- [s8] THE DEAD PREHEADER COPY IS NARROWED, NOT SOLVED. Four independent routes are now dead: (a) a source-level copy statement - re-measured on the current chassis, W1_qcopy = 11, five points WORSE than its base, so s6's pass-level kill survives two further chassis changes; (b) placing the redundant read in the GUARD block so the cse-created copy would land in a different basic block from the base add, where combine.c cannot reach across to propagate it - ZB_readinguard_l1 emits code BYTE-IDENTICAL to Z1 because the read is folded away entirely and no copy is ever created; (c) every addend re-routing (`p` U2 = 11, `slots` U3 = 11 / U4 = 9, fresh read = the candidate, a named preheader intermediate I/J8 = 9, the base written inside the loop body F/U6 = no change); (d) loop.c as the emitter, eliminated by reading its source.

- [s8] loop.c IS ELIMINATED AS THE COPY'S EMITTER (the frontier's named next probe, answered). loop.c emits a preheader copy in exactly two places. `move_movables`'s `m->move_insn` path (loop.c:653-672 sets the flag, :1673-1712 emits it) fires ONLY when the movable's insn carries a REG_EQUIV note, or a REG_EQUAL note whose operand is CONSTANT_P, or a REG_RETVAL libcall block - our addend is a plain memory load and qualifies for none of them. The `m->partial && m->match` path (:1290-1336 builds the match, :1639-1662 emits `gen_move_insn`) is the zero/sign-extension movable combiner: it requires a `reg = 0` movable whose NEXT insn sets a SUBREG of that reg. Neither shape exists in this function. The emitter is therefore some pass AFTER cse2 that combine.c does not undo - the remaining candidates are local-alloc.c, global-alloc.c and reload, and that is where the next forensics session should look.

- [s8] Rederive-modality loop-shape changes were all measured and all lose: `p` reused as the base (A_preuse = 14), a walking pointer through the byte array instead of `base + i` (D_walkptr = 45), the guard count hoisted into a local before the `if` (E_countlocal = 32), the do/while condition through the fresh-read expression (U8 = 21) or assigning into the count local (Y3 = 29), per-loop base locals (C_twobases = 11, a pure rename). The wins were read-routing and variable-reuse levers, not loop shape.

- [s8] No permuter campaign was launched and none was left running. src/ings.c was left at HEAD (every measurement reverts it); the candidate lives only in memory/grind/func_80017848/candidate.c. Harness: tmp/grind/func_80017848/s8/score.sh (scores a whole list of bodies from a clean tree in one blocking call), s8/ad.sh (apply + score + disassembly diff for one body), s8/dis2.sh + s8/norm.py (inherited from s7).

- [s8] FLOOR 11 -> 5, the largest single-session drop on this function so far. Best form saved to memory/grind/func_80017848/candidate.c (s8 variant Z1); full score table in tmp/grind/func_80017848/s8/scores.txt; all 40+ variant bodies in tmp/grind/func_80017848/s8/v/.

- [s8] The winning body stacks exactly three changes on the s7 candidate: (1) the loop base's addend is its own fresh `*(u8 **)(ctx + 0xC)` read per loop rather than the top-guard `slots` read; (2) loop 1's guard is `t = sh + (s32)p; t = *(s32 *)(t + 0x1C); if (i < t)` - one local for both the address and the count; (3) `sh = slot_a << 6;` is a single local assigned once and shared by both loops.

- [s8] MECHANISM, EXACT: cse.c hashes a guard's address expression `(plus shift p)` and records the pseudo holding it. Overwriting THAT pseudo with the value loaded through it invalidates the hash entry (cse_insn invalidates on SET_DEST), so a later recomputation of `(plus shift p)` in the preheader can no longer be folded into a copy of the guard address. Verified in the disassembly: without the reuse loop 1's base is `addu a0,a1,zero`; with it, `addu a0,a1,a0`.

- [s8] The guard-address/count reuse is LOOP-1-ONLY and chassis-specific: applied to loop 2 it costs 26 points (P_treuse_both = 32), applied to the s7 O chassis it costs 31 (S_treuse_O = 37). Loop 2 does not need it because its preheader is already a cse JOIN - target's loop-1 `blez` targets .L8001791C, which sits AFTER both `lw a0,0xC(s2)` and `sll a1,s4,6` (verified in asm/funcs/func_80017848.s:56-59), so cse's table is reset there unaided.

- [s8] TWO s7 CONCLUSIONS ARE RETIRED AS CHASSIS-RELATIVE, both by direct measurement and both in the direction s7 explicitly ruled out: the addend must be a FRESH read, not `slots` (B_basefresh = 9 vs O = 11); and the explicit shift local is worth +1, not -5 (Z1 = 5 vs Q = 6, against s7's K = 16 / X = 17). The frontier's own warning that banked spelling conclusions are chassis-relative is now confirmed twice over - and it applies to s8's own conclusions too.

- [s8] THE ENTIRE REMAINING RESIDUAL IS 5 INSTRUCTIONS, ALL IN THE TWO PREHEADERS (tmp/grind/func_80017848/s8/T.txt vs s8/B.txt): loop 1 target `addu a3,a0,zero` + `addu a0,a1,a3` vs ours `addu a0,a1,a0`; loop 2 target `sll a1,s4,6` + `addu a3,a0,zero` + `addu a0,a1,a3` vs ours `lw v0,12(s2)` + `addu a0,a1,v0`. Every other instruction in the function, including every register name, now matches target.

- [s8] Loop 2's missing `sll` is the deliberate price of sharing `sh` across both loops. Recomputing it before loop 2 (Z5 = 6) or using two shift locals (Z7 = 11) both cost more than the one instruction they buy.

- [s8] THE DEAD PREHEADER COPY IS NARROWED TO ONE SUSPECT REGION. Four routes are now dead: a source-level copy statement (W1 = 11, re-confirming s6 on the current chassis); the redundant read placed in the guard block so the copy would land in a combine-unreachable block (ZB emits code byte-identical to Z1 - the read is folded away and no copy is ever created); every addend re-routing (`p` U2 = 11, `slots` U3 = 11 / U4 = 9, a named preheader intermediate I/J8 = 9, the base written inside the loop body F/U6 = no change); and loop.c as the emitter, eliminated by source reading. What remains is local-alloc.c, global-alloc.c and reload - passes that run after combine and whose output combine never sees.

- [s8] Harness for the next session: tmp/grind/func_80017848/s8/score.sh scores a whole LIST of candidate bodies from a clean tree in ONE blocking call (write the bodies to s8/v/ first); s8/ad.sh applies one body and prints the normalized target-vs-build disassembly diff; s8/dis2.sh + s8/norm.py are inherited unchanged from s7. src/ings.c was left at HEAD - every measurement reverts it - and no permuter campaign was launched or left running.

## Session 9 (2026-08-18, rederive) — floor 5 -> 3; the dead preheader copy is EXPLAINED and REPRODUCED

### E-s9-1 (CONFIRMED, decisive). The dead preheader copy is a COMBINE-SURVIVAL artifact, not a post-combine (allocator/reload) artifact. s6/s7/s8's pass-level conclusion is REFUTED.
Sessions s6, s7 and s8 all concluded that target's `addu a3,a0,zero` / `addu a0,a1,a3`
preheader pair had to be created by a pass running AFTER combine.c (local-alloc,
global-alloc or reload), on the reasoning that "combine deletes any copy it can see"
and that a copy surviving to final assembly therefore cannot have existed at combine
time. That reasoning is wrong in one specific way, and the correction is worth 2 points
per loop.

combine.c does not delete every copy it sees. `can_combine_p` refuses to combine an
insn I2 into I3 when I2's destination register is still USED AFTER I3. A copy insn
`q = p` feeding `base = sh + q` is therefore deleted only when `q` has exactly that one
downstream use. Give `q` a SECOND use that is reached from the base add, and combine
cannot substitute, the copy survives to assembly, and — critically — the addend pseudo
no longer carries REG_DEAD at the base add, so local-alloc's tie-the-output-to-a-dying-
input path (`combine_regs` in block_alloc) does not fire and the base does NOT collapse
into the addend's own hard register.

Reproduced and measured: variant `R2_q_live_past_base` (s9/v/R2_q_live_past_base.c),
loop 1 only:

    q   = *(u8 **)(ctx + 0xC);      /* cse folds the redundant load to `q = p` */
    base = (u8 *)(sh + (s32)q);
    ... do { ... } while (...);
    p = q;                          /* THE SECOND USE */

emits, in loop 1's preheader, `addu a3,a0,zero` followed by `addu a0,a1,a3` — target's
exact two instructions with target's exact register numbers. Before this change the same
preheader was the single instruction `addu a0,a1,a0`.

Supporting RTL evidence (tmp/grind/func_80017848/s9/dump/F_lreg.txt, produced from the
s8 candidate): insn 85 is `(set (reg/v:SI 79) (plus:SI (reg/v:SI 82) (reg/v:SI 78)))`
carrying `(expr_list:REG_DEAD (reg/v:SI 78))`, and the allocno header records
"Register 78 used 5 times across 10 insns; dies in 2 places" / "Register 79 used 10
times across 23 insns; dies in 0 places". That REG_DEAD note on the addend at the base
add is exactly the condition the fix removes.

### E-s9-2 (CONFIRMED). The `sll` shift assignment is a two-site, not four-site, decision, and s8's frontier prediction about it is inverted.
The full 16-way sweep of `sh` vs an inline `(slot_a << 6)` across {loop1 guard, loop1
base, loop2 guard, loop2 base} was run in one batch (s9/v/M0000.c .. M1111.c). Scores
depend ONLY on the loop-2 pair; both loop-1 sites are exactly codegen-neutral:

    (L2G,L2B) = (0,0) -> 6      (0,1) -> 6      (1,0) -> 31     (1,1) -> 5

for every one of the four settings of (L1G,L1B). s8's frontier item #2 predicted that
the shift wanted to be shared by the two GUARDS and recomputed for loop 2's BASE; that
is (L2G,L2B) = (1,0) = 31, the worst cell in the entire space. KILLED.

The correct decoupling, found later in the session, is a SECOND shift local `sh2` for
loop 2 covering BOTH of loop 2's sites (variant V1, 4 -> 3). Guard-only (V2) = 5 and
base-only (V3) = 7; the two sites must move together.

### E-s9-3 (CONFIRMED). The links pointer wants its own local, read BETWEEN the addend read and the base assignment.
`lnk = *(u8 **)(ctx + 0x10);` placed between `q = *(u8 **)(ctx + 0xC);` and
`base = ...` is worth 2 points (R2 = 6 -> S1 = 4); the same local read BEFORE the `q`
read (S2) is 6, i.e. the whole gain is positional. This fixes the position of
`lw a2,16(s2)` relative to the copy/base pair (target: copy, lw a2, base add).
s7 killed the links local outright (its variants X1/X2/X3 = 12/10/7 on the s8 Q
chassis). This is now the FOURTH banked instance of a spelling conclusion inverting
when the chassis changed.

### E-s9-4 (CONFIRMED). Symmetric loop-2 shapes are neutral, not harmful, once each loop owns its own locals.
`t2` as a separate guard-address/count local for loop 2 (mirroring loop 1's s8 lever)
is exactly neutral (P2 = 5 on the s8 chassis); the same lever spelled with the SHARED
`t` local is 31 (P1). s8 had banked "t-reuse on loop 2 costs 26" from the shared-local
spelling only. The full loop-2 mirror of loop 1's preheader (own `r` read, own `lnk2`
local, tail `p = r`) is likewise neutral at 3 (W4) on the V1 chassis.

### E-s9-5 (KILLED, expensive). Every post-loop-2 use site for a live pointer local is fatal.
Target re-reads ctx+0xC three more times in the tail (`lw a1,12(s2)` for
math_Distance3D, and `lw v0,12(s2)` twice for rec_a / rec_b — see s9/T.txt:77, 94, 104).
Routing math_Distance3D's two arguments through a live local so that loop 2's base
addend gains a downstream use costs 19 points (T1 = T3 = T4 = 22 vs V1 = 3). Loop 2
therefore has NO available second-use site after its own do/while, which is precisely
why loop 2 still lacks the copy that loop 1 now has.

### E-s9-6 (KILLED). A source-level `r = p;` copy for loop 2 does not survive, in any of the three placements tried.
`r = p` before loop 2's guard with guard+base both reading r (W1) = 8; `r = p` in loop
2's preheader with a `p = r` tail (W2) = 8; `r = p` in the preheader with no second use
(W3) = 8. All three are 5 points worse than V1. The reason is cse: cse.c registers p
and r as equivalent and the later `p = r` is dead-store-eliminated by flow.c BEFORE
combine ever runs, so r is back down to one use. Loop 1's copy works only because the
copy is manufactured by cse FOLDING A REDUNDANT LOAD (`q = *(u8**)(ctx+0xC)` -> `q = p`)
rather than being written as a copy in the source, and because its second use (`p = q`)
feeds a variable that is genuinely live afterwards (loop 2's guard).

### E-s9-7 (KILLED). Pre-initialising `q` before loop 1's guard so that loop 2's base may legally read it costs 11.
X1 (q pre-initialised by a copy) = 14, X3 (q pre-initialised by its own read) = 14, and
the tail-fresh-read variants X2/X4 = 13. This closes the one route by which loop 2's
base addend could have borrowed loop 1's already-live `q`.

### E-s9-8 (CONFIRMED). Loop-1 base addend `p` vs a fresh re-read is neutral on the s8 chassis; `slots` is not.
N_P_SF = N_F_SF = 5; N_L_SF (the top-guard `slots` local) = 15. s7's conclusion that the
addend should be sourced from `slots` remains dead, and s8's fresh-read conclusion is
confirmed but is now known to be a tie rather than a win.

- [s9] Floor 5 -> 3. Best form is variant V1_sh2_both, saved to memory/grind/func_80017848/candidate.c.

- [s9] Loop 1's preheader is now BYTE-EXACT with target, including the dead reg-reg copy `addu a3,a0,zero` that sessions 6, 7 and 8 all failed to produce and declared invisible to source-level levers.

- [s9] MECHANISM (the session's central result): combine.c does NOT delete every copy it can see. can_combine_p refuses when the copy's destination is used AFTER the insn being combined into. A cse-folded redundant load `q = *(u8**)(ctx+0xC)` becomes a plain copy `q = p`; adding a second downstream use of q (`p = q` at loop 1's exit tail, feeding loop 2's guard) makes the copy survive combine, removes the addend's REG_DEAD note at the base add, and stops local-alloc tying the base into the addend's hard register.

- [s9] RTL evidence: tmp/grind/func_80017848/s9/dump/F_lreg.txt insn 85 is `(set (reg/v:SI 79) (plus:SI (reg/v:SI 82) (reg/v:SI 78)))` with `(expr_list:REG_DEAD (reg/v:SI 78))`; the allocno header reads `Register 78 used 5 times across 10 insns; dies in 2 places` and `Register 79 used 10 times across 23 insns; dies in 0 places`.

- [s9] The copy CANNOT be written as a source-level copy: cse equates the two names and flow.c dead-store-eliminates the second use before combine runs (W1/W2/W3 all = 8). It must be manufactured by cse from a REDUNDANT LOAD, and its second use must feed a variable that is genuinely live afterwards.

- [s9] The 16-cell shift-assignment sweep (M0000..M1111) shows the score depends only on loop 2's guard/base pair; both loop-1 sites are exactly codegen-neutral. (L2G,L2B): (1,1)=5, (0,0)=6, (0,1)=6, (1,0)=31.

- [s9] The correct shift decoupling is a SECOND shift local `sh2 = slot_a << 6;` recomputed immediately before loop 2's guard and used by BOTH of loop 2's sites (V1 = 3). Guard-only (V2) = 5, base-only (V3) = 7 - the two sites must move together.

- [s9] The links local is positional: read after the addend read and before the base assignment = 4; read first = 6.

- [s9] Loop-1 base addend `p` and a fresh re-read are exactly tied at 5 on the s8 chassis (N_P_SF = N_F_SF = 5); the top-guard `slots` local is 15. s7's `slots` conclusion stays dead; s8's fresh-read conclusion is confirmed but is a tie, not a win.

- [s9] The residual 3 instructions are: loop-1 exit tail, target `lw a0,12(s2)` vs ours `addu a0,a3,zero`; loop-2 preheader, target `addu a3,a0,zero` vs ours `lw v0,12(s2)`; and target `addu a0,a1,a3` vs ours `addu a0,a1,v0`.

- [s9] Those 3 are ONE missing fact, not three: loop 2 needs the same cse-folded copy loop 1 now has, which needs a downstream second use, and every second-use site available after loop 2 was measured this session (T/W/X families) and is fatal because target re-reads ctx+0xC freshly three more times in the tail (s9/T.txt lines 77, 94, 104).

- [s9] Symmetric per-loop locals are free; shared locals are catastrophic. s8's 'loop 2 mirror costs 26/31' entries must be re-read as 'SHARED-LOCAL loop 2 mirror costs 26/31' - with its own `t2` the same lever is exactly neutral (P2 = 5), and the full loop-2 mirror is exactly neutral on the V1 chassis (W4 = 3).

- [s9] 11 new disproven forms banked to memory/grind/func_80017848/rejected/ (51 total).

- [s10] CFG FACT, read directly off the target listing (asm/funcs/func_80017848.s:55-66): loop 1's skip branch `blez $v0, .L8001791C` targets 0x8001791C = `addu $v0,$a1,$a0`, the FIRST instruction of loop 2's guard. Loop-1's exit tail (`lw $a0,0xC($s2)` at 0x80017914 and `sll $a1,$s4,6` at 0x80017918) is therefore INSIDE loop 1's if-block, and loop 2's guard block is a JOIN with two predecessors. Our build has byte-identical block structure (branch to 0x144c in the sandbox object).

- [s10] CONSEQUENCE (the session's central structural finding): cse's extended-basic-block cannot reach loop 2's preheader, because an EBB stops at the join label .L8001791C. So target's `addu $a3,$a0,$zero` at 0x80017930 CANNOT be a cse-folded redundant `*(u8 **)(ctx + 0xC)` read - unlike loop 1's copy at 0x800178E4, whose block IS single-predecessor from loop 1's guard. The two copies that look symmetric in the listing have DIFFERENT origins, and s9's H-s9-1 (combine-survival of a cse-folded redundant load) explains loop 1 only.

- [s10] RTL confirmation from the -da dump of the current 3-form (tmp/grind/func_80017848/s10/dump/F_combine.txt): loop 2's preheader is insn 162 `(set (reg 113) (mem (plus (reg/v 72) (const_int 12))))` - a full load, cse never folded it - followed by insn 164 `(set (reg/v 81) (plus (reg/v 85) (reg 113)))` carrying REG_DEAD for BOTH the shift (85) and the pointer (113). Loop 2's base addend dies at the base add, exactly the shape s9 showed produces no copy.

- [s10] A SOURCE-LEVEL copy for loop 2's base addend is dead in every spelling measured: `q2 = p` with a separate local (M4 = 9), with loop 1's `q` shared (M1 = 9, M3 = 8), with a per-loop `base2` (M5 = 9), and with a pre-initialised second pointer split across the guard/base roles (Q3 = 13, Q4 = 9). M4's disassembly shows why: cse propagates the copy into the base add, flow deletes it, AND the loop-1 exit-tail re-read becomes redundant and is deleted too, so the function comes out 12 bytes SHORT. This is H-s9-3 generalised - no spelling of a plain C copy survives to the allocator.

- [s10] SECOND-USE CARRIERS INSIDE THE LOOP ARE WORTHLESS. Routing loop 1's do/while count through the addend (`while (i < *(s32 *)(sh + (s32)q + 0x1C))`, P4 = 6), the element read through the addend (P5 = 7), or both (P6 = 4) never buys the copy, because loop.c hoists the loop-invariant `sh + q` into the preheader and cse2 folds it into the existing `base` - so by the time combine runs the addend has exactly one use again. Same carriers applied to loop 2: P1 = 5, P2 = 6, P3 = 3. This kills s9's frontier probe (a)/(b) as a class, not just as spellings.

- [s10] TAIL SECOND-USE CARRIERS EXTEND s9's T-FAMILY KILL TO EVERYTHING: routing `rec_a` through a live local costs 32 (R2), `rec_b` 32 (R3), both 52 (R4), and via a separate p2 local 14 (R5). Together with s9's math_Distance3D measurement (19-22), ALL THREE of target's post-loop-2 ctx+0xC re-reads are now measured and fatal. There is no free downstream use of loop 2's base addend anywhere in the function.

- [s10] LOOP-2 ADDEND SPELLING IS INSENSITIVE ON THE V1 CHASSIS - inline fresh read (baseline = 3), an explicit `q2` local (P0 = 3), reuse of the s32 local `t` (S1 = 3) and reuse of `p` itself (S4 = 3) all score exactly 3. Only cross-loop sharing hurts: reuse of `slots` = 8, sharing loop 1's `q` = 13. So the addend's NAME/type is not a lever; only its use-count is, and the use-count cannot be raised for free.

- [s10] LOOP-2 LOOP SHAPE RE-CONFIRMED (first re-measurement since s2, on the V1 chassis): `base` precomputed + `while` = 9, plain `while` with the address recomputed in the condition = 14, do/while with the count re-read through the shift+pointer = 7, base recomputed inside the body = 10. The s2-era `if (guard) { do { } while (cond); }` with a `base` local remains strictly optimal.

- [s10] THE SANCTIONED duplicated-statement-into-arms LEVER IS DEAD HERE. Duplicating all of loop 2 into both arms of loop 1's `if` (the only sanctioned way to make loop 2's preheader single-predecessor, which is what cse would need to fold the redundant read into a copy) costs 35 with the fresh-read tail (D1) and 35 with the `p = q` tail (D2). jump2's cross-jumping does not re-merge the arms, because the fall-through arm's preheader gets the copy and the else arm's gets the load, so the tails are not identical.

- [s10] COST ACCOUNTING OF THE RESIDUAL 3, now exact. Loop 1's copy is not free: it is BOUGHT for 2 points by spending the exit tail on `p = q` (`addu a0,a3,zero`) where target has `lw a0,12(s2)`. Removing the purchase (M6: symmetric fresh reads in both loops, no second use anywhere) scores 4 = 3 - 1 (tail now correct) + 2 (loop-1 copy lost). So the true statement of the wall is: each loop's preheader copy costs 2 points to buy and returns 2, and loop 1 only nets out because its purchase instruction happens to sit where target has an instruction of its own.

- [s10] 10 new disproven forms banked to memory/grind/func_80017848/rejected/ (61 total).

- [s10] CFG fact read directly off asm/funcs/func_80017848.s:62 - loop 1's skip branch `blez $v0, .L8001791C` targets 0x8001791C = `addu $v0,$a1,$a0`, the first instruction of loop 2's guard. Loop 1's exit tail (`lw $a0,0xC($s2)` at 0x80017914, `sll $a1,$s4,6` at 0x80017918) is inside loop 1's if-block; loop 2's guard block is a JOIN with two predecessors. Our build has byte-identical block structure.

- [s10] Therefore target's `addu $a3,$a0,$zero` at 0x80017930 cannot be a cse-folded redundant ctx+0xC read: cse's extended basic block stops at the join label. Loop 1's copy at 0x800178E4 sits in a single-predecessor block and IS such a fold. The two visually symmetric copies have different origins, and s9's H-s9-1 explains loop 1 only.

- [s10] RTL confirmation from the -da dump of the current 3-form (tmp/grind/func_80017848/s10/dump/F_combine.txt): loop 2's preheader is insn 162 `(set (reg 113) (mem (plus (reg/v 72) (const_int 12))))` - a full load cse never folded - feeding insn 164 `(set (reg/v 81) (plus (reg/v 85) (reg 113)))` which carries REG_DEAD for BOTH the shift (85) and the pointer (113).

- [s10] A source-level copy for loop 2's base addend is dead in every spelling: private q2 = 9, shared q = 9, shared q with the p=q tail = 8, private base2 = 9, pre-initialised split pointer = 13/9. cse propagates it into the base add, flow deletes it, and the loop-1 exit-tail re-read is deleted with it (M4 comes out 12 bytes short).

- [s10] Second-use carriers inside either loop are worthless as a class: loop.c hoists the loop-invariant `sh + addend` into the preheader and cse2 folds it into the existing `base`, so combine sees a single-use addend again (P4=6, P5=7, P6=4, P1=5, P2=6, P3=3).

- [s10] All three of target's post-loop-2 ctx+0xC re-reads are now measured as second-use carriers and all are fatal: math_Distance3D 19-22 (s9), rec_a 32, rec_b 32, both 52, via a separate local 14.

- [s10] Loop-2 addend spelling is insensitive on the V1 chassis: inline fresh read, an explicit q2 local, reuse of the s32 `t`, and reuse of `p` all score exactly 3. Only cross-loop sharing hurts (slots = 8, loop 1's q = 13).

- [s10] The sanctioned duplicated-statement-into-arms lever is dead here (35/35): jump2's cross-jumping does not re-merge the arms, because the very fold that motivates the duplication makes the two arms textually different.

- [s10] Exact cost accounting of the residual 3: a preheader copy costs 2 points to BUY (the loop's exit tail is spent on the second use, emitting `addu a0,a3,zero` where target has `lw a0,12(s2)`) and returns 2. Loop 1 only nets out because its purchase instruction lands where target has an instruction of its own. Removing the purchase entirely - symmetric fresh reads, no second use in either loop - scores 4, not 1 (M6).

- [s10] 10 new disproven forms banked to memory/grind/func_80017848/rejected/ (61 total). Best form unchanged at 3; candidate.c carries an s10 addendum correcting the s9 header's symmetric-copies reading.

## [s11] STRUCTURAL SWEEP OF THE WHOLE V1 CHASSIS (floor holds at 3)

s11 was the frontier's item #2: re-run every banked s3-era and s8-era spelling
conclusion as a matched pair against the V1 chassis, plus a full structural
sweep of the two regions the residual lives in.  ~120 engine-scored cells, all
with tmp/grind/func_80017848/s11/score.sh (the s9 harness re-pathed); every cell
body is in tmp/grind/func_80017848/s11/v/.  V1 (memory/grind/func_80017848/
candidate.c) re-measured at 3 on a clean tree as cell A_base, and NOTHING found
this session beats it.  The value of the session is that it closes four whole
structural families and inverts two banked s3 conclusions.

### [s11] s3's TWO SURVIVING CONCLUSIONS ARE BOTH RETIRED ON V1.
- **"The loop entry guard's count address must be written POINTER-FIRST" is
  INVERTED.**  On V1 shift-first is optimal at every one of the five address
  sites, and pointer-first is a strict regression at four of them: top guards
  `(s32)slots + (slot_a<<6) + 0x18` = 5 (vs 3), loop-1 guard `(s32)p + sh` = 4,
  loop-1 base `(s32)q + sh` = 4, loop-2 guard `(s32)p + sh2` = 4, loop-2 base
  = 3 (inert).  s3 measured pointer-first 14 / shift-first 16 on the s3 chassis
  and s5 re-confirmed it on the s4 chassis; V1 reverses the sign.  Cells
  A_tg1 / A_g1 / A_b1 / A_c1 / A_d1.
- **"Hoisting the `slots` read ABOVE the two >=0 top guards" is INERT.**
  Deleting the `slots` local entirely and reading `*(u8 **)(ctx + 0xC)` inline
  in both top guards (cell A_tg2) scores 3 - exactly tied with V1.  s3 measured
  no-hoist = 16 against hoisted = 14.  The lever no longer exists.

### [s11] LOOP 2's ENTIRE PREHEADER IS C-INERT ON V1.  This closes s10's
frontier #1 at the C level.  Holding loop-2's guard on the carried `p`, EVERY
spelling of loop 2's preheader produces the identical score:
  - base addend: inline fresh read / named local `q2` / reuse of `q` = same
  - links: inline `*(u8 **)(ctx + 0x10)` / named `lnk2` local = same
  - a second use of the base addend after the loop (`p = q2;`) = same
  - guard shape: inline / two-step through a SEPARATE `t2` local / count-into-
    a-separate-local = all 3
  - association order of the base address = 3 either way
  Measured as the full E cube (12 cells), the G cube (15 cells) and the H cross
  (20 cells: 4 loop-1 tails x 5 loop-2 preheader spellings).  Every carried-
  guard cell equals its loop-1-tail baseline exactly.  The only non-inert cell
  in the H cross is `qreuse` (loop 2 reusing loop 1's `q`/`lnk`), which is worse
  (9/12/14) and is not a loop-2 effect at all - it changes loop 1's codegen.

### [s11] LOCAL DECLARATION ORDER IS COMPLETELY INERT.  56 permutations of the
13-local declaration block (12 rotations, 13 move-to-front, full reverse, 30
random shuffles, seed 1234) ALL score exactly 3.  The sanctioned "named-
intermediate declaration order" family therefore has no purchase on this
function at V1, and no future session should spend a cell on it.  Cells D_*.

### [s11] LOOP 2's GUARD MUST CONSUME THE CARRIED `p` - a hard regime boundary.
Target's asm reads ctx+0xC freshly at the top of loop 2's guard block
(`lw a0,12(s2)`, asm/funcs/func_80017848.s loop-2 preheader), so the obvious
structural move is to spell that as a fresh read in C.  It is catastrophic:
  - fresh read into a NEW local `p2`  = 31 (cell E_fresh_*, 124 insns; the whole
    function's allocation shifts, a0 and v1 swap roles from insn 1 of loop 1)
  - fresh read REUSING `p`            = 9  (R family, 30 cells, all >= 9)
  - fresh read reusing `p` + t-local guard = 36 (S_reuse family, 6 cells)
  - carried `p` (V1)                  = 3
This holds across all 54 R-family cells (3 loop-1 tails x 3 base addends x 3
links spellings x 2 second-use choices) and all 12 S-family cells, i.e. it is
independent of every other loop-2 axis.  It is also why the full target-
symmetric loop 2 (fresh read + two-step guard local + redundant read + links
local, mirroring loop 1 statement for statement) is 10 with fresh names and 36
with variable reuse - both banked in rejected/.

### [s11] s8's TWO LOOP-1 LEVERS ARE RE-CONFIRMED ON V1 (matched pairs).
- loop-1 guard MUST be the two-step `t = sh + (s32)p; t = *(s32 *)(t + 0x1C);
  if (i < t)`: two-step = 3, inline = 7, count-into-local = 7 (G cube).
- reusing `t` for loop 2's guard is still catastrophic: 36 (two-step) / 18
  (count-into-local).  A SEPARATE `t2` local is inert at 3.  s8's kill stands.
- sharing ONE shift local across both loops = 4; s9's second shift local `sh2`
  is still worth 1 (cell T7).

### [s11] THE SECOND-USE MECHANISM IS NOT TARGET'S MECHANISM - proved from the
target listing.  s9's lever buys loop 1's preheader copy by giving the base
addend a downstream use (`p = q;` in loop 1's exit tail).  Target has no such
carrier ANYWHERE: after loop 2 it re-reads ctx+0xC three separate times
(tmp/grind/func_80017848/s10/T.txt lines 77 `lw a1,12(s2)` for the
math_Distance3D args, 94 and 104 for the two record updates), and inside each
loop the addend register (a3) is dead immediately after the base add.  So both
of target's preheader copies exist WITHOUT a second use, and s9's lever is a
coincidental reproduction of loop 1's bytes, not the original mechanism.  The
corollary is that loop 2's copy can never be purchased the s9 way, because
there is no downstream site to purchase it with - independently re-confirming
s9's T-family (19) and s10's R-family kills from the target side rather than
by measurement.

### [s11] THE PURCHASE IS STRUCTURALLY FORCED, AND ITS PRICE IS EXACTLY 1.
Loop-1 exit tail: `p = q;` = 3, `p = *(u8 **)(ctx + 0xC);` = 4, nothing = 12.
The fresh-read tail MAKES loop 1's tail insn match target (`lw a0,12(s2)`) but
loses loop 1's preheader copy, which costs 2 - net +1.  cse cannot rescue it:
the loop-1 exit block's extended basic block starts at the loop-top join, so
the redundant load in the tail has no equivalence entry for `q` and is emitted
as a real load.  Disassembly of both forms is in the session log; H_fr's
residual is 4 = 2 (loop-1 preheader, copy missing, `addu a0,a1,a0` instead of
`addu a3,a0,zero` + `addu a0,a1,a3`) + 2 (loop-2 preheader, unchanged).

### [s11] POINTER-SOURCE VARIANTS, all worse, all matched pairs against V1:
single top read consumed by the top guards AND loop 1 = 9 (re-confirms s5's
E-s5-3 kill on V1); `p = slots;` copy instead of loop 1's fresh read = 6;
loop-1 guard consuming `slots` = 7; loop-2 guard consuming `slots` = 17;
loop-2 guard consuming loop-1's `q` = 12.

- [s11] [s11] V1 (memory/grind/func_80017848/candidate.c) re-measured at 3 on a clean tree as cell A_base; the minimum over all ~120 cells scored this session is 3, so the floor is unchanged.

- [s11] [s11] s3's banked rule 'the loop entry guard's count address must be written POINTER-FIRST' is INVERTED on V1: shift-first is optimal at every one of the five address sites, and pointer-first is a strict regression at four of them (top guards 5 vs 3, loop-1 guard 4, loop-1 base 4, loop-2 guard 4; loop-2 base inert at 3).

- [s11] [s11] s3's banked rule 'hoist the slots read ABOVE the two >=0 top guards' is INERT on V1: deleting the `slots` local entirely and reading *(u8 **)(ctx + 0xC) inline in both top guards scores 3, exactly tied (s3 measured no-hoist = 16 vs hoisted = 14).

- [s11] [s11] LOOP 2's ENTIRE PREHEADER IS C-INERT ON V1. Holding loop 2's guard on the carried `p`, 47 distinct spellings all score identically: base addend (inline fresh read / named local q2 / reuse of q), links (inline / named lnk2), a second use of the addend after the loop, guard shape (inline / two-step through a separate t2 / count-into-a-separate-local), and both association orders. Measured as the E cube (12 cells), the G cube (15 cells) and the H cross (20 cells).

- [s11] [s11] LOCAL DECLARATION ORDER IS COMPLETELY INERT: 56 permutations of the 13-local block (12 rotations, 13 move-to-front, full reverse, 30 random shuffles seed 1234) all score exactly 3. No future session should spend a cell on this family for this function.

- [s11] [s11] LOOP 2's GUARD MUST CONSUME THE CARRIED `p` - a hard regime boundary independent of every other loop-2 axis. A fresh read of ctx+0xC at loop 2's guard costs 6 when it reuses `p` (R family, 54 cells, min 9) and 28 when it uses a new local p2 (E_fresh, 6 cells, all 31; 124 insns vs target 127, with a0/v1 swapping roles from loop 1's first instruction). The fully target-symmetric loop 2 is 10 with fresh names and 36 with variable reuse.

- [s11] [s11] s8's loop-1 levers re-confirm on V1 as matched pairs: the two-step guard local `t = sh + (s32)p; t = *(s32 *)(t + 0x1C); if (i < t)` is 3 against inline 7 and count-into-local 7; reusing `t` for loop 2's guard is still catastrophic (36 two-step / 18 count-local) while a separate `t2` is inert; sharing ONE shift local across both loops is 4, so s9's second shift local is worth 1.

- [s11] [s11] THE s9 SECOND-USE LEVER IS NOT TARGET'S MECHANISM, proved from the target listing rather than by measurement. Target has no live carrier of ctx+0xC out of either loop: it re-reads ctx+0xC three separate times in the tail (s10/T.txt:77 for the math_Distance3D args, :94 for rec_a, :104 for rec_b) and each loop's addend register a3 dies at its base add. So loop 2's copy can NEVER be purchased the s9 way - there is no downstream purchase site - which re-confirms s9's T-family (19) and s10's R-family kills from the target side.

- [s11] [s11] The purchase is structurally forced and its price is exactly 1. Loop-1 exit tail: `p = q;` = 3, `p = *(u8 **)(ctx + 0xC);` = 4, nothing = 12. The fresh-read tail MAKES loop 1's tail instruction match target (`lw a0,12(s2)`) but loses loop 1's preheader copy, which costs 2 - net +1. cse cannot rescue it: the loop-1 exit block's extended basic block starts at the loop-top join, so the redundant load has no equivalence entry for `q` and is emitted as a real load. H_fr's residual is 4 = 2 (loop-1 preheader copy missing) + 2 (loop-2 preheader unchanged).

- [s11] [s11] Pointer-source variants, all worse, all matched pairs against V1: a single top read consumed by the top guards AND loop 1 = 9 (re-confirms s5's E-s5-3 kill on V1); `p = slots;` copy instead of loop 1's fresh read = 6; loop-1 guard consuming `slots` = 7; loop-2 guard consuming `slots` = 17; loop-2 guard consuming loop-1's `q` = 12.

- [s11] [s11] The 3-instruction residual is confirmed unchanged and is a clean SWAP: loop-1 exit tail target `lw a0,12(s2)` vs ours `addu a0,a3,zero`; loop-2 preheader target `addu a3,a0,zero` + `addu a0,a1,a3` vs ours `lw v0,12(s2)` + `addu a0,a1,v0`. Instruction COUNT already matches target's 127 exactly; only these three opcodes/registers differ.

- [s12] THE PARAMETER / TOP-REGION AXIS IS DEAD (s11 frontier item #3, the "prologue/callee-save shape" hypothesis). Homing any parameter into a named local and using that local throughout is a strict regression on V1: `sa = slot_a` = 6, `cx = ctx` = 6, `a1v = arg1` = 6, both slots homed = 7 (either assignment order). The top block's own spelling is inert-or-worse: fused `&&` guard = 3, an extra redundant `slots` re-read between the two guards = 3, the `slots` read moved ABOVE the `slot_a == slot_b` early return = 7, `sh` hoisted above the two guards and shared with loop 1 = 6, the guard VALUES staged through the existing `t` local = 41, slot_b's shift staged into `t` early = 18. The prologue stays byte-exact in every one of these cells; nothing in the parameter-homing region moves the loop-2 tie-break.

- [s12] THE LOOP-BODY SPELLING AXIS IS DEAD. Applied symmetrically to both loops on V1: `<< 4` written as `* 16` = 3, the `!= slot_b { i++; } else { return 0; }` form = 3, staging the byte through `t` = 3, links-base-first association = 5, `(i + 0x24)` grouping = 7, an element-pointer local `q = base + i` = 13. The three ties are true no-ops (identical bytes), not compensating changes.

- [s12] THE EXIT-FORM AND LOOP-SHAPE AXES ARE DEAD. Target's four `return 0` sites all share one block (.L800178AC -> `j .L80017A1C; addu v0,zero,zero`) and jump.c already cross-jumps our inline returns into exactly that shape, so writing it explicitly only hurts: `goto end0` for all four returns = 15, mixed form (loop bodies goto, top returns inline) = 8, an `res` accumulator with a single `end: return res;` = 9. Loop shape: reversed comparison `*(s32 *)(base+0x1C) > i` = 3, a second counter local `j` for loop 2 = 3, reloading the count into a local before the test = 30, for-loop form = 21.

- [s12] THE BASE-POINTER DESTINATION-VARIABLE-IDENTITY AXIS IS DEAD (the sanctioned variable-reuse family, swept over WHICH local receives the base add rather than what feeds it). Separate `base1`/`base2` locals = 3 (either loop alone, or both), loop 2's base written into `p` = 3, into `q` = 8, into `slots` = 8, into `lnk` = 10, into `rec_a` = 41; loop 1's base into `q` = 5, into `slots` = 8.

- [s12] NO COMPOUND WIN EXISTS AMONG THE NEUTRAL CELLS. Eight stacked combinations of every 3-scoring lever found this session (fused top guard, the `!=`-else loop body, `* 16` shift, reversed while test, second counter `j`, separate base1/base2, `(dist << 1) + dist`, redundant slots re-read) - including one cell stacking all seven at once - every one scores exactly 3. The neutral set is a genuine plateau, not a set of mutually cancelling changes.

- [s12] THE SYMMETRIC CHASSIS IS A CLEANER STATEMENT OF THE WALL THAN V1 IS. With target's own loop-1 exit tail (`p = *(u8 **)(ctx + 0xC);`, i.e. `lw a0,12(s2)`), the residual is 4 and PERFECTLY SYMMETRIC: both preheaders are missing target's `addu a3,a0,zero` and both base adds read the wrong register. Loop 1 comes out one instruction SHORT (126 vs target's 127); loop 2 has the right instruction COUNT but emits `lw v0,12(s2)` where target has the copy. So loop 2's copy is a redundant ctx+0xC load that target's compiler FOLDED to a copy and ours did not - even though loop 2's guard block is a two-predecessor join that cse's extended basic block cannot reach (s10's finding, re-verified this session directly from asm/funcs/func_80017848.s: the `blez v0,.L8001791C` at listing line 36 lands on the label at listing line 58, which sits AFTER the exit tail's `lw a0,12(s2); sll a1,s4,6`).

- [s12] ON THE SYMMETRIC CHASSIS THE PREHEADER ADDEND MUST BE A FRESH ctx+0xC READ, AND EVERY SPELLING OF IT IS ONE SCORE. 12 cells: loop-1 addend from `p` / a fresh `q` / a `q = p` copy / a staged `t` / pointer-add / inline fresh = 4 uniformly; loop-2 addend inline-fresh / named `q` / staged `t` / pointer-add / re-associated = 4 uniformly. Every cell whose loop-2 addend is a LIVE pointer instead (the carried `p`, `q = p`, or `slots`) jumps to 9. Dropping loop 2's `lnk` local is mandatory on this chassis (with it, 12); loop 1's `lnk` local is mandatory (without it, 6).

- [s12] DUPLICATING LOOP 2 INTO BOTH ARMS OF THE LOOP-1 GUARD DOES NOT GET CROSS-JUMPED BACK. The idea was to give loop 2's preheader a single-predecessor block so cse could fold its redundant load, then let jump.c's cross-jumping re-merge the two copies into target's join. Measured 35 in both cells (loop-1 arm carrying `p`, and carrying `q`) - jump2 does not merge them, and the duplication costs ~32 points of divergence. This closes the "remove the join" idea for good.

- [s12] The floor is unchanged at 3 (chassis A = memory/grind/func_80017848/candidate.c, re-measured on a clean tree this session as cell T0). ~90 cells measured, none below 3.

- [s12] PARAMETER HOMING IS A STRICT REGRESSION, closing s11 frontier item #3: slot_a into a local 6, ctx 6, arg1 6, both slots 7 either order. The prologue stays byte-exact in every cell, so the 'pseudo creation order in the parameter-homing region' theory has no C-level handle at all.

- [s12] TOP-BLOCK SPELLING is inert or worse: fused && 3, de Morgan two-arm 3, a redundant `slots` re-read between the guards 3, `sh` hoisted above the guards 6, `slots` read above the equality early return 7, slot_b's shift staged early 18, guard VALUES staged through `t` 41.

- [s12] LOOP-BODY SPELLING is dead: `* 16` 3, `!=`-else 3, byte-into-`t` 3 (all byte-identical), links-first association 5, `(i + 0x24)` grouping 7, element-pointer local 13.

- [s12] EXIT FORM is dead AND informative: jump.c already cross-jumps our four inline `return 0`s into target's single shared exit block (.L800178AC -> `j .L80017A1C; addu v0,zero,zero`), so spelling it explicitly only costs - all-goto 15, mixed 8, accumulator 9. LOOP SHAPE likewise: reversed test 3, second counter `j` 3, count-into-local 30, for-form 21.

- [s12] BASE-ADD DESTINATION VARIABLE IDENTITY is dead (the sanctioned variable-reuse family applied to the RECEIVING local rather than the addend): base1/base2 3, loop-2 base into `p` 3, into `q` 8, `slots` 8, `lnk` 10, `rec_a` 41; loop-1 base into `q` 5, `slots` 8.

- [s12] NO COMPOUND WIN: eight stacks of the 3-scoring levers, including one stacking all seven, all score exactly 3.

- [s12] THE SYMMETRIC CHASSIS (new, and the best hand-off this session produces). Replacing the `p = q;` loop-1 exit tail with target's own `p = *(u8 **)(ctx + 0xC);` gives residual 4 that is PERFECTLY SYMMETRIC: both preheaders missing `addu a3,a0,zero`, both base adds reading the wrong register. Loop 1 comes out one instruction SHORT (126 vs target's 127); loop 2 has the right instruction COUNT but emits `lw v0,12(s2)` exactly where target has the copy. Banked as rejected/s12_symmetric_fresh_reload_tail_costs_4.c and rejected/s12_symmetric_no_q_fresh_reload_tail_costs_4.c.

- [s12] On the symmetric chassis the preheader addend MUST be a fresh ctx+0xC read - all 11 fresh-read spellings score 4, all 3 live-pointer spellings (carried `p`, `q = p`, `slots`) score 9. Loop 2's `lnk` local is forbidden there (12) and loop 1's is mandatory (6 without).

- [s12] The join was re-verified this session directly from asm/funcs/func_80017848.s: `blez v0,.L8001791C` at listing line 36 (the loop-1 guard) lands on the label at listing line 58, which sits AFTER the loop-1 exit tail's `lw a0,12(s2); sll a1,s4,6`. Loop 2's guard block therefore has exactly two predecessors and cse's extended basic block stops at it - so target's loop-2 preheader copy is a redundant load FOLDED to a copy in a block our cse provably cannot fold in.

- [s12] DUPLICATING LOOP 2 INTO BOTH GUARD ARMS to remove that join scores 35 in both spellings - jump2 does not merge the duplicated blocks. The join cannot be removed from C.

- [s12] src/ings.c was left untouched (every cell was applied and reverted by tmp/grind/func_80017848/s12/score.sh); the session's only tracked-file changes are the ledger under memory/grind/func_80017848/.

## s13 (permuter) — the automated-breadth modality is now measured, and it is dead on this chassis

- Floor unchanged at 3. `sandbox --disable all` on memory/grind/func_80017848/candidate.c
  re-measured 3 at the start of the session and 3 again at the end, on a clean tree.
- THREE campaigns, all launched through tools/permuter_campaign.py (telemetry in
  metrics/events.jsonl), all waited on IN-TURN via `permuter_campaign.py wait`, all
  `harvest --stop`ed before the outcome was written; `permuter_campaign.py status`
  confirms both surviving pids dead at session end and no permuter.py process alive.
    * s13a  tmp/perm_ings_s13a  V1 chassis, two PERM_RANDOMIZE regions (top guards +
      loop 1; the whole tail), loop 2 frozen.  base perm 405.  25,905 iters / ~27 min.
      2 finds: perm 380 -> engine 5, perm 310 -> engine 4.  Reseeded per the fresh-seed
      discipline after 12 min with no novel find.
    * s13c  tmp/perm_ings_s13c  V1 chassis, NO PERM macros (whole-function
      randomization, loop 2's preheader included).  base perm 405.  27,827 iters /
      ~28 min.  7 finds: engine 3 (= the base, reformatted, perm 405), 4, 5, 7, 9, 11, 12.
    * s13b  tmp/perm_ings_s13b  the s12 symmetric chassis (floor 4), whole-function
      randomization.  base perm 310.  44,872 iters / ~50 min.  8 finds, 7 re-appliable:
      engine 4, 4, 5, 5, 6, 8, 15.  Its floor 4 was never beaten either.
- 98,604 iterations, 16 finds, ZERO engine-scored improvements on either chassis.
- THE PERM-SCORE / ENGINE-DISTANCE ANTI-CORRELATION IS NOW A HARD FACT ON THIS FUNCTION,
  measured a third time.  On s13b the permuter's BEST find (perm 220, a 90-point
  improvement over base) re-scores to engine 5, while the cells that actually tie the
  chassis floor at engine 4 sit at perm 310 = exactly the base score.  Any future
  campaign on this function MUST re-score every output with `sandbox --disable all`;
  the permuter's own ranking is worse than useless here because the residual is 2-3
  register-identity differences that its weighted metric prices below the reorderings
  it happily trades them for.
- What this closes: the s10/s11/s12 frontier item "a fresh-seed permuter campaign
  anchored on the V1 chassis is the highest-yield remaining modality" is now SPENT, on
  both of the two chassis the ledger names, with and without the loop-2 fence.  The
  multi-region combination space that s12 could only sample 8 points of by hand has been
  sampled ~98k times by machine, and it contains nothing below 3.
- Banked forms: rejected/s13_permuter_wholefn_best_find_costs_4.c (s13c's best genuine
  find) and rejected/s13_permuter_symmetric_best_perm_score_costs_5.c (s13b's
  perm-score champion, engine 5 - the anti-correlation exhibit).
- Artifacts: tmp/grind/func_80017848/s13/{perm_ings_s13a,perm_ings_s13b,perm_ings_s13c}
  _engine_scores.txt, harvest_b.json, harvest_c.json, chassis_A.c, chassis_B.c, plus the
  workspace campaign.log files under tmp/perm_ings_s13{a,b,c}/.

- [s13] Floor unchanged at 3. candidate.c re-measured `sandbox --disable all` = 3 at the start of the session on a clean tree and 3 again at the end, after all campaigns were stopped and src/ings.c restored.

- [s13] Three campaigns, 98,604 total permuter iterations, 16 distinct finds, ZERO engine-scored improvements on either chassis. s13a (V1, loop 2 fenced): 25,905 iters, 2 finds, best engine 4. s13c (V1, no fences): 27,827 iters, 7 finds, best genuine engine 4. s13b (symmetric chassis, no fences): 44,872 iters, 8 finds, best engine 4 = its own floor.

- [s13] THE PERM-SCORE / ENGINE-DISTANCE ANTI-CORRELATION IS NOW A HARD, THRICE-MEASURED FACT ON THIS FUNCTION. On s13b the permuter's best find (perm 220, ninety points better than base) re-scores to engine 5, while the two cells that actually tie the chassis floor at engine 4 sit at perm 310 = exactly the base score. Mechanism: the residual here is 2-3 register-IDENTITY differences, which the permuter's weighted metric (regs x5) prices far below the instruction reorderings (x60) it will happily trade them for. Any future campaign on this function MUST re-score every output with `sandbox --disable all`; the permuter's own ranking is worse than useless here.

- [s13] The unfenced campaign (s13c) is the affirmative closure of loop 2's preheader against MACHINE search, complementing s11's 47 hand cells and s12's 23 symmetric-chassis cells: with the randomizer free to mutate the preheader jointly with every other region, the engine-score distribution (3, 4, 5, 7, 9, 11, 12) is monotone in perm score with no outlier below the base.

- [s13] Campaign hygiene: all three campaigns were launched through tools/permuter_campaign.py (telemetry to metrics/events.jsonl), waited on IN-TURN via `permuter_campaign.py wait` (one blocking ~9-min call per window, never hand-polled across turns), and `harvest --stop`ed before this outcome was written. `permuter_campaign.py status` reports both surviving pids dead and `pgrep -af permuter.py` is empty - no campaign outlives the session.

- [s13] s13a was reseeded rather than simmered: after 12 minutes with no novel find it was harvested with --stop and the fresh seed (s13c) was a structurally DIFFERENT chassis (fences removed), per the fresh-seed discipline in .claude/rules/permuter-directives.md.

- [s13] Banked: rejected/s13_permuter_wholefn_best_find_costs_4.c (s13c's best genuine find) and rejected/s13_permuter_symmetric_best_perm_score_costs_5.c (s13b's perm-score champion, engine 5 - the anti-correlation exhibit). src/ings.c is left clean at HEAD; only memory/grind/ ledger files are modified.

- [s13] What this closes for the ladder: the frontier item 'a fresh-seed permuter campaign anchored on the V1 chassis is the highest-yield remaining modality', carried unchanged by s10, s11 and s12, is now SPENT - on both chassis the ledger names, with and without the loop-2 fence. Automated breadth is no longer an untried modality on this function.

- [s14] Floor unchanged at 3. candidate.c re-measured `sandbox --disable all` = 3 at the start of the session (cell M0) and src/ings.c restored to HEAD at the end.

- [s14] THE TRANSLATION-UNIT CONTEXT AXIS - the s13 frontier's item #1 and "the only named axis in thirteen sessions that has never been probed" - IS MEASURED DEAD. Five distinct TU mutations, each applied on top of the unmodified V1 candidate body and re-scored with `sandbox --disable all`: M1 = 40 `extern s32 tu_probe_N(s32);` declarations inserted immediately above func_80017848 -> 3. M2 = 200 extern declarations inserted at the very top of the file, above every existing declaration -> 3. M3 = the math_Distance3D and math_Distance3D_16 DEFINITIONS moved to after func_80017848, replaced in place by prototypes (so the callee is a forward-declared prototype at the call site rather than a completed in-TU definition) -> 3. M4 = an extra complete function definition (`s32 tu_probe_fn(s32 a, s32 b) { return a * b + (a << 3); }`) inserted immediately above func_80017848 -> 3. M5 = ten file-scope `static const s32` declarations immediately above -> 3. Every cell scores exactly 3. Driver: tmp/grind/func_80017848/s14/tu.py.

- [s14] Mechanism for why the TU axis is inert, now that it is measured: GCC 2.7.2 resets its per-function state (pseudo-register numbering via init_emit, cse's hash tables, reg_n_refs / local-alloc's allocno arrays) at each function, so the only state that genuinely accumulates across the translation unit is label_num and the varasm/obstack bookkeeping - none of which is an input to allocation or scheduling. The cc1-first-pass-scheduler-bug memory (~1400 preceding declarations changing cc1's behaviour) is a SEGFAULT / first-pass-scheduler pathology, not a codegen-selection channel, and it does not generalise to allocno ordering. This closes the s13 frontier's highest-yield item at a cost of six sandbox runs.

- [s14] PERMUTER CAMPAIGN 4 (s14a, tmp/perm_ings_s14a, label s14a_V1_directed_crossproduct): 47,221 iterations, 14 finds, best engine re-score 3 = the base floor, i.e. no improvement. This is the first DIRECTED campaign on this function: rather than pure random mutation (which is all s13a/s13b/s13c did), the chassis carries three PERM_GENERAL cross-products over levers the ledger had individually measured at exactly 3 - the top-guard shape (nested ifs vs fused &&), loop 1's element read (`<< 4` vs `* 16` vs the byte staged into the named intermediate `t`), and loop 2's block (base into `base` vs into `p` vs with an explicit `lnk` links local) - layered on top of PERM_RANDOMIZE over loop 1, loop 2 and the tail. The point was precisely the compound space s12 could only sample 8 cells of by hand. The cross-product plus randomization produced nothing below 3 in 47k iterations. Logs: tmp/grind/func_80017848/s14/perm_ings_s14a_engine_scores.txt, perm_body.c.

- [s14] PERMUTER CAMPAIGN 5 (s14n, tmp/perm_ings_s14n, label s14n_m2c_derived_sh_recompute): 34,647 iterations, 11 finds, best engine re-score 3 = the base floor. Seeded on a chassis that did not exist before this session (cell N1, below), whole-function PERM_RANDOMIZE, base perm score 405. Logs: tmp/grind/func_80017848/s14/perm_ings_s14n_engine_scores.txt.

- [s14] THE ANTI-CORRELATION IS NOW MEASURED A FOURTH AND FIFTH TIME, on two chassis neither s13 campaign touched. s14a: the permuter's best find (perm 315, ninety points better than the 405 base) re-scores to engine 5; the only engine-3 output sits at perm 405 = exactly the base score. s14n: same shape - perm 315 -> engine 5, perm 320 -> engine 5, and both engine-3 outputs sit at perm 405. On this function the permuter's ranking is not merely uninformative, it is inverted, and that now holds across five campaigns and 180,472 total iterations (s13's 98,604 plus s14's 81,868).

- [s14] A GENUINELY NEW CHASSIS AT THE FLOOR: cell N1, `sh` RECOMPUTED ON LOOP 1'S TAKEN PATH. Transcribed from m2c's reading of the target asm, which places `var_a1 = temp_s4 << 6` inside loop 1's exit edge rather than in loop 2's preheader. Concretely: delete the `sh2` local entirely, append `sh = slot_a << 6;` as the last statement of loop 1's guarded block (after `p = q;`), and let loop 2's guard and base both read `sh`. Scores 3. Its residual is BYTE-IDENTICAL to V1's - the same three instructions at the same positions (`move a0,a3` vs `lw a0,12(s2)` at insn 52; `lw v0,12(s2)` vs `move a3,a0` at 59; `addu a0,a1,v0` vs `addu a0,a1,a3` at 61) - so it is a genuinely different spelling that lands in exactly the same place. Banked as rejected/s14_sh_recompute_loop1_exit_ties_3_identical_residual.c. The near-miss variant N2 (the same recompute placed BEFORE `p = q;` instead of after) costs 4 - banked as rejected/s14_sh_recompute_before_pq_costs_4.c. A third variant N3 (`sh2 = sh;` on the skip path plus `sh2 = slot_a << 6;` on the taken path, i.e. the dual-path spelling) also ties at 3.

- [s14] M2C'S RAW READING OF THE TARGET IS A BAD BASIN, MEASURED: transcribing `python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c asm/funcs/func_80017848.s` faithfully into compilable C (rotated loops with the increment at the top and `> 0` preheader guards, a WALKING element pointer `elem = base + i` instead of `base + i` indexing, one carried pointer local reused as the copy source in both preheaders, a separate counter for loop 2) scores 49 - sixteen times the V1 floor. This is the direct measurement of a thing eight sessions assumed in passing: m2c's output reflects the SCHEDULED, allocated asm, not the source shape GCC was given, so it is not a useful chassis seed for this function even though it is the most structurally alien starting point available. Banked as rejected/s14_m2c_transcribed_rotated_walking_pointer_costs_49.c. The one durable thing m2c contributed is the `sh`-recompute placement that became N1.

- [s14] Campaign hygiene: both campaigns launched via tools/permuter_campaign.py (telemetry to metrics/events.jsonl), waited on IN-TURN with `permuter_campaign.py wait` in a bounded loop (tmp/grind/func_80017848/s14/waitloop.sh - one blocking call per window, never hand-polled across turns), both `harvest --stop`ed before this outcome was written, `permuter_campaign.py status` reports zero alive. src/ings.c restored to HEAD; only memory/grind/ ledger files and tmp/ scratch are touched.

- [s14] Floor unchanged at 3; candidate.c body unchanged. src/ings.c restored to HEAD; only memory/grind/ ledger files plus tmp/ scratch are touched.

- [s14] TU-context axis measured dead: 6 sandbox cells (baseline + 5 mutations), all exactly 3. 40 externs adjacent, 200 externs at file top, callee definitions moved below the function and replaced by prototypes, an extra whole function definition above, ten file-scope static consts above.

- [s14] Campaign s14a: 47,221 iterations, 14 finds, best engine re-score 3 = base floor. First DIRECTED (PERM_GENERAL cross-product) campaign on this function; harvest --stop'ed after a 547s no-novel-find window.

- [s14] Campaign s14n: 34,647 iterations, 11 finds, best engine re-score 3 = base floor. Seeded on the new N1 chassis, whole-function PERM_RANDOMIZE; harvest --stop'ed.

- [s14] Perm-score / engine-distance ANTI-CORRELATION reproduced a 4th and 5th time on two chassis s13 never touched: s14a perm 315 -> engine 5 while the only engine-3 output sits at perm 405 = base; s14n perm 315 -> engine 5, perm 320 -> engine 5, both engine-3 outputs at perm 405. Five campaigns, 180,472 total iterations, all agreeing.

- [s14] NEW STRUCTURAL FACT (cell N1): `sh` recomputed on loop 1's TAKEN path (delete the `sh2` local; append `sh = slot_a << 6;` as the last statement of loop 1's guarded block, after `p = q;`; loop 2's guard and base both read `sh`) ties the floor at 3 with a residual byte-identical to V1's. Placing the recompute BEFORE `p = q;` costs 4. The dual-path spelling N3 also ties at 3.

- [s14] m2c's faithful transcription (rotated loops, walking element pointer, carried copy-source local, separate loop-2 counter) scores 49 - m2c is not a chassis seed for this function.

- [s14] Both campaigns launched through tools/permuter_campaign.py with telemetry, waited on IN-TURN via `permuter_campaign.py wait` in a bounded loop, and harvest --stop'ed before this outcome was written; `permuter_campaign.py status` reports zero alive campaigns.

- [s14] Banked: rejected/s14_m2c_transcribed_rotated_walking_pointer_costs_49.c, rejected/s14_sh_recompute_before_pq_costs_4.c, rejected/s14_sh_recompute_loop1_exit_ties_3_identical_residual.c (the tied-at-3 alternate chassis, kept for a future session that wants a non-V1 seed).

## s15 (2026-08-18, forensics) — the preheader copy's producing pass is NAMED, and loop 2's preheader is no longer "C-inert"

Floor unchanged at 3 (candidate body re-measured on a clean tree = 3).  All work
done with the INSTRUMENTED cc1 (`tools/gcc-2.7.2/cc1`, the BB2_*_DEBUG binary —
NOT `build/cc1`), re-proved CODEGEN-IDENTICAL to the frozen `build/cc1` on this
whole TU in every one of the four dump runs (harness:
`tmp/grind/func_80017848/s15/idump.sh`, which diffs `shipped.s` vs `ings.s`
ignoring the `# options` comment before emitting anything).  Per-pass RTL slices
for func_80017848 are cut out of the full-TU dumps by
`tmp/grind/func_80017848/s15/icand/ex.py` into `F_<pass>.txt`.

### E-s15-1  THE PASS IS local-alloc.c's `optimize_reg_copy_1` (MEASURED, not inferred)

Loop 1's preheader in the candidate is, after cse (F_cse.txt):

    (insn 83)  (set (reg/v 80) (reg/v 79))                     ; q = p   (cse fold)
    (insn 86)  (set (reg/v 77) (mem (plus (reg/v 72) 16)))     ; lnk
    (insn 89)  (set (reg/v 81) (plus (reg/v 84) (reg/v 79)))   ; base = sh + p

Note insn 89 reads **reg79**, the ORIGINAL pointer, not the copy's dest reg80.
That is still true in `.loop`, `.cse2`, `.flow`, `.combine` and `.sched` — byte
for byte the same insn.  In `.lreg` it reads **reg80**, and reg79's REG_DEAD note
has moved from insn 89 up onto insn 83.  So the operand swap that produces
target's `addu a3,a0,zero` / `addu a0,a1,a3` pair happens inside **local-alloc**,
in `optimize_reg_copy_1` (`tools/gcc-2.7.2/local-alloc.c:700`, dispatched from
`local-alloc.c:1006`).  Its documented contract: "INSN is a copy from SRC to
DEST, both registers, and SRC does not die in INSN.  Search forward to see if SRC
dies before either it or DEST is modified, but don't scan past the end of a basic
block.  If so, we can replace SRC with DEST and let SRC die in INSN ... this may
enable DEST to be tied to SRC, thus often saving one register in addition to a
register-register copy."  It does NOT delete the copy, and nothing after
local-alloc runs DCE, so the copy reaches the assembler.

This retires six sessions of inference (s6/s7/s8 said "a pass after combine";
s9 said combine-survival; s10-s14 said "unidentified origin").  Both halves are
now measured: combine-survival explains why the copy is still there, and
`optimize_reg_copy_1` explains why the base add consumes it.

### E-s15-2  WHY THE COPY SURVIVES COMBINE — flow.c makes LOG_LINKS only inside one basic block

`flow.c:2102` guards LOG_LINK creation with `if (y && (BLOCK_NUM (y) ==
blocknum) ...)`.  combine only ever follows LOG_LINKS.  In loop 1, insn 83's
destination reg80 has exactly ONE pre-lreg use — insn 141 (`p = q`) — and insn
141 lives in the loop-1 EXIT TAIL, a different basic block.  So no LOG_LINK
points at insn 83 and combine never considers folding it.  Confirmed in
F_combine.txt: insn 83 is untouched and insn 89 still reads reg79.
**General rule established:** a reg-reg copy in a preheader survives combine iff
no insn in that same basic block uses its destination.

### E-s15-3  LOOP 2'S PREHEADER LOAD IS A REAL LOAD IN RTL (s10's join argument, confirmed at the RTL level)

F_cse.txt insn 162 is `(set (reg 113) (mem (plus (reg/v 72) 12)))` and insn 164
is `(set (reg/v 81) (plus (reg/v 85) (reg 113)))`.  cse did not fold the load to
a copy, exactly as s10 predicted from the CFG (loop 1's `blez` lands on loop 2's
guard, making it a two-predecessor join outside cse's extended basic block).  In
`.lreg` insn 162 carries a REG_EQUIV note for the MEM and reg113 is REG_DEAD at
insn 164 — a dying, single-use load, the opposite of the shape
`optimize_reg_copy_1` needs.

### E-s15-4  LOOP 2'S PREHEADER IS **NOT** C-INERT — loop.c CAN be made to put a reg-reg copy there

s13/s14's frontier said loop 2's preheader "has no C-level handle at all."  That
is now false.  Cell C1 (`rejected/s15_body_invariant_copy_of_base_hoisted_copy_survives_costs_6.c`)
puts a loop-INVARIANT reg-reg copy inside loop 2's body
(`b2 = base;`, with the body's element read and loop condition using `b2`).
loop.c's `move_movables` (`loop.c:1690-1710`) hoists it verbatim into the
preheader, and because its uses stay inside the loop BODY — a different basic
block — E-s15-2 applies and combine cannot fold it.  The emitted preheader is

    lw   $2,12($18)
    lw   $6,16($18)
    addu $4,$5,$2
    move $5,$4          <-- the hoisted copy, in target's block, from pure C

Score 6 (the copy is of the BASE, not of the addend, so it lands one slot too
late and adds an instruction).  This is a real, previously unknown C-level lever
on the exact block eight sessions declared unreachable.

### E-s15-5  KILLED — the hoisted copy dies the moment its use is hoisted with it

Cell D1 (`rejected/s15_hoisted_copy_plus_base_combine_deletes_copy_costs_3.c`)
names loop 2's addend (`q2 = *(u8 **)(ctx + 0xC);` in the preheader) and puts
BOTH `b2 = q2;` and `base = (u8 *)(sh2 + (s32)b2);` inside the loop body as
invariants, aiming at target's copy-then-base-add order.  loop.c hoists both, and
they land in the SAME basic block, so a LOG_LINK now exists, combine substitutes
and deletes the copy: D1's preheader is byte-identical to the candidate's
(`lw $2,12($18) / lw $6,16($18) / addu $4,$5,$2`) and it scores 3, not less.
D2 (copy invariant in the body, base add left in the preheader) = 9.
C2 (invariant RE-COMPUTE of the base in the body instead of a copy) = 5.
So the survival condition of E-s15-2 is confirmed from both sides.

### E-s15-6  KILLED — every "loop-2 addend is a copy/alias of the live pointer p" cell costs 8, and the reason is now mechanical

A1 (`r = p;` in the join block, guard on `p`, base on `r`), A2 (guard on `r`,
base on `p`), A3 (base reuses `p` directly) and A4 (`r = p;` inside the
preheader) ALL score 8.  The `-da` dump of A1 shows why, and it is not
allocation: making the base addend provably equal to the guard's addend lets cse
merge the guard-address add and the base add into ONE insn —

    .L171:  sll  $2,$20,6
            addu $4,$2,$4        <-- serves as BOTH the guard address and the base
            lw   $2,32($4)

whereas target computes `sh2 + ptr` TWICE (`addu v0,a1,a0` for the guard at
0x8001791C and `addu a0,a1,a3` for the base at 0x80017938).  So the whole
"reuse the carried pointer" family is structurally excluded, not merely
expensive: it destroys an instruction target has.  This upgrades s11/s12's bare
numbers (8 / 9 / 19 / 28 / 41) to a mechanism, and it means loop 2's base addend
MUST be a value cse cannot prove equal to the guard's addend — i.e. a fresh
ctx+0xC load, which is exactly what the candidate already does.

### E-s15-7  The residual, restated in RTL terms

Target's loop-2 preheader copy `addu a3,a0,zero` has a destination that is used
exactly once, at the base add, in the same basic block, and dies there (a3
appears in the whole target listing only at 0x800178D0/0x800178D8 and
0x80017930/0x80017938 — `grep a3 asm/funcs/func_80017848.s`).  By E-s15-2 any
such copy present before combine is deleted by combine; by E-s15-5 a hoisted one
is deleted too; and any copy whose destination IS used out-of-block leaves that
use visible in the asm (our loop 1 pays for it with `move $4,$7`, C1 pays for it
inside the body) and target has no such instruction.  The copy therefore has to
be created, or made unfoldable, by something that runs AFTER combine.  The only
remaining candidate identified in the 2.7.2 sources is local-alloc's
`optimize_reg_copy_2` (`local-alloc.c:874`), which fires on a copy whose SRC dies
in it when a REVERSE copy `SRC = DEST` appears later in the same block before any
label / jump / LOOP_BEG / LOOP_END note, replaces DEST with SRC in between, and
then LEAVES BOTH COPY INSNS IN PLACE because no DCE runs after local-alloc.
That predicate has never been probed from C.

### s15 artifacts

    tmp/grind/func_80017848/s15/idump.sh          instrumented-cc1 dump harness (+identity proof)
    tmp/grind/func_80017848/s15/icand/F_*.txt     per-pass RTL slices for the candidate body
    tmp/grind/func_80017848/s15/icand/cc1.log     PRIODBG/ALLOCDBG/QTYDBG trace, candidate body
    tmp/grind/func_80017848/s15/iA1/ings.s        A1 asm (guard/base add collapse)
    tmp/grind/func_80017848/s15/iC1/ings.s        C1 asm (hoisted copy present in loop 2 preheader)
    tmp/grind/func_80017848/s15/iD1/ings.s        D1 asm (hoisted copy deleted by combine)
    tmp/grind/func_80017848/s15/scores.txt        cell scores

- [s15] The instrumented cc1 is tools/gcc-2.7.2/cc1 (NOT build/cc1) and was re-proved CODEGEN-IDENTICAL to the frozen build/cc1 on the entire src/ings.c translation unit in all four dump runs this session; harness tmp/grind/func_80017848/s15/idump.sh performs that diff before emitting anything.

- [s15] MEASURED: loop 1's base add reads reg79 (the original pointer pseudo) in .cse, .loop, .cse2, .flow, .combine and .sched, and reg80 (the preheader copy's destination) in .lreg. reg79's REG_DEAD note moves from the base add onto the copy in the same step. The producing routine is optimize_reg_copy_1, tools/gcc-2.7.2/local-alloc.c:700, dispatched at local-alloc.c:1006.

- [s15] MEASURED: flow.c:2102 builds a LOG_LINK only when BLOCK_NUM(y) == blocknum, so combine can never fold a copy whose destination is used in a different basic block. Loop 1's copy dest reg80 has exactly one pre-lreg use, insn 141 (`p = q`) in the loop-1 exit tail, which is why the copy survives combine untouched (F_combine.txt).

- [s15] MEASURED: loop 2's preheader load is a genuine MEM load in RTL (F_cse.txt insn 162, `(set (reg 113) (mem (plus (reg/v 72) 12)))`, REG_EQUIV note in .lreg, reg113 REG_DEAD at the base add insn 164). cse did not fold it - the first RTL-level confirmation of s10's join-block argument.

- [s15] NEW LEVER: a loop-invariant reg-reg copy written inside loop 2's BODY is hoisted verbatim into loop 2's preheader by loop.c's move_movables (loop.c:1690-1710) and survives combine. Cell C1's preheader is `lw $2,12($18) / lw $6,16($18) / addu $4,$5,$2 / move $5,$4`. This refutes s13/s14's frontier statement that the block has no C-level handle.

- [s15] MEASURED KILL: hoisting the copy together with its consumer puts both in one block, so combine deletes the copy - cell D1 scores 3 with a preheader byte-identical to the candidate's. D2 = 9, C2 = 5.

- [s15] MEASURED KILL with mechanism: A1/A2/A3/A4 (every spelling of 'loop 2's base addend is a copy or alias of the carried pointer') all score 8, because cse merges the loop-2 guard-address add and the base add into ONE insn while target emits both. A1 asm: `.L171: sll $2,$20,6 / addu $4,$2,$4 / lw $2,32($4)`.

- [s15] Target's loop-2 copy destination a3 is used exactly once, at the base add, in the same basic block, and dies there: `grep a3 asm/funcs/func_80017848.s` returns only the prologue use plus 0x800178D0/0x800178D8 and 0x80017930/0x80017938.

- [s15] Candidate body re-measured on a clean tree this session: sandbox --disable all = 3. src/ings.c was reverted to HEAD before the session ended; no build-pipeline file was touched.

## s16 (2026-08-18, forensics) — the SURVIVAL TRICHOTOMY is complete, and the trigger for it is named

Floor unchanged at 3 (A_base re-measured on a clean tree = 3).  20 cells scored
(`tmp/grind/func_80017848/s16/scores.txt`, `scores2.txt`); three instrumented-cc1
`-da` dump runs, each re-proved CODEGEN-IDENTICAL to the frozen `build/cc1` on the
whole `src/ings.c` TU by `tmp/grind/func_80017848/s16/idump.sh`.  Harness:
`tmp/grind/func_80017848/s16/{gen.py,gen2.py,score.sh,dump.sh,ex.py}`.

### E-s16-1  THE PREHEADER-COPY SURVIVAL TRICHOTOMY (all three arms now measured)

A reg-reg copy written into a loop preheader from C meets exactly one of three
fates, and which one is decided by WHERE its destination is used:

  (a) DEST UNUSED  -> deleted by **cse**, three passes before local-alloc.
      Cell I1 (`rejected/s16_dead_copy_loop2_preheader_deleted_by_cse_costs_3.c`)
      writes `q = p;` into loop 2's preheader and leaves the base add on a fresh
      inline `*(u8 **)(ctx + 0xC)` read.  Occurrence count of `reg/v:SI 80` per
      dump: .rtl 4, .jump 4, **.cse 3**, .loop 3, .cse2 3, .flow 3, .combine 3.
      The copy is gone at cse; flow never even sees it.  Score 3 = base, i.e. the
      construct is completely inert.  (This retires the standing assumption in
      s15's frontier that flow.c's DCE is the gate — cse gets there first.)

  (b) DEST USED IN THE SAME BASIC BLOCK -> deleted by **combine**.  Measured in
      cell E2 below.  flow.c:2102 builds the LOG_LINK (same block), combine
      substitutes and emits NOTE_INSN_DELETED over the copy.

  (c) DEST USED IN A DIFFERENT BASIC BLOCK -> **survives** to the assembler
      (no LOG_LINK, and nothing sees it as dead because the out-of-block use is
      real).  This is the candidate's loop 1, and the out-of-block use costs one
      instruction (`move $4,$7` for `p = q`).

Target's loop-2 copy fits NONE of the three: its destination `a3` is used exactly
once, at the base add, in the SAME block (arm (b)), yet it is not deleted.

### E-s16-2  THE cse CANONICALISATION FLIP — the loop-1 exit tail decides arm (b) vs (c)

This is the session's central new measurement, and it is a same-insn-number,
same-loop-1-C comparison of two dumps:

    candidate (V1, tail `p = q;`)      s15/icand/F_cse.txt:177
      (insn 89 ... (set (reg/v:SI 81) (plus:SI (reg/v:SI 84) (reg/v:SI 79))))
    cell E2   (tail `p = *(u8**)(ctx+0xC);`)  s16/iE2/F_cse.txt:178
      (insn 89 ... (set (reg/v:SI 81) (plus:SI (reg/v:SI 84) (reg/v:SI 80))))

Loop 1's preheader C is character-identical in the two cells
(`q = *(u8**)(ctx+0xC); lnk = *(u8**)(ctx+0x10); base = (u8*)(sh + (s32)q);`),
insn 83 is `(set (reg/v:SI 80) (reg/v:SI 79))` (the cse fold of the redundant
read) in BOTH, and reg79 = `p`, reg80 = `q` in both.  The ONLY C difference is
the loop-1 EXIT TAIL, which is downstream of insn 89.  Yet cse canonicalises the
base add's addend to the ORIGINAL (reg79) in the candidate and to the COPY DEST
(reg80) in E2.  Consequence, measured in E2's F_combine.txt:196-210 — insn 83
becomes `(note 83 81 86 "" NOTE_INSN_DELETED)` and insn 89 is rewritten back to
`(plus (reg 84) (reg 79))`, i.e. combine deletes the copy — whereas in the
candidate the copy is untouched through .combine and `optimize_reg_copy_1` later
re-points the base add onto it (E-s15-1).

So the whole 3-instruction residual traces to ONE cse decision, and the C-level
handle on that decision is the loop-1 exit tail's spelling.  The cse-internal
reason (which member of the equivalence class `qty_first_reg` names at insn 89,
and why a downstream set of reg79 changes it) is NOT yet pinned and is the
sharpest forensics question left on this function.

### E-s16-3  KILLED — reg_n_sets > 1 does not protect a copy from combine

H-s16-A predicted that giving the copy's destination TWO sets (one shared local
serving as the addend in both preheaders, mirroring target's `a3` which is set at
0x800178D0 and 0x80017930 and used at 0x800178D8 and 0x80017938) would make
combine refuse the substitution.  It does not.  In cell E2 `q` (reg80) is set in
both preheaders — F_cse.txt insn 83 and insn 162 — and combine still deleted
insn 83.  Scores: E1 (V1 chassis, shared `q`) = 13, E2 (symmetric chassis, shared
`q`) = 4 (identical to the plain symmetric chassis, i.e. sharing is neutral
there), E6 (shared `q` + loop-2 links local) = 9, E7 (symmetric + both) = 33.
E2's emitted loop-1 preheader is `lw $6,16($18) / addu $4,$5,$4 / addu $2,$4,$3`
— one instruction SHORT of target, exactly s12's 126-vs-127 observation, now with
the producing pass named.

### E-s16-4  KILLED — `optimize_reg_copy_2` (s15's frontier item #1)

Two independent grounds, one analytic and one measured.
  * ANALYTIC: `local-alloc.c:874-935` never deletes either copy insn — it rewrites
    the range and removes two REG_DEAD notes, leaving BOTH `(set D S)` and
    `(set S D)` in the stream.  Any function it fires on therefore emits TWO move
    instructions in that block.  Target's loop-2 preheader contains exactly one
    move (`addu $a3,$a0,$zero`, 0x80017930).  `optimize_reg_copy_2` cannot be the
    producer of target's shape, whatever C is written.
  * MEASURED: both C spellings of its predicate cost 13 —
    G1 (`r = read; q = r; r = q; base = sh2 + r;`, a fresh pair) and
    G2 (`q = read; p = q; q = p; base = sh2 + q;`, spelled through the live
    locals as the s15 frontier asked) —
    `rejected/s16_optimize_reg_copy_2_roundtrip_pair_costs_13.c`.
  With this the s15 frontier's item #1 is closed, and with it the last named
  2.7.2 routine that could manufacture a redundant copy after combine.  Whatever
  makes target's copy, it is NOT a post-combine creation: it must be a
  pre-combine copy that combine DECLINED to fold, i.e. arm (b) of E-s16-1 with
  the LOG_LINK missing or the substitution rejected.

### E-s16-5  KILLED — the s15 frontier's item #2 (hoisted invariant copy of the ADDEND)

Both spellings the s15 frontier specified were built and scored:
  F1, do-while limit re-expressed through the addend copy
      (`while (i < *(s32 *)(sh2 + (s32)q + 0x20))`) = 15
      (`rejected/s16_body_limit_via_hoisted_addend_copy_costs_15.c`)
  F2, element read re-expressed through the addend copy
      (`*(u8 *)(sh2 + (s32)q + i + 0x2C)`) = 16
      (`rejected/s16_body_elem_via_hoisted_addend_copy_costs_16.c`)
Neither is close; both re-materialise the `sh2 + q` add instead of folding back
into the existing giv.  Together with s15's C1 (6), C2 (5), D1 (3) and D2 (9),
the loop.c/move_movables lever on loop 2's preheader is now fully enumerated and
dead: it can PLACE a copy there, but never a copy of the addend whose
destination is free.

### E-s16-6  KILLED with a control — the s8 guard-clobber trick does not transplant to loop 2

The best-motivated compound of the session: loop 1's copy survives partly because
the guard's address pseudo is CLOBBERED by the reuse `t = sh + (s32)p; t = *(s32
*)(t + 0x1C);`, which invalidates cse's hash entry for `sh + p` and stops the base
add folding to a copy of the guard address (the s8 lever).  E-s15-6 showed the
whole "source-level copy addend" family (A1-A4 = 8) dies because cse MERGES the
guard-address add and the base add.  So: apply the guard clobber to loop 2 and
the merge should be blocked, letting a source copy survive.  Measured:
  K1  t-reuse guard + `q = p;` source copy addend            = 35
  K2  K1 + loop-2 links local                                = 32
  K3  K1 with a separate local `r`                           = 32
  K5  K1 on the symmetric chassis                            = 36
  K4  t-reuse guard ALONE, fresh-read addend (the CONTROL)   = 36
The control isolates it: the two-step `t` guard local in loop 2 is worth about
-33 BY ITSELF, so the compounds never tested the copy question at all.  Loop 2's
guard must stay the inline one-expression form; the s8 lever is loop-1-only.
(`rejected/s16_loop2_tguard_plus_source_copy_costs_35.c`,
 `rejected/s16_loop2_tguard_alone_control_costs_36.c`)

### E-s16-7  Other cells measured this session

  E3/E4/E5  loop 2's guard reading `*(u8**)(ctx + 0xC)` freshly (so that cse's
            table would carry mem(ctx+12) into the preheader block and fold the
            preheader's read to a copy) = 31 / 31 / 31 on both chassis.  Third
            independent confirmation of s11's "loop 2's guard MUST consume the
            carried `p`" boundary, and it closes the last route to arm (b) via a
            cse fold.
            (`rejected/s16_loop2_guard_fresh_ctx0xC_read_costs_31.c`)
  J1        a links local `lnk` in loop 2's preheader (target's order is
            copy / `lw a2,0x10(s2)` / base add) = 12 on V1.  s11 had recorded
            loop-2 links spellings as inert at 3; that was with the inline body
            read, not with the read hoisted into the preheader.  Hoisting it is a
            regression.  (`rejected/s16_links_local_in_loop2_preheader_costs_12.c`)
  H1        frontier #3, cell (a): one fresh ctx+0xC read per math_Distance3D
            argument (4 tail reads instead of 3) = 3.  INERT.
  H2        frontier #3, cell (b): rec_b's address derived from rec_a instead of
            a fresh read (2 tail reads) = 44.
            (`rejected/s16_tail_recb_derived_from_reca_costs_44.c`)
  I1        see E-s16-1(a) = 3, inert.

### s16 artifacts

    tmp/grind/func_80017848/s16/gen.py, gen2.py    cell generators (20 cells)
    tmp/grind/func_80017848/s16/score.sh, dump.sh, idump.sh, ex.py, apply.py
    tmp/grind/func_80017848/s16/scores.txt         cells A_base,E*,F*,G*,H*
    tmp/grind/func_80017848/s16/scores2.txt        cells I1,J1,K1-K5
    tmp/grind/func_80017848/s16/iE2/F_*.txt        per-pass RTL slices, cell E2
    tmp/grind/func_80017848/s16/iE2/ings.s         E2 asm (loop 1 one insn short)
    tmp/grind/func_80017848/s16/iI1/F_*.txt        per-pass RTL slices, cell I1

- [s16] MEASURED TRICHOTOMY: a preheader reg-reg copy whose destination is unused is deleted by CSE (not flow) - cell I1, `reg/v:SI 80` occurrences .rtl 4 / .jump 4 / .cse 3 / .loop 3 / .cse2 3 / .flow 3 / .combine 3; used in the same block, deleted by combine (cell E2, insn 83 -> NOTE_INSN_DELETED); used out of block, survives and the use costs an instruction (the candidate's loop 1, `move $4,$7`).

- [s16] MEASURED, and the sharpest open question on this function: the loop-1 EXIT TAIL spelling flips cse's canonicalisation of the loop-1 base add's addend, although it is downstream of that insn. With tail `p = q;` (candidate) F_cse insn 89 reads reg79, the ORIGINAL, and the copy survives combine; with tail `p = *(u8**)(ctx+0xC);` (cell E2) F_cse insn 89 reads reg80, the COPY DEST, and combine deletes the copy. Same insn numbers, character-identical loop-1 preheader C, both dumps on disk.

- [s16] KILLED with a measurement: reg_n_sets > 1 does not protect a copy from combine. In cell E2 `q` (reg80) is set in BOTH preheaders and combine still deleted the loop-1 copy. Sharing the addend local across both preheaders is neutral on the symmetric chassis (4) and a regression on V1 (13).

- [s16] KILLED (analytic + measured): optimize_reg_copy_2, the s15 frontier's sole remaining producer candidate, cannot make target's shape - local-alloc.c:874-935 leaves BOTH copy insns, so it always emits two moves where target has one. Its C predicate scores 13 in both spellings (G1, G2).

- [s16] KILLED: the s15 frontier's hoisted-addend-copy item - F1 (limit via the copy) = 15, F2 (element read via the copy) = 16.

- [s16] KILLED with a control: transplanting loop 1's guard-clobber (`t = sh + p; t = *(t + 0x1C);`) into loop 2 costs ~33 by itself (K4 = 36 with a fresh-read addend), so the K1/K2/K3/K5 compounds (35/32/32/36) never tested the copy question. Loop 2's guard must stay the inline one-expression form.

- [s16] MEASURED: loop 2's guard reading ctx+0xC freshly is 31 on both chassis (E3/E4/E5) - the third independent confirmation of s11's regime boundary, and it closes the last route to getting mem(ctx+12) into cse's table at loop 2's preheader.

- [s16] Tail read-count sweep (s14/s15 frontier #3), first two cells: 4 tail reads (one per math_Distance3D argument) = 3, INERT; 2 tail reads (rec_b derived from rec_a) = 44.

- [s16] Floor re-measured on a clean tree this session: cell A_base (the committed candidate body) = 3 under sandbox func_80017848 --disable all. src/ings.c was left byte-identical to HEAD; no build-pipeline file was touched.

- [s16] 20 cells scored this session (tmp/grind/func_80017848/s16/scores.txt + scores2.txt); three instrumented-cc1 -da dump runs, each re-proved CODEGEN-IDENTICAL to the frozen build/cc1 on the whole src/ings.c TU by tmp/grind/func_80017848/s16/idump.sh before emitting anything.

- [s16] MEASURED TRICHOTOMY for a preheader reg-reg copy: destination unused -> deleted by CSE (cell I1, reg/v:SI 80 occurrences .rtl 4 / .jump 4 / .cse 3 / .loop 3 / .cse2 3 / .flow 3 / .combine 3, score 3 = inert); destination used in the SAME basic block -> deleted by combine (cell E2, insn 83 -> NOTE_INSN_DELETED); destination used in a DIFFERENT block -> survives, and the use costs one instruction (the candidate's loop 1 pays move $4,$7 for `p = q`). Target's loop-2 copy fits none of the three: a3 is used exactly once, at the base add, in the same block, and is not deleted.

- [s16] MEASURED, the session's central fact: the loop-1 EXIT TAIL spelling flips cse's canonicalisation of the loop-1 base add's addend even though it is downstream of that insn. Tail `p = q;` (candidate) -> F_cse insn 89 reads reg79 (the original) and the copy survives combine; tail `p = *(u8**)(ctx+0xC);` (cell E2) -> F_cse insn 89 reads reg80 (the copy dest) and combine deletes the copy. Same insn numbers, character-identical loop-1 preheader C, both dumps on disk (s15/icand/F_cse.txt:177 vs s16/iE2/F_cse.txt:178).

- [s16] KILLED with a measurement: reg_n_sets > 1 does NOT protect a copy from combine. In cell E2 the local `q` (reg80) is SET in both preheaders (F_cse insn 83 and insn 162) and combine still deleted the loop-1 copy. Target's a3 being multiply-set is an allocation coincidence, not the mechanism. Scores: E1 = 13, E2 = 4, E6 = 9, E7 = 33.

- [s16] KILLED analytically AND by measurement: optimize_reg_copy_2 (local-alloc.c:874-935), s15's sole remaining producer candidate, never deletes either copy insn, so it always emits TWO moves where target's loop-2 preheader has ONE. Both C spellings of its predicate score 13 (G1 fresh pair, G2 spelled through the live locals). CONSEQUENCE: no named GCC 2.7.2 routine creates a redundant reg-reg copy after combine, so target's copy is a PRE-combine copy that combine DECLINED to fold - every future hypothesis on this residual must explain a combine REFUSAL, not a post-combine creation.

- [s16] KILLED: the s15 frontier's hoisted-addend-copy item - F1 (do-while limit re-expressed through the copy) = 15, F2 (element read re-expressed through the copy) = 16. With s15's C1 (6), C2 (5), D1 (3), D2 (9) the loop.c/move_movables lever on loop 2's preheader is fully enumerated and dead.

- [s16] KILLED with a control: transplanting loop 1's guard-clobber (t = sh + (s32)p; t = *(s32 *)(t + 0x1C);) into loop 2 costs about -33 BY ITSELF - control cell K4 (guard change alone, unchanged fresh-read addend) = 36 - so the K1/K2/K3/K5 compounds (35/32/32/36) never tested the copy question. Loop 2's guard must stay the inline one-expression form.

- [s16] MEASURED: loop 2's guard reading *(u8 **)(ctx + 0xC) freshly - the last route to getting mem(ctx+12) into cse's table at loop 2's preheader so the preheader read would fold to a copy - is 31 on both chassis (E3 = 31, E4 = 31, E5 = 31). Third independent confirmation of s11's 'loop 2's guard MUST consume the carried p' regime boundary.

- [s16] MEASURED: hoisting the links read into loop 2's preheader as a named local (target's order is copy / lw a2,0x10(s2) / base add) is a regression at 12 on V1 (cell J1). s11 had recorded loop-2 links spellings as inert at 3, but that was with the inline body read, not with the read hoisted into the preheader.

- [s16] Tail ctx+0xC read-count sweep (the s14/s15 frontier item #3), first two cells: 4 tail reads, one per math_Distance3D argument (H1) = 3, INERT; 2 tail reads, rec_b's address derived from rec_a (H2) = 44.

- [s16] 11 new rejected forms banked under memory/grind/func_80017848/rejected/ (bank is now 116 entries), each named for the reason it is dead.

## s17 (2026-08-18, escalation) — the combine.c refusal set is ENUMERATED and every C-reachable member is dead

### E-s17-1  can_combine_p's COMPLETE refusal enumeration for a reg-reg copy (s16 frontier item #2, now CLOSED)

`tools/gcc-2.7.2/combine.c:803-970` (`can_combine_p`) was read end to end and every
early `return 0` path was classified against the shape that matters here: a
pre-combine copy `(set D S)` with D a pseudo, S a pseudo, whose only use is the
loop-2 base add `i3 = (set X (plus sh2 D))` in the SAME basic block. The refusal
set is finite and is now fully enumerated:

  R1  `succ && ! all_adjacent && reg_used_between_p (dest, succ, i3)`
      — needs a three-insn i1/i2/i3 combine; degenerates to "D used again", = R2.
  R2  D still live after i3 (the out-of-block-use arm of E-s16-1(c)).
      MEASURED DEAD across s9/s10/s16: buying the second use costs 2 points and
      returns 2, and loop 2 has no free site for it (every post-loop-2 use site
      was priced at 19-22 in s9, and target re-reads ctx+0xC three more times).
  R3  `! all_adjacent && use_crosses_set_p (src, INSN_CUID (insn))`
      (combine.c:910-916) — the copy's SOURCE pseudo is re-set between the copy
      and the base add. THE ONLY UN-TRIED MEMBER, and the one this session
      built. See E-s17-2. MEASURED DEAD: no free carrier exists.
  R4  `INSN_CUID (insn) < last_call_cuid && ! CONSTANT_P (src)` — needs a CALL
      between the copy and the base add. Target's loop-2 preheader
      (asm/funcs/func_80017848.s:64-67) is `addu $a3,$a0,$zero / lw $a2,0x10($s2)
      / addu $a0,$a1,$a3` — no call, and inserting one is not C-reachable
      without changing the function's semantics. DEAD.
  R5  volatile src / ASM_OPERANDS / volatile insn between (combine.c:963-985)
      — reachable only through the volatile-coercion cheat family. FORBIDDEN.
  R6  `find_reg_note (i3, REG_NO_CONFLICT, dest)` — REG_NO_CONFLICT sequences are
      emitted only for multi-word (DImode) operations, which is the banned
      "DImode chain for scheduling" family AND would add instructions target does
      not have. FORBIDDEN + arithmetically impossible.
  R7  PARALLEL/CLOBBER patterns, `FIND_REG_INC_NOTE` (no autoinc on MIPS),
      REG_RETVAL libcall ends, ZERO_EXTRACT/STRICT_LOW_PART dests, stack-pointer
      dests, `dest == stack_pointer_rtx`, hard-register / `REG_USERVAR_P` paths
      (pre-RA pseudos only, so unreachable) — NOT C-reachable at all for a plain
      pointer copy in this position.

So: of seven refusal paths, two (R5, R6) are forbidden cheat families, three
(R4, R7 twice over) are structurally unreachable, one (R2) is measured dead with
a priced cost, and the last (R3) is measured dead this session. There is no
eighth path — the function is a single flat `if (...) return 0;` chain and this
enumeration is exhaustive by construction, not by sampling.

### E-s17-2  KILLED with two controls — R3 (`use_crosses_set_p`) has no free carrier

H-s17-A: write a C-level copy of the carried pointer into loop 2's preheader AND
re-set the copy's SOURCE between the copy and the base add, so that
`use_crosses_set_p` fires and combine refuses the substitution that E-s16-1(b)
showed it otherwise always makes. The only instruction target has between the
copy and the base add is `lw $a2,0x10($s2)` (the links pointer), so the only
zero-cost carrier for the re-set is to REUSE the pointer local `p` to hold loop
2's links pointer (sanctioned "variable reuse for codegen control" shape):

    if (i < *(s32 *)(sh2 + (s32)p + 0x20)) {
        r = p;                       /* the copy: (set D S), S = p        */
        p = *(u8 **)(ctx + 0x10);    /* SETS S between the copy and i3    */
        base = (u8 *)(sh2 + (s32)r); /* i3 — combine must now refuse      */
        do { ... (s32)p ... } while (...);
    }

Measured on the V1 chassis (A_base re-measured at 3 this session, so the numbers
are directly comparable):

    A_base  candidate, unchanged                                    =  3
    P1      copy into a fresh local `r` + p reused for links        = 14
    P2      copy into `q` + p reused for links                      = 14
    P3      copy into `lnk` + p reused for links                    = 12
    P4      CONTROL — source copy alone, links left inline in body  =  8
    P5      CONTROL — p reused for links alone, fresh-read addend   = 14
    P7      target-shaped loop-1 tail (fresh `lw a0,12(s2)` re-read)
            + P1's loop 2                                           = 12
    P8      P7 with the copy into `q`                               = 12

The controls isolate it exactly, and the failure mode is the SAME one E-s16-6
found for the guard-clobber transplant: the CARRIER costs more than the lever
can possibly return. P5 shows that hoisting loop 2's links read into a local at
all — even reusing an existing pointer local rather than declaring a new one —
is worth -11 by itself, matching s16's J1 (a separate `lnk` local in loop 2's
preheader = 12). The maximum the copy can return is +2 (it replaces
`lw v0,12(s2)` with `addu a3,a0,zero` and re-points the base add). -11 + 2 can
never reach 0, so R3 is unbuyable on this chassis regardless of whether the
mechanism fires. P4 additionally re-confirms s10's "source copy 8-9" number on a
freshly measured chassis.

Note P7/P8: the FULLY target-shaped spelling — loop-1 exit tail as a fresh
`*(u8 **)(ctx + 0xC)` re-read (target's `lw $a0,0xC($s2)` at
asm/funcs/func_80017848.s:56) plus a loop-2 preheader copy — is 12, i.e. writing
C that mirrors target's instruction sequence one-for-one is FOUR TIMES WORSE
than the candidate. This is the clearest statement yet of why this function is
locked: the 3-instruction residual is not a spelling the C can express, it is a
cse/combine/local-alloc interaction whose input predicate has no C handle that
is not simultaneously an 11-point regression elsewhere.

### s17 artifacts

    tmp/grind/func_80017848/s17/v/{A_base,P1,P2,P3,P4,P5,P7,P8}.c   cells
    tmp/grind/func_80017848/s17/scores.txt                          measured scores
    tmp/grind/func_80017848/s17/{score.sh,apply.py}                 harness

- [s17] ENUMERATED AND CLOSED (s16 frontier item #2): combine.c's `can_combine_p` has exactly seven refusal paths for a same-block reg-reg copy; two are forbidden cheat families (volatile, REG_NO_CONFLICT/DImode), three are structurally unreachable (no call in the region, no autoinc on MIPS, no hard-reg/PARALLEL forms), one (out-of-block use) is priced dead at 2-for-2 with no free site, and the last (`use_crosses_set_p`) is killed this session. There is no eighth path.
- [s17] KILLED with two controls: making the copy's source re-set between the copy and the base add (the `use_crosses_set_p` refusal) requires hoisting loop 2's links read into a pointer local, and that hoist alone is -11 (P5 = 14, matching s16's J1 = 12) against a maximum return of +2. P1/P2/P3 = 14/14/12.
- [s17] MEASURED: the fully target-shaped C — loop-1 exit tail as a fresh ctx+0xC re-read plus a loop-2 preheader copy, mirroring target's instruction sequence one-for-one — is 12 (P7/P8), four times worse than the candidate's 3.
- [s17] GATE (a) FAILED: `python3 tools/scan_hand_coded.py --single func_80017848` = tier LOW, score 0/8, no strong hand-coded indicators (S1 0 multu/mflo pairs, S2 no empty-body branches, S3 127 insns / 7 spills / 12 distinct regs, S6 no BIOS jumptable). This is ordinary compiler output, not hand-written asm.

- [s17] sandbox func_80017848 --disable all = 3 (127 target insns / 127 build insns, scorable, rules_dropped 2, cheat_asm_stripped 49), re-measured this session with memory/grind/func_80017848/candidate.c applied to src/ings.c.

- [s17] GATE (a) FAILED: tools/scan_hand_coded.py --single func_80017848 = tier LOW, score 0/8, 'no strong hand-coded indicators' - S1 0 multu/mflo pairs, S2 no empty-body branches, S3 127 insns / 7 spills / 12 distinct regs, S4 max load burst 3, S5 jaccard < 0.5, S6 no BIOS jumptable, S7 all callee-saves saved, S8 no redundant mask. Not STRONG, not a single signal; the canonical-asm grant path does not apply.

- [s17] GATE (b) FAILED: there is no closing construct to cite a precedent for. The residual is an unexplained cse/combine/local-alloc interaction, not a coercion or spelling family awaiting sanction; no SOTN-master file:line or commit hash exists and none is offered.

- [s17] The byte-match is currently held by asmfix.txt lines 60-61: a delete_between over the whole body plus an insert_before whose payload is the ENTIRE 127-instruction function as rule text - the maximal form of the debt category in .claude/rules/asmfix-all-debt-end-state.md. Zero regfix rules, zero cheat-asm in src/ings.c.

- [s17] combine.c refusal enumeration (E-s17-1): of seven paths, R5/R6 are forbidden cheat families, R4/R7 are structurally unreachable, R1 degenerates to R2, R2 is priced dead, R3 is killed this session. The function is a single flat if/return chain, so the enumeration is exhaustive by construction rather than by sampling.

- [s17] The R3 carrier is priced by controls: P5 (reusing p for loop 2's links pointer, no copy) = 14, i.e. -11 by itself, matching s16's J1 (a separate lnk local in loop 2's preheader) = 12. The lever's maximum return is +2 (replace `lw v0,12(s2)` with `addu a3,a0,zero` and re-point the base add). -11 + 2 can never reach 0, so R3 is unbuyable on this chassis regardless of whether the mechanism fires - the same failure shape as s16's E-s16-6 guard-clobber transplant.

- [s17] P4 (a plain source copy in loop 2's preheader, links left inline) = 8, re-confirming s10's 'source copy 8-9' number on a freshly measured chassis.

- [s17] Exhaustion: 17 sessions, 6 distinct modalities (recon/structural/permuter/rederive/synthesis/forensics), floor flat at 3 for the last 8; 5 permuter campaigns totalling 180,472 iterations with 41 finds and ZERO engine-scored improvements on four chassis (including the directed cross-product campaign s14a); ~150 hand-built structural cells; 123 rejected forms banked in memory/grind/func_80017848/rejected/.

- [s17] Disposition filed THIS session in docs/grind/decisions.md under the heading '2026-08-18 - func_80017848 (src/ings.c) - OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE'. Terminal; nothing pending on the owner. src/ings.c was restored to HEAD, so the working tree carries no source edits from this session.

## s18 (2026-08-18, modality `rederive`) — the re-derivation frontier closes; a matched prior-art twin for the residual idiom is found in the decomp.me corpus

Chassis re-measured first: HEAD's committed `src/ings.c` body scores **16**
(`sandbox --disable all`, 125/127 insns); applying
`memory/grind/func_80017848/candidate.c` verbatim re-measures at **3** (127/127),
twice this session (once at the start, once after restoring it at the end). The
floor is unchanged and every number below is on that chassis.

### E-s18-1 — m2c has exactly ONE structural reading of this function; the "different structuring configuration" the s17 frontier asked for does not exist
Five m2c configurations were run on `asm/funcs/func_80017848.s`
(`tmp/grind/func_80017848/s18/m2c.sh`, outputs `m2c_*.c` in the same directory):
`--valid-syntax` alone (default), `--no-andor`, `--passes 2`, `--gotos-only`,
and `--reg-vars v0,v1,a0..a3,s0..s4`.

  * `--passes 2` is **byte-identical** to the default output (`diff` empty).
  * `--no-andor` and `--gotos-only` differ from the default **only** in how the
    top guard and the shared `return 0` exit are spelled (`&&`/`||` fused
    expression + `goto block_3` vs. nested `if` + `goto block_4`/`block_3`). The
    loop decomposition, the local set, the read placement and the preheader
    contents are character-identical.
  * `--reg-vars` produces semantically BROKEN C (`var_v0 = var_v1 < var_v0;`
    immediately overwritten by `var_v0 = var_a0 + var_v1;`, and the loop test
    reads `if (var_v0 == 0)` on an address) — it is not a compilable chassis at
    all, so it cannot be "the first compilable output" of anything.

So the s17 frontier's premise — that a different m2c structuring configuration
would yield a different source-level decomposition — is **false for this
function**. m2c's reading is invariant: rotated do/while loops, an explicit
walking element pointer (`e = base + i` recomputed at the bottom of each loop),
the index increment hoisted above the element test, and a fresh `ctx+0xC` read at
loop 2's guard.

### E-s18-2 — the raw re-derivation scores 53, and the 50-point gap decomposes cleanly into TWO independent costs
The frontier's own closing predicate was "if the raw re-derivation is worse than
12 the lineage question is settled negatively." Three cells settle it and also
attribute the loss, so a future session never has to re-measure the parts:

| cell | shape | score |
|---|---|---|
| **R1** | faithful transcription of m2c `--no-andor`, no hand-tuning beyond the 2-arg `math_Distance3D` prototype and BB2 pointer types | **53** (121/127 insns) |
| **R2** | m2c's CONTROL FLOW (single `block_3:` exit reached by `goto`, whole body inside `if (slot_a != slot_b)`) + the candidate's loop bodies | **15** (125/127) |
| **R3** | the candidate's control flow (four inline `return 0`s) + m2c's walking-pointer rotated loop bodies | **49** (124/127) |

Reading: m2c's goto-structured single-exit CFG costs **+12** (R2 vs the
candidate's 3 — consistent with s12's independently measured "all-goto = 15"),
and m2c's walking-pointer loop form costs **+46** (R3 vs 3). The two are
essentially additive (12 + 46 ≈ 53 - 3 = 50). Banked as
`rejected/s18_m2c_noandor_faithful_rederivation_costs_53.c`,
`rejected/s18_m2c_goto_cfg_single_exit_costs_15.c`,
`rejected/s18_m2c_walking_ptr_both_loops_costs_49.c`.

**Consequence: s17 frontier item #2 is CLOSED negatively.** The s8/s9 lineage is
not an accident of search history — every non-lineage whole-function chassis
derivable from the asm by machine is 12 to 50 points worse, and the two
independent axes on which it is worse are now priced separately.

### E-s18-3 — the walking element pointer is target's own shape, and writing it in C costs 11 at loop 2
Target's loop 2 preheader/body (asm/funcs/func_80017848.s:63-81) literally
contains `addu $v0,$a0,$v1` (e = base + i) in the preheader and again in the
loop-closing delay slot, i.e. GCC's own strength reduction produces the walking
pointer from the candidate's index-addressed C. Writing that pointer explicitly
in C at loop 2 ONLY (cell **W2**, `e = base + i;` initialised in the preheader and
updated after `i++`) scores **14** with the correct instruction COUNT (127/127) —
so it is 11 points of pure register-identity loss, not a shape difference.
Banked as `rejected/s18_loop2_walking_element_ptr_costs_14.c`. Do not re-probe
the explicit-element-pointer family in either loop: 13 (s12, loop 1), 14 (here,
loop 2), 49 (both).

### E-s18-4 — CORPUS PRIOR ART: 3,754 decomp.me scratches mined; a MATCHED twin of the residual idiom exists, and its origin is a nested-loop induction-variable initialisation
The local decomp.me corpus (`tmp/decomp_me_corpus/`, 3,754 scratches for the
three BB2-toolchain compilers) was mined for the exact idiom the residual needs:
a reg-reg copy whose destination is consumed by an immediately following `addu`.
Scripts: `tmp/grind/func_80017848/s18/mine_copy_idiom.py` (copy → add within 2
insns, source dead afterwards) and `mine_exact.py` (target's exact
copy-then-redefine-the-source pattern `$X = $Y; $Y = $Z + $X`).

  * 163 copy→add hits in MATCHED (`is_matching`, score 0) scratches; **50** have
    the source register dead afterwards, i.e. are target-shaped redundant copies.
  * **28** matched scratches contain target's exact copy-then-redefine-source
    pair. Of those, the overwhelming majority are *call-return* moves
    (`jal f; addu $sN,$v0,$zero; addu $v0,$sN,$zero` — a return-value staging
    idiom that has no bearing here) or *constant-multiply expansions*
    (`addu $a0,$v0,$zero; sll $v0,$a0,3; addu $v0,$v0,$a0` — the operand is
    copied because the accumulator reuses its register).
  * The one genuine STRUCTURAL twin is scratch **19TpT** (`func_8009C6D8`,
    compiler `gcc2.7.2-cdk`, flags `-O2 -G0 -g2`, score 0 / matching). Its target
    asm contains, at a loop preheader reached by a join:

        .L8009C704:
        addu $s0, $zero, $zero
        addu $v1, $a0, $zero     <- the preheader copy, source dead after
        .L8009C70C:
        addu $v0, $v1, $a1       <- the base add consuming the copy

    which is the same three-instruction fingerprint as our residual
    (`addu $a3,$a0,$zero` / `lw $a2,0x10($s2)` / `addu $a0,$a1,$a3`). Its C is a
    plain **nested loop** — `for (row = 0; row < 5; row++) for (col = 0; col < 5;
    col++) D_801D3398.cells[row][col].flags = 0;` — where `$a0` is the OUTER
    loop's row-base address (live across the whole inner loop, incremented once
    per outer iteration) and `$v1` is the inner loop's induction base copied from
    it at the inner preheader. The copy survives because its consumer sits in the
    inner loop BODY, a different basic block (s16's out-of-block leg of the
    survival trichotomy), and it costs nothing because the outer loop needs the
    original to stay live anyway.

**What this proves and what it does not.** It proves the idiom is producible from
ordinary pure C under this exact compiler — the residual is not evidence of hand
asm (consistent with `scan_hand_coded` = LOW 0/8). It does NOT give this function
a carrier: the prior art's precondition is a live outer-loop value that the inner
loop copies, and func_80017848 has no nested loop and no value live across loop 2
(s11: target re-reads `ctx+0xC` three times in the tail; s9/s11: routing any tail
read through a live local costs 19-22). Fabricating an enclosing loop to
manufacture the precondition is the duplicated-region family s12 already priced
at 35 and is a structural cheat smell besides.

## s18 second dispatch (2026-08-18, modality `rederive`) - the sibling/Kengo transplant leg closes; the residual's seam idiom is unique in the binary

Chassis re-measured FIRST, before any probe: the ledger candidate body applied to
`src/ings.c` scores **3 at 127/127 instructions** (`sandbox func_80017848
--disable all`, `rules_dropped: 2`, `cheat_asm_stripped: 49`). The dispatch brief
reported the HEAD chassis measurement as unavailable; it is 3 with the candidate
in place, unchanged from s9-s18, so every banked spelling conclusion is still
chassis-valid.

### E-s18-5  No whole-function sibling exists in BB2 (the transplant leg's first prong)

`tmp/grind/func_80017848/s18b/sibling_scan.py` extracts the opcode sequence of
every one of the 1,437 `asm/funcs/*.s` bodies, keeps those in the 0.6x-1.8x
instruction-count band around func_80017848's 127, and scores each by the
fraction of func_80017848's 5-gram opcode multiset it reproduces. The maximum
over the whole binary is **0.120** (func_8005BA8C and func_8006BD28); the top
fifteen all sit between 0.096 and 0.120. There is no near-duplicate body in the
game whose whole-function shape could be transplanted onto this function, so the
"sibling transplant" leg of the rederive modality has no source material at the
function level.

### E-s18-6  The residual's seam idiom occurs exactly three times in 1,437 functions, and never in matched pure C at a loop preheader

`tmp/grind/func_80017848/s18b/fp_exact.py` scans every function for target's
exact seam fingerprint: `addu D,S,$zero` (a plain register copy) followed within
three instructions, with no intervening label, by a THREE-REGISTER `addu` that
consumes `D` and writes `S` (i.e. the copy's source register is redefined by the
consumer). The complete result set for the whole binary:

  - **func_80017848** - twice, `addu $a3,$a0,$zero` -> `addu $a0,$a1,$a3`, once
    per scan loop (asm/funcs/func_80017848.s:39-40 and :64-66). Both loops carry
    the SAME construct; the candidate reproduces loop 1's and pays one
    instruction for it in the exit tail.
  - **func_800200DC** (src/code6cac.c) - `addu $v1,$v0,$zero` ->
    `addu $v0,$a2,$v1` at asm/funcs/func_800200DC.s:94-96. NOT a loop preheader:
    it is call-return staging, `$v1 = $v0` in the shadow of `jal SquareRoot0`
    (:92) with `sll $a2,$s3,5` between. The function also carries 14
    register-allocation regfix rules (regfix.txt:628), so it is not a clean
    pure-C precedent for anything.
  - **func_8005E54C** - `addu $s0,$v0,$zero` -> `addu $v0,$s3,$s0`. Body is still
    asm (`asmfix replace_with_asmfile`), queue-active at honest distance 799,
    `hand_coded_tier: LOW`. No C exists to transplant. (The first version of the
    scan mis-attributed this one to `src/code6cac_c2.c`; that file only carries
    its `extern` prototype at :109. `fp_exact2.py` fixes the detector to require
    a real definition with a body.)

Widening the filter to any consumer of the copy (`addu`/`addiu`/`sll`/`subu`/
load/store) over functions that are pure-C-defined, rule-free and NOT in
`engine/queue.json` yields 31 hits (`fp_exact2.py`). Every one is call-return
staging (`$s0 = $v0` after a `jal`, then `$v0` reused) or shift staging
(`$s0 = $v0` then `sll $v0,$s0,16`). **Not one is a loop-preheader base copy.**

Consequence: the idiom the residual needs has NO in-tree pure-C precedent to
copy, and the one in-tree spelling that a compiler demonstrably does produce
depends on a preceding CALL - which is s17's R4 refusal leg (combine.c's
`INSN_CUID (insn) < last_call_cuid`), already recorded as not C-reachable here
without changing the function's semantics. The in-tree census is therefore an
independent empirical confirmation of E-s17-1's R4 classification rather than a
new lever.

### E-s18-7  A Kengo transplant is structurally impossible - Kengo supplies symbols, not source

The `Kengo/` tree (889 MB) contains the retail PS2 disc image
(`Kengo - Master of Bushido (USA).bin/.cue`), an extracted `disc/`, and the debug
symbol dumps `kengo_functions_full.txt` / `kengo_globals_full.txt` - which give
per-function NAME + SIZE + originating source path (`// FILE -- src/ishito/
is_coli.c`) and nothing else. **There is no Kengo C source in the repository.**
So the Kengo channel can only ever supply names, sizes and module attribution; it
cannot supply a source shape to transplant, for this or any other BB2 function.
That is a permanent property of the artifact, not a gap to be filled later, and
it generalises the two prior negative results in [[slog-kengo-dead-end]] from "no
equivalent for these two functions" to "no source-shape channel at all".

Supporting detail: `kengo_matches.csv` has zero rows for func_80017848.
`tools/kengo_match.py --bb2 func_80017848` cannot run from the Windows side
(`objdump` is not on PATH; it is a WSL toolchain binary). The `ings.c` rows that
DO exist map its neighbours to `src/hide/hi_landhit.c` (func_800164AC ->
gnd_land_hit_char, func_80016A8C -> gnd_land_hit_char_poly2),
`src/ishito/is_coli.c` (func_80017A44 -> coli_MakeKatanaVec, the immediately
following function) and `src/numata/nm_cpu.c` (func_80016E60 ->
cpu_check_kamaekae). Reading the full `is_coli.c` symbol list (32 functions) and
grepping the whole Kengo symbol table for `link|pair|near|regist` finds nothing
that reads as a pair/link registration in the 120-150-instruction band. Kengo's
collision module was restructured for the PS2 title; there is no equivalent
symbol to even name this function after.

### E-s18-8  Cell X1 - the fully target-shaped loop-1 exit tail is 4 at 126/127, and the shift site is inert

Target re-materialises BOTH values the join block needs on loop 1's exit edge:
`lw $a0,0xC($s2)` then `sll $a1,$s4,6` (asm/funcs/func_80017848.s:56-57), and the
join `.L8001791C` then computes loop 2's guard address from them before the copy.
Cell X1 writes exactly that shape: loop 1's exit tail becomes
`p = *(u8 **)(ctx + 0xC); sh = slot_a << 6;`, `sh2` is deleted, and loop 2's
guard and base both read the recomputed `sh`.

X1 = **4 at 126/127 instructions** - loop 1 one instruction SHORT, which is
precisely E-s16-2's signature: with a fresh read instead of the candidate's
`p = q` downstream of loop 1's base add, cse canonicalises the base add onto the
COPY DEST, combine deletes the copy, and loop 1 loses target's
`addu $a3,$a0,$zero`.

The value of the cell is that it isolates a variable s12 could not: s12's
symmetric chassis (fresh-read tail, `sh2` recomputed just before loop 2's guard)
is also 4/126. Moving the recompute up onto the exit edge so it sits exactly
where target's `sll $a1,$s4,6` is changes NOTHING. **The +1 of the symmetric
chassis is entirely the tail read; the shift's placement is inert.** Banked as
`rejected/s18b_target_shaped_tail_fresh_read_plus_sh_recompute_costs_4.c`.

### Durable one-liners from the s18 second dispatch

- [s18b] Honest floor re-measured at 3 (127/127) with the ledger candidate in place; HEAD's committed body is 16. Chassis unchanged.
- [s18b] Whole-binary 5-gram similarity census: max overlap with func_80017848 is 0.120 over all 1,437 functions in the size band. No sibling body exists.
- [s18b] The exact seam fingerprint (copy + three-register add consuming it and redefining its source) occurs in exactly 3 of 1,437 functions; the only non-func_80017848 instances are call-return staging in a 14-regfix-rule function and an unmatched asm body. Zero matched pure-C loop-preheader precedents in the tree.
- [s18b] Every in-tree matched pure-C instance of copy-then-consume (31 hits) is call-return or shift staging - i.e. the combine refusal that makes the idiom free in-tree is R4 (call boundary), which is not C-reachable here. The census empirically confirms E-s17-1's R4 leg.
- [s18b] Kengo ships debug SYMBOLS only (name/size/source-path); no Kengo C source exists in the tree, so the Kengo transplant channel cannot supply a source shape for ANY BB2 function. Generalises [[slog-kengo-dead-end]].
- [s18b] Cell X1 (target-shaped exit tail: fresh ctx+0xC read AND the shift recomputed on loop 1's exit edge, sh2 deleted) = 4 at 126/127. Equal to s12's symmetric chassis, so the shift site is inert and the symmetric chassis's +1 is the tail read alone.

- [s18] Honest floor re-measured at the start of the session with the ledger candidate applied to src/ings.c: score 3, target_insns 127, build_insns 127, rules_dropped 2, cheat_asm_stripped 49. HEAD's committed body scores 16. The chassis is unchanged from s9-s18, so every banked spelling conclusion is still chassis-valid.

- [s18] Whole-binary 5-gram opcode-sequence census: the maximum similarity to func_80017848 among all 1,437 asm/funcs bodies in the 0.6x-1.8x size band is 0.120. No sibling body exists to transplant at the function level.

- [s18] The residual's exact seam fingerprint (register copy followed within three insns by a three-register addu that consumes the copy's dest and redefines its source) occurs in exactly 3 of 1,437 functions: func_80017848 (both loops), func_800200DC (call-return staging, 14 regfix RA rules) and func_8005E54C (unmatched asm body, distance 799). Zero matched pure-C loop-preheader precedents exist anywhere in the tree.

- [s18] All 31 in-tree instances of the wider copy-then-consume pattern in matched, rule-free, pure-C functions are call-return staging or shift staging - i.e. the combine refusal that makes the idiom free in this codebase is s17's R4 leg (combine.c's INSN_CUID (insn) < last_call_cuid, a call between the copy and the consumer), which is not C-reachable in func_80017848 without changing its semantics. The census is an independent empirical confirmation of E-s17-1's R4 classification, not an eighth refusal path.

- [s18] Kengo/ contains the retail PS2 disc plus debug SYMBOLS only (per-function name + size + source path); there is no Kengo C source in the repository, so the Kengo channel can supply names and module attribution but never a source shape - for func_80017848 or any other BB2 function. This generalises the two prior per-function negatives in memory/project/slog-kengo-dead-end.md to the whole channel.

- [s18] func_80017848's ings.c neighbours map to Kengo's src/hide/hi_landhit.c (func_800164AC -> gnd_land_hit_char, func_80016A8C -> gnd_land_hit_char_poly2), src/ishito/is_coli.c (func_80017A44 -> coli_MakeKatanaVec, the immediately following function) and src/numata/nm_cpu.c (func_80016E60 -> cpu_check_kamaekae); no symbol in those modules reads as a pair/link registration, and kengo_matches.csv has no row for func_80017848 at all.

- [s18] Cell X1 (target's own exit-edge spelling: fresh ctx+0xC read AND the shift recomputed on loop 1's exit edge, sh2 deleted, loop 2's guard and base sharing sh) = 4 at 126/127, equal to s12's symmetric chassis. The shift's placement is therefore inert and the symmetric chassis's +1 is entirely its tail read.

- [s18] Fresh read of the target listing this session pinned the seam's block structure precisely: loop 1's exit edge is `lw $a0,0xC($s2)` / `sll $a1,$s4,6` (asm/funcs/func_80017848.s:56-57), the join .L8001791C computes the guard address `addu $v0,$a1,$a0` then `lw $v0,0x20($v0)` / `blez`, and loop 2's preheader is `addu $a3,$a0,$zero` / `lw $a2,0x10($s2)` / `addu $a0,$a1,$a3` / `addu $v0,$a0,$v1` - so both loops carry the SAME copy+add construct and the copy sits AFTER the guard's branch, in a single-predecessor block whose predecessor is the join.

- [s18] Process note: the previous dispatch of this session was discarded by the validator for returning owner-gated under `rederive` modality. Its ledger work (H-s18-1..3, E-s18-1..4, four rejected forms) was on disk and is preserved; this dispatch corrected the disposition claim in docs/grind/decisions.md so the s18 addendum no longer asserts a terminal disposition, and returns `progress`.

## s19 (2026-08-18, modality `synthesis`) — the residual is reduced to a TWO-CLAUSE theorem, and the last two copy carriers are killed

Chassis re-measured at dispatch with `memory/grind/func_80017848/candidate.c`
applied to `src/ings.c`: **score 3, target_insns 127, build_insns 127, scorable,
rules_dropped 2, cheat_asm_stripped 49** — identical to s9..s18, so every banked
spelling conclusion remains chassis-valid. HEAD's committed body still scores 16.

### E-s19-1  THE MERGED MODEL — target's preheader pair needs TWO clauses at once, and only loop 1 can pay for both

Target's construct, once per loop, is

    COPY:  dest = addend            (`addu $a3,$a0,$zero`)
    BASE:  base = sh + dest         (`addu $a0,$a1,$a3`)

with `dest` used exactly once, at BASE, in the same basic block, dying there
(E-s15-7). Merging E-s15-1, E-s15-2, E-s16-1 and E-s16-2, the only production
path for this pair in GCC 2.7.2 is: a reg-reg copy insn exists before combine and
survives it, and then local-alloc's `optimize_reg_copy_1` re-points BASE's addend
from the copy's SRC onto its DEST. For a C-written preheader that requires two
independent clauses to hold simultaneously:

**CLAUSE A (POSITION).** The copy insn must be emitted BEFORE the base add. The
only construct that does that is a *redundant memory read at the addend's own
read point* which cse folds to a copy in place. A copy written as a C statement,
or hoisted into the preheader by `loop.c`'s `move_movables`, is emitted where its
consumer's operand becomes available — and for every semantically legal carrier
in this function that is AFTER the base add, because the only preheader value the
loop body can legally consume is `base` itself (cse rewrites every addend-based
body address back to `base + i`).

**CLAUSE B (SURVIVAL).** The copy's destination needs a consumer OUTSIDE the
preheader block (E-s16-1's arm (c)); that consumer always materialises as one
instruction.

Loop 1 pays for both: A via the redundant `q = *(u8 **)(ctx + 0xC);` read that
cse folds (its extended basic block reaches back to the top block's read of the
same address), B via `p = q;` in the exit tail — whose single instruction lands
where target has `lw $a0,0xC($s2)`, so the region nets ONE mismatch instead of
two. Loop 2 can pay for neither:

- **A is structurally impossible.** Loop 2's preheader is downstream of the join
  `.L8001791C` (loop 1's `blez $v0,.L8001791C`, `asm/funcs/func_80017848.s:62`),
  cse's extended basic block therefore *starts* at the join, and the redundant
  `ctx + 0xC` read in the preheader is never folded — it stays `lw $v0,12($s2)`,
  which is literally residual instruction #2.
- **B has no free carrier.** Every out-of-block consumer ever measured adds an
  instruction target does not have: post-loop math_Distance3D args 19 (s9),
  `rec_a` 32 / `rec_a`+`rec_b` 52 (s10), in-body carriers 5..16 (s15 C1 = 6,
  s16 F1/F2 = 15/16, and this session's Z2 = 6 / Z4 = 5).

So **residual 3 = 1 (loop 1's clause-B purchase) + 2 (loop 2's missing copy and
its base add's addend register)**, and it is a fixed point of two proven
constraints rather than an unfound spelling. This single statement subsumes all
133 banked rejected forms.

### E-s19-2  KILLED — the last two untried clause-A carriers (a NON-INVARIANT body use of the addend, and an invariant limit read through the addend)

The one carrier class the ledger had never written is a body use of the *addend*
that `loop.c` cannot hoist because it varies with `i`. Five cells, all measured on
the V1 chassis:

| cell | construct | score | build_insns |
|---|---|---|---|
| Z1 | loop 1's body element read via `(s32)q + sh + i + 0x24` | 6 | 128 |
| Z2 | loop 2: named addend `q2`, body element read via `(s32)q2 + sh2 + i + 0x2C` | 6 | 128 |
| Z3 | Z1 + Z2 together | 9 | 129 |
| Z4 | loop 2: named addend `q2`, loop limit read via `(s32)q2 + sh2 + 0x20` (invariant) | 5 | 128 |
| Z5 | loop 2: named addend `q2` only (control) | 3 | 127 |

The disassembly diffs are the point, not the scores. Z2's loop-2 preheader emits

    lw   $v0,12($s2)
    lw   $a2,16($s2)
    addu $a0,$a1,$v0        <- base add
    addu $a1,$a0,$zero      <- THE COPY, one slot LATE
    addu $v0,$a1,$v1

and Z4 emits the same shape with the copy again after the base add. cse rewrites
the body's `q2 + sh2 + i` address back into `base + i` (the loop-invariant part is
common with the base add), so the value the body actually consumes is `base`, not
`q2` — and the surviving copy is therefore a copy of `base`. This reproduces s15's
cell C1 (copy of the BASE, one slot too late, 6) from a completely different C
construct, which upgrades C1 from an isolated result to the general rule stated as
CLAUSE A above: **every body-anchored preheader copy in this function is a copy of
`base`, and `base` is defined after the addend, so the copy can never precede the
base add.**

Banked as `rejected/s19_l1_body_elem_via_addend_and_shift_costs_6.c`,
`rejected/s19_l2_body_elem_via_addend_and_shift_copy_lands_after_base_costs_6.c`,
`rejected/s19_both_loops_body_elem_via_addend_costs_9.c`,
`rejected/s19_l2_limit_via_addend_licm_hoisted_copy_after_base_costs_5.c`,
`rejected/s19_l2_named_addend_local_alone_inert_3.c`.

### E-s19-3  The three exits the theorem leaves, and their measured state

Any future proposal must break one of exactly three clauses; there is no fourth:

- **(α) Kill the join** so cse's extended basic block reaches loop 2's preheader.
  The only C-level attempt in nineteen sessions is duplicating loop 2 into both
  arms of loop 1's guard (s10/s12, = 35; `jump2` does not re-merge the copies).
  No other legal restructure of `if (g1) {loop1} if (g2) {loop2}` removes the
  second `if`'s two predecessors.
- **(β) Find a clause-B consumer that lands on an instruction target already
  has.** Exactly one site exists: target's post-loop-2 block reads
  `lw $a1,0xC($s2)` ONCE and derives both `math_Distance3D` arguments from it
  (`sll $s0,$s4,6 / lw $a1,0xC($s2) / sll $s1,$s3,6 / addu $a0,$a1,$s0 / jal /
  addu $a1,$a1,$s1`). s9 measured "math args via a live local" at 19, but on the
  pre-V1 chassis and routing BOTH args through the local as separate expressions;
  the tail block's own spelling has never been swept on V1 (three isolated cells
  only: s12 link-store order 9, s12 `rec_a` base 41, s16 `rec_b` derived from
  `rec_a` 44).
- **(γ) A combine refusal on an in-block single use.** s17 enumerated all seven
  `can_combine_p` paths; two are forbidden cheat families, three are structurally
  unreachable, one is loop 1's own out-of-block lever, and the last
  (`use_crosses_set_p`) was built and killed with two controls.

### s19 artifacts

    tmp/grind/func_80017848/s19/apply.py     body-swap harness (LF-safe)
    tmp/grind/func_80017848/s19/gen.py       cell generator (Z1..Z5)
    tmp/grind/func_80017848/s19/dis.sh       normalised target-vs-build diff
    tmp/grind/func_80017848/s19/z1.c .. z5.c the five measured cells
    tmp/grind/func_80017848/s19/T.txt, B.txt normalised listings (last cell run)

### Durable one-liners from s19

- [s19] Honest floor re-measured at dispatch: 3 (127/127) with the ledger candidate applied; HEAD's committed body 16. Chassis unchanged from s9.
- [s19] THE TWO-CLAUSE THEOREM: target's preheader copy needs (A) to be emitted before the base add — only a cse-folded redundant read at the addend's read point does that — and (B) an out-of-block consumer for its destination, which always costs one instruction. Loop 1 pays both (its clause-B instruction lands on one target has); loop 2 fails A structurally (its preheader is downstream of the join, so cse cannot fold) and has no free clause-B site.
- [s19] Every body-anchored preheader copy in this function is a copy of `base`, never of the addend, because cse rewrites addend-based body addresses back to `base + i`; therefore it always lands one slot AFTER the base add. Measured on a non-invariant element read (Z2 = 6) and an invariant limit read (Z4 = 5), independently reproducing s15's C1 = 6.
- [s19] Exactly three exits remain from the theorem — kill the join (only known attempt 35), find a clause-B consumer landing on an instruction target already has (only candidate site: the post-loop-2 `lw $a1,0xC($s2)` feeding both math_Distance3D args, measured 19 on a pre-V1 chassis), or a combine refusal on an in-block single use (all seven paths enumerated and dead in s17).

- [s19] Chassis check: with memory/grind/func_80017848/candidate.c applied to src/ings.c the honest floor is 3 at 127 target / 127 build insns, scorable, rules_dropped 2, cheat_asm_stripped 49 - unchanged from s9 through s18, so every banked spelling conclusion remains chassis-valid. HEAD's committed body scores 16.

- [s19] THE TWO-CLAUSE THEOREM (E-s19-1): target's preheader pair (copy then base add consuming it) requires (A) the copy to be emitted before the base add, which only a cse-folded redundant read at the addend's read point achieves, and (B) an out-of-block consumer for the copy's destination, which always costs one instruction. Loop 1 pays both and nets 1 mismatch because its clause-B instruction lands where target has `lw $a0,0xC($s2)`; loop 2 fails clause A structurally and has no free clause-B site, so it pays 2.

- [s19] Residual 3 decomposes exactly as 1 (loop 1's clause-B purchase, `addu $a0,$a3,$zero` where target has `lw $a0,12($s2)`) + 2 (loop 2's missing copy and its base add's addend register).

- [s19] Every body-anchored preheader copy in this function is a copy of `base`, never of the addend, because cse rewrites addend-based body addresses back to `base + i`; it therefore always lands one slot AFTER the base add. Measured this session on a non-invariant element read (Z2 = 6, 128 insns) and an invariant limit read (Z4 = 5, 128 insns), independently reproducing s15's cell C1 = 6 from a structurally different construct.

- [s19] Five new cells banked in memory/grind/func_80017848/rejected/ (133 forms total): Z1 = 6, Z2 = 6, Z3 = 9, Z4 = 5, Z5 = 3 (inert control confirming that a named loop-2 addend local by itself changes nothing).

- [s19] Exactly three exits remain from the theorem, and two are already measured dead: (a) kill the join so cse's extended basic block reaches loop 2's preheader - the only known attempt is duplicating loop 2 into both arms of loop 1's guard, 35, because jump2 does not re-merge the copies; (b) find a clause-B consumer that lands on an instruction target already has - the single candidate site is the post-loop-2 `lw $a1,0xC($s2)` from which target derives BOTH math_Distance3D arguments, measured at 19 by s9 but on the pre-V1 chassis and with both args routed as separate expressions; (c) a combine refusal on an in-block single use - all seven can_combine_p paths enumerated and dead (s17).

- [s19] The post-loop-2 tail block has never been swept on the V1 chassis: only three isolated cells exist across the whole ledger (s12 link-store order = 9, s12 loop-2 base written into rec_a = 41, s16 rec_b derived from rec_a = 44), so exit (b) is the only remaining region whose spelling space is genuinely unenumerated.

---

## s20 (structural, 2026-08-18) — the copy-materialisation predicate, measured at RTL

Chassis re-measured at dispatch: V1 body = **3** at 127/127 (rules_dropped 2,
cheat_asm_stripped 49). 16 new cells, one instrumented `-da` dump run
(`pwsh tools/grinder/dump.ps1`, cell U1). Artefacts: `tmp/grind/func_80017848/s20/`.

### E-s20-1 (CONFIRMED, from the U1 `.jump` vs `.cse` dumps) — cse DELETES a redundant preheader read; it only leaves a COPY when the destination has a use in a LATER extended basic block.

The U1 dump is the first direct RTL observation of what cse does to the
preheader redundant `*(u8 **)(ctx + 0xC)` read:

* loop 1 (`.jump` label 61 block): insn 64 `reg79 = mem(reg72+12)` (the guard
  read), insn 72 `reg86 = reg84 + reg79` (guard address), insn 75
  `reg86 = mem(reg86+28)` (guard load, **clobbering the address register**),
  insn 83 `reg80 = mem(reg72+12)` (preheader redundant read), insn 89
  `reg81 = reg84 + reg80` (base add).
* in `.cse`: **insn 83 is GONE — not turned into a copy, deleted outright** —
  and insn 89 has been rewritten to `reg81 = reg84 + reg79`, i.e. cse
  substituted the ORIGINAL pseudo at every use inside the same EBB.

So the "cse folds the redundant read to a reg-reg copy" model that sessions
s9-s19 worked from is only half right. cse folds the read; whether an insn
survives is decided by whether reg80 still has a use cse could NOT rewrite —
and cse can only rewrite uses inside the EBB it is currently processing. A use
in a later EBB forces cse to materialise `reg80 = reg79`, which is exactly the
`p = q;` loop-1 exit tail of the V1 candidate. **Clause B is not a way to make
an existing copy survive combine — it is the only reason a copy insn exists at
all.** This supersedes the s15/s16 wording (which described clause B as
combine-survival) without contradicting any of its measurements.

### E-s20-2 (CONFIRMED, same dump) — the guard SELF-CLOBBERING two-step is what keeps the base add alive; without it cse merges the two address adds.

Same dump, loop 2 (`.jump` label 140 block): insn 143 `reg79 = mem(reg72+12)`,
insn 151 `reg109 = reg85 + reg79` (guard address), insn 155
`reg111 = mem(reg109+32)` — the guard load writes a **different** pseudo, so
reg109 stays available — insn 162 preheader read, insn 168 `reg81 = reg85 +
reg80` (base add). In `.cse` **both insn 162 AND insn 168 are gone**: the base
add was merged into the guard add.

The discriminator is purely the guard spelling. Loop 1's guard in the V1
candidate is the two-step `t = sh + (s32)p; t = *(s32 *)(t + 0x1C);` — the SAME
local receives the address and the loaded value, so the address pseudo is
clobbered and cse cannot reuse it. Loop 2's guard is written inline, GCC picks
a fresh destination for the load, and the address survives to be reused. This
is the mechanism behind s15's E-4 ("A1/A2/A3/A4 all = 8, cse merges the guard
address add and the base add"), which s15 recorded as a bare number.

Direct consequence: the two-step is a *necessary* condition for loop 2 to even
have a base add of its own, and it is cheaply available — cell **W1** (V1 plus a
loop-2 guard two-step through a fresh local `t2`) scores exactly **3**, tying the
floor at 127/127. It is banked as `rejected/s20_t2_clobber_guard_two_step_inert_3.c`
because it does not improve, but it is the first loop-2 guard spelling that is
structurally equivalent to loop 1's rather than merely tied.

### E-s20-3 (KILLED) — s19's CLAUSE A is REFUTED as stated: cse *does* reach loop 2's preheader.

s19's theorem said loop 2's preheader can never be folded because it sits
downstream of the join `.L8001791C` that loop 1's `blez` creates. That is false.
The join label opens a NEW extended basic block, but loop 2's guard block and its
fall-through preheader are both *inside* that EBB, so a ctx+0xC read placed in
the guard block does make the preheader read redundant and cse does eliminate it
(measured: U1's `.cse` insn 162 deleted). Target's own asm is exactly this
shape — `lw a0,12(s2)` at T.txt:52 is **after** the join label, in the guard
block, not in loop 1's exit tail.

What actually blocks loop 2 is E-s20-2 + E-s20-1 in combination, and the two
pull in opposite directions:

* to fold the preheader read you must put a ctx+0xC read in the guard block;
* the guard block read is necessarily the guard's own addend, so the guard
  address `sh2 + <addend>` is available, and unless the guard is written in the
  self-clobbering two-step form cse eats the base add too (E-s20-2);
* and even with the two-step, cse *deletes* the folded read rather than leaving
  a copy unless the destination has a later-EBB use (E-s20-1), which costs an
  instruction.

### E-s20-4 (KILLED) — the whole "read in the join block" family is 10-37, uniformly worse than V1's 3.

Twelve cells, all banked in `rejected/`:

| cell | shape | score | insns |
|---|---|---|---|
| U1 | join-block `p = *(u8 **)(ctx + 0xC);` between the loops, `q` preheader addend in both loops, p reused for the math args | 11 | 124 |
| U2 | U1, math args back to inline double reads | 13 | 124 |
| U3 | U1, loop-2 preheader read left inline | 11 | 124 |
| U4 | U1 + loop-2 guard two-step through the SHARED `t` | 37 | 125 |
| U5 | U4, math args inline | 36 | 125 |
| U6 | U4 + loop-1 exit tail `p = q;` restored | 37 | 125 |
| U7 | U1 + loop-2 guard two-step through a FRESH `t2` | 10 | 125 |
| U9 | U1 + shift recomputed for loop 2's base | 11 | 124 |
| U10 | U1 + loop-2 exit tail `p = q;` (the canonicalisation flip) | 12 | 123 |
| U11 | U10 + loop-1 exit tail `p = q;` as well | 12 | 123 |
| T0 | symmetric chassis control (`p = *(u8 **)(ctx + 0xC);` as loop-1 exit tail, named loop-2 addend) | 4 | 126 |
| S1/S2 | fully target-shaped both loops, loop-2 clause-B `q2 = r2` + math args from q2 | 22 | 124 |

U7 is the sharpest of the family: with both guards self-clobbering, both base
adds survive, both preheader reads are folded away, and the result is exactly
two instructions SHORT (125) — the two missing copies. That is the cleanest
possible statement that the copies are the entire residual and that clause B is
their only C-level source.

### E-s20-5 (KILLED) — every clause-B consumer for loop 2 is self-cancelling, re-measured on the V1 chassis.

s19's frontier item (b) predicted that routing both `math_Distance3D` arguments
off loop 2's named preheader addend would be a free clause-B consumer, because
target derives both arguments from ONE `lw a1,12(s2)` (T.txt:77-82). It is not
free: consuming a live local there **deletes** that `lw`, so the purchase is
+1 (the materialised copy/move) and -1 (target's tail load), and the move lands
in loop 2's exit block where target has nothing. Measured on V1:

* **S3** (loop-2 addend named `r2`, carried to both math args, pre-initialised
  `r2 = p` so the skip path stays defined) = **7** at 127/127. Its diff shows
  loop 1's exit tail expanded to `addu a0,a3,zero / addu a1,a0,zero`, the tail
  `lw a1,12(s2)` gone, and loop 2's preheader still `lw a1,12(s2)`.
* **T1** (symmetric loop-1 tail + loop-2 clause-B into the math args) = 11 (125).
* **T2** (clause-B consumer = `rec_a` base) = 26 (128).
* **T3** (V1 tail + clause-B into the math args) = 21 (124).
* **W2** (W1's two-step chassis + named `q2` + `p = q2;` loop-2 exit tail + math
  args from p) = 21 (124) — the move is coalesced away entirely, so even the +1
  never materialises.
* **W3** (W1 + `q2` consumed by `rec_a`) = 14 at 127/127.

Frontier item (b) is therefore closed with six independent measurements, on the
V1 chassis it was never tested on.

### E-s20-6 (KILLED) — frontier item (a) is closed by construction, not by enumeration.

The probe s19 specified ("enumerate control-flow spellings that give loop 2's
guard exactly one predecessor") is unnecessary: U1 shows that a statement placed
between the two loops *is* the join block, loop 2's guard sits in the same basic
block as that statement, and cse's EBB therefore already covers loop 2's
preheader. The predecessor count of the guard label is not the blocker; the
add-merge (E-s20-2) and the copy-materialisation predicate (E-s20-1) are. No
CFG spelling changes either of those, because both are decided inside a single
EBB that already contains the preheader.

### The s20 restatement of the residual (supersedes the s19 two-clause theorem)

A preheader `addu DST,SRC,$zero` exists in GCC 2.7.2 output iff BOTH hold:

1. **(materialisation)** the redundant ctx+0xC read whose destination is DST has
   at least one use that cse cannot rewrite — i.e. a use in an EBB later than the
   one containing the preheader. Uses inside the same EBB are substituted and the
   read is deleted (E-s20-1).
2. **(add survival)** the guard block's own address computation must be dead by
   the time the base add is reached, which requires the guard to be written as a
   self-clobbering two-step (`t = sh + p; t = *(s32 *)(t + K);`). Otherwise cse
   merges the base add into the guard add (E-s20-2).

Condition 2 is free and now available for both loops (W1 = 3). Condition 1 costs
exactly one instruction, and the price is paid where the later-EBB use sits.
Loop 1 pays it in its exit tail, which is a slot target also fills (with a load
rather than a move) — hence loop 1 nets 1 mismatch instead of 2. Loop 2's only
later-EBB sites are the post-loop-2 tail, and every one of them either deletes an
instruction target has (the math base load) or adds one target does not have
(rec_a / rec_b staging). **Target's own bytes satisfy condition 1 with no visible
consumer at all**, which after twenty sessions remains the single unexplained
fact; s20 narrows it from "some pass creates a copy after combine" (s15) or "a
combine refusal" (s17) to "cse declined to substitute reg79 at the base add",
i.e. exactly E-s16-2's canonicalisation question, now with the cse-side
predicate named.

- [s20] Chassis re-measured at dispatch on a clean tree: the V1 candidate body scores 3 at 127/127 (rules_dropped 2, cheat_asm_stripped 49); HEAD's committed src/ings.c body still scores 16. src/ings.c was reverted to HEAD at the end of the session.

- [s20] E-s20-1 (CONFIRMED, RTL): in cell U1's dumps, loop 1's preheader redundant read `.jump` insn 83 `(set (reg/v 80) (mem (plus (reg/v 72) (const_int 12))))` is ABSENT from `.cse`, and the base add insn 89 has been rewritten from `(plus (reg/v 84) (reg/v 80))` to `(plus (reg/v 84) (reg/v 79))`. cse deletes the folded read and substitutes the original pseudo at every use it can rewrite; a copy insn appears only when the destination has a use in a LATER extended basic block.

- [s20] E-s20-2 (CONFIRMED, RTL): in the same dumps, loop 2's guard address `.jump` insn 151 `(set (reg 109) (plus (reg/v 85) (reg/v 79)))` is followed by insn 155 writing a DIFFERENT pseudo (reg111), so the address stays available and `.cse` deletes BOTH the preheader read (insn 162) and the base add (insn 168). Loop 1's guard load (insn 75) writes back into the address pseudo reg86, so loop 1's base add insn 89 survives. The discriminator is the guard's two-step self-clobbering spelling, nothing else.

- [s20] Cell W1 - the V1 candidate plus a loop-2 guard two-step through a fresh local `t2` (`t2 = sh2 + (s32)p; t2 = *(s32 *)(t2 + 0x20); if (i < t2)`) - scores 3 at 127/127. It is the first loop-2 guard spelling that satisfies the add-survival half of the predicate for free, and is the recommended chassis for future work. Banked at memory/grind/func_80017848/rejected/s20_t2_clobber_guard_two_step_inert_3.c.

- [s20] E-s20-3 (KILLED): s19's CLAUSE A ('loop 2's preheader is downstream of the join so cse can never fold it') is refuted. The join label opens a new EBB, but loop 2's guard block and its fall-through preheader are both inside that EBB; a ctx+0xC read in the guard block does make the preheader read redundant and cse does eliminate it. Target's `lw a0,0xC($s2)` (T.txt:52) is AFTER the join label, in the guard block, not in loop 1's exit tail.

- [s20] E-s20-4 (KILLED): the join-block-read family scores 10-37 across twelve cells (U1 11, U2 13, U3 11, U4 37, U5 36, U6 37, U7 10, U9 11, U10 12, U11 12, T0 4, S1/S2 22). U7 lands at 125 instructions - exactly two short, both copies missing and nothing else wrong.

- [s20] E-s20-5 (KILLED): s19's frontier item (b) is closed with six V1-chassis measurements (S3 7, T1 11, T2 26, T3 21, W2 21, W3 14). Consuming the post-loop-2 math base from a live local DELETES target's `lw a1,0xC($s2)`, so a clause-B purchase there is +1 move / -1 load with the move in a slot target leaves empty; in W2 the move is coalesced away entirely.

- [s20] E-s20-6 (KILLED): s19's frontier item (a) needs no CFG enumeration - a statement placed between the loops IS the join block and loop 2's guard sits in the same basic block as it, so the EBB already covers the preheader. Predecessor count is not the blocker.

- [s20] The s20 restatement of the residual (supersedes s19's two-clause theorem): a preheader `addu DST,SRC,$zero` exists iff (i) the folded read's destination has a use in a LATER EBB (materialisation - costs exactly one instruction) and (ii) the guard is written self-clobbering so the base add survives (free, and now available for both loops). Loop 1 pays (i) into a slot target also fills; loop 2 has no such slot. Target satisfies (i) with no visible consumer anywhere in its 127-instruction listing - that single fact is the whole remaining mystery, now narrowed from 'a pass after combine creates the copy' (s15) and 'a combine refusal' (s17) to 'cse declined to substitute reg79 at the base add', i.e. E-s16-2's canonicalisation question with the cse-side predicate named.

- [s20] 16 cells measured this session, all banked (13 new files in memory/grind/func_80017848/rejected/, bringing the bank to 146 entries). No cell scored below 3; no cheat-family construct was written or proposed.

## s21 (2026-08-18, STRUCTURAL) — floor 3, re-measured at 127/127 this session

Chassis check at dispatch: cell W1 (`memory/grind/func_80017848/rejected/
s20_t2_clobber_guard_two_step_inert_3.c` — the V1 body plus loop 2's guard
written as the same self-clobbering two-step `t2 = sh2 + (s32)p; t2 =
*(s32 *)(t2 + 0x20);` that loop 1 uses) re-measures at **score 3, 127/127**,
and the canonical V1 body of `candidate.c` re-measures at **score 3, 127/127**
at the end of the session. Both numbers were taken with
`sandbox func_80017848 --disable all` (rules_dropped 2, cheat_asm_stripped 49).
19 cells this session, plus objdump-normalised residual diffs for the cells that
changed the residual's SHAPE rather than only its score.

### E-s21-1 (KILLS s20 frontier item #1). A later-EBB use of loop 2's PREHEADER
ADDEND does NOT materialise a copy of the addend — it materialises a copy of
`base`, one slot AFTER the base add, even on the W1 chassis where the base add
provably survives.

s20's frontier item #1 was this session's designated highest-yield probe: name
loop 2's preheader addend (`q2 = *(u8 **)(ctx + 0xC); base = (u8 *)(sh2 +
(s32)q2);`) and give `q2` a use inside loop 2's BODY, which is a later extended
basic block (the loop top is a branch target, so cse starts a fresh EBB there and
cannot rewrite the use). The prediction was that cse would then be forced to
leave `DST = SRC` in the preheader, and that E-s20-2 — satisfied for free by W1's
self-clobbering guard two-step — would keep the base add alive so the copy landed
in FRONT of it, which is target's exact shape. Five cells were built on W1:

  A1  element read as `*(u8 *)((s32)q2 + sh2 + i + 0x2C)` (i-varying body use)
      -> score 6, 128 insns
  A2  back-edge limit as `*(s32 *)((s32)q2 + sh2 + 0x20)` (invariant body use)
      -> score 5, 128 insns
  A3  element associated pointer-first, `*(u8 *)(q2 + i + sh2 + 0x2C)`
      -> score 10, 128 insns
  A4  BOTH body uses via q2, so `base` is dead and the base add disappears
      -> score 3, 127 insns (inert — GCC re-derives identical addressing)
  A5  A1 with the base add itself associated pointer-first
      -> score 6, 128 insns

The residual diff for A1 and A2 is the same shape and it is decisive. Loop 2's
preheader comes out as

    lw   v0,12(s2)        <- the addend read, NOT folded away
    lw   a2,16(s2)
    addu a0,a1,v0         <- base add
    addu a1,a0,zero       <- THE COPY: its source is a0 = BASE, not the addend
    addu v0,a1,v1         <- element pointer, now taken off the copy

against target's

    addu a3,a0,zero       <- copy of the ADDEND
    lw   a2,16(s2)
    addu a0,a1,a3         <- base add consumes the copy
    addu v0,a0,v1

i.e. the copy IS created and DOES survive, but it is a copy of `base` sitting one
slot too late — byte-for-byte the same failure s15's cell C1 and s19's cells
Z1/Z2 produced on the INLINE-guard chassis. The W1 chassis was the one variable
those measurements had not controlled for, and it changes nothing; the extra
instruction (128 vs 127) is the un-folded addend load target does not have.

MECHANISM. This is s19's CLAUSE A re-confirmed, and it survives s20's refutation
of the CFG half of that clause. Whatever the EBB boundary does, by the time a
copy is placed the body's `q2 + sh2 + <offset>` address has already been
re-expressed in terms of `base` (loop.c's strength reduction plus cse2 find the
two equal, because `base` IS `sh2 + q2`), so the value that needs a later-EBB
carrier is `base`, never the addend. A body use can therefore never be the
clause-B consumer target uses. The only way to make the ADDEND the carried value
is a use that is not expressible as `base + constant` — and every such site lies
outside the loop, where E-s21-3 prices it.

### E-s21-2 (NEW, and the session's sharpest positive result). s15's standing
instruction "loop 2's base addend MUST stay a fresh `*(u8 **)(ctx + 0xC)` read;
do not re-probe that family" is CHASSIS-RELATIVE and is now superseded: on the W1
two-step chassis the reuse-p family scores 5, not 8, and one of its cells reaches
4 at the correct 127/127 with the residual's SHAPE changed for the first time
since s9.

s15 (4) measured cells A1..A4 — loop 2's base addend reusing the carried pointer
`p` instead of a fresh read — at a uniform 8, and attributed it to a mechanism:
if the base addend is provably equal to the GUARD's addend, cse merges the
guard-address add and the base add into one insn, whereas target computes
`sh2 + ptr` twice. But E-s20-2 established that loop 1's self-clobbering guard
two-step is exactly what prevents that merge (the address pseudo is dead, so cse
has nothing to reuse). W1 makes that two-step available for loop 2, which makes
s15's stated mechanism inapplicable — and the measurements follow:

  D1  W1 + loop 2's base addend reuses carried `p`     -> score 5, **126 insns**
  D2  D1 + body element read via `p`                   -> score 7, 127 insns
  D3  D1 + back-edge limit read via `p`                -> score 4, **127 insns**
  D4  D3 with the base add associated pointer-first    -> score 4, 127 insns
  D5  D3 with the limit read associated pointer-first  -> score 4, 127 insns
  D7  D3 + a named `lnk` local for loop 2's links      -> score 11, 127 insns

D1 is exactly ONE instruction short of target, and what is missing is exactly the
preheader copy: the fresh `lw v0,12(s2)` that the V1 chassis emits in that slot
is gone entirely. D3's residual is 4 and its loop-2 preheader is

    addu a0,a1,a0        <- base add, reusing the addend's own register in place
    addu a1,a0,zero      <- copy of base

against target's `addu a3,a0,zero / addu a0,a1,a3`. So on this chassis the
preheader contains the RIGHT TWO INSTRUCTION KINDS in the wrong order with the
wrong operands, rather than a load where target has a copy. That is a strictly
better structural description of the residual than V1's, at a cost of one point.
D4/D5 show the association order is inert on this chassis (both 4), matching s11.

### E-s21-3. Post-loop (exit-block) clause-B consumers are dead on the reuse-p
chassis as well, and for the reason s20 (4) found on V1: consuming the pointer
from a live local DELETES a reload target performs.

  E1  D1 + both math_Distance3D args derived from `p`   -> 21, 123 insns
  E2  D1 + `rec_a` derived from `p`                     -> 31, 126 insns
  E3  D3 + both math_Distance3D args derived from `p`   -> 20, 124 insns
  E4  D1 + math args AND `rec_a` derived from `p`       -> 15, **121 insns**

The instruction counts are the whole story: every post-loop consumer removes one
to six `lw ...,12(s2)` reloads that target keeps. There is no free clause-B slot
after loop 2 on any chassis measured to date (V1: 7/11/14/21/26 per s20 (4);
reuse-p: 15/20/21/31 here).

### E-s21-4. Making loop 2's GUARD the clause-B consumer costs BOTH copies —
and the cells isolate the direction of E-s20-1's materialisation predicate.
The one construction that would make loop 1's clause-B consumer free is to let
loop 2's guard address add — an instruction target already emits — be the
later-EBB use of loop 1's copy, instead of paying for it with the `p = q;` exit
tail. Two spellings:

  B1  outer/inner names swapped (`q` read at the top and used by loop 1's guard,
      `p` re-read inside loop 1's block as the addend), `p = q;` tail deleted,
      loop 2's guard two-step reading `q`                -> 12, **125 insns**
  B2  W1 with `q = p;` inserted before loop 1's guard so `q` is defined on the
      skip path, `p = q;` tail deleted, loop 2's guard on `q` -> 12, 125 insns

Both land at 125 — TWO instructions short, i.e. NEITHER preheader copy survives.
The reason is visible in the construction: in both cells the later-EBB use is a
use of the copy's SOURCE, not of its DESTINATION, so E-s20-1's predicate is not
triggered at all and cse deletes the redundant read outright. Useful negative: it
is the DESTINATION's later-EBB use that creates the copy, and no renaming of the
outer/inner pair changes which of the two loop 2's guard consumes.

### E-s21-5. s20 frontier item #3 is dead. Folding the clause-B consumer onto the
strength-reduced element pointer `addu v0,a0,v1` that target already emits at
loop 2's exit does not happen: cell C1 (named `q2`, plus a post-loop
`end = (u8 *)((s32)q2 + sh2 + i);` whose only consumer is the immediately
following math-arg base derivation) scores 20 at 127 insns. GCC does not fold the
post-loop expression onto the induction-variable update; it rebuilds the address
and the surrounding reload pattern changes wholesale.

- [s21] Floor re-measured twice this session on a clean tree: cell W1 (the s20 t2-two-step chassis) = 3 at 127/127 at dispatch, and the canonical V1 body of candidate.c = 3 at 127/127 at the end (rules_dropped 2, cheat_asm_stripped 49 in both runs).

- [s21] E-s21-1: a later-EBB use of loop 2's preheader ADDEND materialises a copy of BASE one slot AFTER the base add, never a copy of the addend before it — A1 6 / A2 5 / A3 10 / A5 6, all at 128 insns, control A4 inert at 3. The W1 chassis (base add kept alive by E-s20-2) was the one variable s15's C1 and s19's Z1/Z2 had not controlled for; it changes nothing. Mechanism: loop.c's strength reduction plus cse2 re-express any body address as `base + constant` before a copy is placed, so the value needing a later-EBB carrier is always `base`.

- [s21] E-s21-2: s15 (4)'s prohibition on reusing the carried pointer as loop 2's base addend is retracted as chassis-relative. On W1 the family is 5, not 8, and the guard/base merge s15 cited does not occur. D1 = 5 at 126 insns — one instruction short, and the missing instruction is exactly loop 2's preheader copy; the fresh `lw v0,12(s2)` that V1 emits in that slot is gone entirely.

- [s21] E-s21-2 (continued): D3 (D1 + the back-edge limit read taken via `p`) = 4 at the correct 127 insns, with loop 2's preheader `addu a0,a1,a0 / addu a1,a0,zero` against target's `addu a3,a0,zero / addu a0,a1,a3`. For the first time since s9 the preheader contains the right two instruction KINDS (an add and a reg-reg copy) rather than a load where target has a copy — wrong order, wrong operands, one point off the floor. D4/D5 confirm association order is inert on this chassis (both 4); D2 = 7; D7 (named loop-2 links local) = 11.

- [s21] E-s21-3: post-loop clause-B consumers are dead on the reuse-p chassis exactly as on V1 — E1 21 / E2 31 / E3 20 / E4 15, at 123 / 126 / 124 / 121 insns. Each one deletes reloads target keeps, so the purchase is net NEGATIVE in instruction count. Nine such consumers now measured across two chassis (V1: 7/11/14/21/26 per s20 (4)), none free.

- [s21] E-s21-4: making loop 2's guard the free clause-B consumer for loop 1 costs BOTH copies — B1 and B2 are both 12 at 125 insns, two short. The cells isolate the direction of E-s20-1's materialisation predicate: only a later-EBB use of the copy's DESTINATION creates a copy; a use of its SOURCE lets cse delete the redundant read outright. No renaming of the outer/inner pointer pair changes which of the two loop 2's guard consumes, because the guard necessarily consumes whichever name is live across the join.

- [s21] E-s21-5: s20 frontier item #3 is dead — cell C1 (post-loop `end` pointer derived from loop 2's addend, feeding the math-arg base) = 20 at 127 insns; GCC does not fold the expression onto the existing `addu v0,a0,v1` induction-variable update.

- [s21] Bookkeeping: 18 new forms banked in memory/grind/func_80017848/rejected/ (bank now 164 files); src/ings.c restored to its committed HEAD body at the end of the session; no build-pipeline file, rule file or engine file was touched.

## s22 (2026-08-18, FORENSICS) — floor 3, re-measured 127/127; the preheader-copy question moves from behaviour to RTL

Chassis check at dispatch: the D3 cell (`memory/grind/func_80017848/rejected/
s21_reuse_p_plus_body_limit_use_costs_4_127insns.c`) re-measures at **score 4,
127/127**, and the canonical V1 body of `candidate.c` re-measures at **score 3,
127/127** (both `sandbox func_80017848 --disable all`, rules_dropped 2,
cheat_asm_stripped 49). The floor is unchanged at 3. Two full `-da` dump sets
were captured with the CANONICAL cc1 (`tools/gcc-2.7.2/build/cc1`, via
`pwsh tools/grinder/dump.ps1 func_80017848`, no `-Instrumented` flag, so no
codegen-identity question arises) — one for D3 and one for V1 — and the
per-function RTL regions were extracted to `tmp/grind/func_80017848/s22/*.rtl`.

### E-s22-1 (CONFIRMS s15 (1) at RTL level, and sharpens it into a two-condition predicate). The copy-before-base-add ORDER is produced by local-alloc's `optimize_reg_copy_1`, and the two conditions it needs are now readable.

In the D3 dumps, loop 1's preheader is (insn numbers are stable across passes):

    .cse   insn 83  (set (reg/v:SI 80) (reg/v:SI 79))            <- the copy
           insn 86  (set (reg/v:SI 77) (mem (plus reg72 16)))    <- lnk load
           insn 89  (set (reg/v:SI 81) (plus (reg 84) (reg 79))) <- base add
                                                       ^^^^^^^^ reads the ORIGINAL

    .lreg  insn 83  (set (reg 80) (reg 79))  REG_DEAD reg79      <- unchanged
           insn 86  ... unchanged ...
           insn 89  (set (reg 81) (plus (reg 84) (reg 80)))
                                                       ^^^^^^^^ reads the COPY

Nothing between cse and lreg (loop, cse2, combine, flow) touches insn 89's
operand; the rewrite happens inside local-alloc. That is exactly
`optimize_reg_copy_1` (local-alloc.c:700, dispatched :1006): when a copy's SOURCE
dies at the copy (`REG_DEAD reg79` is present on insn 83 in `.lreg`), the pass
re-points every subsequent use of SRC onto DEST. So target's shape
`addu a3,a0,zero / addu a0,a1,a3` requires BOTH of:

  (i) a copy insn `DEST = SRC` sitting textually BEFORE the base add, and
  (ii) SRC dying at that copy (no later use of SRC).

This closes s21's frontier item #2 question ("does the base add read the original
or the copy?") with the dump instead of a sandbox sweep: at cse it reads the
ORIGINAL in every cell; the order is never a cse decision.

### E-s22-2 (KILLS s21 frontier item #2 outright). D3's loop-2 preheader copy is NOT a cse-materialised copy at all — it is created by **loop.c**, one slot too late, and it copies BASE. `optimize_reg_copy_1` provably cannot reorder it.

D3's loop-2 preheader in `.cse` contains exactly ONE insn:

    insn 162 (set (reg/v:SI 81) (plus (reg 85) (reg 79)))   <- the base add

In `.loop` (and thereafter in `.cse2`, `.combine`, `.lreg`) two further insns
have appeared AFTER it, both absent from `.cse`:

    insn 337 (set (reg:SI 116) (mem (plus reg72 16)))   <- LICM'd ctx+0x10 load
    insn 338 (set (reg:SI 122) (reg/v:SI 81))           <- THE COPY: of BASE

i.e. the copy is loop.c's own invariant-hoisting temporary for the back-edge
limit read, born after the base add. Condition (ii) of E-s22-1 cannot hold for
it: its SRC is `reg81`, defined by the immediately preceding base add, so SRC
does not die *before* the base add and there are no earlier uses to re-point.
No variation of what D3's body-limit use looks like — s21's proposed (a) offset
split, (b) second named local, (c) `while` instead of `do/while` — can change
that, because all three keep the copy's source equal to base. **s21 frontier
item #2 is dead: D3's preheader order is not a tunable.**

### E-s22-3 (NEW, positive). Loop 1's preheader is byte-exact with target INCLUDING hard-register assignment, so the construct itself is proven; the residual is now a pure SITE-SWAP.

D3's and V1's loop-1 preheader assemble as

    addu $7,$4,$0 / lw $6,16($18) / addu $4,$5,$7 / addu $2,$4,$3

which is target's `addu a3,a0,zero / lw a2,0x10(s2) / addu a0,a1,a3 /
addu v0,a0,v1` register-for-register ($7=a3, $4=a0, $5=a1, $6=a2, $2=v0, $3=v1).
Target's two loop preheaders are themselves byte-identical to each other apart
from the body offsets, so the original C used the SAME construct for both loops.

The objdump-normalised residual of V1 (re-taken this session; the three
branch/jal lines in the raw diff are relocation-normalisation noise) is exactly
three instructions, and they form a SWAP of two sites:

    loop-1 exit      target `lw a0,12(s2)`       ours `addu a0,a3,zero`
    loop-2 preheader target `addu a3,a0,zero`    ours `lw v0,12(s2)`
    loop-2 base add  target `addu a0,a1,a3`      ours `addu a0,a1,v0`

Target RE-READS where we COPY, and COPIES where we READ. In `.lreg` the V1
loop-2 preheader is `insn 162 (set (reg 113) (mem (plus reg72 12)))` carrying a
`REG_EQUIV` note, feeding `insn 164` the base add — target's copy occupies insn
162's slot exactly. So the whole remaining gap is: make loop 2's addend arrive as
a reg-reg copy and loop 1's exit tail arrive as a fresh load.

### E-s22-4. Why cse cannot fold loop 2's fresh read into a copy (the structural reason V1's site is a `lw`), and why the obvious swap costs 13.

cse works per extended basic block. Loop 2's guard label has two predecessors
(loop 1's skip branch and loop 1's fall-through exit), so cse starts a fresh EBB
there with no mem->reg equivalences; a `*(u8 **)(ctx + 0xC)` read in that block
is therefore always a real load (V1's insn 162 even carries `REG_EQUIV`, the
signature of an unfolded load). The same argument explains target's loop-1 exit
`lw a0,12(s2)`: the exit block is likewise a fresh EBB, so a C-level re-read
there is a load — which is precisely what target has and what our `p = q;` copy
is not.

The direct attempt at the swap was built and measured:

  G1  V1 + `q = p;` pre-init before loop 1's guard, loop-1 tail rewritten as a
      fresh `p = *(u8 **)(ctx + 0xC);` re-read, loop 2's base addend reading `q`
      (so `q`'s later-EBB use keeps loop 1's copy alive)   -> **13 at 126 insns**

banked as `rejected/s22_q_preinit_tail_reread_l2_addend_q_costs_13.c`. Its
residual shows why: the pre-init `q = p;` relocates loop 1's copy (it becomes
`addu a0,a2,zero` under a shifted register assignment) and loop 2 still gets NO
copy, while the added definition perturbs block ordering (a `blez`/`j` pair
moves), losing one instruction net. The definedness requirement is structural:
loop 2's addend must be defined on BOTH paths into the join, so it can only be
the carried variable or a fresh read — a loop-1-local copy destination can never
serve without a pre-init, and the pre-init itself costs.

- [s22] Floor re-measured 3 (V1, 127/127) and D3 re-measured 4 (127/127) on a
  clean tree; chassis unchanged. src/ings.c restored to its committed HEAD body
  at the end of the session; no build-pipeline, rule or engine file touched.
- [s22] E-s22-1: `.cse` vs `.lreg` prove the copy/base-add ORDER is local-alloc's
  `optimize_reg_copy_1` rewrite, gated on (i) a copy textually before the base
  add and (ii) REG_DEAD on the copy's source. At cse the base add ALWAYS reads
  the original — the order is never a cse decision.
- [s22] E-s22-2: D3's loop-2 copy (insn 338) is created by loop.c, copies BASE,
  and sits after the base add — provably outside `optimize_reg_copy_1`'s
  precondition. s21 frontier item #2 (vary the body-limit use to flip the order)
  is dead without further sweeps.
- [s22] E-s22-3: loop 1's preheader is byte-exact with target including hard
  registers; V1's 3-instruction residual is a pure SITE-SWAP (target re-reads at
  loop 1's exit and copies at loop 2's preheader; we do the opposite).
- [s22] E-s22-4: cse cannot fold a read in loop 2's guard block because that
  label has two predecessors and starts a fresh EBB (V1's insn 162 carries
  REG_EQUIV). The direct swap cell G1 = 13 at 126; the pre-init needed to make a
  loop-1-local name defined on both paths is itself the cost.
- [s22] Correction to candidate.c's s9 header: that header attributes the
  preheader copy's survival to combine's `can_combine_p` refusal. The dumps show
  the copy is already present at `.cse` and unchanged through `.combine`; what
  combine decides is only whether a copy with an in-block use is folded away
  (s16's trichotomy). The ORDER — the part that matters for target's shape — is
  decided later, in local-alloc.

- [s22] Floor re-measured this session on a clean tree: V1 (candidate.c) = 3 at 127/127; D3 = 4 at 127/127 (rules_dropped 2, cheat_asm_stripped 49). Chassis unchanged from s21.

- [s22] E-s22-1: .cse vs .lreg prove the copy/base-add ORDER is local-alloc's optimize_reg_copy_1 rewrite, gated on (i) a copy textually before the base add and (ii) REG_DEAD on the copy's source. At cse the base add ALWAYS reads the original.

- [s22] E-s22-2: D3's loop-2 copy (insn 338) is created by loop.c, copies BASE, and sits after the base add - provably outside optimize_reg_copy_1's precondition. s21 frontier item #2 dies without any sandbox sweep.

- [s22] E-s22-3: loop 1's preheader is byte-exact with target including hard-register assignment; V1's 3-instruction residual is a pure SITE-SWAP (target re-reads at loop 1's exit and copies at loop 2's preheader; we do the opposite).

- [s22] E-s22-4: cse cannot fold a ctx+0xC read placed in loop 2's guard block because that label has two predecessors and starts a fresh EBB - V1's insn 162 carries REG_EQUIV, the signature of an unfolded load. The same argument explains target's own `lw a0,12(s2)` at loop 1's exit.

- [s22] Cheapest future diagnostic (new): for any cell, one .cse read answers whether it can ever reach target - does the loop-2 preheader contain (set (reg X) (reg Y)) with Y the ADDEND? Every cell to date that produced a loop-2 preheader copy produced a copy of BASE (loop.c), which E-s22-2 shows is terminal.

- [s22] Correction to candidate.c's s9 header: it attributes the preheader copy's survival to combine's can_combine_p refusal. The dumps show the copy already exists at .cse and is unchanged through .combine; combine only decides whether a copy with an IN-BLOCK use is folded away (s16's trichotomy). The ORDER is decided later, in local-alloc.

- [s22] One new cell measured and banked (G1 = 13 at 126); bank now 165 files. src/ings.c restored to its committed HEAD body; no build-pipeline, rule, engine or tools file touched; nothing committed.

## s23 (2026-08-18, FORENSICS) — floor 3; the loop-2 preheader copy is BUILT for the first time, and the family is killed at RTL by an allocation conflict

Chassis check at dispatch, on a clean tree, all `sandbox func_80017848 --disable all`
(rules_dropped 2, cheat_asm_stripped 49): **V1 (`candidate.c`) = 3 at 127/127**,
**W1 (loop-2 guard two-step, `tmp/grind/func_80017848/s21/W1.c`) = 3 at 127/127**,
**D3 = 4 at 127/127**. Identical to s21/s22 — the chassis has not moved. All s23
cells are built on W1, whose loop-2 guard two-step E-s20-2 showed keeps the
loop-2 base add alive for free.

### E-s23-1 (NEW, POSITIVE — the largest structural result since s9). `use_crosses_set_p` DOES fire from pure C, and it produces target's ENTIRE instruction stream: both preheader copies AND the loop-1 exit fresh read, at 127/127.

s17's H-s17-A tested the `use_crosses_set_p` combine-refusal path on the V1
chassis (loop-2 guard inline) and priced it dead at 12-14. s20/s21 then changed
the chassis (W1's self-clobbering loop-2 guard), so — exactly as s15's addend
prohibition had to be retracted after W1 — the s17 numbers were re-measured here.
They reproduce (Q1 = 14, Q2 = 14, Q3 = 12, control Q4 = 14, control Q6 = 5), so
s17's *scores* stand. What s17 never did was look at the RESIDUAL, and the
residual is a different animal from its score.

The winning shape is the mechanism applied SYMMETRICALLY to both loops
(cell Q10, banked as
`rejected/s23_crosses_set_p_reused_as_links_both_loops_exact_insn_stream_costs_14.c`):

    if (i < t) {
        q = p;                       /* the copy: (set q p)                  */
        p = *(u8 **)(ctx + 0x10);    /* SETS the copy's source between i2/i3 */
        base = (u8 *)(sh + (s32)q);  /* the base add — combine must refuse   */
        do { ... (s32)p ... } while (i < *(s32 *)(base + 0x1C));
        p = *(u8 **)(ctx + 0xC);     /* p was clobbered -> HONEST re-read    */
    }
    ... identical construct for loop 2 ...

Because `p` is reused to hold the loop's links pointer, (a) cse cannot
canonicalise the base add's operand back to `p` (p has been invalidated),
(b) combine's `can_combine_p` hits `use_crosses_set_p` and declines to fold the
copy, and (c) the carried pointer is genuinely dead at the loop exit, so loop 1's
tail MUST re-read `ctx + 0xC` — which is target's `lw $a0,0xC($s2)` at
`asm/funcs/func_80017848.s:56`, the instruction eight sessions bought with a
`move` and called mismatch #1.

Q10's objdump-normalised residual (`tmp/grind/func_80017848/s23/T.txt` vs
`B.txt`) contains **no instruction-kind, no instruction-count and no ordering
difference anywhere in the function** — 127/127, and every one of the following
lines up:

    loop-1 preheader  ours   addu v0,a1,zero / lw a1,16(s2) / addu a0,a0,v0
                      target addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3
    loop-1 exit tail  ours   lw a1,12(s2) / sll a0,s4,6
                      target lw a0,12(s2) / sll a1,s4,6
    loop-2 preheader  ours   addu v0,a1,zero / lw a1,16(s2) / addu a0,a0,v0
                      target addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3

This is the first cell in 23 sessions in which loop 2's preheader contains a
reg-reg copy OF THE ADDEND placed BEFORE the base add with the base add reading
it — s22's cheap `.cse` diagnostic asked exactly that question, and every prior
cell that produced a loop-2 copy produced a copy of BASE (loop.c), which E-s22-2
proved terminal. The score is 14 and it is **100% register identity**.

### E-s23-2 (KILLED, with the RTL that proves it). The `use_crosses_set_p` family can NEVER reach 0: it fuses the carried pointer and the links pointer into ONE pseudo, and target's own allocation proves they are two.

The mechanism's precondition is a set of the copy's SOURCE between the copy and
the base add. The only value target computes there is the links pointer
(`lw $a2,0x10($s2)`), so the C must assign it to the carried pointer variable —
one C local, therefore one pseudo, therefore ONE hard register for both roles.
Target's allocation is incompatible with that:

    target: pointer  = $a0, dies AT the base add  ->  base REUSES $a0
            links    = $a2, live through the body
            copy dst = $a3        shift = $a1        i = $v1

A single pseudo cannot be `$a0` before the base add and `$a2` through the body.
In our dumps the consequence is visible as a conflict: with `p` carrying links,
`p` is live through the loop body, so `p` CONFLICTS with `base` and `base` can
never be given `p`'s register. `.greg` for cell S1 shows `78 in 4  80 in 5
81 in 5` — the pointer gets `$a0` correctly, and both bases are pushed to `$a1`.
The family's floor is therefore the register-identity residual (measured 12-16
across 21 cells), not 0. Do NOT spend another session tuning it.

### E-s23-3 (NEW, CONFIRMED and PREDICTIVE — a general lever, not just for this function). GCC 2.7.2's global-alloc priority is `floor_log2(n_refs) * n_refs / live_length`, it is computable from the `.lreg` header, and moving a C variable across it flips hard-register assignment as predicted.

`tools/gcc-2.7.2/global.c`'s `allocno_compare` sorts allocnos by

    pri = (floor_log2(n_refs) * n_refs / live_length) * 10000 * size

and `.lreg`'s per-register header line (`Register N used R times across L insns`)
gives R and L directly, so the ORDER printed in `.greg`'s `;; N regs to allocate:`
line is predictable by hand. For Q10:

    88 (i)      26/40 -> 2.60      80 (base)  10/22 -> 1.364
    79 (q)       4/6  -> 1.333     78 (p)     12/39 -> 0.923
    72 (ctx)    15/89 -> 0.506     84/85 (sh)  3/8  -> 0.375

and `.greg` prints `;; 13 regs to allocate: 87 97 109 80 79 78 72 77 75 83 84 74
73` — base before p, so base takes `$a0` and p is pushed to `$a1`, which is the
a0/a1 swap that dominates Q10's 14 points.

PREDICTION AND CONFIRMATION: splitting `base` into `base1`/`base2` (one per loop)
halves its refs and live length to 5/11, giving priority `log2(5)*5/11 = 0.909`,
just BELOW p's 0.923 — so p should be allocated first and take `$a0`. Cell S1's
`.greg` prints `;; 14 regs to allocate: 88 98 110 79 78 80 81 ...` (p now ahead
of both bases) and dispositions `78 in 4` (`$a0`, target's register for the
pointer) with `80 in 5  81 in 5`. The prediction held exactly. The score did not
move (14) because the gain at the pointer is paid back at `base`, per E-s23-2.

This makes allocno priority a MEASURABLE, C-STEERABLE quantity for the first
time in this ledger: any variable's hard register can be moved earlier or later
in the allocation order by splitting it (fewer refs / shorter live range, lower
priority) or by fusing it (more refs, higher priority), and the `.lreg` header
gives the numbers without a sandbox run. Register-identity residuals elsewhere in
the queue should be attacked this way rather than by blind spelling sweeps.

### E-s23-4. Twenty-one allocation-tuning cells on the exact-instruction-stream chassis, all 12-16.

    Q10 symmetric both loops                              14 @127
    Q11 separate copy dests q/r                           14 @127
    Q12 loop 1 only                                       14 @127
    Q13 pointer-first base association (both loops)       14 @127
    Q14 pointer-first guard association (both loops)      12 @127
    Q1/Q2/Q3 loop-2-only, copy into r / q / lnk    14 / 14 / 12 (Q3 @125)
    Q4 CONTROL links-reuse alone, no copy                 14 @127
    Q5 + body limit read via the copy dest                16 @128
    Q6 CONTROL bare source copy, links left inline         5 @126
    R1 shift assigned before the pointer                  14 @127
    R2 pointer-first guards AND bases                     12 @127
    R3 R2 + shift-first creation order                    12 @127
    R4 single shift local for both loops                  14 @127
    R5 loop-1 mechanism + loop-2 named links local        12 @126
    R6 inline (non two-step) guards                       14 @127
    R8 `i = 0` after the shift in the join block          14 @127
    S1 split base1/base2                                  14 @127
    S2 S1 + separate copy dests                           14 @127
    S3 separate copy dests only                           14 @127
    S4/S5 S1/S2 + pointer-first guards               16 / 16 @127

The family is flat: no association, creation-order, splitting or guard-shape
change moves it below 12, and every cell keeps the instruction stream exact.
That flatness is E-s23-2 showing through — the mismatches are a fixed
consequence of the pointer/links pseudo fusion, not of any spelling.

- [s23] Floor unchanged at 3 (V1 re-measured 127/127 at dispatch; W1 = 3, D3 = 4).
  src/ings.c restored to its committed HEAD body at the end of the session; no
  build-pipeline, rule, engine or tools file touched; nothing committed.
- [s23] E-s23-1: the `use_crosses_set_p` construct (carried pointer reused to hold
  the loop's links pointer) reproduces target's ENTIRE instruction stream at
  127/127 — both preheader copies and the loop-1 exit `lw` — for the first time
  in 23 sessions. Score 14, all of it register identity.
- [s23] E-s23-2: that family is structurally incapable of reaching 0 — it forces
  the carried pointer and links into one pseudo, while target keeps them in `$a0`
  (dying at the base add, its register reused by `base`) and `$a2`. Confirmed by
  S1's `.greg` conflicts/dispositions.
- [s23] E-s23-3: `allocno_compare` priority = floor_log2(n_refs)*n_refs/live_length
  is computable from `.lreg` headers and predicted, correctly, that splitting
  `base` moves the pointer from `$a1` to `$a0`. Allocation order is now a
  steerable, cheaply-measurable C-level quantity.
- [s23] Bank now 170 files (5 new). Artifacts: `tmp/grind/func_80017848/s23/`.

- [s23] Chassis re-measured at dispatch on a clean tree: V1 (candidate.c) = 3 at 127/127, W1 (loop-2 two-step guard) = 3 at 127/127, D3 = 4 at 127/127; rules_dropped 2, cheat_asm_stripped 49. Unchanged from s21/s22.

- [s23] E-s23-1: cell Q10 - the carried pointer local reused to hold each loop's links pointer - reproduces target's ENTIRE instruction stream at 127/127, including both preheader copies (addu copy / links lw / base add reading the copy) and loop 1's exit-tail fresh ctx+0xC read that eight sessions bought with a move. First cell in 23 sessions whose loop-2 preheader contains a copy OF THE ADDEND before the base add; every previous loop-2 copy was a copy of BASE created by loop.c (E-s22-2, terminal).

- [s23] E-s23-2 (the kill): the mechanism forces the carried pointer and the links pointer into ONE C local, hence one pseudo and one hard register, while target's allocation needs the pointer in $a0 dying at the base add (base reuses $a0) and links in $a2 live through the body. p-as-links is live through the body, conflicts with base, and base can never inherit its register - S1's .greg dispositions ('78 in 4  80 in 5  81 in 5') show it directly. The family's floor is the register-identity residual (12-16 over 21 cells), not 0.

- [s23] E-s23-3: global.c allocno_compare priority = floor_log2(n_refs)*n_refs/live_length, both inputs printed by .lreg; it reproduced .greg's allocation order for Q10 and correctly predicted that splitting base into base1/base2 flips the pointer from $a1 to $a0 (cell S1). Hard-register identity is now a steerable, sandbox-free measurable for this compiler - applicable to any register-identity residual in the queue.

- [s23] E-s23-4: 21 allocation-tuning cells all score 12-16 at 125-128 insns; best of family 12 (pointer-first guards Q14/R2/R3, loop-1-only mechanism with a named loop-2 links local R5). The flatness is the pseudo fusion showing through, not a spelling gap.

- [s23] s17's H-s17-A scores reproduce on the W1 chassis (Q1 = 14 vs P1 = 14, Q3 = 12 vs P3 = 12, control Q4 = 14 vs P5 = 14), so the s17 kill was chassis-independent in PRICE - but s17 never read the residual, which is why the construct's structural exactness went unnoticed for six sessions.

- [s23] src/ings.c restored to its committed HEAD body at the end of the session; no build-pipeline, rule, engine or tools file touched; nothing committed. Rejected bank now 170 files (5 new).

## s24 (2026-08-18, ESCALATION / DISPOSITION) — floor 3; the last named frontier item is killed and the `use_crosses_set_p` family is closed by a COMPLETE DICHOTOMY

Chassis check at dispatch and again at the end of the session, on a clean tree,
`sandbox func_80017848 --disable all` (rules_dropped 2, cheat_asm_stripped 49):
**V1 (`memory/grind/func_80017848/candidate.c`) = 3 at 127/127**, both times.
The chassis has not moved since s21.

### E-s24-1 (KILLED — s23's frontier item #2, the "give links its own pseudo, consumed inside the preheader" exit).

s23's frontier #2 proposed escaping E-s23-2's pointer/links pseudo fusion by
keeping the SET of the copy's source (which the `use_crosses_set_p` refusal
requires) while immediately consuming it into a separate local inside the
preheader, so the carried pointer dies before the loop body and stops conflicting
with `base`.  Two cells, both built on the V1 chassis (loop 1 unchanged, so loop
1's bytes stay exact and only loop 2 carries the new construct):

    F2  q = p; p = *(u8 **)(ctx + 0x10); lnk  = p; base = (u8 *)(sh2 + (s32)q);
        body reads (s32)lnk                                   ->  8 at **126** insns
    F3  same, but into a freshly declared local `lnk2`
        (so the loop-1 links local is not reused)              ->  9 at **126** insns

Both are ONE INSTRUCTION SHORT of target, and the missing instruction is the
copy itself: the preheader copy does not materialise at all.  The reason is
mechanical and complete — `lnk = p` immediately after `p = <load>` is a plain
register copy that copy-propagation folds, so `lnk` and `p` collapse back to ONE
pseudo whose set is the load.  The set therefore no longer sits BETWEEN the copy
and the base add (it IS the pseudo the base add's operand was folded against),
`use_crosses_set_p` never fires, and combine substitutes `q := p` into the base
add and deletes the copy.  Banked as
`rejected/s24_split_links_local_preheader_consumed_copy_dies_costs_8_126insns.c`
and `rejected/s24_split_links_distinct_local_lnk2_copy_dies_costs_9_126insns.c`.

### E-s24-2 (the DICHOTOMY — this is the general statement s23's frontier #1 was asking for, and it is negative).

Combining E-s23-2 with E-s24-1 gives an exhaustive two-way split over the ONLY
mechanism known to produce a surviving loop-2 preheader copy:

  * The refusal requires an RTL SET of the copy's SOURCE pseudo between the copy
    insn and the base add (that is what `use_crosses_set_p` tests).
  * The only value target computes at that point is the links pointer
    (`lw $a2,0x10($s2)`), so in C the carried-pointer local must receive it.
  * EITHER the loop body then reads that same local — one pseudo carries both
    roles, it is live through the body, it conflicts with `base`, and `base` can
    never inherit the register the pointer dies in.  Measured floor of that
    branch: **12-16 across 21 cells (s23) + 2 more here**, 100% register identity.
  * OR the value is copied out into a second local so the pointer can die early —
    copy propagation refolds the two pseudos into one, the required SET vanishes,
    and the copy is deleted.  Measured: **8-9 at 126 insns (E-s24-1)**.

There is no third position: the pointer either carries links into the body or it
does not.  s23's frontier #1 ("a THIRD producer whose blocking condition is NOT a
set of the copy's source") is therefore not a gap in this family — it is a request
for a DIFFERENT combine refusal, and s17's E-s17-1 already enumerated all seven
`can_combine_p` refusal paths (2 forbidden cheat families, 3 structurally
unreachable, 1 = V1's own loop-1 out-of-block-use lever priced dead for loop 2,
1 = `use_crosses_set_p`, closed here).  E-s20-1 further supersedes the combine
framing with the cse-side predicate (a copy exists ONLY when the folded read's
destination has a use in a LATER EBB), and every clause-B consumer site for loop
2 is measured dead (post-loop 15-32, in-body 5-16, guard 12 at 125 insns,
math-arg 7-26).

### E-s24-3 (s23's frontier item #3 answered by construction — no measurement needed).

The allocno-priority instrument (E-s23-3) acts on HARD-REGISTER IDENTITY.  V1's
residual is not a register-identity residual: it is two instruction-KIND
differences (`lw $a0,0xC($s2)` vs our `addu a0,a3,zero` in loop 1's exit tail;
`addu $a3,$a0,$zero` vs our `lw v0,12(s2)` in loop 2's preheader) plus the
operand of the following base add.  No allocation order can turn a load into a
copy, so the instrument has no purchase on the floor body; it is only meaningful
on the Q10-family chassis, where E-s23-2 has already priced its ceiling.  The
instrument remains valid and transferable — it belongs to the QUEUE, not to this
function.

### E-s24-4 (disposition gates, re-measured this session).

    $ python3 tools/scan_hand_coded.py --single func_80017848
      HAND_CODED: tier=LOW  score=0/8  (func_80017848, 127 insns)
      S1..S8 all unset (0 multu pairs, no empty-body branches, 7 spills /
      12 distinct regs, max load burst 3/8, jaccard < 0.5 vs all siblings,
      no BIOS jumptable, all callee-saves saved, no redundant mask-before-shift)

  Gate (a) canonical-asm: **FAIL** (LOW, 0/8 — unchanged from s17).
  Gate (b) in-hand SOTN-master precedent for the closing construct: **FAIL** —
  no citation exists.  s18's decomp.me corpus mining (163 hits, structural twin
  scratch 19TpT) is prior art that the IDIOM is compiler-producible pure C, which
  is why the scanner is correctly LOW, but it is not a SOTN-master file:line and
  it explicitly documents that the twin's enabling precondition (a nested loop
  whose outer body keeps the copy's source live for free) does not exist here.
  What holds the byte-match: **2 asmfix.txt rules** (`asmfix.txt:60-61` — a
  `delete_between "^\.frame" "^\.end func_80017848$"` plus an `insert_before`
  carrying the entire 127-instruction body as rule text).

### E-s24-5 (re-dispatch verification — the disposition's numbers are reproduced, and the discard cause is repaired).

The first s24 run was discarded by the outcome validator on a
`UnicodeDecodeError` (`grindlib.py:388` reads `docs/grind/decisions.md` as UTF-8;
the run had written 8 cp1252 `0x97` em-dashes into it, first at byte offset
1237144). The bytes are repaired (UTF-8 U+2014, file decodes cleanly) and every
load-bearing measurement was re-run independently rather than inherited:

    committed HEAD body        sandbox --disable all = 16   (127 target / 125 build)
    candidate.c in src/ings.c  sandbox --disable all =  3   (127 / 127)
    scan_hand_coded --single   tier=LOW score=0/8, S1..S8 all unset
    byte-match holder          asmfix.txt:60-61 (delete_between + whole-body insert_before)

Raw capture: `tmp/grind/func_80017848/s24/reverify_s24r.txt`. `src/ings.c` was
restored to its committed HEAD body; no build-pipeline, rule, engine or tools file
was touched. The disposition (both endgame-lock gates fail; owner's standing
ruling 2026-07-27 applies) is unchanged and terminal.


- [s24] Re-measured this session: committed HEAD src/ings.c gives sandbox --disable all = 16 (127 target / 125 build); memory/grind/func_80017848/candidate.c applied gives 3 (127/127), rules_dropped 2, cheat_asm_stripped 49 in both. The ledger floor of 3 is current, not stale.

- [s24] Gate (a) FAILS: scan_hand_coded --single func_80017848 = tier=LOW score=0/8 with all eight signals unset. This is ordinary compiled C, not hand-written assembly.

- [s24] Gate (b) FAILS: no SOTN-master file:line or commit hash exists for any construct that would close the residual. Session 18's decomp.me corpus mining (3,754 scratches, structural twin scratch 19TpT = func_8009C6D8, gcc2.7.2-cdk -O2) is external prior art that the idiom is compiler-producible, and it corroborates gate (a)'s LOW verdict, but it is not a precedent citation and is not claimed as one; the twin's enabling precondition (a nested loop keeping the copy's source live for free) is one func_80017848 cannot buy.

- [s24] The byte-match is held by exactly two asmfix.txt rules, lines 60-61: a delete_between of the .frame/.end region plus an insert_before whose payload is the entire 127-instruction body as rule text. Zero regfix rules; the C body carries no cheat-asm.

- [s24] Exhaustion (from the ledger, unchanged): 24 sessions, floor flat at 3 since session 9, modalities rederive / synthesis / structural / forensics / permuter / escalation; 172 rejected forms banked; ~180,000 permuter iterations across five telemetried campaigns on two chassis with zero engine-scored improvements; translation-unit context, m2c re-derivation, BB2 sibling transplant (max 5-gram overlap 0.120 over 1,437 functions) and Kengo transplant all closed with measurements.

- [s24] The residual is two instruction-KIND differences plus one operand: loop-1 exit tail target `lw $a0,0xC($s2)` vs ours `addu $a0,$a3,$zero`; loop-2 preheader target `addu $a3,$a0,$zero` / `addu $a0,$a1,$a3` vs ours `lw $v0,0xC($s2)` / `addu $a0,$a1,$v0`.

- [s24] s24's dichotomy (E-s24-2) stands and is the reason no further spelling search is warranted: a surviving loop-2 preheader copy needs an RTL set of the copy's SOURCE between the copy and the base add, the only value target computes there is the links pointer, so either the body reads that same local (one pseudo, live through the body, conflicts with base - 12-16 across 23 cells) or the value is copied out so the pointer dies early (copy propagation refolds it, combine deletes the copy - 8-9 at 126 insns). No third position exists.

- [s24] Repaired the defect that discarded the previous run: 8 cp1252 0x97 bytes in docs/grind/decisions.md (first at offset 1237144) made grindlib.py:388's UTF-8 read raise UnicodeDecodeError inside validate_outcome's owner-gated branch. Re-encoded to UTF-8 U+2014; the file decodes cleanly and every subsequent owner-gated outcome on any function can now validate.

- [s24] src/ings.c was restored to its committed HEAD body at the end of the session (git checkout -- src/ings.c). No build-pipeline, rule, engine or tools file was touched; nothing was committed.
