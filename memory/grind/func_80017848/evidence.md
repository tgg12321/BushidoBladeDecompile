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

## s25 (2026-08-25, escalation/disposition)

- [s25] THE CHASSIS CHANGED UNDER THE 2026-08-18 ESCALATION ENTRY: func_80017848 no longer carries ANY asmfix rule. The 2026-08-19 asm-until-matched migration retired both rules (migration_pin.json: rules_retired 2, pre-migration floor 16) and src/ings.c:590 is now `INCLUDE_ASM("asm/funcs", func_80017848);`. `grep -n func_80017848 asmfix.txt` returns nothing. Re-measured this session with candidate.c's body (candidate.c:325-403) spliced over that INCLUDE_ASM line: score 3, target_insns 127, build_insns 127, scorable true, rules_dropped 0, cheat_asm_stripped 4 (the 4 belong to other functions in ings.c). CONSEQUENCE: there is no debt, no cheat and no integration-handoff attached to this function - it is honestly represented and simply 3 instructions short. Any future session quoting "two asmfix rules hold the byte-match" is quoting a stale fact.

- [s25] THE QUEUE ITEM'S OWNER DIRECTIVE (solver modality before deep re-grind) IS EXECUTED AND DEAD - AND THE SOLVER'S TEXT PATH RETURNS A FALSE VERDICT ON THIS CLASS OF FUNCTION. `bash tools/ra_solver/mkasm_honest.sh ings` then `python3 tools/ra_solver/inverse_compose.py classify ings func_80017848` prints "PATH: text-stream classifier (ings.hon.s vs ings.tgt.s); func_80017848 is not `replace_with_asmfile`-wired" and then "FIRST DIVERGENCE: IDENTICAL - the honest stream already equals target for this function", while the sandbox simultaneously reports 3. The verdict is fiction: with the function not asmfile-wired, ings.tgt.s is built from the SAME spliced C body as ings.hon.s, so the classifier diffs our own output against itself. THE HAZARD IS PIPELINE-WIDE, not local: `classify` must only be trusted for functions that ARE replace_with_asmfile-wired. The model-side verdict stands unchanged from H-s24-C: the residual is two instruction KINDS plus one operand, and no allocation order or schedule converts a `lw` into an `addu`, so both ra_solver and sched_solver are structurally inapplicable here.

- [s25] COMPILER FIDELITY IS ELIMINATED: THE ORIGINAL PsyQ cc1psx PRODUCES THE SAME 3-INSTRUCTION RESIDUAL FROM candidate.c's BODY. Diagnostic only (.claude/rules/no-compiler-divergence.md - cc1psx is calibration/self-disproof, never a build path; no toolchain or Makefile file was touched). Preprocessed src/ings.c with the Makefile's CPP_FLAGS, then compiled the SAME .i twice: tools/gcc-2.7.2/cc1 with the canonical CC_FLAGS, and tools/cc1psx_wrapper.sh (cc1psx.exe, GCC 2.7.2.SN.1, under dosemu2) with the flag subset it accepts. Both emit exactly 111 pre-maspsx instructions for func_80017848 and the streams are identical except for (i) label spelling ($L128 vs .L129) and (ii) ONE adjacent-pair scheduling swap at the math_Distance3D preamble: port emits `sll $16,$20,6 / lw $5,12($18)`, cc1psx emits `lw $5,12($18) / sll $16,$20,6`. Every one of the three residual instructions is byte-identical between the two compilers. CONSEQUENCE (a): the residual is a genuine C-SOURCE-STRUCTURE difference, not a decompals-port artifact - "the toolchain is the variable" is disproven for this function by measurement. CONSEQUENCE (b), project-wide: at the single site where the compilers disagree, TARGET (asm/funcs/func_80017848.s:8174 `sll $s0,$s4,6 / lw $a1,0xC($s2)`) agrees with the PORT and disagrees with cc1psx - independent evidence that decompals/mips-gcc-2.7.2 is the correct calibration for SLUS-00663, and that a cc1psx opt-in would make this function WORSE (>= 5), not better. This is the first measured cc1psx-vs-port codegen divergence recorded in any grind ledger.

- [s25] BOTH ENDGAME-LOCK GATES RE-RUN AND STILL FAIL. Gate (a): `python3 tools/scan_hand_coded.py --single func_80017848` -> HAND_CODED: tier=LOW score=0/8 (127 insns), all eight signals unset, unchanged from s17 and s24. Gate (b): no closing construct exists to cite a precedent FOR; no docs/reference/sotn-construct-index.md hit, no file:line, no commit hash. Disposition filed to docs/grind/decisions.md as the 2026-08-25 OWNER-ESCALATION entry (REFUSED / OWNER-ACCEPTED INCOMPLETE under the 2026-07-27 standing ruling), superseding the now-stale 2026-08-18 entry.

- [s25] TARGET'S TWO LOOP PREHEADERS ARE THE SAME NINE-INSTRUCTION SHAPE, VERIFIED INSTRUCTION BY INSTRUCTION FROM asm/funcs/func_80017848.s. Loop 1 (0x800178B4..0x800178D8) and loop 2 (0x80017914..0x80017938) both read `lw $a0,0xC($s2)` / `sll $a1,$s4,6` / `addu $v0,$a1,$a0` / `lw $v0,<0x1C|0x20>($v0)` / `blez` / `addu $v1,$zero,$zero` (delay) / `addu $a3,$a0,$zero` / `lw $a2,0x10($s2)` / `addu $a0,$a1,$a3`. Loop 1's skip branch `blez $v0,.L8001791C` lands at 0x8001791C, i.e. PAST loop 2's `lw $a0`/`sll $a1` pair, so those two instructions execute only on the loop-1-taken path. This is a re-read/re-shift that the guard branch skips, not a cross-jump merge - and it re-confirms s10/s11's finding that loop 2's guard block is a two-predecessor JOIN. Recorded because the symmetry is the clearest statement of what the residual costs: the candidate buys loop 1's copy with a second use (`p = q`) and pays for it in loop 1's exit tail, whereas target's two copies exist with NO second use anywhere (target re-reads ctx+0xC three more times in the tail).

- [s25] POST-MIGRATION CHASSIS: func_80017848 carries zero asmfix rules and zero cheat-asm of its own; src/ings.c:590 is INCLUDE_ASM("asm/funcs", func_80017848). With candidate.c's body spliced over that line, sandbox --disable all = 3 at 127/127 with rules_dropped 0. The disposition is therefore a pure '3 instructions short in honest C' refusal - not a debt, not a cheat cleanup, and NOT an integration handoff. The 2026-08-18 entry's 'two asmfix rules hold the byte-match' is stale and is superseded by the 2026-08-25 entry.

- [s25] COMPILER FIDELITY ELIMINATED: the original PsyQ cc1psx.exe (GCC 2.7.2.SN.1, dosemu2, diagnostic-only) and tools/gcc-2.7.2/cc1 both emit exactly 111 pre-maspsx instructions for func_80017848 from candidate.c's body, identical except for label spelling and one adjacent-pair scheduling swap at the math_Distance3D preamble. All three residual instructions are byte-identical between the two compilers. The residual is a genuine C-source-structure difference; the pure-C match exists in the C.

- [s25] PROJECT-WIDE CALIBRATION DATUM (first measured cc1psx-vs-port codegen divergence in any grind ledger): at the one site where the compilers disagree, TARGET agrees with the PORT (sll $s0,$s4,6 / lw $a1,0xC($s2)) and disagrees with cc1psx. decompals/mips-gcc-2.7.2 is the correct calibration for SLUS-00663, and a cc1psx opt-in would make this function worse (>= 5).

- [s25] PIPELINE HAZARD: tools/ra_solver/inverse_compose.py classify silently falls back to a text-stream path for any function that is not `replace_with_asmfile`-wired, and in that mode it compares our own compiler output against itself. For func_80017848 it printed 'FIRST DIVERGENCE: IDENTICAL' while the sandbox printed 3. Its verdict must not be trusted outside asmfile-wired functions.

- [s25] ENDGAME-LOCK GATE (a) FAILED: python3 tools/scan_hand_coded.py --single func_80017848 -> HAND_CODED: tier=LOW score=0/8 (127 insns), all eight signals unset (0 multu/mflo pairs; no empty-body branches; 127 insns / 7 spills / 12 distinct regs; max load burst 3 in any 8-insn window; no high-similarity sibling, jaccard < 0.5; no BIOS jumptable pattern; every callee-save use has an $sp save; no redundant mask-before-shift). Unchanged from s17 and s24.

- [s25] ENDGAME-LOCK GATE (b) FAILED: there is no closing construct to cite a precedent FOR. No docs/reference/sotn-construct-index.md hit, no file:line, no commit hash. 172 rejected forms are banked; every identified C-level handle is measured dead.

- [s25] TARGET'S TWO LOOP PREHEADERS ARE THE SAME NINE-INSTRUCTION SHAPE (verified instruction by instruction from asm/funcs/func_80017848.s): loop 1 at 0x800178B4 and loop 2 at 0x80017914 both read lw $a0,0xC($s2) / sll $a1,$s4,6 / addu $v0,$a1,$a0 / lw $v0,<0x1C|0x20>($v0) / blez / addu $v1,$zero,$zero / addu $a3,$a0,$zero / lw $a2,0x10($s2) / addu $a0,$a1,$a3. Loop 1's skip branch lands at 0x8001791C, PAST loop 2's re-read/re-shift pair, so those two instructions run only on the loop-1-taken path - a guard-skipped re-read, not a cross-jump merge, re-confirming that loop 2's guard block is a two-predecessor join. Both of target's copies exist with NO second use anywhere; the candidate buys loop 1's copy with `p = q` and pays for it in loop 1's exit tail. That asymmetry is the remaining 3.

- [s25] DISPOSITION FILED: docs/grind/decisions.md now carries the 2026-08-25 entry '## 2026-08-25 - func_80017848 (src/ings.c) - **OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE** (re-filed on the post-migration chassis)'. It asks for no standard to be lowered: no family sanction, no canonical evidence-bar override, no debt acceptance. The file re-reads as valid UTF-8 after the append (the s24 grindlib.py:388 decode hazard was checked).

## s26 (2026-08-30, structural — executes the owner's 2026-08-30 escalation-batch ruling 10 "ACTIVE with modality change")

### E-s26-0 — chassis re-measured
`sandbox func_80017848 --disable all` with `memory/grind/func_80017848/candidate.c`
spliced over `src/ings.c:590` (`INCLUDE_ASM("asm/funcs", func_80017848);`):
**score 3, target_insns 127, build_insns 127, scorable true, rules_dropped 0,
cheat_asm_stripped 4** (all four belong to other functions in ings.c). Identical
to s25. Every banked spelling conclusion still transfers.

### E-s26-1 — the residual, restated exactly: target's two preheaders are the SAME 10-instruction block
Read directly off `asm/funcs/func_80017848.s`. Loop 1 (lines 31-41) and loop 2
(lines 56-67) are register-identical:

    lw   $a0,0xC($s2)        <- loop 1: at .L800178B4 (block entry)
    sll  $a1,$s4,6              loop 2: on loop 1's TAKEN edge, BEFORE the join label
    [.L8001791C:]            <- loop 2 only: the join (loop-1 guard's blez lands here)
    addu $v0,$a1,$a0            guard address   (0x1C / 0x20)
    lw   $v0,0x1C($v0)
    blez $v0,<skip>
     addu $v1,$zero,$zero       i = 0 in the delay slot
    addu $a3,$a0,$zero       <- THE COPY (both loops, same registers)
    lw   $a2,0x10($s2)          links
    addu $a0,$a1,$a3            base = sh + copy
    addu $v0,$a0,$v1            element address

`$a3` appears exactly 5 times in the whole listing: the prologue's `addu $s3,$a3,$zero`
(param 4) and the two copy/use pairs. So in both loops the copy's dest is used
once, in the same block, and dies there.

On the candidate chassis the 3-point residual is a POSITION SWAP of one `lw` and
one `move` between loop 1's exit tail and loop 2's preheader — the two builds
carry the SAME instruction multiset:

    loop-1 exit tail:  target `lw a0,12(s2)`     ours `addu a0,a3,zero`
    loop-2 preheader:  target `addu a3,a0,zero`  ours `lw v0,12(s2)`
                       target `addu a0,a1,a3`    ours `addu a0,a1,v0`

### E-s26-2 — CELL A: target's join shape reproduced instruction-for-instruction (score 4, 126 insns)
Replace the candidate's `p = q;` loop-1 exit tail with target's own two
statements, in target's order, and delete `sh2` (loop 2's guard AND base then
read the re-computed `sh`):

        } while (i < *(s32 *)(base + 0x1C));
        p = *(u8 **)(ctx + 0xC);
        sh = slot_a << 6;
    }
    i = 0;
    if (i < *(s32 *)(sh + (s32)p + 0x20)) {
        base = (u8 *)(sh + (s32) * (u8 **)(ctx + 0xC));

Result: **score 4, 127 target / 126 build**. The build's join region is byte-exact
against target through the whole guard, including `lw a0,12(s2)` / `sll a1,s4,6`
sitting on loop 1's TAKEN edge before the join label — the first time any chassis
has reproduced that. The ENTIRE residual on chassis A is the two missing copies:

    loop 1 preheader (ours):  blez / move v1,zero / lw a2,16(s2) / addu a0,a1,a0 / addu v0,a0,v1
    loop 2 preheader (ours):  blez / move v1,zero / lw v0,12(s2) / lw a2,16(s2) / addu a0,a1,v0 / addu v0,a0,v1

i.e. loop 1's redundant `q` read is cse-folded to a copy and then DELETED by
combine (dest used in-block — the s16 trichotomy), leaving 126; loop 2's redundant
read is not folded at all (cse's extended basic block begins at the join, so the
load of `p` is outside cse's window) and stays a real `lw`. Chassis A is a strictly
better DESCRIPTION of the wall than s12's symmetric chassis (which also scored 4
but did not reproduce the taken-edge shift placement); it is not a better score.
Banked as `rejected/s26_target_join_shape_reload_tail_costs_4.c`.
Dumps: `tmp/grind/func_80017848/s26/build_A.txt`, `tmp/grind/func_80017848/s26/target.txt`.

### E-s26-3 — the reload/shift ORDER in loop 1's exit tail is load-bearing (cell C = 6)
Chassis A with the two tail statements swapped (`sh = slot_a << 6;` before
`p = *(u8 **)(ctx + 0xC);`) scores **6** (127/126). Target's order is
load-then-shift and nothing else ties it. Banked as
`rejected/s26_join_shape_shift_before_reload_costs_6.c`.

### E-s26-4 — loop 2's preheader naming is inert on chassis A too (cell D = 4)
Mirroring loop 1 exactly — a named `q2 = *(u8 **)(ctx + 0xC);` addend local plus a
named `lnk2 = *(u8 **)(ctx + 0x10);` links local, read in loop-1's order — scores
**4**, identical to chassis A's inline spelling. s11's 47-cell "loop 2's preheader
is C-INERT" result reproduces on the new chassis. Banked as
`rejected/s26_join_shape_named_l2_addend_links_costs_4.c`.

### E-s26-5 — KILL: a dedicated PRE-JOIN carrier for loop 2's addend costs 9 over chassis A (cells E/F = 12)
Rationale probed: cse cannot equate two pseudos across the join, so a variable `r`
that already holds ctx+0xC when loop 2's guard block is entered would let the base
add read a register cse never merges with the guard's add (the s15(4) merge that
prices the reuse-`p` family at 8). Two spellings, `r` assigned at BOTH of `p`'s
assignment sites (the pre-loop-1 read and loop 1's exit tail) so it is live on both
predecessor edges:
  - cell E, `r = p;` (a source-level copy) — **12** (127/127)
  - cell F, `r = *(u8 **)(ctx + 0xC);` (a fresh read) — **12** (127/127)
Both materialise `r` separately on each predecessor path, which is exactly the one
instruction per path target does not spend. **KILLED.** Banked as
`rejected/s26_prejoin_carrier_copy_for_l2_addend_costs_12.c` and
`rejected/s26_prejoin_carrier_fresh_read_for_l2_addend_costs_12.c`.

### E-s26-6 — KILL: an in-BODY second use of loop 2's addend hoists a BASE copy, not an ADDEND copy (cell H = 6)
The s16 trichotomy says a copy survives combine iff its dest's only use is
out-of-block; loop 2's only out-of-block sites after the loop are priced 19-22
(s10/s11, register pressure — target already burns s0-s5). The one out-of-block
site NEVER tried is loop 2's own BODY, which is a different basic block from the
preheader. Cell H gives the named addend `q` a second use in loop 2's do-while
CONDITION (`while (i < *(s32 *)(sh + (s32)q + 0x20))` instead of
`*(s32 *)(base + 0x20)`), on chassis A. Result **6** at the correct 127/127.
The dump shows why:

    lw   v0,12(s2)      <- the addend is STILL a load
    lw   a2,16(s2)
    addu a0,a1,v0       <- base
    move a1,a0          <- the surviving copy: a copy of the BASE, one slot LATE
    addu v0,a0,v1

loop.c's `move_movables` hoists the DERIVED invariant address `sh + q`, not the
addend, so the copy that survives is a copy of the base — reproducing cell C1's
(s15) failure mode on a new chassis. **KILLED**: an in-body second use cannot
produce an addend copy. Banked as
`rejected/s26_l2_addend_second_use_in_body_hoists_base_copy_costs_6.c`.
Dump: `tmp/grind/func_80017848/s26/build_H.txt`.

Consequence, stated once for every future session: loop 2 has NO free out-of-block
use site anywhere. Post-loop sites are 19-22 (s10/s11), the loop body is 6 (E-s26-6),
and a pre-join carrier is 12 (E-s26-5). The second-use lever that buys loop 1's copy
is therefore unbuyable for loop 2 on every chassis measured to date.

### E-s26-7 — the owner's 2026-08-30 escalation-batch ruling 10 is now EXECUTED
The queue item's directive (`docs/grind/decisions.md`, the 2026-08-30
escalation-batch entry, ruling 10) returns func_80017848 to ACTIVE "with modality
change per escalation-not-parked" because its own latest ledger entry (the
2026-08-25 disposition, `docs/grind/decisions.md:12080`) states nothing pends the
owner. This session executed that directive: a structural modality, six new cells,
two kills, no re-measurement of any dead axis, and NO fourth escalation packet —
the owner has already ruled three times on this residual and a packet restating
"both endgame-lock gates fail" carries no decidable question, which
`.claude/rules/escalation-not-parked.md` (owner ruling 2026-08-24, second)
classifies as not-a-packet and directs to be returned as `progress` instead.

- [s26] Chassis re-measured this session: `sandbox func_80017848 --disable all` with memory/grind/func_80017848/candidate.c spliced over src/ings.c:590 = score 3, target_insns 127, build_insns 127, scorable true, rules_dropped 0, cheat_asm_stripped 4 (all four belong to other functions in ings.c). Identical to s25; every banked spelling conclusion still transfers.

- [s26] Target's TWO preheaders are the SAME register-identical 10-instruction block (asm/funcs/func_80017848.s lines 31-41 and 56-67): lw $a0,0xC($s2) / sll $a1,$s4,6 / addu $v0,$a1,$a0 / lw guard(0x1C or 0x20) / blez / addu $v1,$zero,$zero / addu $a3,$a0,$zero / lw $a2,0x10($s2) / addu $a0,$a1,$a3 / addu $v0,$a0,$v1. $a3 appears exactly 5 times in the whole listing (the prologue's param-4 save plus the two copy/use pairs), so in both loops the copy's dest is used once, in the same block, and dies there.

- [s26] On the candidate chassis the 3-point residual is a POSITION SWAP of one `lw` and one `move` between loop 1's exit tail and loop 2's preheader - both builds carry the SAME instruction multiset (target: lw in the tail, move in the preheader; ours: move in the tail, lw in the preheader).

- [s26] Cell A (loop-1 exit tail = `p = *(u8 **)(ctx + 0xC); sh = slot_a << 6;`, sh2 deleted, loop 2's guard and base both reading sh) = 4 at 127/126, with target's join region byte-exact including the taken-edge lw/sll placement. It is a strictly better DESCRIPTION of the wall than s12's symmetric chassis and not a better score. Banked as rejected/s26_target_join_shape_reload_tail_costs_4.c.

- [s26] Cell C (the same two tail statements in the reverse order) = 6 - the load-then-shift order is load-bearing, and nothing else ties it.

- [s26] Cell D (loop 2's preheader mirroring loop 1 exactly, with named q2 addend and lnk2 links locals) = 4 - s11's 47-cell 'loop 2's preheader is C-INERT' result reproduces on chassis A.

- [s26] Cells E and F (a dedicated pre-join carrier `r` for loop 2's addend, assigned on both predecessor edges as a source copy and as a fresh read respectively) = 12 and 12 at 127/127.

- [s26] Cell H (an in-body second use of loop 2's addend, via the do-while condition) = 6 at 127/127; the disassembly shows loop.c hoisting the derived invariant and emitting `move a1,a0` - a copy of the BASE one slot after the base add - while the addend stays a `lw`.

- [s26] CONSEQUENCE for every future session: loop 2 has NO free out-of-block use site anywhere. Post-loop sites are 19-22 (s10/s11, register pressure - target already burns s0-s5 and a seventh callee-save would grow the prologue), the loop body is 6 (this session), and a pre-join carrier is 12 (this session). The second-use lever that buys loop 1's copy is therefore unbuyable for loop 2 on every chassis measured to date.

- [s26] The owner's 2026-08-30 escalation-batch ruling 10 (docs/grind/decisions.md, the 2026-08-30 escalation-batch entry) returns func_80017848 to ACTIVE 'with modality change per escalation-not-parked' on the ground that its own latest ledger entry states nothing pends the owner. This session executed that directive as a structural modality and deliberately did NOT file a fourth escalation packet: three dispositions already exist (docs/grind/decisions.md:5752, :5988, :12080), both endgame-lock gates were re-run and FAILED as recently as s25 (scan_hand_coded tier=LOW 0/8; no SOTN-master file:line precedent), and a packet restating 'both gates fail' carries no decidable question - which .claude/rules/escalation-not-parked.md (owner ruling 2026-08-24, second) classifies as not-a-packet and directs to be returned as `progress` with the kills banked.

- [s26] src/ings.c was restored to its committed INCLUDE_ASM state at the end of the session (git status clean for src/); the only tree changes are the ledger files under memory/grind/func_80017848/.

## s27 (escalation, 2026-09-01) — the owner's Ruling-A named probe, EXECUTED

The 2026-09-01 FORECLOSED-BUCKET REVIEW (docs/grind/decisions.md, Ruling A row for
func_80017848) returned this function to active on exactly two grounds, and named the
probe that had to be run: *"duplicate loop 2's 4-insn GUARD into the loop-1 skip path
on chassis A + candidate chassis; run `.cse`/`.combine` dumps via dump.ps1."*
Both halves ran this session. Both are now measured, and the guard-duplication axis is
CLOSED with numbers rather than inference.

### E-s27-0 — chassis re-measurement (do not quote older floors)
`sandbox func_80017848 --disable all` with `memory/grind/func_80017848/candidate.c`
applied to src/ings.c: **score 3, target_insns 127, build_insns 127, scorable true**.
Chassis A (`tmp/grind/func_80017848/s27/body_A.c`, the s26 body): **score 4, 127/126**.
The floor is unchanged at 3 on the post-migration chassis.

### E-s27-1 — guard duplication is measured DEAD at three spellings (18 / 44 / 47)
The frontier's premise was that loop 2's preheader copy is unreachable because
`cse_end_of_basic_block` terminates the extended basic block at the join label that
loop 1's guard branches to, and that duplicating only loop 2's *guard* (not the whole
loop, priced 35 by s12) might remove the join cheaply enough to pay for itself.
Three spellings were built on the candidate chassis, all with a single shared loop body
(so the duplication is strictly the guard, or the guard plus the 4-insn preheader):

| cell | shape | score | build insns |
|---|---|---|---|
| G1 | loop-2 GUARD duplicated into the loop-1 taken path; `goto` into the shared preheader+body | **18** | 134 |
| G2 | guard AND preheader duplicated into the loop-1 taken path; `goto` into the shared do-while body | **44** | 133 |
| G3 | guard AND preheader duplicated into the loop-1 SKIP path; `goto` into the shared do-while body | **47** | 136 |

Every one of them ADDS instructions (134/133/136 against a 127-insn target) — jump2's
cross-jumping does NOT re-merge the duplicated tails, because after cse the two copies
are no longer identical (one arm's redundant read is folded, the other's is not), which
is precisely the condition `do_cross_jump` requires. The cheapest guard-only spelling
G1 is +7 insns and 6x the floor. **KILLED.** The "remove the cse EBB boundary from C"
lever is dead at every spelling that keeps one shared loop body; whole-loop duplication
was already dead at 35 (s12). Banked as
`rejected/s27_l2_guard_dup_shared_preheader_costs_18.c`,
`rejected/s27_l2_guard_plus_preheader_dup_taken_path_costs_44.c`,
`rejected/s27_l2_guard_plus_preheader_dup_skip_path_costs_47.c`.

### E-s27-2 — the dump CONFIRMS the EBB attribution (it was inference until now)
`pwsh tools/grinder/dump.ps1 -Func func_80017848` with body_A.c applied; the post-cse
RTL for the function is `tmp/grind/func_80017848/dumps/ings.cse` lines 4270-4915. The
two loops' fates are visible side by side in one dump, which is what s26's frontier
item asked for:

    (code_label 61 ...)                                  <- loop-1 guard block STARTS here
    (insn 64  (set (reg 79) (mem (plus (reg 72) (const_int 12)))))   <- the load of ctx+0xC
    (insn 72  (set (reg 85) (plus (reg 84) (reg 79))))               <- guard address
    (insn 75  (set (reg 85) (mem (plus (reg 85) (const_int 28)))))
    (jump_insn 79 ... (label_ref 145))                               <- skip to loop 2's guard
    (insn 86  (set (reg 77) (mem (plus (reg 72) (const_int 16)))))   <- links
    (insn 89  (set (reg 81) (plus (reg 84) (reg 79))))  <- PREHEADER: reg 79 reused, the
                                                           redundant re-read is GONE
    ...
    (insn 141 (set (reg 79) (mem (plus (reg 72) (const_int 12)))))   <- loop-1 exit tail
    (insn 143 (set (reg 84) (ashift (reg 74) (const_int 6))))
    (code_label 145 ...)                                 <- THE JOIN. cse block ends here.
    (insn 151 (set (reg 108) (plus (reg 84) (reg 79))))              <- loop-2 guard address
    (insn 155 (set (reg 110) (mem (plus (reg 108) (const_int 32)))))
    (jump_insn 158 ... (label_ref 218))
    (insn 162 (set (reg 112) (mem (plus (reg 72) (const_int 12)))))  <- PREHEADER: still a
                                                                        REAL LOAD, not folded
    (insn 164 (set (reg 81) (plus (reg 84) (reg 112))))

That is the mechanism, dump-confirmed, no longer inferred: `cse_end_of_basic_block`
(tools/gcc-2.7.2/cse.c:8038, the scan loop `while (p && GET_CODE (p) != CODE_LABEL)`)
ends the block at ANY code label, so the load at insn 141 is outside the window in which
cse processes loop 2's preheader (insn 162), while loop 1's load (insn 64) is inside its
own window and its preheader re-read is substituted away.

### E-s27-3 — the same dump RETIRES the cse account as an explanation of TARGET
The dump proves more than the frontier asked. On chassis A cse does not turn loop 1's
redundant read into a surviving copy either — it SUBSTITUTES it (insn 89 reads reg 79
directly; no copy insn exists at all, which is why chassis A is 126 insns, one short).
A copy only survives when a second use orphans it (the candidate chassis's `p = q`).
More decisively: **the target's own control flow has the very join this frontier wanted
removed.** `asm/funcs/func_80017848.s` shows the loop-1 guard's `blez $v0, .L8001791C`
(0x800178C8) branching to the label that loop 2's guard block starts at, and the loop-1
exit tail's `lw $a0, 0xC($s2)` / `sll $a1, $s4, 6` (0x80017914/18) sitting BEFORE that
label. So in the original compilation the load of ctx+0xC was outside cse's window for
loop 2's preheader exactly as it is for us — **cse cannot have produced target's
`addu $a3, $a0, $zero` at 0x80017930.** The EBB-removal frontier was therefore not only
unaffordable (E-s27-1) but aimed at a mechanism the target itself did not use. Any
future proposal for loop 2's copy must explain a producer that works ACROSS a join
label, and must do so without adding an instruction (target is 127 and the candidate is
already 127).

### E-s27-4 — four corollary spellings of "give loop 2's guard block its own read"
If cse's window is the problem, the obvious cheap alternative to duplicating the guard
is to put the load of ctx+0xC *inside* loop 2's guard block (after the join label), so
the preheader's re-read has an in-block equal to fold against. Measured on chassis A:

| cell | shape | score | build insns |
|---|---|---|---|
| J  | loop-2 guard reads `*(u8 **)(ctx + 0xC)` inline instead of `p` | **14** | 125 |
| J2 | J, plus the now-dead loop-1 exit-tail re-read of ctx+0xC dropped | **14** | 125 |
| L  | J + the read named into a fresh local `r`, second use in the do-while CONDITION | **13** | 126 |
| L2 | J + `r` named, second use post-loop (math_Distance3D's first argument) | **21** | 125 |

All four LOSE instructions rather than gaining the copy: cse folds the in-block pair by
SUBSTITUTION (125/126 insns against 127), reproducing E-s27-3's finding at a second
site. Naming the read into `r` and giving it an out-of-block second use — the exact
shape the s16 trichotomy says preserves a copy — does not help either: L is 13 and L2
is 21, consistent with the banked price table (post-loop second uses 19-22, in-body 6).
**KILLED.** Banked as `rejected/s27_chassisA_l2_guard_reads_ctx0xC_inline_costs_14.c`,
`rejected/s27_chassisA_guard_read_plus_dead_tail_reread_dropped_costs_14.c`,
`rejected/s27_chassisA_named_addend_r_second_use_in_cond_costs_13.c`,
`rejected/s27_chassisA_named_addend_r_second_use_postloop_costs_21.c`.

### E-s27-5 — endgame-lock gates, re-run on the current chassis
Gate (a), `python3 tools/scan_hand_coded.py --single func_80017848`:
`HAND_CODED: tier=LOW score=0/8 (func_80017848, 127 insns)` — all eight signals unlit
(S3 reports 127 insns / 7 spills / 12 distinct regs, i.e. ordinary compiler output).
Gate (b): there is no closing construct to seek a precedent FOR — 27 sessions have
produced no C form at distance 0, so the question "is this construct SOTN-sanctioned"
never arises; the cheapest measured form is the standing candidate at 3. What holds the
byte match is nothing: the function carries zero rules and zero cheat-asm of its own
and is committed as `INCLUDE_ASM("asm/funcs", func_80017848);` (the 2026-08-30 record's
"byte-matches only via a cheat" sentence was retracted by the 2026-09-01 review's
Correction 1). Both gates FAIL.

- [s27] Floor re-measured this session on the post-migration chassis with memory/grind/func_80017848/candidate.c applied to src/ings.c: sandbox --disable all = 3, target_insns 127, build_insns 127, scorable true. Chassis A (body_A.c) = 4 at 127/126.

- [s27] Guard-only duplication with one shared loop body: G1 = 18 / 134 insns. Guard+preheader duplication: G2 = 44 / 133 (taken path), G3 = 47 / 136 (skip path). jump2 does not re-merge the duplicated tails. The last untried cse-boundary lever is closed with measurements.

- [s27] Dump-confirmed pass attribution (tmp/grind/func_80017848/dumps/ings.cse:4270-4915): loop 1's preheader re-read is substituted away against its own block's load (insns 64/89); loop 2's stays a real load (insn 162) because code_label 145 ends cse's block scan (cse_end_of_basic_block, tools/gcc-2.7.2/cse.c:8038). Three sessions of inference are now evidence.

- [s27] NEW, load-bearing: target's own control flow carries that same join (blez at 0x800178C8 -> the label starting loop 2's guard block; the loop-1 exit-tail reload of ctx+0xC at 0x80017914 sits BEFORE it), so cse cannot have produced target's 'addu $a3,$a0,$zero' at 0x80017930. Any future proposal must name a producer that works ACROSS a join label at zero instruction cost - target is 127 and the candidate is already 127.

- [s27] Four corollary spellings that move the load into loop 2's guard block instead of removing the label: J = 14, J2 = 14, L = 13, L2 = 21, at 125/125/126/125 insns. cse substitutes rather than orphaning a copy.

- [s27] Endgame gate (a): python3 tools/scan_hand_coded.py --single func_80017848 -> HAND_CODED: tier=LOW score=0/8 (127 insns); all eight signals unlit (S3: 127 insns, 7 spills, 12 distinct regs). FAIL.

- [s27] Endgame gate (b): FAIL and moot - there is no closing construct to seek a SOTN-master precedent for, because 27 sessions have produced no C form at distance 0. The cheapest measured form is the standing candidate at 3.

- [s27] What holds the byte match: nothing. func_80017848 carries zero regfix/asmfix rules and zero cheat-asm of its own and is committed as INCLUDE_ASM("asm/funcs", func_80017848); - the 2026-08-30 record's 'byte-matches only via a cheat' sentence was retracted by the 2026-09-01 review, Correction 1.

- [s27] 7 new disproven forms banked under memory/grind/func_80017848/rejected/ (185 total). src/ings.c was restored to its committed INCLUDE_ASM state; the working tree carries only ledger + docs/grind edits.

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated with the exhaustion window RESET — the 2026-09-01 Ruling-A unpark was re-foreclosed after one session because the window did not reset. The 09-01 named probe is spent (see ledger); work the ladder from its next rung. All standing banned_constructs remain in force. exhaustion_base=27

## s28 (forensics, 2026-09-03) — the frontier's toolchain reading, DONE: the complete
## GCC 2.7.2 census of reg-reg-copy PRODUCERS, plus the one live candidate measured dead

s27's frontier posed a toolchain question rather than a C probe: *"Enumerate every
2.7.2 pass that can emit a `(set (reg) (reg))` not present in the incoming RTL —
candidates not yet read end to end are reload's inheritance/spill-copy emission
(reload1.c emit_reload_insns) and regmove/optimize_reg_copy_3 — and check each against
the constraint 'must fire in a block whose predecessor is a join'."* That enumeration is
now complete, one of its two named candidates does not exist in this compiler, and the
one genuinely new producer it turned up was built in C and measured.

### E-s28-0 — chassis re-measurement (quote THIS floor)
`sandbox func_80017848 --disable all` with `memory/grind/func_80017848/candidate.c`
applied over the `INCLUDE_ASM` anchor at src/ings.c:719: **score 3, target_insns 127,
build_insns 127, scorable true, rules_dropped 0** (cheat_asm_stripped 2, all belonging
to other functions in ings.c). Floor unchanged at 3.

### E-s28-1 — the COMPLETE census of copy-emitting passes in tools/gcc-2.7.2
Mechanical count of `emit_move_insn` / `gen_move_insn` call sites per pass file:

| file | sites | reachable for this function? |
|---|---|---|
| loop.c | 14 | YES — but every site is a movable/giv/biv emitter (see E-s28-4) |
| jump.c | 7 | 5 are if-conversion/`temp2` paths; the loop-relevant one is `duplicate_loop_exit_test` |
| cse.c | **1** (cse.c:7969, in `cse_set_around_loop`) | gated on REG_LOOP_TEST_P — see E-s28-2 |
| reload1.c | 13 | reload only; requires a spill/inheritance situation |
| flow.c | 1 (flow.c:2239) | auto-inc address rewriting; MIPS has no auto-inc |
| unroll.c | 9 | UNREACHABLE — `-funroll-loops` is not in `CC_FLAGS` (Makefile:35) |
| integrate.c | 2 | inlining only; this function inlines nothing |
| combine.c, local-alloc.c, global.c, reload.c, reorg.c, sched.c, stupid.c, caller-save.c | **0** | cannot create a copy at all |

**Both of the frontier's named candidates are eliminated at the file level:**
`regmove.c` **does not exist in GCC 2.7.2** (`ls tools/gcc-2.7.2/r*.c` returns only
real.c, recog.c, reg-stack.c, regclass.c, reload.c, reload1.c, reorg.c, rtl.c,
rtlanal.c), and `optimize_reg_copy_3` **does not exist** — local-alloc.c declares only
`optimize_reg_copy_1` (local-alloc.c:251 / :700) and `_2` (:252 / :874), and the file
contains zero `emit_move_insn`/`gen_move_insn` call sites. Those routines rewrite and
delete existing copies; they cannot create one. The s27 frontier's "not yet read" list
is therefore closed: only `reload1.c` survives as an unmeasured producer, and it needs a
spill this copy site does not have.

### E-s28-2 — the ONE new producer the census turned up, and its exact source-side gate
`cse_set_around_loop` (cse.c:7909-7975) is cse's only copy emitter. At cse.c:7969 it
does `emit_insn_after (gen_move_insn (src_elt->exp, SET_DEST (set)), p)` where `p` is an
insn **before `loop_start`** — i.e. it plants a reg-reg copy in the loop PREHEADER,
which is exactly target's `addu $a3, $a0, $zero` position, and it does so via a backward
scan that STARTS inside the preheader and is merely bounded by a CODE_LABEL — so unlike
cse's EBB scan it is NOT foreclosed by the join label that s27 proved target itself
carries. It is the first producer found in 28 sessions that satisfies the frontier's
"must work across a join" constraint.

Its gate is a single bit: `REG_LOOP_TEST_P (src_elt->exp)` (cse.c:7933). Across the
whole compiler that bit is SET in exactly one place — `jump.c:2253`, inside
`duplicate_loop_exit_test` (jump.c:2163). And `duplicate_loop_exit_test` is called from
exactly one place, `jump_optimize` at jump.c:626, under the guard (jump.c:620-625):

    if (after_regscan && GET_CODE (insn) == NOTE
        && NOTE_LINE_NUMBER (insn) == NOTE_INSN_LOOP_BEG
        && (temp1 = next_nonnote_insn (insn)) != 0
        && simplejump_p (temp1))

**That is the pass-input enumeration result.** A `NOTE_INSN_LOOP_BEG` immediately
followed by an unconditional jump is the RTL that `while (cond) { ... }` / `for (...)`
expands to (the jump-to-the-bottom-test). The candidate's loops are written PRE-ROTATED
as `if (cond) { do { ... } while (cond); }`, which never emits that simplejump — so on
the candidate chassis `duplicate_loop_exit_test` never fires, `REG_LOOP_TEST_P` is never
set, and cse's only copy emitter is structurally unreachable. The source-side input
shape that changes what the pass sees is *the loop's syntactic form*, nothing else.

### E-s28-3 — the while-form built and measured: producer FIRES, copy does NOT appear
Three cells on the candidate chassis, loops rewritten as `while` with the invariants
(links pointer, base address, bound) read inline in the body so loop.c's LICM feeds the
post-guard preheader:

| cell | shape | score | build insns |
|---|---|---|---|
| W1 | BOTH loops as `while`, invariants inline in the body | **29** | 120 |
| W2 | loop 2 only as `while` (loop 1 keeps the candidate's byte-exact do-while) | **19** | 123 |
| W3 | loop 1 only as `while` (loop 2 keeps the candidate's do-while) | **21** | 123 |

`pwsh tools/grinder/dump.ps1 -Func func_80017848` on W1 confirms the producer's gate
opened: `NOTE_INSN_LOOP_VTOP` — emitted only by `duplicate_loop_exit_test`
("Mark the exit code as the virtual top of the converted loop", jump.c) — is present for
BOTH loops in `tmp/grind/func_80017848/dumps/ings.jump` (notes 319 and 328), and the
duplicated guards carry the freshly-mapped pseudos the pass's `reg_map`/`replace_regs`
creates (loop 1: regs 160/162/163 at insns 312/314/315; loop 2: regs 164/165/167/168 at
insns 320/321/323/324), distinct from the in-loop pseudos 101/99 and 118/115. So the
rotation fired exactly as read.

**But no copy is emitted.** The dumps show `cse_set_around_loop` planting nothing:
loop 1's preheader in `ings.loop` is
`(insn 335 (set (reg 101) (plus (reg 84) (reg 79))))` /
`(insn 336 (set (reg 107) (mem (reg 72 + 16))))` /
`(insn 337 (set (reg 99) (mem (reg 101 + 28))))` — three LICM hoists, zero reg-reg
copies — and loop 2's preheader is the same shape (insns 331/332/333/334). The
while-form also COSTS instructions rather than adding them: LICM hoists the loop BOUND
load out (insn 337 / insn 334), which target does not do — target reloads it every
iteration (`lw $v0, 0x1C($a0)` at 0x80017900) — leaving the build at 120/123 insns
against a 127-insn target. **KILLED at three spellings.** Banked as
`rejected/s28_both_loops_while_licm_preheader_costs_29.c`,
`rejected/s28_loop2_while_licm_preheader_costs_19.c`,
`rejected/s28_loop1_while_licm_preheader_costs_21.c`.

### E-s28-4 — why loop.c's 14 emitters cannot be target's producer either
Read end to end this session. Two families, both excluded by target's own asm:
(a) the movable path, `emit_move_insn (m->set_dest, m->set_src)` at loop.c:1702, only
runs when `m->move_insn` is set, which loop.c:657-670 sets only when the invariant
carries a `REG_EQUIV` note or a CONSTANT `REG_EQUAL` note — the source is then that
note's expression, never an arbitrary pseudo, so it cannot produce `a3 = a0`;
(b) the strength-reduction emitters (loop.c:3940, 3948, 3972, 4065, 5868, 5891, 6098)
all require a giv/biv to have been created. Target's loops are NOT strength-reduced:
the address is recomputed from scratch every iteration as `addu $v0, $a0, $v1`
(0x800178DC in the preheader, 0x80017910 in the bottom delay slot) with the index `$v1`
incremented separately by `addiu $v1, $v1, 0x1` — there is no incremented address
register anywhere in the listing, which is the signature strength reduction would leave.

### E-s28-5 — what this leaves
After E-s28-1..4 the census is closed with exactly ONE producer still unmeasured for
this function: `reload1.c`'s reload/inheritance copy emission (13 sites), which requires
a spill. `scan_hand_coded --single` reports 7 spills in the 127-insn target
(s27, E-s27-5), so the possibility is not zero, but the copy at 0x800178D0 sits between
two values that are both live in hard registers across it and no stack slot is involved.
The honest reading of the whole census is that target's preheader copy is not *created*
by any pass: it is an ordinary expand-time copy from the source — the same thing the
candidate's `p = q` buys for loop 1 — and the 3-point residual remains the price of
buying that copy a second time for loop 2, which the banked price table puts at 6-28
everywhere it has been sited.

- [s28] Floor re-measured: 3 at 127/127 with candidate.c applied. Unchanged.
- [s28] Complete census of GCC 2.7.2 reg-reg-copy producers banked (E-s28-1). regmove.c and optimize_reg_copy_3 DO NOT EXIST in this compiler; local-alloc.c has ZERO move-emitting call sites; unroll.c's 9 sites are unreachable without -funroll-loops (Makefile:35). Only reload1.c remains unmeasured, and it needs a spill this copy site does not have.
- [s28] NEW producer found and its source-side gate identified: cse_set_around_loop (cse.c:7909, emitter at cse.c:7969) plants a reg-reg copy in the PREHEADER and is not foreclosed by a join label. Gate = REG_LOOP_TEST_P, set only at jump.c:2253 in duplicate_loop_exit_test, called only at jump.c:626 when NOTE_INSN_LOOP_BEG is immediately followed by a simplejump — i.e. only for a source-level `while`/`for` loop, never for the candidate's pre-rotated `if + do/while`.
- [s28] The while-form was built and measured: W1 (both loops) = 29 / 120 insns, W2 (loop 2 only) = 19 / 123, W3 (loop 1 only) = 21 / 123. duplicate_loop_exit_test DID fire (NOTE_INSN_LOOP_VTOP at notes 319 and 328 in ings.jump; remapped guard pseudos 160-163 and 164-168), but cse_set_around_loop emitted no copy and LICM hoisted the loop bound out of the loop, which target does not do (target reloads it at 0x80017900). All three cells LOSE instructions against the 127-insn target.
- [s28] loop.c's 14 emitters excluded by reading: the movable path (loop.c:1702) needs REG_EQUIV/constant-REG_EQUAL and so cannot emit a pseudo-to-pseudo copy; the seven giv/biv emitters need strength reduction, and target's loops are demonstrably not strength-reduced (address recomputed as `addu $v0,$a0,$v1` each iteration at 0x800178DC/0x80017910, index incremented separately by `addiu $v1,$v1,1`).
- [s28] src/ings.c restored to its committed INCLUDE_ASM state; tree changes are ledger + rejected forms only.

- [s28] Floor re-measured this session with memory/grind/func_80017848/candidate.c applied over the src/ings.c:719 INCLUDE_ASM anchor: sandbox func_80017848 --disable all = score 3, target_insns 127, build_insns 127, scorable true, rules_dropped 0 (cheat_asm_stripped 2, all belonging to other functions in ings.c). Unchanged from s25/s26/s27.

- [s28] Complete census of reg-reg-copy producers in tools/gcc-2.7.2 by emit_move_insn/gen_move_insn call-site count: loop.c 14, reload1.c 13, unroll.c 9, jump.c 7, integrate.c 2, cse.c 1, flow.c 1; combine.c, local-alloc.c, global.c, reload.c, reorg.c, sched.c, stupid.c and caller-save.c all 0.

- [s28] Both producers the s27 frontier named as unread are eliminated at the file level: regmove.c does not exist in GCC 2.7.2 (ls tools/gcc-2.7.2/r*.c returns only real.c, recog.c, reg-stack.c, regclass.c, reload.c, reload1.c, reorg.c, rtl.c, rtlanal.c), and optimize_reg_copy_3 does not exist - local-alloc.c declares only _1 (:251/:700) and _2 (:252/:874) and contains zero move-emitting call sites.

- [s28] NEW producer identified: cse_set_around_loop (cse.c:7909-7975) emits emit_insn_after (gen_move_insn (src_elt->exp, SET_DEST (set)), p) at cse.c:7969 with p before loop_start - a reg-reg copy planted in the loop PREHEADER, exactly target's addu $a3,$a0,$zero position, via a backward scan that starts inside the preheader and so is NOT foreclosed by the join label s27 proved target carries. It is the first producer in 28 sessions to satisfy the frontier's across-a-join constraint.

- [s28] Its source-side gate is a single bit: REG_LOOP_TEST_P (cse.c:7933), set in exactly one place in the whole compiler - jump.c:2253 inside duplicate_loop_exit_test (jump.c:2163) - which is called from exactly one place, jump.c:626, only when a NOTE_INSN_LOOP_BEG is immediately followed by a simplejump (jump.c:620-625). That is the RTL a source-level while/for loop expands to; the candidate's pre-rotated if + do/while never emits it, so cse's only copy emitter is structurally unreachable on the candidate chassis.

- [s28] While-form cells measured on the candidate chassis: W1 (both loops as while, invariants inline in the body) = 29 at 120 build insns; W2 (loop 2 only) = 19 at 123; W3 (loop 1 only) = 21 at 123. All three LOSE instructions against the 127-insn target.

- [s28] Dump-confirmed that the gate opened and the copy still did not appear: NOTE_INSN_LOOP_VTOP (emitted only by duplicate_loop_exit_test) is present for both loops in tmp/grind/func_80017848/dumps/ings.jump as notes 319 and 328, and the duplicated guards carry the pass's remapped pseudos (loop 1: 160/162/163 at insns 312/314/315; loop 2: 164/165/167/168 at insns 320/321/323/324) distinct from the in-loop pseudos 101/99 and 118/115. In ings.loop both preheaders hold only LICM hoists - loop 1 insns 335/336/337, loop 2 insns 331/332/333/334 - with no reg-reg copy.

- [s28] The while-form's instruction loss is located: LICM hoists the loop BOUND load into the preheader (insn 337 for loop 1, insn 334 for loop 2), which target does not do - target reloads the bound every iteration at 0x80017900 (lw $v0, 0x1C($a0)).

- [s28] loop.c's fourteen emitters excluded by reading: the movable path at loop.c:1702 requires m->move_insn, which loop.c:657-670 sets only for a REG_EQUIV or constant-REG_EQUAL invariant, so its emitted source is the note's expression and never an arbitrary pseudo; the seven giv/biv emitters (3940, 3948, 3972, 4065, 5868, 5891, 6098) require strength reduction, and target's loops are demonstrably not strength-reduced (address recomputed as addu $v0,$a0,$v1 at 0x800178DC and 0x80017910, index incremented separately by addiu $v1,$v1,1 at 0x800178FC).

- [s28] 3 new disproven forms banked under memory/grind/func_80017848/rejected/ (188 total). src/ings.c was restored to its committed INCLUDE_ASM state; the working tree carries only ledger edits under memory/grind/func_80017848/.

## s29 (forensics, 2026-09-03)

- **E-s29-0 CHASSIS.** `sandbox func_80017848 --disable all` with
  `memory/grind/func_80017848/candidate.c` applied over the `src/ings.c:719`
  INCLUDE_ASM anchor = **3**, 127 target insns / 127 build insns, `rules_dropped: 0`.
  The chassis is unchanged from s28. The three residual instructions are still the
  swap-pair: loop-1 exit tail target `lw a0,12(s2)` vs ours `addu a0,a3,zero`;
  loop-2 preheader target `addu a3,a0,zero` vs ours `lw v0,12(s2)`; loop-2 base
  target `addu a0,a1,a3` vs ours `addu a0,a1,v0`.

- **E-s29-1 RELOAD1.C FRONTIER CLOSED BY MEASUREMENT (s28 frontier item 1).**
  reload emitted **zero** insns into this function. The `.lreg` dump region
  (ings.lreg:7203-8162) and the `.greg` dump region (ings.greg:6283-7030) both
  contain exactly **88 `(insn ...)` UIDs, and the two UID sets are identical**
  (`comm -13` over the sorted lists is empty): tmp/grind/func_80017848/s29/
  reload_lreg_uids.txt vs reload_greg_uids.txt. Structurally, reload1.c's 13
  move-emitting sites are only two functions: `gen_reload` (reload1.c:6907, 6935,
  6956, 6957, 6963) and `inc_for_reload` (reload1.c:7116, 7137, 7152, 7154, 7168).
  `inc_for_reload` is auto-increment reload, which MIPS cannot reach. `gen_reload`
  emits a bare reg-reg move only at reload1.c:6963, and only for an operand for
  which `find_reloads` actually pushed a reload — an `addu` whose two inputs are
  already GENERAL_REGS pushes none. Combined with the E-s28-1 census this closes
  the producer question: **no optimisation pass in this compiler creates target's
  preheader copies. They are in the RTL from expand, i.e. they came from the C.**

- **E-s29-2 THE COMBINE GATE, NAMED EXACTLY.** A preheader reg-reg copy `i2 = (set q p)`
  feeding `i3 = (set base (plus sh q))` survives combine on exactly three escapes,
  all in `can_combine_p`/`try_combine`:
  1. `combine.c:1458  added_sets_2 = ! dead_or_set_p (i3, i2dest);` — if `q` is still
     live after i3, combine must keep i2's set, `total_sets` becomes 2, the PARALLEL
     fails `recog` on MIPS and try_combine aborts. This is the "second downstream use"
     lever the candidate already spends on loop 1 (`p = q` in the exit tail).
  2. `combine.c:914-917  (! all_adjacent && ... use_crosses_set_p (src, INSN_CUID (insn)))`
     — if a register **used in i2's SOURCE** is SET by an insn between i2 and i3,
     can_combine_p returns 0 and the copy is never touched. i2's source is the single
     pseudo `p`, so the intervening insn must assign to `p` itself.
  3. `combine.c:928  (INSN_CUID (insn) < last_call_cuid && ! CONSTANT_P (src))` — a
     CALL between i2 and i3. Not reachable in either preheader.
  Additionally `flow.c:2102` only creates the LOG_LINK combine walks when the next use
  `y` satisfies `BLOCK_NUM (y) == blocknum`, so a copy and its consumer in DIFFERENT
  basic blocks are never paired at all — but every zero-cost spelling of that split has
  already been priced (s26 pre-join carriers = 12, s9/s21 q-preinit = 11/12).

- **E-s29-3 REG_N_SETS IS NOT THE GATE (kill re-audit, mandated).** Re-measured the
  closest banked score-4 form,
  `rejected/s16_sym_shared_addend_local_reg_n_sets_2_costs_4.c` (cell A), on the
  CURRENT chassis: **4**, 127 target / **126** build insns. No FAKE construct exists in
  any form of this function, so FAKE-ablation is vacuous. Its diff
  (tmp/grind/func_80017848/s29/A_diff.txt) shows BOTH preheader copies are gone:
  loop 1 collapses to `addu a0,a1,a0` (one insn short of target's copy+base pair) and
  loop 2 emits `lw v0,12(s2)` / `addu a0,a1,v0`. Giving `q` two SETS therefore does not
  stop combine from substituting — consistent with E-s29-2, where nothing in
  can_combine_p or the added_sets_2 test reads `reg_n_sets` of i2dest.

- **E-s29-4 THREE NEW SYMMETRY CELLS, ALL WORSE.** On chassis A: adding a named
  `lnk = *(u8 **)(ctx + 0x10);` inside loop 2's preheader so both preheaders are
  source-symmetric = **12** (cell B, 126 insns); B plus writing loop 1's guard inline
  instead of two-step through `t` = **13** (cell C); B plus writing loop 2's guard
  two-step through `t` like loop 1 = **36** (cell D). Banked as
  rejected/s29_symmetric_lnk_*.

- **E-s29-5 THE RESIDUAL IS NOW A PURE REGISTER-SEAT PROBLEM ON AN EXACT STREAM.**
  Re-measured `rejected/s23_crosses_set_p_reused_as_links_both_loops_exact_insn_stream_costs_14.c`
  (cell E) on the current chassis: **14**, 127/127. Its diff
  (tmp/grind/func_80017848/s29/E_diff.txt, streams in E_T.txt / E_B.txt) contains
  **no opcode, offset or operand-count difference anywhere in the 127 instructions** —
  every one of the 14 is the same instruction with different register seats, and the
  differences are one systematic permutation repeated identically in both loops:
  | value | target seat | our seat |
  |---|---|---|
  | ctx+0xC pointer (`p`) | `$a0` | `$a1` |
  | `slot_a << 6` (`sh`) | `$a1` | `$a0` |
  | preheader copy (`q`) | `$a3` | `$v0` |
  | ctx+0x10 links (`lnk`) | `$a2` | `$a1` (p reused) |
  `tools/ra_solver/inverse_compose.py classify ings func_80017848 --target-object
  build/src/ings.o --ours-object tmp/sandbox/func_80017848/ings.o` prints
  `FIRST DIVERGENCE: RA / next tool: tools/ra_solver/inverse.py (global / local)` and
  lists exactly those four pairs. The func_80045294-s57 mis-typing hazard does NOT
  apply here: its disambiguator is "check whether the divergent register is already
  correct on the pseudo's OTHER references", and here every reference of each pseudo
  carries the WRONG seat consistently, which is a genuine allocation divergence, not a
  cse/canon_reg operand substitution.

- **E-s29-6 WHY THE EXACT-STREAM FORM PAYS 14, MECHANISTICALLY.** Cell E buys the copy
  through escape 2 of E-s29-2: it reuses the variable `p` to hold the links pointer, so
  `lw <lnk>,16(s2)` sets `p` between the copy and the base add and `use_crosses_set_p`
  fires. But that reuse is exactly what displaces the seats: the links value now lives
  in `p`'s pseudo and is read on every iteration of the loop body, so `p`'s live range
  spans the whole loop instead of dying in the preheader, and local-alloc seats it at
  `$a1` while the short-lived shift takes `$a0` — the reverse of target, which keeps
  links in a separate pseudo at `$a2`. The escape and the seats are therefore coupled
  through one source decision, and the open question is whether any input perturbation
  that keeps the crossing set can restore the target seating.

- [s29] Chassis re-measured: sandbox func_80017848 --disable all with candidate.c applied = 3, 127 target / 127 build insns, rules_dropped 0, unchanged from s28.

- [s29] reload emitted ZERO insns into this function: 88 (insn ...) UIDs in the .lreg region (ings.lreg:7203-8162), 88 in the .greg region (ings.greg:6283-7030), identical UID sets (comm -13 empty). Banked as reload_lreg_uids.txt / reload_greg_uids.txt.

- [s29] reload1.c's 13 move sites are only gen_reload (6907/6935/6956/6957/6963) and inc_for_reload (7116/7137/7152/7154/7168); the only bare reg-reg emitter is reload1.c:6963 and it needs find_reloads to have pushed a reload for that operand.

- [s29] The three and only three combine escapes for a preheader copy: combine.c:1458 added_sets_2 = ! dead_or_set_p (i3, i2dest); combine.c:914-917 ! all_adjacent && use_crosses_set_p (src, INSN_CUID (insn)); combine.c:928 INSN_CUID (insn) < last_call_cuid. flow.c:2102 additionally gates LOG_LINK creation on BLOCK_NUM (y) == blocknum.

- [s29] cse.c:8038 re-read confirms the s27 EBB finding verbatim: while (p && GET_CODE (p) != CODE_LABEL) - cse's block scan ends at ANY code label, and the follow-jumps extension at cse.c:8104 requires LABEL_NUSES (JUMP_LABEL (p)) == 1, which target's join label does not satisfy.

- [s29] Kill re-audit (mandated): rejected/s16_sym_shared_addend_local_reg_n_sets_2_costs_4.c re-measures 4 at 127/126 on the current chassis; its diff shows BOTH copies deleted, so REG_N_SETS is not the survival gate. FAKE-ablation is vacuous - no form of this function has ever carried a FAKE construct.

- [s29] New cells on chassis A: B (symmetric named lnk in loop 2's preheader) = 12, C (B + inline loop-1 guard) = 13, D (B + two-step loop-2 guard) = 36. All banked to rejected/.

- [s29] rejected/s23_crosses_set_p_reused_as_links_both_loops_exact_insn_stream_costs_14.c re-measures 14 at 127/127 with ZERO opcode/offset/operand-count differences - the entire residual is four register seats, repeated once per loop: p $a0->$a1, sh $a1->$a0, copy $a3->$v0, lnk $a2->$a1.

- [s29] inverse_compose.py classify on cell E prints FIRST DIVERGENCE: RA / next tool: tools/ra_solver/inverse.py (global / local) and lists exactly those four pairs; the func_80045294-s57 mis-typing hazard is excluded by its own disambiguator (every reference of each pseudo carries the wrong seat, so it is not an operand substitution).

## s30 (rederive, 2026-09-03)

- **E-s30-0 CHASSIS.** `sandbox func_80017848 --disable all` with
  `memory/grind/func_80017848/candidate.c` applied over the `src/ings.c:719`
  INCLUDE_ASM anchor = **3**, 127 target insns / 127 build insns, scorable. The
  dispatch brief reported "measurement unavailable" for the HEAD floor; the ledger
  floor of 3 is re-confirmed by direct measurement this session. Every score below
  is on that same chassis, measured through
  `tmp/grind/func_80017848/s30/cells.ps1` (the s29 harness, repointed at s30).

- **E-s30-1 INTEGRATE.C — THE LAST UNMEASURED COPY PRODUCER — IS CLOSED BY
  MEASUREMENT.** The s28 census (E-s28-1) enumerated every reg-reg-copy producer in
  this compiler by emit_move_insn/gen_move_insn call-site count and left
  `integrate.c` (2 sites) unmeasured because no form of this function had ever used
  an inlined callee. This session built that form three ways and measured all three.
  A `static inline` helper carrying the scan loop is genuinely inlined by GCC 2.7.2
  at `-O2` (verified in the emitted stream: `tmp/grind/func_80017848/s30/H_diff.txt`
  contains no `jal` to the helper), but inlining does not plant a surviving
  preheader copy and it COSTS instructions:
  | cell | shape | score | build insns |
  |---|---|---|---|
  | F | helper takes the already-computed `base` + `lnk`, loop body only | 42 | 137 |
  | G | helper takes `(sh, p, lnk, slot_b)` and computes `base` itself | 45 | 136 |
  | H | helper carries the guard AND the loop, called unconditionally | 30 | 136 |
  All three OVERSHOOT the 127-insn target by 9-10. The overshoot is located in
  `H_diff.txt`: each inlined callee materialises its return value and its inline
  return label, so the target's single `beq $v0,$s3,.L800178AC` (hit -> branch
  straight into the shared `return 0` block) becomes, per helper, a
  `j <inline-return>` + `addiu $v0,$zero,1` pair plus the caller-side test. Nothing
  in jump.c cross-jumps that back together. Cell H's preheader also shows the copy
  is NOT bought: it emits `sll v0,s4,6` / `addu a0,v0,v1` with no `addu a3,a0,zero`
  anywhere, i.e. integrate.c's two move sites produce no surviving arg copy here
  (combine substitutes it exactly as it does for a source-level copy).

- **E-s30-2 STRUCT-TYPED REDERIVATION PRICED (new family, never in the bank).** The
  records at `ctx+0xC` are a 0x40-byte array (`s32 unk0[6]; s32 f18; s32 na; s32 nb;
  u8 la[8]; u8 lb[8]; s32 unk34[3]`) and the links at `ctx+0x10` are a 0x10-byte
  array (`s32 dist; u16 b; s16 a; s32 dist3; s32 owner`); writing the function
  against those types instead of the `u8 *` + literal-offset idiom is a structurally
  different C shape. Measured: A (fresh `(*(IngRec **)(ctx+0xC))[slot_a]` reads at
  every use) = **34** at 123 insns — struct indexing lets cse fold four reads the
  target keeps, so the build LOSES 4 instructions; B (struct types over the
  candidate's proven preheader shape) = **7** at 127; C (B plus a symmetric `q`
  re-read in loop 2) = **14** at 127; D (`u8 *` scaffolding with struct-typed
  `base`/`lnkp` only inside the loop bodies) = **7** at 127. Struct typing is
  therefore a uniform +4 over the equivalent `u8 *` spelling on this chassis and
  never reaches the floor.

- **E-s30-3 SIBLING-IDIOM CENSUS IS NEGATIVE.** No other function in `src/ings.c` or
  `src/ings2.c` contains the `(index << 4) + links + field` link-lookup idiom
  (`grep -n "<< 4)"` returns only this function's two loops and its own tail store).
  The one adjacent matched helper that touches the same geometry is
  `obj_CalcOffset(a0, a1) { return (a0 << 6) + (a1 << 4); }` at src/ings.c:693 — it
  confirms the author's own idiom is explicit shift arithmetic over a byte base
  (which is what the candidate already uses) and it is NOT called from
  func_80017848 (no `jal` in the target listing), so there is no sibling to
  transplant.

- **E-s30-4 DECLARATION BLOCK-SCOPE IS INERT ON BOTH CHASSIS.** Moving `q`, `lnk`
  and `base` out of the function-scope declaration list and into the loop blocks
  they are used in (C89 block-head declarations) changes nothing: cell I (candidate
  shape, block-scoped) = **3** at 127/127, tying the baseline exactly; cell J (the
  s23/s29 exact-instruction-stream form, block-scoped) = **14** at 127/127, tying
  cell E exactly. This extends the s11 "declaration ORDER is inert" finding to
  declaration SCOPE, and it matters for the live frontier: block scope does not
  perturb local-alloc's allocno ordering enough to move the four-seat permutation.

- **E-s30-5 SOURCE ORDER OF THE TWO PREHEADER DEFS IS ALSO INERT ON THE EXACT
  STREAM.** Cell K = cell E with `sh = slot_a << 6;` written BEFORE
  `p = *(u8 **)(ctx + 0xC);` (target loads the pointer first, then shifts; the seat
  divergence is exactly p<->sh) = **14** at 127/127, identical to E. The p/sh seat
  swap is therefore not driven by the order in which the two values are written in
  the source.

- [s30] Chassis re-measured: candidate.c applied = 3, 127/127, scorable. Ledger floor 3 confirmed; the brief's "measurement unavailable" is resolved.
- [s30] integrate.c (the last of the 7 copy-producers from the s28 census to be measured) does not plant a surviving preheader copy: three static-inline-helper spellings score 42/45/30 at 136-137 build insns against a 127-insn target, and the inlined stream carries no `addu a3,a0,zero`.
- [s30] The inline-helper overshoot is mechanical and per-callee: return-value materialisation (`j <inline-return>` + `addiu v0,zero,1`) plus the caller-side test replaces target's single `beq v0,s3,<shared return 0>`; jump.c does not cross-jump it away.
- [s30] Struct-typed rederivation (0x40-byte record array + 0x10-byte link array) priced for the first time: 34 (fresh indexed reads, 123 insns - cse folds 4 reads target keeps), 7, 14, 7. Uniform +4 over the equivalent u8* spelling.
- [s30] Sibling-transplant axis closed: the (index<<4)+links+field idiom appears nowhere else in ings.c/ings2.c; the only adjacent matched helper with the same geometry is obj_CalcOffset (src/ings.c:693) and func_80017848 does not call it.
- [s30] Declaration block-scope is inert: cell I (candidate, block-scoped) = 3 at 127/127; cell J (exact-stream form, block-scoped) = 14 at 127/127. Extends the s11 decl-ORDER-inert finding to decl SCOPE.
- [s30] Writing the shift before the pointer read on the exact-stream chassis (cell K) = 14 at 127/127, identical to cell E: the p<->sh seat swap is not driven by source write order.
- [s30] 10 new forms banked to memory/grind/func_80017848/rejected/ (201 total). src/ings.c restored to its committed INCLUDE_ASM state.

- [s30] Chassis re-measured this session: sandbox func_80017848 --disable all with memory/grind/func_80017848/candidate.c applied over the src/ings.c:719 INCLUDE_ASM anchor = 3, target_insns 127, build_insns 127, scorable. The dispatch brief's 'measurement unavailable' for the HEAD floor is resolved; the ledger floor of 3 stands.

- [s30] integrate.c, the last of the seven reg-reg-copy producers in the s28 census never exercised by this function, is now closed by measurement: three static-inline-helper spellings score 42/45/30 at 136-137 build insns and none of them plants a surviving preheader copy.

- [s30] GCC 2.7.2 at -O2 does inline a `static inline` helper here - tmp/grind/func_80017848/s30/H_diff.txt contains no jal to either helper - so the inline-helper measurements are a real test of integrate.c rather than a failed inline.

- [s30] The inline-helper overshoot is mechanical and per-callee: return-value materialisation (`j <inline-return>` + `addiu $v0,$zero,1`) plus a caller-side test replaces target's single `beq $v0,$s3,.L800178AC` into the shared return-0 block; jump.c does not cross-jump it away. Any future multi-block rederivation that routes the two hit paths through a returned flag pays the same ~4 insns per site.

- [s30] Struct-typed rederivation priced for the first time (new family, not previously in the rejected bank): IngRec is 0x40 bytes (s32 unk0[6]; s32 f18; s32 na; s32 nb; u8 la[8]; u8 lb[8]; s32 unk34[3]) and IngLink is 0x10 (s32 dist; u16 b; s16 a; s32 dist3; s32 owner). Scores 34 (123 insns), 7, 14, 7 - a uniform +4 over the equivalent u8 * spelling.

- [s30] The fully indexed struct form LOSES instructions (123 vs 127): struct indexing lets cse fold four ctx+0xC re-reads that the target keeps, which is the same failure mode as the s28 while-form (LICM hoisting the loop bound the target reloads).

- [s30] Sibling-transplant axis closed: the (index<<4)+links+field idiom exists nowhere else in src/ings.c or src/ings2.c; the only adjacent matched helper with the same geometry is obj_CalcOffset (src/ings.c:693) and this function does not call it.

- [s30] Declaration block-scope is inert on both chassis (cell I = 3 at 127/127, cell J = 14 at 127/127), extending the s11 declaration-ORDER-inert finding to declaration SCOPE.

- [s30] Source write order of the two preheader definitions is inert on the exact-stream chassis (cell K = 14 at 127/127, identical to cell E), so the p<->sh seat swap is not a write-order effect.

- [s30] Taken together, E-s30-4 and E-s30-5 sharpen the standing frontier: the cell-E four-seat permutation does not move for declaration scope or source write order, so the perturbation that reaches target's seats has to come from inside the modelled local-alloc/global-alloc inputs (live-range shape, reference counts, preference edges) rather than from source presentation.

- [s30] 10 new disproven forms banked to memory/grind/func_80017848/rejected/ (201 total). src/ings.c was restored to its committed INCLUDE_ASM state; the working tree carries only ledger edits under memory/grind/func_80017848/.

## s31 (rederive, 2026-09-03) — the RA frontier is closed by form, not by model fidelity

- [s31] CHASSIS. `sandbox func_80017848 --disable all` over the HEAD
  `src/ings.c:719` INCLUDE_ASM anchor: candidate body = **3** at 127 target /
  127 build insns; cell E (the s23/s29 exact-instruction-stream form) = **14** at
  127/127. Both tie the ledger, so the floor-3 chassis is unchanged from s30.
  FAKE-ablation is vacuous for this function (no form has ever carried a FAKE
  construct), so the mandated kill re-audit is the cell-E re-measurement.
  All cell scores: `tmp/grind/func_80017848/s31/scores.txt`.

- [s31] THE RESIDUAL IS EXACTLY THREE INSTRUCTIONS, and they are named for the
  first time in one place (`tmp/grind/func_80017848/s31/dis.sh` diff of the
  candidate against `asm/funcs/func_80017848.s`):
    1. loop-1 exit tail — target `lw a0,12(s2)`,  ours `addu a0,a3,zero`
    2. loop-2 preheader — target `addu a3,a0,zero`, ours `lw v0,12(s2)`
    3. loop-2 base add  — target `addu a0,a1,a3`,  ours `addu a0,a1,v0`
  Everything else in the 127-instruction stream is byte-identical.

- [s31] THE SOLVER SUITE RETURNS A STRUCTURAL NEGATIVE ON CELL E. `extract.py`
  builds a 13-pseudo model (57 dispositions); `simulate.py` reproduces the dump
  **13/13 with sort order MATCH**, so the forward global.c model is exact on this
  chassis. `goal_from_tgt.py goal ings func_80017848 --show` then aligns
  127 vs 127 with 14 `replace` pairs and 0 skips and prints the substitution table
  ($a1->$a0 x6, $a0->$a1 x6, $v0->$a3 x4, $a1->$a2 x4) with per-instruction
  detail — but **derives an EMPTY goal**, because only two pseudos hold $a1
  (78 = loop 1's `p`, 123 = loop 2's `p`) and each is required to be BOTH $a0
  (the copy-source role, `lw a1,12(s2)` / `move v0,a1`) and $a2 (the links role,
  `lw a1,16(s2)` / `addu v0,v0,a1`). Cell E's C reuses the single variable `p`
  for the record pointer AND the link pointer — that reuse IS its
  `use_crosses_set_p` escape — and GCC 2.7.2 creates one pseudo per declaration,
  which receives one hard register. Target seats the two roles separately, so
  cell E's seat map is unsatisfiable by that form: inverse.py has nothing to
  search. Full report: `tmp/grind/func_80017848/s31/goal_E_report.txt`; model:
  `tmp/grind/func_80017848/s31/E_model.json`.
  CONSEQUENCE: any spelling of combine escape 2 that reaches 127 insns here forces
  the p/links pseudo merge, because the only instruction target places between the
  copy and the base add is the links load, so the links load is the only possible
  intervening set of `p`.

- [s31] `use_crosses_set_p` SCANS ONLY BETWEEN i2 AND i3 — i3 setting i2's source
  is not an escape. Measured, not read: cells L/M reuse `p` as the base
  (`q = p; lnk = ...; p = (u8 *)(sh + (s32)q);`) in both loops and come out at
  125 and 126 build insns (scores 12 / 13), i.e. combine substitutes the copy
  away exactly as in the plain spelling. Frontier item 2's sub-probe (i) is dead.

- [s31] A SECOND USE THAT IS A REDUNDANT RECOMPUTATION IS INVISIBLE TO COMBINE.
  Cell P gives loop 2's copy dest `q` a second use in the do-while bottom test
  (`while (i < *(s32 *)(sh2 + (s32)q + 0x20));`) — a site target already spends an
  instruction on, so it should have been free. Result 9 at 126 insns: cse folds
  `sh2 + (s32)q` back to the available base pseudo BEFORE combine runs, `q` is
  back to one reference, and the copy dies. Any future escape-1 site must be a use
  cse cannot fold, not merely a use.

- [s31] SHARING ONE `p`/`sh` PAIR ACROSS BOTH LOOPS IS A WHOLE-FUNCTION SEAT
  ROTATION, NOT A PREHEADER FIX. Cells N1/N2 delete `sh2` and re-assign `p` and
  `sh` in loop 1's exit tail (the literal shape of target's
  `lw a0,12(s2)` / `sll a1,s4,6`): 30 at 126 and 32 at 125. The N1 diff
  (`tmp/grind/func_80017848/s31/N1_diff.txt`) shows the loop counter moving
  `$v1 -> $a0` and the base `$a0 -> $v1` in BOTH loops, which is where most of the
  regression lives.

- [s31] TWO CORRECTIONS TO THE STANDING NARRATIVE, both measured:
  (a) The "cse folds the redundant load into an ORPHANED copy that combine never
      sees" mechanism recorded in `candidate.c`'s s9 header is NOT sufficient on
      its own. N1's loop 1 still writes `q = *(u8 **)(ctx + 0xC);` and still gets
      the cse fold, but without the downstream `p = q;` the copy is DELETED. The
      candidate's loop-1 copy is bought by escape 1 (a live second use), full stop.
  (b) A fresh `*(u8 **)(ctx + 0xC)` read in loop 2's preheader never materialises
      as a cse-folded reg-reg copy — it is always a real `lw`. `.L8001791C` has two
      predecessors and therefore starts a new cse extended basic block in which no
      load of `ctx+0xC` is available, so there is nothing for cse to fold against.

- [s31] TARGET'S COPY IS USE-ONCE, WHICH RULES OUT ESCAPE 1 AS ITS PRODUCER.
  `$a3` is written once per loop (0x800178D0, 0x80017930), read once per loop
  (0x800178D8, 0x80017938) and appears nowhere else in the function except the
  prologue's `addu $s3, $a3, $zero`; loop 1's exit tail reloads the pointer
  (`lw $a0, 0xC($s2)`) instead of copying `$a3`. Escape 1 needs the copy's
  destination live past the base add, and target's is not. So the candidate
  reproduces loop 1's bytes for a reason target cannot be using, and the open
  question for s32 is not "where does loop 2's second use go" (escape 1 prices:
  in-body 6, s31 bottom-test 9, pre-join 12, post-loop 19-22) but "what makes a
  use-once copy survive combine at all". The two `emit_move_insn` producers from
  the s28 census that no session has read end-to-end for this function, and that
  no measured form has exercised, are `jump.c` (7 sites) and `flow.c` (1 site).

- [s31] Chassis re-measured this session over the HEAD src/ings.c:719 INCLUDE_ASM anchor: candidate body = 3 at 127 target / 127 build insns; cell E = 14 at 127/127. Both tie the ledger, so the floor-3 chassis is unchanged from s30. FAKE-ablation is vacuous (no form of this function has ever carried a FAKE construct), so the mandated kill re-audit is the cell-E re-measurement.

- [s31] The residual is exactly three instructions and is now named in one place: (1) loop-1 exit tail, target `lw a0,12(s2)` vs ours `addu a0,a3,zero`; (2) loop-2 preheader, target `addu a3,a0,zero` vs ours `lw v0,12(s2)`; (3) loop-2 base add, target `addu a0,a1,a3` vs ours `addu a0,a1,v0`. The other 124 instructions are byte-identical.

- [s31] tools/ra_solver forward fidelity on this function is exact: extract.py builds a 13-pseudo model with 57 dispositions and simulate.py reproduces the dump 13/13 with sort order MATCH. Any negative from the inverse side is therefore about the form, not the model.

- [s31] goal_from_tgt.py aligns cell E against the target 127 vs 127 with 14 `replace` pairs and 0 skips, prints the substitution table ($a1->$a0 x6, $a0->$a1 x6, $v0->$a3 x4, $a1->$a2 x4) with per-instruction detail, and derives an EMPTY goal: only pseudos 78 (loop 1's p) and 123 (loop 2's p) hold $a1, and each must become both $a0 (copy-source role) and $a2 (links role). Cell E merges the record pointer and the link pointer into one C variable, GCC 2.7.2 gives one pseudo per declaration and one hard register per pseudo, so the target seat map is unsatisfiable by that form.

- [s31] Consequence of the above: any spelling of combine escape 2 reaching 127 instructions in this function forces the p/links pseudo merge, because the only instruction target places between the copy and the base add is the links load, so the links load is the only possible intervening set of p.

- [s31] use_crosses_set_p scans only the insns strictly BETWEEN i2 and i3 - i3 setting i2's source is not an escape. Measured: cells L/M reuse p as the base in both loops and come out at 125 / 126 build insns (scores 12 / 13), with the copy substituted away exactly as in the plain spelling.

- [s31] A second use of the copy destination that is a redundant recomputation of an available expression is invisible to combine, because cse runs first. Cell P routes loop 2's do-while bottom test through q and scores 9 at 126 insns: cse folds sh2 + (s32)q back to the base pseudo, q drops to one reference, and the copy dies.

- [s31] Sharing one p / sh pair across both loops (cells N1/N2, sh2 deleted and both re-assigned in loop 1's exit tail) scores 30 / 32 at 126 / 125 insns; the diff shows the loop counter moving $v1 -> $a0 and the base $a0 -> $v1 in BOTH loops, i.e. a whole-function seat rotation rather than a preheader effect.

- [s31] CORRECTION to candidate.c's s9 header: the 'cse folds the redundant load into an orphaned copy that combine never sees' mechanism is not sufficient on its own. Cell N1's loop 1 still writes q = *(u8 **)(ctx + 0xC); and still gets the fold, but with the downstream p = q; removed the copy is DELETED. The candidate's loop-1 copy is bought by escape 1 (a live second use) and by nothing else.

- [s31] A fresh *(u8 **)(ctx + 0xC) read in loop 2's preheader always materialises as a real `lw`, never a cse-folded reg-reg copy: .L8001791C has two predecessors and therefore starts a new cse extended basic block in which no load of ctx+0xC is available.

- [s31] Target's copy is USE-ONCE: $a3 is written once and read once in each loop and appears nowhere else except the prologue's addu $s3,$a3,$zero, and loop 1's exit tail reloads the pointer (lw $a0,0xC($s2)) instead of copying $a3. Escape 1 needs the copy's destination live past the base add, so target cannot be using escape 1 - the candidate matches loop 1's bytes for the wrong reason.

- [s31] The two emit_move_insn/gen_move_insn producers from the s28 census that no session has read end-to-end for this function, and that no measured form has exercised, are jump.c (7 sites) and flow.c (1 site). loop.c (s28), reload1.c (s29) and integrate.c (s30) are already closed by measurement.

## [s32] STRUCTURAL — the copy-survival question is answered by measurement, and
## the FIRST use-once surviving copy in 32 sessions was produced (two ways)

Chassis: HEAD `src/ings.c:719` INCLUDE_ASM anchor with the cell body pasted in;
`candidate.c` (the s9 V1 body) re-measured **3 at 127/127** at the top of the
session, so the floor is unchanged and every number below is floor-3-relative.
No FAKE constructs in any cell. All cell bodies, the score table and the two
RTL-insn digests are in `tmp/grind/func_80017848/s32/`.

### E-s32-1. PASS ATTRIBUTION, SETTLED: combine deletes the use-once copy.
The s31 frontier's first item asked whether target's preheader copy is emitted
by `jump.c` (7 emit_move_insn sites) or `flow.c` (1) rather than surviving
combine. It is neither, and the question is now closed by a dump rather than by
reading those files. Cell R (`tmp/.../s32/body_R.c`) spells loop 2's preheader
as an explicit `q = p;` with a single downstream use. In
`tmp/grind/func_80017848/dumps/ings.cse2` the copy is present as
`(insn 162 ... (set (reg/v:SI 80) (reg/v:SI 79)))`; in `ings.combine` insn 162
is gone while loop 1's copy (insn 83, the one the candidate buys with the
`p = q` second use) is still there. **Every pass up to and including cse2 keeps
a use-once reg-reg copy; combine is what removes it.** The two digests are
banked as `s32/S_cse2_insns.txt` and `s32/S_combine_insns.txt`.

### E-s32-2. WHY the redundant preheader load usually never becomes a copy.
On the s12 symmetric chassis (re-measured this session at 4, 127/126) loop 1's
preheader `q = *(u8 **)(ctx + 0xC);` does not survive as a copy at all: cse
substitutes the already-loaded pseudo into the base add and `cse_main`'s
closing `delete_trivially_dead_insns` removes the now-dead set, so `ings.cse2`
shows the base add reading `reg79` (p) directly with no copy insn anywhere.
Loop 2's redundant load, by contrast, is NOT folded — cse's table is flushed at
the loop-2 guard's join label (`code_label 143`), so `mem[ctx+12]` is not an
available expression there and the load survives as the `lw v0,12(s2)` that is
one of the candidate's three residual instructions.

### E-s32-3. THE CANONICALISATION RULE (cse.c:826 `make_regs_eqv`).
Which register a folded copy leaves behind is decided by `qty_first_reg`, and
`make_regs_eqv` promotes the NEW register to canonical only when
`(uid_cuid[regno_last_uid[new]] > cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start)`
AND `uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]`
(tools/gcc-2.7.2/cse.c:840-855). This is the C-level handle on "does the base
add read `p` or read the copy's destination": the copy's destination must be a
variable that is already mentioned before the current cse block starts (or
lives past its end) AND whose last mention is later than `p`'s. Measured
directly: in cell S the copy destination `q` (first mentioned back in loop 1)
IS promoted, and `ings.cse2` insn 154 shows the loop-2 GUARD add itself
rewritten to read `reg80` (q) instead of `reg79` (p).

### E-s32-4. ESCAPE #8 — a missing LOG_LINK, i.e. combine never sees the copy.
`flow.c:2102` builds a LOG_LINK from a set to its next use only
`if (y && (BLOCK_NUM (y) == blocknum) ...)`. A copy whose destination's next use
is in a DIFFERENT basic block therefore has no LOG_LINK, combine never tries the
substitution, and the copy survives with a SINGLE use and at ZERO instruction
cost. This is not one of the seven `can_combine_p` refusal paths s17 enumerated
— it is upstream of `can_combine_p` entirely. Demonstrated in cell S: `q = p;`
written in loop 2's GUARD block with its only use (the base add) in the
preheader block leaves `(insn 165 (set (reg/v:SI 81) (reg:SI 109)))` alive in
`ings.combine`. First measured instance of a use-once surviving copy in this
function's 32 sessions.

### E-s32-5. ESCAPE #9 — `use_crosses_set_p`, at zero cost, via variable reuse.
`can_combine_p` refuses when `! all_adjacent && use_crosses_set_p (src, INSN_CUID (insn))`
(combine.c:914 + combine.c:10107): if any register of the copy's SOURCE is set
between the copy and i3, the substitution is rejected. Target's preheader is
already non-adjacent — `lw a2,0x10(s2)` sits between the copy and the base add
— so the only missing ingredient is for that intervening insn to write `p`.
Spelling the links read as `p = *(u8 **)(ctx + 0x10);` (ordinary variable reuse,
zero extra instructions) does exactly that. This is the same construct s23
banked as `crosses_set_p_reused_as_links_both_loops`; s32 re-derived it from the
combine source and re-measured it, and it is the only mechanism that produces
target's instruction stream.

### E-s32-6. THE EXACT-INSTRUCTION-STREAM FORM, RE-MEASURED (cell AK = 14).
`tmp/.../s32/body_AK.c` — both loops on escape #9, loop-1 guard on the `t`
two-step, loop-2 guard inline — builds **127 instructions against target's 127
and is instruction-for-instruction identical to target across the whole
function**, including BOTH `addu a3,a0,zero` preheader copies and BOTH
`lw a0,12(s2)` / `sll a1,s4,6` exit tails. The complete residual is a register
permutation: target `a0`(p)->build `a1`, `a1`(sh)->`a0`, `a3`(q)->`v0`,
`a2`(links)->`a1`. Score 14. This confirms s31's cell-E reading from an
independent construction and re-confirms its verdict: because escape #9 puts the
record pointer and the links pointer in ONE pseudo, target's `a0`/`a2` split is
not an allocation this RTL admits, whatever the allocator does.

### E-s32-7. Escape #8 and target's instruction ORDER are incompatible.
Escape #8 requires the copy's def and its use to be in different basic blocks.
At flow time the loop preheader is one block ending at the loop-top label, so
the only way to separate them is to put the copy in the GUARD block — which
emits it BEFORE the `blez`, where target emits it after. Measured across five
orderings (S, S2, Z1, Z2, Z3): 8-40, never 127 instructions.

- [s32] Floor re-measured at 3 (127 target insns / 127 build insns, scorable) with memory/grind/func_80017848/candidate.c applied to the HEAD INCLUDE_ASM anchor; src/ings.c restored to HEAD at end of session.

- [s32] PASS ATTRIBUTION, dump-proven, closing the s31 frontier's first item: a use-once reg-reg copy in this function is deleted by COMBINE. Cell R's copy is insn 162 in tmp/grind/func_80017848/dumps/ings.cse2 and absent from ings.combine; loop 1's copy (insn 83) survives combine only because the candidate's p = q exit tail gives it a second use. jump.c's seven emit_move_insn sites and flow.c's one are not involved.

- [s32] ESCAPE #8, new and not among the seven can_combine_p refusal paths s17 enumerated: flow.c:2102 builds a LOG_LINK from a set to its next use only if that use is in the same basic block, so a copy whose next use is in a different basic block is invisible to combine and survives with a SINGLE use at zero instruction cost. Measured in cell S: (insn 165 (set (reg/v:SI 81) (reg:SI 109))) is alive in ings.combine.

- [s32] ESCAPE #8 is incompatible with target's instruction ORDER: the copy must sit in the guard block (before the blez) for the LOG_LINK to be missing, while target emits it after. Five orderings measured (S 14, S2 14, Z1 8, Z2 40, Z3 40), all at 124-125 insns.

- [s32] ESCAPE #9 (combine.c:914 use_crosses_set_p, bought free by writing the links read as p = *(u8 **)(ctx + 0x10)) reproduces target's instruction stream EXACTLY: cell AK is 127/127 and instruction-for-instruction identical to asm/funcs/func_80017848.s across the whole function, both preheader copies and both exit tails included. Residual is purely a register permutation (a0 and a1 swapped for p/sh, a3->v0 for q, a2->a1 for links), score 14 - the same four seat divergences s31 recorded for cell E, reached independently.

- [s32] Escape #9's structural cost is the pseudo merge: the record pointer and the links pointer become ONE pseudo, so target's a0/a2 split is not an allocation this RTL admits, which re-confirms s31's empty goal derivation from the C side. Control AC - links kept in their own local, base computed into p - does not trip the refusal and loses both copies (4 at 125).

- [s32] cse.c:826 make_regs_eqv is the C-level handle on WHICH register a folded copy leaves behind: the new register becomes canonical only when (uid_cuid[regno_last_uid[new]] > cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start) AND uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]] (cse.c:840-855). Measured live in cell S, where the promoted q rewrote the loop-2 GUARD add itself (ings.cse2 insn 154 reads reg80, not reg79).

- [s32] Naming / declaration-order / base-destination inertness re-confirmed on the new escape-#9 chassis: AA = AH (q declared before p) = AG (base merged into slots) = AB (p and q roles swapped) = 36, exactly.

- [s32] The s12 symmetric chassis re-measures at 4 (127/126) and its dump shows loop 1's preheader load is ELIMINATED rather than folded to a copy, while loop 2's is a genuine lw because cse's table is flushed at the loop-2 join label - so s12's written account of the wall is superseded.

## [s33] structural sweep - 30 cells over the loop-2 preheader/guard axis

- [s33] Chassis re-audit: BASE (candidate.c) = 3 at 127/127, AK = 14 at 127/127,
  SYM = 4 at 127/126, all reproducing their banked scores. No FAKE construct exists
  anywhere in the tree, so fake_ablate.py has nothing to ablate.
- [s33] E-s33-1: the loop-2 guard two-step (t = sh2 + (s32)p; t = *(s32*)(t+0x20);)
  is BYTE-NEUTRAL when written with a fresh s32 (H4 = 3 at 127/127, residual
  identical to the candidate's three instructions) and costs +33 when it reuses
  loop 1's t (D8 = 36 at 127/127). Variable reuse across the two guards produces the
  same whole-function counter/base seat rotation s31 measured for a shared
  pointer/shift pair. H4 is banked as candidate_alt_h4_t2_twostep_3.c and is a
  second, independent score-3 chassis in which loop 2's guard add is INVALIDATED -
  the precondition for a preheader base add that survives cse.
- [s33] E-s33-2: a plain reg-reg copy in loop 2's preheader is never free. Nine
  spellings (copy source p / slots / shared q; inline guard vs t2 two-step; both exit
  tails) all lose the instruction: 124 insns with the inline guard (the copy dies AND
  the base add folds onto the guard's available sh2+p) and 125-126 with the two-step
  (base add survives, copy still dies). Naming the fresh read into a local q2 is
  exactly inert (G4 = H4 = 3 at 127/127).
- [s33] E-s33-3: p is DEAD after loop 2 in every chassis - the tail re-reads
  *(u8**)(ctx+0xC) for math_Distance3D, rec_a and rec_b - so a post-loop p = q2;
  second use is removed by DCE before combine runs. F4 is byte-identical to F1 (9 at
  124), D4 to D3 (30 at 125), D9 to D7 (30 at 126). The ledger's 19-22 price for
  post-loop second-use sites described sites that WRITE something the tail reads;
  the plain p = q2 site is not merely expensive, it is a no-op.
- [s33] E-s33-4: combine.c:914's use_crosses_set_p escape is NOT tied to clobbering
  p. Any variable that provably holds *(u8**)(ctx+0xC) can be the copy's source and
  then be re-assigned to the links pointer. Using slots (the top-guard record
  pointer) instead of p buys BOTH preheader copies at zero instruction cost for
  score 10 at 127/127 (J2 on the SYM tail, K1 on the BASE tail) against AK/J3's 14.
  Control J1 (same copy, no clobber) = 10 at 126. The residual is a four-link seat
  chain caused by slots staying live from the top guard through loop 1: slots
  ->, loop-1 lnk ->, loop-1 q ->, loop-2 copy ->. The
  escape's price is therefore the seat distance between the merge partner's two
  roles, not a fixed constant - a partner whose first-role register is  would be
  free, and none of the function's variables has that property.
- [s33] E-s33-5: declaration order over the merged variable is exactly inert
  (K3/K4/K5/K6 all 10), re-confirming s11/s12/s32 on this chassis.
- [s33] E-s33-6: OPEN CONTRADICTION. Loop 1's exit tail written as a fresh
  *(u8**)(ctx+0xC) read (H4) and written as p = q; (BASE) CONVERGE to the same
  addu a0,a3,zero, even though cse's extended basic block provably ends at the
  loop-body CODE_LABEL (cse.c:8039) and the exit tail therefore sits in a fresh EBB
  with a cleared memory table. Target has BOTH the preheader copy and a real
  lw a0,0xC(s2) reload; no measured form in 33 sessions has produced both. Fresh -da
  dumps for the H4 chassis are in tmp/grind/func_80017848/dumps/ and are UNREAD.

- [s33] Chassis re-audit reproduces every reference score: BASE (candidate.c) = 3 at 127/127, AK = 14 at 127/127, SYM = 4 at 127/126. No FAKE construct exists anywhere in the tree, so fake_ablate.py has nothing to ablate.

- [s33] The loop-2 guard two-step is BYTE-NEUTRAL with a fresh s32 (H4 = 3 at 127/127, residual identical to the candidate's three instructions) and costs +33 when it reuses loop 1's t (D8 = 36). H4 is a second, independent score-3 chassis in which loop 2's guard add is invalidated - the precondition for a preheader base add that survives cse - and it is banked as memory/grind/func_80017848/candidate_alt_h4_t2_twostep_3.c.

- [s33] A plain reg-reg copy in loop 2's preheader is never free: nine spellings (source p / slots / shared q, inline guard vs t2 two-step, both exit tails) land at 124-126 build insns against a 127-insn target. With the inline guard TWO instructions are lost, because the copy dies in combine and the base add is then folded onto the guard's still-available sh2 + p.

- [s33] Naming loop 2's fresh preheader read into a local (q2) is exactly inert: G4 = BASE = 3 and H4 = 3, both 127/127.

- [s33] p is DEAD after loop 2 on every chassis, so a post-loop p = q2; second use is DCE'd before combine runs - F4 is byte-identical to F1, D4 to D3, D9 to D7. The ledger's 19-22 price for post-loop second-use sites applies only to sites that write a variable the tail actually reads.

- [s33] combine.c:914's use_crosses_set_p escape does NOT require clobbering p. Clobbering slots (the top-guard record pointer) instead buys BOTH preheader copies at zero instruction cost: J2 = 10 and K1 = 10 at 127/127 against AK/J3 = 14, with control J1 (copy, no clobber) at 10/126. The escape's price is the seat distance between the merge partner's two roles, not a constant.

- [s33] K1's residual is a four-link seat chain rooted in slots staying live from the top guard through loop 1: slots $v1->$a2, loop-1 lnk $a2->$a3, loop-1 q $a3->$t0, loop-2 copy $a3->$v0.

- [s33] Declaration order over the merged variable is exactly inert on this chassis (K3/K4/K5/K6 all 10), re-confirming the s11/s12/s32 inertness sweeps.

- [s33] OPEN CONTRADICTION: loop 1's exit tail written as a fresh *(u8 **)(ctx + 0xC) read and written as p = q; converge to the same addu a0,a3,zero, even though cse's EBB provably ends at the loop-body CODE_LABEL (cse.c:8039) and the exit tail sits in a fresh EBB with a cleared memory table. Target carries both the preheader copy and a real lw a0,0xC(s2) reload; no measured form has produced both.

## [s34] SYNTHESIS — the residual is re-localized: every remaining instruction gap
## is ONE combine deletion, and the join-shape chassis proves cse is not the obstacle

Chassis re-audit first (mandated). `sandbox func_80017848 --disable all` over the
HEAD `src/ings.c:719` INCLUDE_ASM anchor with the cell body pasted in:
BASE (`candidate.c`, the s9 V1 body) = **3** at 127 target / 127 build insns;
K1 (the s33 `slots`-merge escape-#9 form, the instance kill whose stream sits
closest to target) = **10** at 127/127; AK = **14** at 127/127. All three tie
their banked numbers exactly, so the floor-3 chassis is unchanged from s33.
No FAKE construct exists anywhere in this function's tree — no form in 34
sessions has ever carried one — so `tools/fake_ablate.py` has nothing to ablate
and the cell re-measurements above ARE the kill re-audit. All s34 cell scores:
`tmp/grind/func_80017848/s34/scores.txt`.

- [s34] TARGET'S TWO LOOPS ARE STRUCTURALLY IDENTICAL, and the ledger's "loop-1
  exit tail" is a misnomer that has shaped 25 sessions of search. Normalising
  `asm/funcs/func_80017848.s` (`tmp/grind/func_80017848/s34/T.txt`) shows both
  loops share one nine-instruction opening:
    `lw a0,0xC(s2)` / `sll a1,s4,6` / `addu v0,a1,a0` / `lw v0,GUARD(v0)` /
    `blez v0,...` / `addu v1,zero,zero` / `addu a3,a0,zero` / `lw a2,0x10(s2)` /
    `addu a0,a1,a3`
  with GUARD = 0x1C for loop 1 and 0x20 for loop 2. The residual instruction the
  ledger calls "loop 1's exit tail" is loop 2's copy of `lw a0,0xC(s2)`, i.e. the
  record-pointer read that precedes loop 2's GUARD — it lives at the JOIN, after
  loop 1's `if` closes, not inside it. Every previously banked chassis puts that
  read (or a `p = q` copy) INSIDE loop 1's if-block, which is why loop 2's
  preheader then had no available `mem[ctx+0xC]` to fold against.

- [s34] THE JOIN-SHAPE CHASSIS (new; cells P0-P4). Moving that read out of the
  `if` to the join makes the C structurally identical to target for both loops:

      p = *(u8 **)(ctx + 0xC); sh = slot_a << 6; i = 0;
      t = sh + (s32)p; t = *(s32 *)(t + 0x1C);
      if (i < t) { q = *(u8 **)(ctx + 0xC); lnk = ...0x10; base = sh + (s32)q; do {} while (); }
      i = 0; sh2 = slot_a << 6;
      p = *(u8 **)(ctx + 0xC);              <- AT THE JOIN, not inside the if
      t2 = sh2 + (s32)p; t2 = *(s32 *)(t2 + 0x20);
      if (i < t2) { q = *(u8 **)(ctx + 0xC); base = sh2 + (s32)q; do {} while (); }

  P0 (distinct q/q2) and P1 (one shared q) both score **6 at 127/125**. The two
  missing instructions are exactly the two preheader copies.

- [s34] E-s34-1, THE CENTRAL MEASUREMENT: on the join chassis BOTH redundant
  preheader reads DO survive cse as reg-reg copies, and BOTH are deleted by
  COMBINE. Fresh `-da` dumps for P1 are in `tmp/grind/func_80017848/dumps/`
  (digests: `s34/P1_cse.txt`, `s34/P1_cse2.txt`, `s34/P1_combine.txt`).
  In `ings.cse` AND `ings.cse2`, loop 1's preheader is

    insn 83 `(set (reg/v:SI 80) (reg/v:SI 79))`      <- q = p, the folded copy
    insn 86 `(set (reg/v:SI 77) (mem (plus (reg 72) (const_int 16))))`
    insn 89 `(set (reg/v:SI 81) (plus (reg/v:SI 84) (reg/v:SI 80)))`  <- base reads q

  i.e. cse.c:826 `make_regs_eqv` DID promote the new register `q` to canonical
  (both s32/E-s32-3 conditions hold on this chassis: `q`'s last mention is in
  loop 2, past the EBB end, and later than `p`'s), so the copy is NOT trivially
  dead and cse keeps it. In `ings.combine` insn 83 is GONE and insn 89 reads
  `reg79` (p) directly, carrying `REG_DEAD (reg79)`. Loop 2 behaves identically.
  CONSEQUENCE: the cse-fold half of the problem is SOLVED by the join shape, and
  the whole remaining gap in this function is one question asked twice —
  "how does a use-once reg-reg copy survive combine at zero instruction cost".
  This supersedes the s31/s32 account in which loop 2's preheader read "can never
  be folded because the EBB is flushed at the join label": the flush is real, but
  the join block's own `lw ctx+0xC` re-establishes `mem[ctx+0xC]` as an available
  expression for the rest of that EBB, and loop 2's preheader is in it.

- [s34] E-s34-2, THE ZERO-COST ESCAPE SET IS EXHAUSTED BY READING can_combine_p
  AGAINST THIS GEOMETRY (`tools/gcc-2.7.2/combine.c:880-928`). Target's preheader
  contains exactly three insns — copy, links load, base add — so:
    * `all_adjacent` is 0 (the links load separates i2 from i3), so the
      `use_crosses_set_p` test at combine.c:914 IS evaluated; it fires only if a
      register of the copy's SOURCE is set strictly between i2 and i3, and the
      links load is the only insn there. Making it set the copy's source means
      the record pointer and the links pointer become ONE pseudo (escape #9).
    * combine.c:902 `succ && ! all_adjacent && reg_used_between_p (dest, succ, i3)`
      requires `succ != 0`, i.e. a THREE-insn combination, which requires the
      base add's other feeder (the shift) to carry an in-block LOG_LINK. Target
      computes the shift once, in the GUARD block (`sll a1,s4,6` feeds both the
      guard add and the base add), so flow.c:2102 builds no link for it.
    * `INSN_CUID (insn) < last_call_cuid` needs a CALL between the copy and the
      base add; this function's only call is after both loops.
    * `find_reg_note (i3, REG_NO_CONFLICT, dest)` needs a multi-word/DImode
      sequence — a banned family.
    * escape #1 (a second use downstream of i3) and escape #8 (a missing
      LOG_LINK from a cross-block use) are already priced dead at 6-22 and
      8-40 respectively (s31, s32/E-s32-7).
  So `use_crosses_set_p` is the only zero-cost escape this geometry admits.

- [s34] E-s34-3, AND ESCAPE #9 CANNOT BE BYTE-EXACT, FOR A REASON THAT IS NOW
  CHASSIS-INDEPENDENT. Firing combine.c:914 forces one C variable — hence one
  GCC 2.7.2 pseudo, hence one hard register — to carry BOTH a record-pointer role
  and a links role. Target seats those roles in DIFFERENT registers everywhere:
  loop 1 `a0`(pointer) / `a2`(links), loop 2 `a0`(pointer) / `a2`(links), top
  guard `v1`(pointer). Measured on the join chassis this session: Q1 (both loops
  merge p+links) = **14 at 127/127**, reproducing AK's number and its four-way
  substitution residual from a different chassis; Q2 (loop 1 merges slots+links,
  loop 2 merges p+links) = **15 at 127/127**. K1's 10 remains the best
  exact-instruction-stream form ever measured. Together with s31's empty goal
  derivation and s32's independent re-derivation, escape #9 now carries three
  independent negatives.

- [s34] E-s34-4: hoisting loop 2's links read into a named local is NOT inert on
  the join chassis, and the sign depends on whether the local is shared. P4
  (fresh `lnk2`) = **4 at 127/125** — the best join-shape cell, one point off the
  floor with two instructions still missing — while P2/P3 (loop 2's links hoisted
  into loop 1's `lnk`) = 12. Sharing `q` across both preheaders is exactly inert
  against distinct `q`/`q2` (P0 = P1 = 6), re-confirming the s11/s12/s32/s33
  naming-and-declaration inertness sweeps on a new chassis.

- [s34] E-s34-5: the combine.c:902 three-insn path was probed and NOT provoked.
  R1 recomputes loop 2's preheader shift as `(slot_a << 5) << 1` to defeat cse's
  available-expression fold; the build is still **125 insns (score 6)**, i.e. the
  two shifts are re-merged before combine and no second in-block feeder exists.
  A real probe of combine.c:902 needs a second feeder that survives to combine,
  and target spends only one `sll` for both the guard add and the base add.

- [s34] THE MERGED READING FOR s35+. Three facts now bracket the answer:
  (1) target's preheader copy is use-once (s31); (2) combine deletes a use-once
  copy on every chassis measured, including the structurally target-identical one
  (E-s34-1); (3) the only zero-cost escape from combine forces a pseudo merge
  target's register assignment does not admit (E-s34-2, E-s34-3). At least one of
  these must have a false premise, and the weakest is (2)'s implicit premise that
  target's copy is a COPY at combine time. The join chassis makes that testable
  for the first time, because it reproduces target's exact block structure: the
  next question is not which C spelling saves the copy, but which pass could emit
  `addu a3,a0,zero` into an already-combined preheader — loop.c's 14
  `emit_move_insn` sites (closed in s28 by score, never by dump) are the only
  producer in the s28/s29/s30 census still unread on a chassis whose preheader is
  block-for-block target's.

- [s34] Chassis re-audit (mandated) reproduces every reference number on the HEAD src/ings.c:719 INCLUDE_ASM anchor: BASE (candidate.c) = 3 at 127/127, K1 (the closest-to-target instance kill) = 10 at 127/127, AK = 14 at 127/127. No FAKE construct exists anywhere in this function's tree, so tools/fake_ablate.py has nothing to ablate and these re-measurements are the kill re-audit.

- [s34] Target's two loops open with one identical nine-instruction sequence - lw a0,0xC(s2) / sll a1,s4,6 / addu v0,a1,a0 / lw v0,GUARD(v0) / blez / addu v1,zero,zero / addu a3,a0,zero / lw a2,0x10(s2) / addu a0,a1,a3 - with GUARD = 0x1C for loop 1 and 0x20 for loop 2 (tmp/grind/func_80017848/s34/T.txt). The ledger's 'loop 1 exit tail' residual instruction is loop 2's copy of the record-pointer read and belongs at the JOIN, outside loop 1's if-block.

- [s34] NEW CHASSIS: the join shape (loop-1 reload moved to the join) is block-for-block target's structure and scores 6 at 127/125 (P0, P1); the two missing instructions are exactly the two preheader copies. Banked as memory/grind/func_80017848/candidate_alt_join_shape_6.c.

- [s34] DUMP-PROVEN: on the join chassis cse KEEPS both preheader copies. ings.cse and ings.cse2 show insn 83 (set (reg/v:SI 80) (reg/v:SI 79)) with insn 89's base add reading reg80 - cse.c:826 make_regs_eqv promoted the copy destination to canonical, so the copy is not trivially dead. ings.combine has insn 83 deleted and insn 89 reading reg79 with REG_DEAD. Combine, not cse, is the whole remaining obstacle, in BOTH loops.

- [s34] This supersedes s31/s32's account that loop 2's preheader read can never be cse-folded because the EBB is flushed at the join label: the flush is real, but the join block's own lw ctx+0xC re-establishes mem[ctx+0xC] as available for the rest of that EBB, which contains loop 2's preheader.

- [s34] can_combine_p read end to end at tools/gcc-2.7.2/combine.c:880-928 against target's three-insn preheader: all_adjacent is 0 so combine.c:914 use_crosses_set_p is evaluated but needs the links load to set the copy's source (the pointer/links pseudo merge); combine.c:902 needs succ != 0 hence a three-insn combination hence a second in-block LOG_LINK on the base add, and target computes its one sll a1,s4,6 in the guard block where flow.c:2102 builds no link; the last_call_cuid path needs a call inside the preheader; the REG_NO_CONFLICT path needs a DImode sequence.

- [s34] Escape #9 measured on the join chassis: Q1 (both loops merge p and links) = 14 at 127/127 with a pure register-permutation residual, Q2 (slots+links then p+links) = 15 at 127/127. K1's 10 stands as the best exact-instruction-stream form in 34 sessions.

- [s34] R1 (loop 2's preheader shift recomputed as (slot_a << 5) << 1) still builds 125 insns at score 6 - the two shifts re-merge before combine, so no second in-block feeder and no three-insn try_combine attempt was created; the combine.c:902 path remains unprobed rather than measured.

- [s34] Inertness re-confirmed on the new chassis: P0 = P1 = 6 exactly (shared vs distinct copy destination). The links carrier is NOT inert: P4 (fresh lnk2) = 4 vs P2/P3 (shared lnk) = 12.

- [s34] Merged reading for s35+: three facts bracket the answer - target's copy is use-once (s31); combine deletes a use-once copy on every chassis measured including the block-identical one (s34); the zero-cost escape from combine forces a pseudo merge target's register assignment does not admit (s34). The weakest premise is that target's copy is a COPY at combine time, and the join chassis makes that testable for the first time on target's own block structure.

- [s35] Chassis re-audit (mandated) on the HEAD src/ings.c:719 INCLUDE_ASM anchor: BASE (candidate.c) = 3 at 127/127, K1 = 10 at 127/127, P4 (the join-shape best cell) = 4 at 127/125. Every banked reference number reproduces. No FAKE construct exists anywhere in this function's tree, so tools/fake_ablate.py has nothing to ablate and these re-measurements ARE the kill re-audit.

- [s35] LEDGER CORRECTION: K1 is NOT instruction-exact, and BASE at 3 - not K1 at 10 - is the form closest to the target instruction stream. K1's first-ever disassembly (tmp/grind/func_80017848/s35/dis.sh) shows seven divergences, not the "four-link seat chain" s33 recorded: slots v1 -> a2 in the top guard; loop 1's copy destination a3 -> t0; loop 1's links load a2 -> a3; the base add following; the JOIN instruction target `lw a0,12(s2)` against ours `addu a0,t0,zero` (an instruction divergence, not a seat); and loop 2's copy `addu v0,a2,zero` where target has `addu a3,a0,zero`, its SOURCE being the top guard's slots value rather than the join read. The only forms that have ever reproduced target's exact 127-instruction stream are AK (s32) and Q1 (s34), both at 14.

- [s35] THE RESIDUAL, RESTATED PRECISELY. BASE (3) emits, at the two positions in question, exactly the two instructions target emits - but SWAPPED. At the join, target has `lw a0,12(s2)` and BASE has `addu a0,a3,zero`; at loop 2's preheader, target has `addu a3,a0,zero` and BASE has `lw v0,12(s2)`; the third divergence is the base add's register following from the second. The cse mechanism is understood in both directions: BASE's `p = q;` sits inside loop 1's if-block, still inside the EBB in which mem[ctx+0xC] is available, so it folds to a copy, while loop 2's preheader read is on the far side of the loop-1 exit CODE_LABEL and cse's memory table has been flushed, so it stays a real load. The join chassis (P-series) moves that read past the label and inverts both: a real `lw` at the join and a folded copy in loop 2's preheader - target's shape exactly - at the cost of both copies then being use-once and deleted by combine (125 insns).

- [s35] FIVE CONSECUTIVE INERTNESS RESULTS ON THE JOIN CHASSIS, all exactly P4's 4 at 127/125: A1 (both loops reuse `p` as the base pointer), A2 (loop 2 only), E1 (copy destination is the already-live guard variable t / t2), E3 (copy destination is `slots`, dead after the top guard), E4 (loop 2's links read placed BEFORE the copy). So the preheader copy is deleted regardless of which C variable carries it, whether that variable has a prior live range, whether the base add reuses the record pointer's own pseudo, and where the links load sits relative to the copy. Target's a0-for-both-roles allocation is not evidence of a shared pseudo: a cell with no copy at all emits `addu a0,a1,a0` anyway, because local-alloc ties the base to the dying pointer.

- [s35] ESCAPE #1 RE-PRICED ON THE JOIN CHASSIS (s34 frontier item 2), and its failure mode is now positional rather than numeric: B1 (loop 2's bottom test written through q) = 6 at 127/126 and B2 (loop 2's body address written through q) = 7 at 127/126. Both DO buy an instruction, which is what s34 predicted, but B1's disassembly shows the bought insn is `addu a1,a1,a0` / `addu a0,a1,zero` - the base computed into one register and then COPIED to another, i.e. a copy AFTER the base add - where target has the copy BEFORE it. Loop 1 of the same cell carries the deleted-copy signature `addu a0,a1,a0`.

- [s35] ESCAPE #9 IS SYMMETRIC AND PER-LOOP: C1 (loop 2 only writes the links read into p) = 12 at 127/126 and C2 (loop 1 only) = 12 at 127/126, identical. Each merge buys exactly one instruction and costs 8 points of seat damage over P4's 4; Q1 (both loops, s34) = 14 at 127/127 is the consistent sum. The a0/a2 split between the record-pointer role and the links role is load-bearing in both loops equally, so no one-sided merge escapes the seat penalty.

- [s35] s34 FRONTIER ITEM 1 CLOSED BY DUMP. With body_P4.c applied, the only reg-reg copies inside func_80017848 in tmp/grind/func_80017848/dumps/ings.loop and ings.cse2 are insns 4/6/8/10 (incoming argument moves), 83 and 162 (the two preheader folds, both `(set (reg/v:SI 80) (reg/v:SI 79))`), 233/235 (math_Distance3D argument setup) and 239 (its return value). loop.c added NO move to either preheader; the copies are cse's folds and both are gone in ings.combine. Note both loops share the same pseudo pair (reg80 <- reg79) on this chassis.

- [s35] THE POST-COMBINE COPY-PRODUCER CENSUS IS NOW CLOSED MORE STRICTLY THAN s28 CLOSED IT. s28 counted emit_move_insn / gen_move_insn call sites only; this session also counted `gen_rtx (SET` plus emit_insn_before / emit_insn_after in every pass that runs after combine. sched.c, local-alloc.c and global.c contain ZERO insn-creating sites of any kind. reorg.c has exactly one `gen_rtx (SET, ...)` site, reorg.c:3660, which builds `dest = dest -/+ other` to invert an increment for a delay slot and cannot produce a plain register copy. reload1.c (5) and caller-save.c (4) need a spill or a call-crossing live range, neither of which this preheader has.

- [s35] PASS ORDER PINNED FROM SOURCE (tools/gcc-2.7.2/toplev.c): cse_main :2865, loop_optimize :2895, cse_main again :2926, flow_analysis :2988, combine_instructions :3004, sched :3033, local_alloc :3052, global_alloc/reload :3080-3082, sched2 :3117. There is exactly one loop pass and it precedes combine, and flow_analysis runs immediately before combine, so LOG_LINKS are rebuilt on the post-loop chain - an insn that loop.c moved or re-emitted cannot survive combine by carrying stale or absent links.

- [s35] combine.c:902 RETIRED BY READING (s34 frontier item 3). combine_instructions calls try_combine (i3, link) for each of i3's LOG_LINKS independently of the three-insn try_combine (i3, link, nextlink); can_combine_p is called with succ = 0 for the i2 slot and succ = i2 for the i1 slot. combine.c:902 is unreachable with succ = 0, and the preheader copy feeds i3 directly so it always occupies the i2 slot in a two-insn attempt. Provoking a second in-block feeder can only refuse the OTHER feeder; it can never protect the copy. s34's R1 negative was therefore not a failed provocation - the path is irrelevant.

- [s35] THE SHARPENED CONTRADICTION FOR s36+. Four premises now bracket the residual, and at least one must be false because the matching C exists: (1) target's preheader copy is use-once (a3 is read only by the base add, asm/funcs/func_80017848.s); (2) combine deletes a use-once reg-reg copy whose single in-block use is the next-but-one insn (dump-proven on the block-identical join chassis in s34, re-confirmed inert against five new spellings here); (3) can_combine_p's refusal set is closed on this geometry and its one zero-cost member forces a pseudo merge target's a0/a2 seating does not admit (now priced symmetrically, C1 = C2 = 12); (4) no pass after combine can create a plain register copy here (dump- and source-closed this session). Premise (2) is the only one still resting on "every form measured so far" rather than a cited predicate, and its untried space is narrow: a second use whose value must be evaluated BEFORE the base add.

- [s35] 9 new disproven forms banked under memory/grind/func_80017848/rejected/ (234 total). P4 banked as memory/grind/func_80017848/candidate_alt_join_p4_4.c. candidate.c unchanged (BASE, 3, still the floor). src/ings.c restored to its committed INCLUDE_ASM state.

- [s35] Chassis re-audit on the HEAD src/ings.c:719 INCLUDE_ASM anchor: BASE (candidate.c) = 3 at 127/127, K1 = 10 at 127/127, P4 = 4 at 127/125. No FAKE construct exists anywhere in this function's tree, so tools/fake_ablate.py has nothing to ablate and these re-measurements are the kill re-audit.

- [s35] LEDGER CORRECTION: K1 is not instruction-exact. Its first disassembly shows seven divergences including the join instruction (target lw a0,12(s2) vs ours addu a0,t0,zero) and a loop-2 copy sourced from the top guard's slots value rather than the join read. BASE at 3 is the closest form; only AK (s32) and Q1 (s34), both 14, have ever built target's exact 127-instruction stream.

- [s35] THE RESIDUAL RESTATED: BASE emits target's two instructions at the two positions in question, but SWAPPED - a copy where target loads and a load where target copies. BASE's p = q sits inside loop 1's if-block where cse still has mem[ctx+0xC] available so it folds to a copy, while loop 2's preheader read is past the loop-1 exit CODE_LABEL where the memory table is flushed so it stays a load. The join chassis inverts both correctly at the cost of both copies becoming use-once and being deleted by combine (125 insns).

- [s35] Five consecutive cells land on exactly P4's 4 at 127/125: A1, A2 (p reused as the base pointer, both loops / loop 2), E1 (copy destination is the live guard variable), E3 (copy destination is slots), E4 (links read before the copy). Carrier identity, carrier prior liveness, base/pointer pseudo identity and links-read order are all byte-neutral on this chassis.

- [s35] Target's a0-for-both-roles allocation is not evidence of a shared pseudo: a cell with no copy at all still emits addu a0,a1,a0, because local-alloc ties the base to the dying pointer.

- [s35] Escape #1 on the join chassis buys an instruction but in the wrong position: B1 = 6 at 127/126 emits addu a1,a1,a0 then addu a0,a1,zero (copy AFTER the base add) where target has addu a3,a0,zero then addu a0,a1,a3 (copy BEFORE it). B2 = 7 at 127/126.

- [s35] Escape #9 is symmetric and per-loop: C1 (loop 2 only) = 12 at 127/126, C2 (loop 1 only) = 12 at 127/126, each buying one instruction for 8 points of seat damage; Q1 (both loops) = 14 at 127/127 is the sum. The a0/a2 split between pointer and links roles is load-bearing in both loops equally.

- [s35] loop.c adds no move to either preheader on the join chassis (dump-proven): the only reg-reg copies in ings.loop / ings.cse2 are the four argument moves, the two preheader folds (insns 83 and 162, both (set (reg/v:SI 80) (reg/v:SI 79))), the two math_Distance3D argument moves and its return-value move; both preheader copies are absent from ings.combine.

- [s35] The post-combine copy-producer census is now closed on gen_rtx (SET, ...) as well as emit_move_insn: sched.c, local-alloc.c and global.c contain zero insn-creating sites; reorg.c's single site (reorg.c:3660) builds dest = dest -/+ other for a delay-slot increment inversion and cannot emit a plain copy; reload1.c and caller-save.c need a spill or a call-crossing live range this preheader does not have.

- [s35] Pass order pinned from tools/gcc-2.7.2/toplev.c: cse_main :2865, loop_optimize :2895, cse_main :2926, flow_analysis :2988, combine_instructions :3004, sched :3033, local_alloc :3052, global_alloc/reload :3080-3082, sched2 :3117. One loop pass, before combine; flow_analysis immediately before combine, so LOG_LINKS are always fresh.

- [s35] combine.c:902 is structurally irrelevant to the copy's survival: it is gated on succ != 0, i.e. the i1 slot of a three-insn combination, while the preheader copy always occupies the i2 slot of a two-insn attempt that is made independently.

- [s35] THE SHARPENED CONTRADICTION for s36+: four premises bracket the residual and one must be false because the matching C exists - (1) target's copy is use-once, (2) combine deletes such a copy on every chassis measured including the block-identical one, (3) can_combine_p's refusal set is closed and its one zero-cost member forces a pseudo merge target's a0/a2 seating does not admit, (4) no post-combine pass can create a plain copy here. Premise (2) is the only one still resting on enumeration rather than a cited predicate, and its untried space is narrow: a second use whose value must be evaluated BEFORE the base add.

- [s35] 9 new disproven forms banked under memory/grind/func_80017848/rejected/ (234 total); P4 banked as memory/grind/func_80017848/candidate_alt_join_p4_4.c; candidate.c unchanged (BASE, 3); src/ings.c restored to its committed INCLUDE_ASM state.

## s36 (2026-09-03, modality `escalation` — disposition session)

Chassis re-audited before any probe: `sandbox func_80017848 --disable all` with
`memory/grind/func_80017848/candidate.c` applied over the HEAD
`src/ings.c:719 INCLUDE_ASM` anchor = **3 at 127 target / 127 build insns,
scorable**. The floor is unchanged for the ninth consecutive session.

- [s36] KILL RE-AUDIT (driver-mandated). The instance kill whose form sits
  closest to the target instruction stream is Q1 (s34: the use_crosses_set_p
  merge on the join chassis, target's complete 127-instruction stream with a
  pure register-permutation residual). Re-measured this session on the current
  chassis: **Q1 = 14 at 127/127**, reproducing its banked number exactly. FAKE
  ablation is vacuous here and was confirmed mechanically:
  `python3 tools/fake_ablate.py --func func_80017848 --file ings --candidate
  memory/grind/func_80017848/candidate.c` prints "no FAKE-annotated constructs
  found ... nothing to ablate". No kill in this ledger was ever measured with a
  FAKE carrier occupying a target pseudo, because the function has never carried
  a FAKE construct in any cell.

- [s36] FRONTIER ITEM 1 (a second use of the preheader copy's destination
  evaluated BEFORE the base add) measured on the BASE (floor-carrier) chassis
  rather than s35's join chassis, which is the combination s35 left untried.
  Four cells, all with a named local `q2` holding loop 2's redundant
  `*(u8 **)(ctx + 0xC)` read:
      D1  bottom test through q2  (`while (i < *(s32*)(sh2 + (s32)q2 + 0x20))`)
          = **5 at 127/128**
      D2  body element address through q2                = **6 at 127/128**
      D3  guard and base sharing one read of q2 (the read hoisted above the
          guard, so the guard's own comparison is the earlier use)
                                                          = **31 at 127/124**
      D4  q2 + a post-loop `p = q2;` second use           = **3 at 127/127**
  D1 and D2 both OVERSHOOT the target instruction count: the second use does not
  merely reposition the copy, it materialises its own address expression as an
  extra instruction, so the build reaches 128 against target's 127 and the
  score rises. D3 collapses to a single pointer pseudo (124 insns, no copy at
  all, +28 points) — the same shape as the s35 A1/A2 merge kill, reconfirmed on
  a second chassis. D4 ties BASE exactly in score AND instruction count, the
  third independent confirmation (s33 F4, s35 E1/E3/E4) that a post-loop second
  use of the copy destination is byte-neutral on this function.

- [s36] GATE (a), canonical-asm evidence: **FAILED**, unchanged from s17.
  `python3 tools/scan_hand_coded.py --single func_80017848` →
  `HAND_CODED: tier=LOW score=0/8 (func_80017848, 127 insns)`, all eight signals
  clear (S1 0 multu/mflo pairs, S2 no empty-body branches, S3 127 insns/7
  spills/12 distinct regs, S4 max load burst 3, S5 no high-similarity siblings,
  S6 no BIOS jumptable pattern, S7 all callee-save uses have $sp saves, S8 no
  redundant mask-before-shift). Not STRONG; not one signal.

- [s36] What holds the byte-match today: nothing but the original assembly.
  `asmfix.txt` and `regfix.txt` no longer exist in the tree (the rules-to-zero
  milestone, 2026-08-25), and `src/ings.c:719` is
  `INCLUDE_ASM("asm/funcs", func_80017848);` per the asm-until-matched ruling.
  There is zero cheat-asm and zero regfix carrying this function; the s17 entry's
  "two asmfix rules" description is superseded.

- [s36] GATE (b), in-hand SOTN-master precedent for the closing construct:
  **FAILED**. `docs/reference/sotn-construct-index.md` (2,746 lines) contains
  zero entries matching preheader / dead-copy / redundant-register-copy
  constructs. There is in any case no closing CONSTRUCT to cite a precedent for:
  the residual is a three-instruction emission-order-plus-seat difference with no
  identified C-level handle, not a coercion or spelling family awaiting sanction.
  The one piece of external prior art the ledger holds (s18's decomp.me scratch
  19TpT) is not SOTN master and was already shown to require a nested-loop
  precondition this function cannot buy.

- [s36] Chassis re-audit: sandbox func_80017848 --disable all with candidate.c applied = 3 at 127 target / 127 build insns, scorable - ninth consecutive session at 3.

- [s36] Exhaustion window counted post-unpark only: state.json exhaustion_base = 27, so sessions 28-35 = 8 consecutive flat sessions across 4 distinct modalities (forensics s28/s29, rederive s30/s31, structural s32/s33, synthesis s34/s35). The 2026-09-02 foreclosure-mechanics ruling's fresh window is satisfied on its own terms; the owner's named probe was spent inside it as progress, not as a disposition.

- [s36] GATE (a) canonical-asm FAILED: tools/scan_hand_coded.py --single func_80017848 returns tier=LOW score=0/8 with all eight signals clear (0 multu/mflo pairs, no empty-body branches, 127 insns / 7 spills / 12 distinct regs, max load burst 3, no high-similarity siblings, no BIOS jumptable pattern, all callee-save uses have $sp saves, no redundant mask-before-shift).

- [s36] GATE (b) SOTN-master precedent FAILED: docs/reference/sotn-construct-index.md (2,746 lines) has zero preheader / dead-copy / redundant-register-copy entries, and there is no closing construct to cite a precedent for - the residual is a three-instruction emission-order-plus-seat difference with no C-level handle. The only external prior art (s18's decomp.me scratch 19TpT) is not SOTN master and needs a nested-loop precondition this function cannot buy.

- [s36] What holds the byte-match today is nothing but the original assembly: asmfix.txt and regfix.txt no longer exist in the tree (rules-to-zero, 2026-08-25) and src/ings.c:719 is INCLUDE_ASM("asm/funcs", func_80017848); per the asm-until-matched ruling. Zero cheat-asm, zero regfix rules - the s17 entry's 'two asmfix rules' description is superseded by the migration.

- [s36] Floor 3 <= ENDGAME_LOCK_MAX_FLOOR (5), so the 2026-07-27 standing ruling is this residual's subject and the RESOLVED BY STANDING RULING title is the correct one (owner ruling 2026-09-02, foreclosure mechanics).

- [s36] Cumulative exhaustion: 35 prior sessions, 237 banked rejected forms, 32 instance kills + 1 class kill (predicate tools/gcc-2.7.2/local-alloc.c:251), five permuter campaigns totalling 180,472 iterations with zero engine-scored improvements on four chassis.

- [s36] src/ings.c was restored to the committed HEAD INCLUDE_ASM anchor after the last measurement; git status shows only ledger / decisions / metrics changes plus the three new rejected/ cells.

## s37 (2026-09-06, modality `solver`; owner directive 2026-09-06 executed first)

- [s37] CHASSIS RE-AUDIT. src/ings.c drifted since s36: `main` was decompiled
  into ings.c and func_80016A8C's prototype changed, moving the
  `INCLUDE_ASM("asm/funcs", func_80017848);` anchor from line 719 to line 820.
  `sandbox func_80017848 --disable all` with the unchanged candidate body
  (tmp/grind/func_80017848/s37/body_BASE.c, identical to candidate.c's body)
  applied over the new anchor = **3 at 127 target / 127 build insns, scorable**.
  The floor and the residual are unchanged by the drift. The residual, from
  tmp/grind/func_80017848/s37/dis.sh (normalised diff T.txt vs B.txt): loop-1
  exit tail target `lw a0,12(s2)` vs build `addu a0,a3,zero`; loop-2 preheader
  target `addu a3,a0,zero ; lw a2,16(s2) ; addu a0,a1,a3` vs build
  `lw v0,12(s2) ; lw a2,16(s2) ; addu a0,a1,v0`. No FAKE construct anywhere.

- [s37] OWNER DIRECTIVE (foreclosed-bucket review 2026-09-06), EXECUTED: the
  loop-1-exit / loop-2-preheader copy geometry was built as minimal scratch
  translation units under the project's exact CC_FLAGS
  (`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin
  -w -mel`, canonical tools/gcc-2.7.2/build/cc1, plus `-da`), files in
  tmp/grind/func_80017848/s37/mini/ (m0..m6 .c, every pass dump, runner
  run.sh, flattener flat.py). Findings, each read from the dumps:
  (1) m5_iso_noloop: `b` first mentioned in an earlier basic block (a loop
      back-edge boundary), then `a = *(ctx+0xC); t = (sa<<6)+a; t = *(t+0x1C);
      if (t<=0) return; b = a; c = *(ctx+0x10); d = (sa<<6)+b;`. The .cse2 dump
      holds EXACTLY the frontier's geometry: insn 71 `(set (reg/v 77) (reg/v 76))`
      (the copy, dest promoted to canonical by cse.c:826 make_regs_eqv), insn 74
      `(set (reg/v 78) (mem ctx+16))`, insn 78 `(set (reg/v 79) (plus (reg 87)
      (reg/v 77)))` reading the COPY DESTINATION. .flow: LOG_LINK 78 -> 71,
      REG_DEAD reg76 on 71, REG_DEAD reg77 on 78. .combine: insn 71 is GONE and
      insn 78 reads reg76 directly. Final .s: `addu $2,$5,$3` with no move.
      So combine deletes the promoted use-once copy on minimal geometry, with
      nothing but the three insns present. The mechanism is try_combine's
      ordinary two-insn path: `added_sets_2 = ! dead_or_set_p (i3, i2dest)`
      (tools/gcc-2.7.2/combine.c:1458) is 0 because flow put REG_DEAD reg77 on
      the add, so i2 is not re-added and is deleted.
  (2) m4_iso_loop: identical geometry with the add feeding a do/while loop
      (target's shape). Same outcome: insn 70 (the copy) present in .cse2 with
      the add reading the copy dest; absent in .combine; final .s preheader is
      `lw $4,16($4) ; addu $5,$5,$7` with no move.
  (3) m6_iso_notpromoted (control, `b` has no earlier mention): cse2 already has
      the add reading `a` (a stays canonical) and no copy insn survives to
      .cse2 at all. This is the ordinary non-promoted case.
  (4) m2/m3 (promoted copy but WITHOUT the `t` reassignment that invalidates
      the guard add's pseudo): cse2 folds the preheader add away entirely,
      insn 79 `(set (reg 90) (mem (plus (reg 86) 36)))` reads the GUARD's add
      pseudo reg 86 directly, and the copy dies at flow. This is a dump-level
      proof that target's preheader add can only coexist with a cse-produced
      copy of the guard's load when the guard add's own pseudo has been
      invalidated before the preheader (candidate.c's `t = sh + p; t =
      *(s32*)(t+0x1C)` reassignment is load-bearing, not cosmetic).
  (5) m0_twoloop (the naive two-loop spelling alone): the preheader comes out
      as `move $10,$2 ; lw $9,16($4) ; move $8,$3 ; addu $2,$10,$7` and the loop
      bottom tests `slt $2,$7,$8` against a HOISTED bound register. Target
      reloads the bound (`lw v0,28(a0)`) every iteration and copies the LOAD,
      not the guard's add. The naive shape is therefore two invariants further
      from target than the candidate's hand-hoisted chassis, which is why the
      ledger's chassis exists.
  CONCLUSION for the directive: premise (2) is CONFIRMED outside this function
  and frontier item 3 is CLOSED. There is NO protecting context in the 15-local
  chassis: combine's deletion of a promoted use-once reg-reg copy whose
  destination carries a REG_DEAD note on the consuming add is decided locally
  by combine.c:1458 and happens identically in a 3-insn scratch TU. The
  "protecting context" the frontier hoped to name does not exist; the only
  ways a copy of that shape reaches the assembler remain the three the ledger
  already priced: i2dest not dead in i3 (a second use, escape #1, measured at
  +1 instruction or byte-neutral), no LOG_LINK (a basic-block boundary between
  copy and add, escape #8, incompatible with target's instruction order per
  E-s32-7), and use_crosses_set_p (escape #9, Q1 = 14 from the seat merge).

- [s37] SOLVER MODALITY, rule (1) executed: `inverse_compose.py classify ings
  func_80017848 --target-object build/src/ings.o --ours-object
  tmp/sandbox/func_80017848/ings.o` on the BASE chassis (report:
  tmp/grind/func_80017848/s37/classify_BASE.txt) reports `FIRST DIVERGENCE:
  RA, same instructions, different registers` and names inverse.py as the next
  tool. That verdict is a mis-triage of THIS residual and must not be spent:
  the classifier matched opcode multisets (`addu a0,a1,v0 / lw v0,12(s2) / move
  a0,a3` against `addu a0,a1,a3 / lw a0,12(s2) / move a3,a0`), but the diff
  in dis.sh shows the instructions sit at DIFFERENT positions: BASE has the
  `move` in loop 1's exit tail and the `lw` in loop 2's preheader, target has
  the `lw` in the exit tail and the `move` in the preheader. No register
  renaming maps a `move` to a `lw`, so the residual is positional (which pass
  emits which instruction where), not a seat permutation; s31's empty RA goal
  on the exact-stream form said the same thing from the model side. inverse.py
  was therefore not run - a goal derived from this classify would be the
  func_80041188-s2 subset-goal failure mode.

- [s37] src/ings.c restored to tmp/grind/func_80017848/s37/ings.orig.c (HEAD)
  after the last measurement; git status shows only ledger / metrics changes.

- [s37] Chassis drift: src/ings.c anchor INCLUDE_ASM("asm/funcs", func_80017848); now at line 820 (was 719); BASE unchanged = 3 at 127/127 scorable.

- [s37] Residual (s37/dis.sh): loop-1 exit tail target lw a0,12(s2) vs build addu a0,a3,zero; loop-2 preheader target addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3 vs build lw v0,12(s2) / lw a2,16(s2) / addu a0,a1,v0.

- [s37] Owner directive executed: seven scratch TUs (m0..m6) under exact CC_FLAGS with -da in tmp/grind/func_80017848/s37/mini/; m4/m5 reproduce the frontier's three-insn geometry verbatim at .cse2 and combine deletes the copy in isolation (combine.c:1458); no protecting context exists.

- [s37] m0 (naive two-loop spelling alone) hoists the loop bound into a register and copies the guard's add (move $10,$2 / lw $9,16($4) / move $8,$3), two invariants further from target than the candidate's hand-hoisted chassis.

- [s37] Solver classify (object path) reports FIRST DIVERGENCE: RA on BASE, but the differing instructions are positionally swapped (move vs lw between exit tail and preheader), so the RA verdict is a mis-triage and no inverse.py goal exists; consistent with s31.

- [s37] src/ings.c restored to HEAD after the last measurement; no FAKE constructs anywhere; fake_ablate remains vacuous (s36).

## s38 (2026-09-06, modality `forensics`; owner directive 2026-09-06 acknowledged)

- [s38] CHASSIS RE-AUDIT. HEAD src/ings.c carries the anchor
  `INCLUDE_ASM("asm/funcs", func_80017848);` at line 820 (unchanged since
  s37). The candidate body (tmp/grind/func_80017848/s38/body_BASE.c, byte-
  identical to candidate.c's function body) applied over the anchor scores
  `sandbox func_80017848 --disable all` = **3 at 127 target / 127 build,
  scorable**. Floor 3 stands. No FAKE construct anywhere; fake_ablate remains
  vacuous.

- [s38] KILL RE-AUDIT (mandated: closest instance kill re-measured on the
  current chassis). Q1 (s34, tmp/grind/func_80017848/s38/body_Q1.c, escape #9
  fired in both loops by writing the links read into `p`) = **14 at 127/127**
  on the current chassis, identical to s34 and s35. The kill is not stale:
  the instruction stream is target's and the 14 points are register seats.

- [s38] OWNER DIRECTIVE (2026-09-06 foreclosed-bucket review) - explicit
  acknowledgment for the driver's consistency audit: the directive was
  EXECUTED in s37 (scratch TUs m0..m6 under exact CC_FLAGS, .cse2/.combine
  read, answer: no protecting context exists; combine.c:1458 deletes the
  promoted use-once copy on three-insn geometry) and is EXTENDED this session
  by m7a/m7b (below). Nothing in the directive remains unexecuted.

- [s38] FRONTIER ITEM 1 (escape #8: a basic-block boundary between the
  preheader copy and the base add whose label vanishes after combine),
  measured on minimal geometry under the exact project flags
  (tmp/grind/func_80017848/s38/mini/, runner run.sh, all -da dumps kept):
  (a) m7a_fwd_branch: s37's m5 geometry with a REAL forward conditional
      (`if (sb < 0) return -1;`) between `b = a` and the base add. .cse2:
      copy insn 71 `(set (reg/v 77) (reg/v 76))`, jump_insn 74, links load 83,
      add 87 `(plus (reg 87) (reg/v 77))` reading the copy destination.
      .flow: insn 87 has NO LOG_LINK (insn_list nil) because flow.c:2084
      only links a set to a use with the same BLOCK_NUM; REG_DEAD 77 sits on
      87 anyway. .combine: insn 71 untouched (combine never tries it). Final
      .s: `bltz $6,.L8 ; move $2,$3` - the copy is emitted in the branch's
      delay slot, i.e. in the PREDECESSOR block, and the add `addu $2,$5,$2`
      reads the copy destination. So escape #8 is real at the pass level,
      but the copy lands before/inside the branch. Target's guard is
      `lw v0,28(v0) ; nop ; blez v0 ; addu v1,zero,zero ; addu a3,a0,zero` -
      the load-delay nop is unfilled and the copy follows the delay slot, so
      no branch can sit between target's copy and add. Confirms E-s32-7 on
      minimal geometry.
  (b) m7b_loop_hoist: copy `b = a` in the preheader, links load and base add
      written INSIDE the do/while body (loop-top label between copy and add
      at RTL generation). .loop: loop.c hoists them as insns 144/145/146
      immediately after the copy (insn 70) and before NOTE_INSN_LOOP_BEG -
      the same basic block. .flow: insn 146 carries `(insn_list 70)` and
      REG_DEAD 77. .combine: the copy is gone. Final .s preheader:
      `lw $4,16($4) ; addu $5,$5,$7` with no move. A back-edge label cannot
      separate them because every invariant is hoisted into the copy's block.
  (c) Post-combine label deleters, enumerated from toplev.c pass order
      (combine 3004 -> sched1 3033 -> local_alloc/global/reload 3082 ->
      sched2 3117 -> jump2 3142 `jump_optimize (insns, 1, 1, 0)` -> reorg
      3167): jump2 deletes unreferenced labels at jump.c:270-276, deletes
      no-op moves at jump.c:437-455 (delete_computation at :449), and
      cross-jumps at jump.c:1950-2061 (which CREATES labels, the wrong
      direction). The pre-flow jump pass at toplev.c:2929 runs the same
      jump_optimize with cross_jump=0, noop_moves=0, so a jump that is
      deletable at jump2 but not before flow can only be one whose
      jumped-over block became no-op moves after reload. In C that block is
      a conditional whose whole body is a register self-copy - the
      dead-conditional-store family (forbidden). A REAL conditional keeps
      its branch bytes (m7a). Frontier item 1 is therefore closed.

- [s38] THE s31 MOVE-PRODUCER CENSUS LEFTOVER IS CLOSED BY READING. s31 left
  jump.c (7 emit/gen_move_insn sites) and flow.c (1 site) unread. Read this
  session: jump.c:907 and :951 and :1008 are the "if (...) x = exp" hoists
  gated on `! reload_completed && BRANCH_COST >= 3` (resp. >= 4); mips.h:2937
  defines BRANCH_COST as 1 for every cpu except R4000/R6000 (2), so under
  -mcpu=3000 they never fire. jump.c:1144/1146 are the HAVE_conditional_move
  path (absent on MIPS I). jump.c:1313 and :1429 are the store-flag
  conversions of `if (c) x = a; else x = b;` into compare-result arithmetic,
  run in jump1 BEFORE combine, so any move they emit is subject to the same
  deletion. flow.c:2239 is inside `#ifdef AUTO_INC_DEC`, and rtl.h:658
  defines AUTO_INC_DEC only when HAVE_PRE/POST_INCREMENT is defined;
  mips.h:2175/2179 leave both commented out. No post-combine producer of a
  plain `(set (reg) (reg))` exists in this cc1 for this target.

- [s38] can_combine_p REFUSAL ENUMERATION for i2 = `(set q p)` into i3 =
  `(set base (plus sh q))` with pseudos, both in one block (combine.c
  882-1030): stack-pointer / field-assignment / self-set-with-REG_EQUAL /
  CALL src / call-argument / REG_INC / REG_RETVAL (n/a); `succ &&
  reg_used_between_p` (:903, three-insn only); `! all_adjacent &&
  use_crosses_set_p (src)` (:914-917, escape #9, priced Q1 = 14);
  REG_NO_CONFLICT on i3 (:920-922, DImode only - s17 R6); `INSN_CUID (insn) <
  last_call_cuid && ! CONSTANT_P (src)` (:929, a CALL between copy and add -
  none in target); hard-register dest/src (:943-956, n/a);
  `volatile_insn_p` on an insn strictly between i2 and i3 (:985-988). The
  last one was checked because target's links load sits exactly between the
  copy and the add: rtlanal.c:1366 volatile_insn_p returns 0 for `case MEM`
  and 1 only for UNSPEC_VOLATILE / volatile ASM_OPERANDS, so a volatile-
  qualified C load of the links pointer would NOT save the copy; only an
  `asm volatile` would, which is a forbidden family (s17 R5 already said so
  on policy grounds; this is the toolchain-side confirmation that C volatile
  is inert here). With LOG_LINK absence (m7a/m7b) and dead_or_set_p (s37
  m4/m5) also measured, every refusal clause is now either n/a to this
  geometry, priced, or forbidden.

- [s38] src/ings.c restored to tmp/grind/func_80017848/s38/ings.orig.c (HEAD)
  after the Q1 measurement; `git diff --quiet -- src/ings.c` passes.

- [s38] Chassis: anchor src/ings.c:820 unchanged; BASE = 3 at 127/127 scorable; Q1 = 14 at 127/127 on the same chassis (kill re-audit passes).

- [s38] Owner directive 2026-09-06 acknowledged and executed (s37 m0-m6, extended by s38 m7a/m7b); no protecting context exists for the promoted use-once copy.

- [s38] Frontier item 1 (vanishing block boundary, escape #8) closed: a forward branch keeps the copy but emits it in the predecessor block (m7a), a back-edge is undone by loop.c hoisting into the copy's block (m7b), and the only post-combine zero-residue label deletion is jump2's no-op-move path (jump.c:449), whose C spelling is a dead conditional self-copy.

- [s38] s31's unread move producers (jump.c x7, flow.c x1) are all gated off for this target (BRANCH_COST 1, no conditional move, no AUTO_INC_DEC) or run before combine.

- [s38] rtlanal.c:1366 volatile_insn_p ignores volatile MEMs, so combine.c:985 cannot be reached from C volatile on the links load; only asm volatile reaches it (forbidden).

- [s38] Chassis: src/ings.c anchor INCLUDE_ASM("asm/funcs", func_80017848); at line 820 (unchanged since s37); BASE = 3 at 127/127 scorable; Q1 = 14 at 127/127 on the same chassis.

- [s38] Owner directive 2026-09-06 acknowledged: executed in s37 (m0-m6) and extended in s38 (m7a/m7b); combine.c:1458 deletes the promoted use-once copy with no protecting context.

- [s38] Frontier item 1 (escape #8 with a vanishing label) closed: m7a keeps the copy but emits it in the branch delay slot before the branch; m7b's back-edge is undone by loop.c hoisting into the copy's block; the only post-combine zero-residue label deletion is jump2's no-op-move path (jump.c:449), whose C form is a dead conditional self-copy.

- [s38] can_combine_p refusal clauses fully enumerated for this geometry (combine.c:882-1030): :903 succ-only, :914 use_crosses_set_p (Q1 = 14), :920 REG_NO_CONFLICT (DImode), :929 call-crossing (no call in target between copy and add), :985 volatile_insn_p (asm/UNSPEC only per rtlanal.c:1366), plus LOG_LINK absence (m7a/m7b) and dead_or_set_p (s37 m4/m5).

- [s38] s31's unread move producers (jump.c x7, flow.c x1) are gated off for this target: BRANCH_COST 1 (mips.h:2937), no HAVE_conditional_move, no AUTO_INC_DEC (rtl.h:658, mips.h:2175/2179), or run before combine.

- [s38] src/ings.c restored to HEAD after the last measurement; no FAKE constructs anywhere; fake_ablate vacuous.

## s39 (2026-09-06, modality `forensics`; owner directive already executed s37/s38)

- [s39] CHASSIS RE-AUDIT. HEAD src/ings.c carries the anchor
  `INCLUDE_ASM("asm/funcs", func_80017848);` at line 820 (unchanged since
  s37). tmp/grind/func_80017848/s39/body_BASE.c (candidate.c's body) applied
  over the anchor: `sandbox func_80017848 --disable all` = **3 at 127 target /
  127 build, scorable**. Kill re-audit (mandated): body_Q1.c = **14 at
  127/127** (identical to s34/s35/s38). The join chassis
  candidate_alt_join_p4_4.c (body_P4.c) = **4 at 127/125**. No FAKE construct
  anywhere in any cell; fake_ablate remains vacuous. src/ings.c restored from
  s39/ings.orig.c after the last cell; `git diff --quiet -- src/ings.c` passes.

- [s39] FRONTIER ITEM 1 (interposer redirecting flow's LOG_LINK) EXECUTED on
  s37's m5 geometry under the exact project CC_FLAGS with -da
  (tmp/grind/func_80017848/s39/mini/, runner run.sh, all dumps kept):
  (a) m8a_interposer_load: `b = a; x = *(s32 *)(b + 8); c = *(ctx+0x10);
      d = (sa<<6) + b;` with x consumed once at the end. .cse2: copy insn 71
      `(set (reg/v 77) (reg/v 76))`, interposer 74 `(set 83 (mem (plus 77 8)))`,
      links load 77, add 81 `(plus (reg 88) (reg/v 77))`. .flow: insn 74 carries
      `(insn_list 71)`; insn 81 has NO LOG_LINK to 71 (flow.c:2101 links a set
      only to the NEAREST later same-block use, which is now the interposer),
      REG_DEAD 77 sits on 81. .combine: insn 71 UNTOUCHED, 74 and 81 still read
      77 - combine's copy->interposer attempt substitutes 76 into 74 but 77 is
      not dead there, so added_sets_2 = 1 (combine.c:1458), the PARALLEL fails
      recog and with i1 == 0 no split path exists (combine.c:1990 requires i1),
      so it is undone. Final .s: `blez $2,.L7 ; move $8,$3` (copy, in the
      delay slot because this geometry has no i=0 to fill it) `; addu $2,$5,$8`
      (the add reads the copy destination) `; lw $3,16($4) ... lw $3,8($8)` -
      the interposer is a real instruction, scheduled after the add.
      So the frontier's mechanism is REAL and reproduces target's copy+add
      shape at the pass level, at the price of one instruction.
  (b) m8b_interposer_arith: interposer `x = (s32)b + sb`. .combine ends with
      71/74/81 all present and 74/81 reading 77 (same as m8a). But final .s
      has NO move: `addu $2,$5,$4` / `addu $4,$4,$6` - global gave pseudo 77
      the register $4 that 76 released at the copy, the copy became
      `(set $4 $4)` and jump2 deleted it as a no-op move (jump.c:441).
      LESSON banked: survival through combine is necessary but not sufficient;
      target's copy is a real instruction because its source register a0 is
      taken by the add's destination, which forces Q into a different seat.
  (c) m8e_roundtrip_add_reads_a: interposer `a = b` (re-sets the copy's own
      source) with the add reading a. .cse2 already lacks insn 74: cse
      canonicalised `a = b` to a set whose source equals its destination
      (cse.c:6730 prices such a source at -1 and the insn is deleted) and
      rewrote the add to read 77, so flow sees the bare copy->add pair and
      combine deletes 71 as in s37 m5. Final .s: `addu $2,$5,$3`, no move.
  (d) m8f (same, add reads b): identical outcome; `a = b` is dead at flow.
  (e) m8g_fresh_third_var: interposer `e = b` into a never-mentioned pointer,
      add reads e. .cse2 again lacks insn 74 (canonicalised away) and the add
      reads 77; combine deletes 71. Final .s identical to m8e.
  (f) m8c / m8d (second use of b AFTER the add, escape #1 controls): the copy
      survives (`move $8,$3 ; addu $2,$5,$8 ; ... lw $3,48($8)`), i.e. the
      known +1-instruction price.
  CONCLUSION: an interposer that keeps combine away from the copy must (1) sit
  in the copy's basic block and read Q (flow.c:2101), (2) compute a value
  distinct from Q's - every same-value register copy is removed by cse2 before
  flow runs (cse.c:6730, m8e/m8f/m8g) - and therefore (3) reaches the
  assembler as an instruction unless jump2 deletes it as a no-op move
  (jump.c:441), which needs its destination to share Q's hard register AND be
  read later by an instruction that reads a3. Target's preheader block holds
  only `lw a2,0x10(s2)` (reads s2) and the add, and no instruction after the
  add reads a3 anywhere in the function (loop 2 body/bottom: v0/v1/a0/a2/s3;
  .L80017974 onward: s0/s1/s4/s3/s2/a0/a1). Frontier item 1 is CLOSED as a
  class kill.

- [s39] THE ROUND-TRIP ON THE REAL CHASSIS. J1 = P4 (join chassis) with
  `q = *(u8 **)(ctx + 0xC); p = q; lnk = ...; base = sh + p;` in BOTH loops
  (tmp/grind/func_80017848/s39/body_J1.c) = **4 at 127/125**, byte-identical
  to P4: cse2 removes `p = q` exactly as m8e predicted. Banked as
  rejected/s39_join_roundtrip_p_eq_q_add_reads_p_costs_4.c.

- [s39] A SECOND POST-COMBINE COPY PRODUCER READ AND CLOSED: reload's
  input-reload path (reload1.c:5848-5853) calls find_equiv_reg and, when a
  hard register already holds the reloaded value, emits `gen_move_insn
  (reloadreg, oldequiv)` - a reg-reg copy combine never sees. Its gate is
  `reg_renumber[REGNO (old)] < 0` (reload1.c:5851): the pseudo must have
  received NO hard register from global. global.c:350-368 only ORDERS
  preference by regs_used_so_far; global.c:1172-1190 shows a call-crossing
  allocno takes any free callee-saved register before caller-saves kicks in,
  and target's prologue saves only s0-s5 (s6/s7 free), so no pseudo of this
  function is left unallocated; the 127-instruction target carries no
  sp-relative spill traffic either. The reload copy producer is therefore not
  target's producer (class kill, predicate reload1.c:5851).

- [s39] WHERE THIS LEAVES THE COPY QUESTION. With s37 (combine.c:1458 deletes
  the promoted use-once copy locally), s38 (block boundary, volatile,
  post-combine jump/flow producers, can_combine_p refusal list) and s39
  (interposer, round-trip, reload find_equiv_reg) every mechanism by which a
  `(set Q P)` whose only reader is the base add could reach the assembler is
  now either measured dead, priced at +1 instruction, priced at 14 seat
  points (use_crosses_set_p, Q1), or forbidden. The remaining unexplained
  facts are target-side: BOTH loops carry the copy with no visible second
  reader of a3, and loop 1's exit tail re-reads p from memory and recomputes
  sh on the loop path only. The next session should stop attacking the copy's
  survival and instead attack what makes the target's Q have a SECOND READER
  that leaves no bytes: the only unmeasured candidate class is a reader that
  jump2's cross-jump (jump.c:1950-2061, runs after reload) merges into the
  shared `.L800178AC` return-0 tail - which reads no register, so a class
  kill is the likely outcome and should be banked by dump, not by reading.

- [s39] Artifacts: tmp/grind/func_80017848/s39/mini/m8{a,b,c,d,e,f,g}*.c and
  their .rtl/.jump/.cse/.loop/.cse2/.flow/.combine/.sched/.lreg/.greg/.sched2/
  .jump2/.dbr/.s dumps; s39/body_{BASE,Q1,P4,J1}.c; s39/cells.ps1;
  s39/ings.orig.c.

- [s39] Chassis: anchor src/ings.c:820 unchanged; BASE = 3 at 127/127 scorable; Q1 = 14 at 127/127; P4 = 4 at 127/125; J1 = 4 at 127/125.

- [s39] Frontier item 1 (interposer) closed as a class kill: an interposer must read Q in the copy's block (flow.c:2101), must compute a distinct value (cse2 deletes same-value copies, cse.c:6730), and so costs an instruction unless jump2 no-op deletion applies (jump.c:441), which needs a later a3 reader target lacks.

- [s39] m8b: a copy that survives combine can still vanish when global gives Q the register P released; target's copy is real only because the add's destination takes a0.

- [s39] reload1.c:5851 find_equiv_reg copy path needs an unallocated pseudo; none exists here (s6/s7 free, no spill traffic), so reload is not target's copy producer.

- [s39] Chassis: anchor src/ings.c:820 unchanged; BASE = 3 at 127/127 scorable; Q1 = 14 at 127/127; P4 = 4 at 127/125; J1 = 4 at 127/125; no FAKE constructs; src/ings.c restored to HEAD.

- [s39] Owner directive 2026-09-06 was executed in s37 (m0-m6) and extended in s38 (m7a/m7b); s39 extends it again with m8a-m8g on the same minimal geometry; nothing in the directive remains unexecuted.

- [s39] m8a reproduces target's copy+add shape at the pass level (flow gives the interposer the LOG_LINK, combine leaves the copy, final move + addu reading the copy destination) at the price of the interposer instruction.

- [s39] Every same-value interposer (a = b, e = b, p = q on the chassis) is canonicalised away by cse2 before flow (cse.c:6730), so it never redirects the LOG_LINK.

- [s39] m8b: a copy that survives combine still vanishes when global gives Q the register P released (jump2 no-op deletion, jump.c:441); target's copy is a real instruction because the add's destination takes a0.

- [s39] reload's find_equiv_reg copy path (reload1.c:5851) requires an unallocated pseudo; global leaves none here (s6/s7 free, no spill traffic), so reload is not target's copy producer.

- [s39] With s37/s38/s39 together, every mechanism for a use-once (set Q P) to reach the assembler is measured dead, priced at +1 instruction, priced at 14 seat points (Q1), or forbidden; the unexplained facts are now target-side (both loops carry the copy with no visible a3 reader; loop 1's exit tail re-reads p and recomputes sh on the loop path only).

## s40 (2026-09-06, modality `rederive`; owner directive already executed s37/s38/s39)

- [s40] CHASSIS RE-AUDIT. HEAD src/ings.c carries the anchor
  `INCLUDE_ASM("asm/funcs", func_80017848);` at line 820 (unchanged since s37).
  tmp/grind/func_80017848/s40/body_BASE.c (candidate.c's body, byte-identical to
  s39's) applied over the anchor: `sandbox func_80017848 --disable all` = **3 at
  127 target / 127 build, scorable**. The residual is the same three instructions
  (dis.sh diff): build has `addu a0,a3,zero` at loop 1's exit tail where target
  has `lw a0,0xC(s2)`, and `lw v0,0xC(s2)` / `addu a0,a1,v0` at loop 2's
  preheader where target has `addu a3,a0,zero` / `addu a0,a1,a3`. No FAKE
  construct in any cell; fake_ablate remains vacuous. src/ings.c restored from
  s40/ings.orig.c; `git diff --quiet -- src/ings.c` passes.

- [s40] RE-DERIVATION LEG 1 - A CALL ARGUMENT AS THE COPY'S BYTE-FREE SECOND
  READER (new reader class, never in the ledger). Reasoning that led here:
  target's copy destination a3 is the 4th MIPS argument register, and a pseudo
  passed to a call in a3 has its arg move tied to a3 by global's hard-reg copy
  preference so the move leaves no bytes. Scratch TU m9a (full BASE body, loop 1's
  `p = q` tail removed, loop 2 re-reading p with the s37 t-reassignment guard and a
  named `q`/`lnk` preheader, `extern s32 math_Distance3D(s32 *, s32 *, u8 *, u8 *)`,
  call `math_Distance3D(pa, pb, lnk, q)`; built under the exact project flags with
  -da, tmp/grind/func_80017848/s40/mini/m9a_call_arg_reader.*). RESULT: the
  mechanism is REAL at the pass level - loop 2's preheader is
  `move $9,$5 / lw $8,16($18) / addu $3,$2,$9`, i.e. the cse-produced copy
  survives combine (q not dead at the add because the call reads it) and the add
  reads the copy destination (optimize_reg_copy_1) - target's exact three-insn
  shape. But the SEATS are wrong: q = t1 ($9), lnk = t0 ($8), base = v1, and the
  call site pays `move $6,$8 / move $7,$9`. Cause, by reading global.c
  global_conflicts: q and lnk are live at function ENTRY on the two loop-skip
  paths (the call reads them whether or not either loop ran), so they are in
  basic_block_live_at_start[0] where the incoming hard argument registers a2/a3
  are still live (the `(set s3 (reg a3))` param copy has not executed yet) and
  global records a conflict with a2/a3, forcing them into t0/t1. Any variant that
  defines q on the skip paths costs an instruction there (the skip path
  `.L80017974` holds no a3 write in target). Instance kill; the entry-liveness
  conflict is the predicate a future variant must escape.

- [s40] RE-DERIVATION LEG 2 - FRONTIER ITEM 1 (a reader of the copy destination
  in the loop-body return arm, merged by jump2's cross-jump into the shared
  return-0 tail) EXECUTED on minimal geometry with dumps AND on the chassis.
  m9c (tmp/grind/func_80017848/s40/mini/m9c_return_arm_reader.*): both loops'
  `return 0` arms replaced by `return (s32)q;` with loop 2 naming `q`.
  .s: the two arms ARE cross-jumped into one tail `.L19: j .L17 / move $2,$5`,
  but the tail CARRIES THE READ (`move $2,$5`) - cross-jump only merges insns
  that are rtx_renumbered_equal_p (jump.c:2525), so identical reads merge into
  one read, they never vanish. On the chassis (body_M9C.c, loop 1 already named
  q): **15 at 127/129** - the shared tail becomes a register read and costs two
  instructions (banked as
  rejected/s40_return_arm_reads_q_crossjump_tail_keeps_read_costs_15.c).
  CLASS KILL: a reader that reaches the merged return tail leaves its read in the
  tail; target's tail `j .L80017A1C / addu v0,zero,zero` reads no register, so no
  return-arm reader of a3 exists in target. Predicate: jump.c:2525.

- [s40] THE a3 SEAT, MEASURED FROM THE ALLOCATOR ITSELF (instrumented cc1
  tools/gcc-2.7.2/cc1, harness s40/idump.sh, CODEGEN-IDENTICAL to build/cc1 on the
  whole TU in both runs). On the BASE body loop 1's copy is insn 83
  `(set (reg 80) (reg 79))`, its tail reader insn 141 `(set (reg 79) (reg 80))`.
  BB2_FINDREG_DEBUG=80 (s40/ifr80/cc1.log):
      conflicts: 2 3 4 5 6 18 19 29   (v0 v1 a0 a1 a2 s2 s3 sp)
      someone_prefers: (none)   own_copy_prefs: (none)
      used_so_far: 0-19 24-29 31   pass0_used: 0-6 18-23 26-31
      -> a3 (7) is the lowest register that is neither conflicting nor
         first-used, so find_reg's pass 0 (global.c:993-1001) takes it.
  reg80 conflicts with v0/v1/a0/a1/a2 ONLY because it is live across the whole
  loop-1 body (its reader is the exit tail). Loop 2's preheader load pseudo
  reg113, whose only use is the base add in the same block, is LOCAL and
  local-alloc gives it v0 (`Register 113 in 2` in .lreg; local-alloc.c:472-475
  admits it, combine_regs cannot tie it to the global source pseudo because
  reg_qty[src] < 0, local-alloc.c:1827, and find_free_reg takes the lowest free
  register - MIPS defines no REG_ALLOC_ORDER). CONSEQUENCE, now measured rather
  than inferred: target's a3 in BOTH preheaders means both copy destinations were
  GLOBAL allocnos that conflict with v0,v1,a0,a1,a2, i.e. each is live across its
  own loop body - each has a flow-time reader after (or inside) its loop. Every
  visible instruction after either loop reads only v0/v1/a0/a1/a2/s-regs, so that
  reader left no bytes. Combined with s37-s39, the whole residual reduces to ONE
  question: which flow-time reader of a pseudo vanishes without bytes AND without
  a no-op-move tie (a tie to a3 would need its own byte-free reader - regress).
  The call-argument reader (leg 1) was the last untried instance of that class
  and dies on entry liveness; the cross-jump reader (leg 2) dies on jump.c:2525.

- [s40] READING CHECKS banked so no session re-derives them: (a) the reload
  find_equiv_reg copy path (reload1.c:5848-5853) also fires when `old` is a MEM,
  but an add with a MEM operand never reaches reload on MIPS (expand loads it into
  a pseudo; combine/validate_replace_rtx must recog), and an unallocated pseudo
  cannot arise here: reg_live_length is set negative ONLY for setjmp-live pseudos
  (flow.c:1240, :1260), update_equiv_regs merely doubles the live length of a
  REG_EQUIV pseudo (local-alloc.c:1064), the `-2 parameter` case in global.c:583
  has no setter in this source tree, and find_reg cannot fail with t0-t9 free.
  (b) cse_set_around_loop (cse.c:7909) only fires on REG_LOOP_TEST_P regs, which
  jump.c:2253 sets on the ORIGINAL exit-test pseudos of a duplicated while/for
  loop; it plants the copy AFTER the pre-loop load (before the guard), not in the
  preheader, and the duplicated form hoists the bound because NOTE_INSN_LOOP_VTOP
  (emitted before the original exit code by duplicate_loop_exit_test) resets
  maybe_never at loop.c:936 - the exact reason s28's W1/W2/W3 hoisted the bound.
  So the guard+do/while chassis is not merely the best-scoring form, it is the
  only loop shape whose bound stays in the loop bottom.

- [s40] Artifacts: tmp/grind/func_80017848/s40/mini/m9a_call_arg_reader.{c,s,rtl,
  jump,cse,loop,cse2,flow,combine,sched,lreg,greg,sched2,jump2,dbr},
  s40/mini/m9c_return_arm_reader.{same set}, s40/mini/run.sh, s40/idump.sh,
  s40/icand/ings.i.{lreg,greg,combine,...} + cc1.log, s40/ifr80/cc1.log,
  s40/body_{BASE,Q1,P4,M9C}.c, s40/cells.ps1, s40/dis.sh, s40/ings.orig.c.

- [s40] Chassis: anchor src/ings.c:820 unchanged; BASE = 3 at 127/127 scorable; M9C = 15 at 127/129; no FAKE constructs; src/ings.c restored to HEAD.

- [s40] Chassis: anchor src/ings.c:820 unchanged; BASE = 3 at 127/127 scorable; M9C = 15 at 127/129; no FAKE constructs anywhere; src/ings.c restored to HEAD (git diff --quiet passes).

- [s40] Owner directive 2026-09-06 (minimal-TU forensics of the copy geometry) was executed in s37 and extended in s38/s39; s40 extends it with m9a/m9c on the full-body minimal TU plus an allocator-level dump of the seat.

- [s40] m9a: a call-argument reader reproduces target's copy / links-load / add-reads-copy shape at the pass level, but the arg pseudos are live at entry on the loop-skip paths and conflict with the incoming a2/a3 hard registers (global.c global_conflicts, block 0), so they seat in t0/t1 and the call pays two moves.

- [s40] m9c: cross-jump merges the two return arms but the merged tail keeps the read (move $2,$5); jump.c:2525 rtx_renumbered_equal_p only merges identical insns. Chassis cost 15 at 127/129. Class kill of the brief's frontier item 1.

- [s40] Allocator dump (BB2_FINDREG_DEBUG=80): loop 1's copy destination gets a3 because it conflicts with v0,v1,a0,a1,a2,s2,s3,sp and a3 is the lowest already-used non-conflicting register in find_reg pass 0; loop 2's load pseudo reg113 is local and gets v0. Target's a3 in both preheaders therefore requires both copy destinations to be global allocnos live across their loop bodies.

- [s40] Reading checks banked: reg_live_length goes negative only for setjmp-live pseudos (flow.c:1240, :1260); update_equiv_regs only doubles a REG_EQUIV pseudo's live length (local-alloc.c:1064); global.c:583's -2 parameter case has no setter in this tree; so no unallocated pseudo exists for reload1.c:5851's find_equiv_reg copy path. cse_set_around_loop (cse.c:7909) needs REG_LOOP_TEST_P (jump.c:2253, duplicated exit tests only) and plants its copy before the guard, and duplicated while/for loops hoist the bound because NOTE_INSN_LOOP_VTOP resets maybe_never at loop.c:936 - the reason s28's W-forms lost.

- [s40] Rejected bank now 240: s40_call_arg_reader_q_lnk_entry_live_seats_t0_t1.c (scratch-TU form) and s40_return_arm_reads_q_crossjump_tail_keeps_read_costs_15.c.

## s41 (2026-09-06, modality `rederive`; owner directive already executed s37/s38/s39/s40)

- [s41] CHASSIS RE-AUDIT. HEAD src/ings.c carries the anchor `INCLUDE_ASM("asm/funcs",
  func_80017848);` at line 820 (unchanged since s37). body_BASE.c (candidate.c's body,
  byte-identical to s40's) over the anchor: `sandbox func_80017848 --disable all` = **3 at
  127/127, scorable**. Mandated kill re-audit: Q1 (the exact-instruction-stream form,
  s40/body_Q1.c) re-measured **14 at 127/127** - unchanged. No FAKE construct exists in any
  form of this function, so tools/fake_ablate.py is vacuous (same as s29-s40). Q1's
  normalised diff (tmp/grind/func_80017848/s41 via dis.sh): seats only - p(X)=a1, sh=a0,
  q(Y)=v0, links(=p reused)=a1, base=a0 against target X=a0, sh=a1, Y=a3, links=a2, base=a0.
  Q1's Y is v0 because it is a block-local pseudo (copy -> add) and local-alloc's
  find_free_reg takes the lowest free register; target's Y in a3 is not local.

- [s41] RE-DERIVATION CELLS (four un-banked shapes, tmp/grind/func_80017848/s41/body_*.c,
  measured through s41/cells.ps1, all on the HEAD chassis, no FAKE constructs):
    M1  BASE with loop 1's tail `p = q;` relocated to right after `base = sh + q`   12 at 125
    M2  M1 plus loop 2 spelled like loop 1 (named q/lnk, `p = q;` after its add)     14 at 125
    F1  both loops `for (i = 0;; i++) { if (i >= count) break; ... }`                 22 at 119
    G1  both loops `i = 0; goto testK; do { ...; i++; testK:; } while (i < count);`   43 at 117
  M1 diff (s41/B_M1.txt vs T.txt): cse deletes the relocated `p = q` (q is known equal to
  p in that extended block), so p becomes live across loop 1; loop 2's guard no longer
  reloads slots, BOTH preheader copies vanish (`addu a0,a0,a1` reads the guard's load
  directly) and p/sh swap seats to a1/a0. So a same-value re-store of p inside the copy's
  own extended basic block is not a flow-time reader of q at all - cse removes it before
  flow, exactly as s39's J1/P4 showed for the between-copy-and-add position. G1 diff
  (s41/B_G1.txt): the guard block collapses to `sll v0,s4,6 / lw a0,12(s2) / lw a1,16(s2) /
  j <bottom test> / addu a0,v0,a0`, the frame drops from 64 to 48 bytes (the two-step
  guard's phantom slots go with the guard), and the bottom test keeps its `lw` (loop.c
  never runs LICM on an invalid loop). F1: exit test at the loop top, no rotation - the
  guard-add/base-add pair and the bottom bound reload are gone.

- [s41] READING CHECKS banked (tools/gcc-2.7.2, canonical sources):
  (a) global.c set_preference (global.c:1671-1740) resolves BOTH operands through
      reg_renumber before testing for a hard register, so a pseudo copied to/from a
      LOCAL-ALLOC pseudo that local-alloc already seated inherits that seat as a
      hard_reg_copy_preferences bit; find_reg (global.c:1080-1110) then prefers that
      register over the lowest free one. This is the only route to a3 for a copy
      destination that is NOT live across its loop body. In this function no local qty
      can reach a3: local-alloc's find_free_reg takes the lowest register not busy in
      the block, global pseudos are invisible to it, and every block here holds a single
      local chain (v0). The only hard-register a3 write in the RTL is the entry copy of
      slot_b's parameter pseudo, which crosses the call and is seated in s3.
  (b) regs_someone_prefers (global.c:877-935) collects hard_reg_full_preferences of
      LOWER-priority conflicting allocnos; the only pseudos with v0/a0 preferences here
      (the call's argument pseudos and the call-result pseudo) never overlap a preheader
      copy destination unless it is live across the call, which a3 (call-clobbered) forbids.
      So s40's conclusion stands on a second, independent reading: target's a3 requires the
      copy destination to conflict with v0,v1,a0,a1,a2, i.e. to be live across its loop.
  (c) duplicate_loop_exit_test (jump.c:2163-2342) emits NOTE_INSN_LOOP_VTOP directly
      before the ORIGINAL exit code at the loop bottom (jump.c:2338), and scan_loop resets
      maybe_never there (loop.c:936-937); the may_trap_p gate at loop.c:715-716 is the only
      thing keeping a bound load in a loop, so a source `while`/`for` bound is hoisted
      whenever it is invariant (no store, no call in the loop - true here). The target's
      per-iteration `lw v0,28(a0)` therefore rules out every while/for spelling with an
      invariant bound, independent of s28's W-cells.
  (d) loop.c find_and_verify_loops marks a loop invalid on setjmp, forced labels,
      REG_LABEL uses, or a jump into it from outside (loop.c:2253-2322, :2596). G1 is the
      C spelling of the last case; measured 43 - the invalid loop also loses the guard.
  (e) optimize_reg_copy_1 (local-alloc.c:700-870) re-points the add at the copy destination
      only after combine kept the copy; it moves SRC's death to the copy and, if DEST dies
      inside the scanned range, moves DEST's death to SRC's old death insn. So the
      instruction stream `copy / lw links / add reads copy-dest` is reachable in two ways
      only: DEST live past the add at flow time (added_sets_2 keeps the copy; target's
      seats), or the use_crosses_set_p refusal (Q1; DEST local, seat v0).

- [s41] Artifacts: tmp/grind/func_80017848/s41/{body_BASE,body_Q1,body_M1,body_M2,body_F1,
  body_G1}.c, s41/cells.ps1, s41/dis.sh, s41/norm.py, s41/apply_win.py, s41/T.txt,
  s41/B.txt (last cell), s41/B_M1.txt, s41/B_G1.txt, s41/raw_build.txt, s41/ings.orig.c,
  s41/write_ledger.py.

- [s41] Rejected bank now 244: s41_p_eq_q_after_base_add_cse_folds_costs_12.c,
  s41_both_loops_p_eq_q_after_base_add_costs_14.c, s41_for_break_at_top_no_rotation_costs_22.c,
  s41_goto_into_loop_invalid_no_guard_costs_43.c.

- [s41] Chassis: anchor src/ings.c:820 unchanged; BASE = 3 at 127/127 scorable; Q1 = 14 at
  127/127; no FAKE constructs anywhere; src/ings.c restored from s41/ings.orig.c
  (git diff --quiet passes).

- [s41] Chassis: anchor src/ings.c:820 unchanged since s37; BASE = 3 at 127/127 scorable; Q1 = 14 at 127/127; no FAKE constructs anywhere; src/ings.c restored from s41/ings.orig.c (git diff --quiet passes).

- [s41] Owner directive 2026-09-06 (minimal-TU forensics of the copy geometry) was executed in s37 and extended s38-s40; s41 acknowledges it and adds chassis-level shape measurements plus allocator reading checks, no new scratch TU was needed.

- [s41] M1/M2: a same-value p = q re-store after the base add is deleted by cse before flow (q known equal to p in the copy's own extended basic block), so it is not a reader; the deletion additionally makes p live across loop 1 and removes loop 2's slots reload (build 125 insns).

- [s41] F1: for(;;) with break at the top is never rotated by expand/jump1, so the guard-add/base-add pair and the bottom bound reload disappear (119 insns).

- [s41] G1: a goto into the loop body invalidates the loop (loop.c:2596) and drops the duplicate exit test; the guard block becomes a plain jump to the bottom test and the frame shrinks 64 -> 48 (117 insns), confirming by measurement that the guard two-step is what buys the 64-byte frame.

- [s41] Reading (a): global.c set_preference resolves reg_renumber of both operands, so the a3 seat could come from a copy to/from a local-alloc pseudo already in a3 or from hard register a3; neither exists in this function's RTL (only local chain is v0; only hard a3 write is the entry parameter copy seated s3).

- [s41] Reading (b): regs_someone_prefers only carries preferences of lower-priority conflicting allocnos; the v0/a0-preferring pseudos (call args, call result) cannot conflict with a preheader copy destination without crossing the call, which a3 (call-clobbered) forbids.

- [s41] Reading (c): duplicate_loop_exit_test emits NOTE_INSN_LOOP_VTOP directly before the original bottom exit code (jump.c:2338) and scan_loop resets maybe_never there (loop.c:936-937), so any while/for loop with an invariant bound hoists it (loop.c:715-716 is the only trap gate) - target's per-iteration lw v0,28(a0) rules out every while/for spelling independent of s28's W-cells.

- [s41] Reading (e): optimize_reg_copy_1 (local-alloc.c:700-870) re-points the add at the copy destination and moves death notes; the copy / lw links / add-reads-copy stream is reachable only via a flow-time-live destination (target seats) or the use_crosses_set_p refusal (Q1, destination local, seat v0).

- [s41] Rejected bank now 244 entries.

## s42 (2026-09-06, modality `structural`; owner directive already executed s37-s41)

Chassis: HEAD `src/ings.c:820` INCLUDE_ASM anchor with each cell body pasted in
by `tmp/grind/func_80017848/s42/apply_win.py`; `candidate.c` (= s42/body_BASE.c)
re-measured **3 at 127/127** first (E-s42-0). No FAKE constructs exist in any
form of this function, so `tools/fake_ablate.py` is vacuous. All cell bodies,
normalised diffs (`B_<cell>.txt` vs `T.txt`), and instrumented cc1 dumps
(`i<cell>/ings.i.*`, canonical build/cc1 verified CODEGEN-IDENTICAL on every
cell) are in `tmp/grind/func_80017848/s42/`.

### E-s42-0 - chassis re-audit + kill re-audit (quote THIS floor)
BASE = 3 at 127/127; Q1 (the exact-stream escape-#9 form, closest instance kill)
= 14 at 127/127. Both unchanged from s41, so the Q1 kill is not stale.

### E-s42-1 - CORRECTION: the true BASE residual, from a fresh normalised diff
`s41/B.txt` was NOT the BASE stream (it lacks loop 1's copy; it is the last
cell s41 ran). The fresh diff of BASE (`s42/B_BASE.txt` vs `T.txt`) shows loop 1
is INSTRUCTION-EXACT (p=a0, sh=a1, q=a3, links=a2, base=a0, `addu a3,a0,zero`
present), and the whole 3-point residual is:
  (a) loop-1 exit tail: target `lw a0,12(s2)` vs BASE `addu a0,a3,zero`
      (the `p = q;` device is a real move, target reloads p);
  (b) loop-2 preheader: target `addu a3,a0,zero` vs BASE `lw v0,12(s2)`;
  (c) loop-2 base add: target `addu a0,a1,a3` vs BASE `addu a0,a1,v0`.
Everything else, including the join geometry (blez landing on `addu v0,a1,a0`,
`lw/sll` on the fall-through only), already matches.

### E-s42-2 - MECHANISM, dump-proven: target's tail geometry is reorg.c's
### redundant-insn thread redirect, NOT a C-level exit tail
Cell U1 writes an UNCONDITIONAL `p = *(u8 **)(ctx + 0xC);` in the join block
before loop 2's guard (no `p = q`). Final asm (`s42/raw_U1.txt`): the loop-1
`blez v0` targets 0x1468 = `sll v0,s4,6`, one instruction PAST the join block's
`lw a0,12(s2)` at 0x1464. Dumps (`s42/iU1_sched2_insns.txt`,
`s42/iU1_dbr_insns.txt`): in `.sched2` `code_label 140` precedes insn 143
(`(set (reg a0) (mem (plus s2 12)))`); in `.dbr` label 140 is gone, a NEW
`code_label 380` sits after insn 143, and jump_insn 79 (the blez) now carries
`label_ref 380`. This is `reorg.c:3442-3459` (`fill_slots_from_thread`: a
thread insn `redundant_insn` with an insn before the branch on a NON-owned
thread sets `new_thread = next_active_insn (trial)`) followed by
`reorg.c:3714-3716` (`get_label_before (new_thread)` + `reorg_redirect_jump`).
The guard-1 block's own `lw a0,12(s2)` makes the join block's identical load
redundant on the taken path. In target BOTH `lw a0,12(s2)` and `sll a1,s4,6`
are skipped because both are register-identical to guard 1's; in U1 only the
lw is skipped because U1's sh2 landed in v0. CONSEQUENCE: target's C needs no
`p = q` exit tail; loop 2's guard is plausibly written exactly like loop 1's.

### E-s42-3 - the NATURAL SYMMETRIC CHASSIS measures 6 at 127/125 (U4/U4b)
U4 = no exit tail; unconditional reload + `i = 0; sh2 = slot_a << 6;
t2 = sh2 + p; t2 = *(s32 *)(t2 + 0x20); if (i < t2)` (DISTINCT two-step local
per loop); loop 2 preheader `q = p; base = sh2 + q` (U4b: fresh read; identical
bytes). Diff vs target is EXACTLY the two `addu a3,a0,zero` copies, the two base
adds reading a0 for a3, and loop 2's links seat (a1 for a2, a consequence of the
missing copy). `iU4/ings.i.cse2` has both copies (insns 83 and 162, both
`(set (reg/v:SI 80) ...)`); `iU4/ings.i.combine` has neither. Same body as
s34's `candidate_alt_join_shape_6.c` modulo declaration order (diffed).
Intermediate cells: U1/U2 = 9 at 124 (loop 2 guard inline, so the join-block
load makes the preheader read redundant and cse folds base2 into the guard's
address temp); U3/U3b = 32 at 125 (SHARED `t` two-step across both loops =
the s9 t-reuse seat rotation, t->v1, i->a0, p->v0).

### E-s42-4 - BASE's loop-1 copy is bought ENTIRELY by the `p = q` read (U5)
U5 = BASE + `p = *(u8 **)(ctx + 0xC);` right after `p = q;` (inside the
if-body): 4 at 127/126. The tail becomes target's `lw a0,12(s2)` but `p = q` is
now a dead store, flow deletes it BEFORE counting its read of q (flow.c
`insn_dead_p` path skips `mark_used_regs` for dead insns), loop 1's copy dies in
combine. A dead-store reader is not a reader.

### E-s42-5 - reader / producer classes closed by reading this session
(i) `combine.c:803-930` (`can_combine_p`) read end-to-end: the only refusal
predicates a use-once same-block copy `q = p` -> `base = sh + q` can trip are
`use_crosses_set_p` (escape #9, merges p with the intervening def) and
`REG_NO_CONFLICT` notes (multi-word no-conflict blocks only, optabs); the
`last_call_cuid` and `REG_INC`/`REG_RETVAL`/`ASM_OPERANDS` paths need a call,
autoinc, libcall or asm between the copy and the add. Nothing new.
(ii) reload's input-reload copy producer (`reload1.c:5843-5851`,
`find_equiv_reg` -> `gen_move_insn (reloadreg, oldequiv)`) fires only for
`GET_CODE (old) == MEM` or an UNALLOCATED pseudo. A MEM operand in the add at
reload time can only come from `local-alloc.c:1079-1082`
(`reg_n_refs == 2 && reg_basic_block < 0 && REG_EQUIV`), and REG_EQUIV notes are
created only by `function.c:3838-3854` (stack-passed parameters) or by
`local-alloc.c:1051-1055` for `reg_basic_block >= 0` (LOCAL) pseudos. A global
pseudo can therefore never carry the note in this 4-register-arg function:
the reload copy producer is unreachable here (class kill, H-s42-6).
(iii) `global.c:414-432`: every pseudo with refs and `reg_live_length != -1`
gets an allocno; -1 is set only through the equiv machinery above; so no
pseudo of this function is left unallocated for reload to rematerialise.
(iv) USE insns (`function.c:3066` `use_variable`) are emitted only under
`obey_regdecls` (stmt.c:3268/3499), i.e. -O0: no byte-free flow-time USE
exists at -O2.

- [s42] Floor re-audited at 3 (127/127) on the HEAD chassis; Q1 re-measured 14 (127/127); no FAKE constructs.
- [s42] s41/B.txt was not BASE; the true 3-point residual is (a) tail `lw a0,12(s2)` vs `addu a0,a3,zero`, (b) loop-2 preheader `addu a3,a0,zero` vs `lw v0,12(s2)`, (c) `addu a0,a1,a3` vs `addu a0,a1,v0`. Loop 1 is instruction-exact in BASE.
- [s42] DUMP-PROVEN: target's join geometry (lw/sll on the fall-through only, blez landing on the guard add) is produced by reorg.c:3442-3459 + :3714-3716 (redundant-insn thread redirect) from an UNCONDITIONAL reload in the join block: U1's .sched2 has label 140 before the reload insn 143, .dbr has new label 380 after it with the blez retargeted. No `p = q` exit tail is needed for the tail bytes.
- [s42] The natural symmetric chassis (U4: unconditional reload, distinct two-step guard per loop, `q = p` copies) is 6 at 127/125 with a residual of exactly the two use-once copies (cse2 insns 83/162 present, gone in .combine) and the consequent links seat; it equals s34's join-shape form.
- [s42] U5 (BASE + reload after `p = q`) = 4 at 126: the dead `p = q` is deleted by flow before its read counts, so BASE's loop-1 copy is bought entirely by that read.
- [s42] Reload's find_equiv_reg copy producer is unreachable in this function (class kill): it needs a MEM operand or an unallocated pseudo at reload time; MEM operands come only from local-alloc.c:1079-1082 which needs a REG_EQUIV on a GLOBAL pseudo, and REG_EQUIV is created only for stack params (function.c:3838-3854) or LOCAL pseudos (local-alloc.c:1051-1055).

- [s42] Floor re-audited at 3 (127/127) on the HEAD src/ings.c:820 chassis; Q1 14 (127/127); no FAKE constructs exist.

- [s42] CORRECTION: s41/B.txt was not the BASE stream. In BASE loop 1 is instruction-exact (p=a0, sh=a1, q=a3, links=a2, addu a3,a0,zero present); the 3-point residual is the exit tail (target lw a0,12(s2) vs move), loop 2's addend (target copy into a3 vs lw v0,12(s2)) and loop 2's base add reading v0.

- [s42] DUMP-PROVEN: target's join geometry is reorg.c:3442-3459 + :3714-3716 redundant-insn thread redirect on an UNCONDITIONAL reload in the join block (U1: .sched2 label 140 before reload insn 143; .dbr new label 380 after it, blez retargeted). The p = q exit tail is not what target does there.

- [s42] Natural symmetric chassis U4 (unconditional reload, distinct two-step guard per loop, q = p copies) = 6 at 127/125; residual is exactly the two use-once copies (cse2 insns 83/162, deleted by combine) plus the consequent links seat; equals s34's join-shape form.

- [s42] U5 (BASE + reload after p = q) = 4 at 126: a dead-store reader is deleted by flow before its read counts; BASE's loop-1 copy is bought entirely by the p = q read.

- [s42] Class kill: reload's find_equiv_reg copy producer is unreachable here (needs a MEM operand or unallocated pseudo; REG_EQUIV on a global pseudo only exists for stack parameters, function.c:3838-3854; local-alloc.c:1079).

## s43 (2026-09-06, modality `structural`; owner directive already executed s37-s42)

Chassis: HEAD `src/ings.c:820` INCLUDE_ASM anchor with each cell body pasted in
by `tmp/grind/func_80017848/s43/apply_win.py` (cells.ps1 / cellsd.ps1 /
post.sh); `candidate.c` (= s43/body_BASE.c, s42's body) re-measured **3 at
127/127** first, and s42's natural symmetric chassis U4 re-measured **6 at
127/125** (E-s43-0). No FAKE constructs exist in any form of this function, so
`tools/fake_ablate.py` is vacuous. Cell bodies, normalised diffs (`D_<cell>.txt`
= diff of `T.txt` vs `B_<cell>.txt`), raw objdumps and instrumented cc1 dumps
(`i<cell>/ings.i.*`, CODEGEN-IDENTICAL to build/cc1 on every dumped cell) are in
`tmp/grind/func_80017848/s43/`.

### E-s43-0 - chassis re-audit + kill re-audit (quote THIS floor)
BASE = 3 at 127/127; U4 (closest natural form, s42) = 6 at 127/125. Both
unchanged from s42. src/ings.c restored to HEAD after every cell
(`git diff --quiet -- src/ings.c` passes at session end).

### E-s43-1 - W1: loop-1 exit tail INSIDE the if-body is a seat swap (12)
U4 with `p = *(u8 **)(ctx + 0xC); sh = slot_a << 6;` written after loop 1 inside
the guard's then-block (join reload removed, loop 2's two-step guard through the
same `sh`, `sh2` deleted): 12 at 127/125. D_W1.txt: both guards become
`lw a1,12(s2) / sll a0,s4,6 / addu v0,a0,a1` (target `a0/a1/addu v0,a1,a0`),
both preheaders `addu a0,a0,a1` (no copy), loop 2's links seat a1. Reusing `sh`
and `p` across the tail gives each pseudo two definitions whose union of
conflicts rotates the seats; the copies are deleted exactly as in U4. So the
join-block reload of U4 (with reorg's redundant-insn redirect, E-s42-2) is the
better spelling and the tail-in-if spelling is dead on this chassis.

### E-s43-2 - W2: a C self-assignment of the copy destination emits NO RTL
U4 + `q = q;` at the top of both loop bodies (intended as a loop-carried
flow-time reader that would keep q live across the body and seat it in a3):
6 at 127/125, object identical to U4. `iW2/ings.i.rtl` contains zero insns of
the shape `(set (reg/v:SI N) (reg/v:SI N))` (grep across .rtl/.jump/.cse/.loop/
.cse2/.flow/.combine: 0 in every pass). Mechanism: `expr.c:2845` store_expr
emits a move only when `temp != target`; expand_expr of the VAR_DECL with the
variable's own DECL_RTL as target returns that rtx, so nothing is emitted.
Reading also closes the fallback: even an emitted self-set could not seed its
own liveness, because propagate_block (`flow.c:1590`, `if (! insn_is_dead)
mark_used_regs`) never marks the uses of an insn that is dead under the current
`old` set, in the non-final passes as well as the final one, so life_analysis
converges on the least fixpoint in which the self-set is dead. A loop-carried
self-use is therefore not a reader class at all (class kill, H-s43-2).

### E-s43-3 - the sanctioned do/while(0) wrap, measured for the first time here
Four placements on U4, both loops each:
  X1 wrap around the copy statement only ................ 6 at 127/125
  X2 wrap around the whole then-block (preheader+loop) .. 6 at 127/125
  X4 wrap around the three preheader statements ......... 6 at 127/125
  X3 wrap around the inner do/while statement only ...... 10 at 127/125
X2's and X4's raw objdumps are byte-identical (address-stripped diff empty) and
carry U4's exact residual (both copies absent, `addu a0,a1,a0` twice, loop-2
links in a1). X3 (D_X3.txt): loop 1's `sh` seats a2 and `lnk` a1
(`sll a2,s4,6 / addu v0,a2,a0 / lw a1,16(s2) / addu a0,a2,a0`), loop 2's links
a1, still no copy anywhere - the wrap's extra loop_depth re-weights flow's
reg_n_refs / reg_live_length for the pseudos referenced inside it and rotates
global's allocation order, but the copy is gone before allocation (combine).
The wrap changes nothing combine sees: its loop label is unreferenced once the
`while (0)` test folds, jump1 deletes it before cse, so the copy and the add
stay one basic block with a LOG_LINK.

### E-s43-4 - reading banked: the a3 seat cannot come from pass-0 exclusion
`global.c:919-928` (prune_preferences): `regs_someone_prefers[allocno]` is the
union of `hard_reg_full_preferences` of allocnos LATER in `allocno_order`
(lower priority) that conflict with it. `global.c:899-910`: each allocno's own
preferences are first pruned by its conflicts and, when it crosses a call, by
`call_used_reg_set`. The only pseudos preferring a0-a3 are the four parameter
copies (`(set s2..s5 (reg a0..a3))`); all four cross the call, so their a0-a3
copy preferences are pruned before they can be merged into anyone's
regs_someone_prefers. `global.c:1000` (`IOR_COMPL (used, regs_used_so_far)`)
cannot exclude v0/a0 either because both are seeded used by local-alloc qtys
(the guard temps in v0, the call's a0 argument). Hence find_reg's pass 0 gives
the copy destination v0 unless v0 and a0 are in `hard_reg_conflicts`, and a0
enters that set only if q is live when `base` is born (q not dead at the add)
while v0 enters it only through the loop body's temps. Third independent
confirmation of s40's requirement; the allocator route is closed by reading.

### E-s43-5 - reading banked: what a byte-free flow-time reader would need
(i) `flow.c:1590`: only non-dead insns mark uses, so the reader must be an insn
whose destination is itself needed (or a jump/store/call). (ii) cse2 processes
the guard block and the preheader as one extended basic block and knows q == p
there, while combine only links within the basic block; any algebraic reader
combine could fold away, cse2 folds first and flow then deletes the reader
before its use counts. (iii) After RA the only deleters are no-op moves
(jump.c:437-455, reload) and reorg's redundant-insn redirect (E-s42-2), so a
byte-free reader chain must end in a no-op move whose destination is read by an
instruction target has - and no instruction of target after either preheader
add reads a3. This is the s40 conclusion restated with the pass predicates that
enforce it; it is why the frontier below points at the guard-block placement
(conflicts without a reader) and the grant-rescan item, not at another reader.

- [s43] Floor re-audited at 3 (127/127) on the HEAD chassis; U4 re-audited at 6 (127/125); no FAKE constructs; src/ings.c restored.
- [s43] W1 (exit tail reload + sh recompute inside the if, no join reload) = 12: p<->sh seat swap in both guards, no copy.
- [s43] W2 (`q = q;` in both loop bodies) = 6 with ZERO self-set insns in .rtl: expr.c:2845 store_expr emits no move for a self-assignment; flow.c:1590 would not let a self-set seed liveness anyway (class kill).
- [s43] do/while(0) wrap (sanctioned family) measured for the first time on this function: X1/X2/X4 = 6 (objects identical to U4), X3 (inner loop only) = 10 (seat rotation sh->a2, lnk->a1); no placement keeps the copy.
- [s43] Reading: global.c:919-928 + :899-910 close the pass-0 preference route (parameter prefs pruned as call-crossing; someone_prefers is lower-priority only); the a3 seat needs hard_reg_conflicts with v0 and a0, i.e. q live past the base add.
- [s43] Artifacts: tmp/grind/func_80017848/s43/{body_*.c, D_*.txt, B_*.txt, raw_*.txt, T.txt, iW1/, iW2/, iX2/, iX3/, cells.ps1, cellsd.ps1, post.sh, apply_win.py, norm.py, idump.sh, ings.orig.c, bank.py}.

- [s43] Floor re-audited at 3 (127/127) on the HEAD src/ings.c:820 chassis; U4 re-audited at 6 (127/125); no FAKE constructs exist; src/ings.c restored to HEAD (git diff --quiet passes).

- [s43] W1 (exit-tail reload + sh recompute inside the if, no join reload) = 12: p<->sh seat swap in both guards, no copy (D_W1.txt).

- [s43] W2 (`q = q;` in both loop bodies) = 6 with ZERO self-set insns in iW2/ings.i.rtl: expr.c:2845 store_expr emits no move for a self-assignment; flow.c:1590 would not let a self-set seed liveness anyway.

- [s43] do/while(0) wrap measured for the first time on this function: X1/X2/X4 = 6 (X2/X4 objects byte-identical to each other and carrying U4's residual), X3 (inner loop only) = 10 with seat rotation sh->a2, lnk->a1; no placement keeps the copy (D_X2.txt, D_X3.txt, iX2/, iX3/).

- [s43] Reading: global.c:919-928 (someone_prefers = lower-priority conflicting allocnos only) + global.c:899-910 (call-crossing parameter preferences pruned) + global.c:344-372/:1000 (v0/a0 seeded used) close the pass-0 preference route to a3; the a3 seat needs hard_reg_conflicts with v0 and a0, i.e. q live past the base add.

- [s43] Reading: a byte-free flow-time reader must be a non-dead insn (flow.c:1590), cannot be an algebraic fold (cse2 sees the guard->preheader extended block and folds first), and after RA can only vanish as a no-op move or by reorg's redundant-insn redirect; no instruction of target after either preheader add reads a3.

## s44 (2026-09-06, modality `synthesis`; owner directive already executed s37-s43)

Chassis: HEAD `src/ings.c:820` INCLUDE_ASM anchor with each cell body pasted in
by `tmp/grind/func_80017848/s44/apply_win.py` (cells.ps1 saves obj_<cell>.o and
src_<cell>.c per cell; post.sh makes D_<cell>.txt from the saved object and the
instrumented dump i<cell>/ from the saved source). `candidate.c` (= s43's body,
s44/body_BASE.c) re-measured **3 at 127/127** first; U4 re-measured **6 at
127/125** (E-s44-0). No FAKE constructs exist in any form of this function, so
`tools/fake_ablate.py` remains vacuous (the kill re-audit is the BASE/U4
re-measurement). Sibling func_8005E54C has no candidate.c and shares no block
with this function (nothing to transplant). Cell bodies, normalised diffs,
raw objdumps, saved objects and instrumented dumps (CODEGEN-IDENTICAL to
build/cc1 on every dumped cell) are in `tmp/grind/func_80017848/s44/`.

### E-s44-0 - chassis re-audit + kill re-audit (quote THIS floor)
BASE = 3 at 127/127; U4 = 6 at 127/125. Both identical to s42/s43; the ledger's
banked kills are not stale. src/ings.c restored to HEAD after every cell
(`git diff --quiet -- src/ings.c` passes at session end).

### E-s44-1 - frontier item 1 (copy in the guard block before the two-step) is dead
F1a = U4 with `q = p;` right after the guard load and before `sh = slot_a << 6`
/ `t = sh + p` in both loops: **6 at 127/125**, same residual as U4. F1b (fresh
read `q = *(u8 **)(ctx + 0xC)` at the same position) = **6**. Mechanism: the
guard block and the preheader form one cse2 extended block; q's last reference
(the base add) lies inside it, so cse.c:826-855 make_regs_eqv keeps p as the
canonical register, the base add is rewritten to `sh + p`, and the copy is dead
before flow (flow deletes it; combine never sees it). The s32 cell S survived
only because q was ALSO read after the extended block (the loop-2 guard) and so
became canonical. Escape #8 therefore needs a post-block reference of q in
addition to the block boundary - and E-s32-7 already showed the boundary
position is incompatible with target's stream. Frontier item 1 closed.

### E-s44-2 - frontier item 2 (roles swapped) is dead; a real post-loop reader seats q in a1
F2 (guard loads q, preheader `p = q`, `base = sh + p`, both loops) = **6 at
127/125**: the short-lived copy destination p dies at the add, combine folds it,
identical to U4. F2b (F2 in loop 1 plus loop 2's guard reading q instead of
reloading) = **12 at 127/124**: the copy is folded INTO the base add
(`addu a0,a0,a1` reads q), q takes a1 and sh a0 (both guards swap), loop-2
links a1. q was allocated before sh because its priority (more refs) is
higher. A real post-loop reader cannot both keep the copy and seat q in a3.
Frontier item 2 closed.

### E-s44-3 - NEW MECHANISM, DUMP-PROVEN: a combine-folded loop-body reader keeps the copy with zero bytes
Instrument M1 (a CHEAT by construction - dead algebra `m = slot_b & 1; w = m >> 1;`
in the loop `y = i & w; z = (s32)q & y;` with the exit test reading
`base + z + 0x1C`; never a candidate) = 14 at 127/128. Its loop 1 preheader is
`addu a1,a0,zero / lw t0,16(s2) / addu a0,a2,a1`: the use-once copy SURVIVES and
the base add reads the copy destination, while the loop body contains NO read of
a1 (the `and` chain is folded to constants). Pass attribution from iM1/: the
reader `z = q & y` is present in .cse2 and .flow (flow marks q live at the loop
top, so the base add carries no REG_DEAD for q); in .combine the chain
`y = i & w` -> 0 (combine.c nonzero_bits of w = (ashiftrt m 1) with
nonzero_bits(m) = 1) and `z = q & 0` -> 0 are folded into the exit-test address,
so q's loop read vanishes AFTER flow; try_combine(copy -> base add) then fails
at combine.c:1458 (`added_sets_2 = ! dead_or_set_p (i3, i2dest)` is true) because
the PARALLEL of two SETs is not recognised and the two-insn case has no split
path (combine.c:1704/1718 need REG_UNUSED, :1900/:1996 need i1). global.c then
allocates q with the loop's pseudos in conflict because
`basic_block_live_at_start` (flow.c) is never recomputed between combine and
global_alloc. CONSEQUENCE: E-s43-5's enumeration of byte-free readers ("after
RA the only deleters are no-op moves and reorg's redundant-insn redirect") is
REFUTED - combine is a pre-RA deleter of flow-time readers, and a reader it
deletes leaves both the copy and q's loop-wide conflicts behind at zero bytes.
This is the first mechanism in 44 sessions that reproduces (copy alive, add
reads the copy destination, no reader instruction) without a real move.

### E-s44-4 - M5 reproduces target's INSTRUCTION STREAM exactly (127/127); the residual is allocation ORDER
M5 (instrument: `y = (sh & i) & 0x3F; z = (s32)q & y;` as separate statements
in both loops, exit tests reading `base + z + off`; a CHEAT, never a candidate)
= **12 at 127/127**: both preheader copies alive, no reader bytes, D_M5.txt is a
pure register permutation (q a1 for a3, sh a2 for a1, lnk a3 for a2) plus loop
2's `addu/lw` order (a consequence of the seats). iM5/cc1.log ALLOCDBG order:
i(88) 30000 -> base(81) 13043 -> **q(80) 9230 (nrefs 8, livelen 26) -> a1** ->
p(79) 8571 -> sh(84)/sh2(85) 5555 (livelen 18: the instrument's in-loop read of
sh stretches sh's range) -> ... -> lnk(77) 2500 -> a3. Priority is
global.c:615 `floor_log2 (n_refs) * n_refs / live_length * 10000 * size`.
Target's a3 seat needs q allocated AFTER sh and lnk; in U4's natural geometry
(s43/iX2 ALLOCDBG) sh is 11428 (4/7), sh2 13333, lnk 8333 (5/12), p 16000,
base 27826. M6 (distinct q/q2 per loop) = 12 with q at 6153 (4 refs / 13):
still before the instrument-depressed sh (5555). Inference (unmeasured): a copy
destination with 4 weighted refs (def, add, one in-loop read at loop weight 2)
and live length >= 10, in a geometry that leaves sh/lnk at their U4 priorities,
sorts after both and takes a3 (v0/v1/a0/a1/a2 all conflict once q is live
across the loop). M7 (fold sourced from the loop's own lbu value) turned the
named `idx` into a global pseudo (priority 30000, v1) and rotated every seat
(35 at 126) - the instrument must not add a cross-block pseudo.

### E-s44-5 - what combine does and does not fold (instrument spellings, banked)
M2/M2a (`if (idx >> 8) return (s32)q;`): the `sra` and its `bne` survive
(45/38 at 135/131) - combine's shift path does not zero an ashiftrt whose
operand's nonzero_bits are shifted out, and the jump keeps a real reader.
M3/M3a (`if ((sh & i) & 0x3F) return (s32)q;`): the and-chain folds to
`y = 0` (`addu v0,zero,zero`) but the constant-condition `bne v0,zero` is NOT
folded by combine, so the reader block stays reachable (14/16 at 133/132);
copies survive only as real uses. M4/M4a (`z = q & ((sh & i) & 0x3F)` in one
expression): fold-const reassociates the constant onto q, `q & 63` becomes a
hoisted invariant (`andi`), a real second use (10/9 at 129/127, q in v0).
Only the `and`-chain-into-address spelling (M1/M5/M6) is byte-free.

### E-s44-6 - policy position of the finding
Every instrument is dead algebra (T1 no semantic purpose, T2 a human would not
write it) and is banked as `rejected/s44_INSTRUMENT_cheat_*.c` so it is never
re-proposed as a candidate. The finding's value is forensic: target's C
contains, in each loop, a READ of the copy destination that combine (not cse,
not loop.c, not flow) removes. The natural-C search is now narrowed to loop-body
expressions on the slots pointer (or a value derived from it) whose RTL combine
proves constant or unreachable, e.g. comparisons of the zero-extended byte index
against out-of-range constants (`idx == -1`, `idx >= 0x100`) IF the 2.7.2 front
end leaves them to RTL (combine.c simplify_comparison folds them via
nonzero_bits and jump2 deletes the dead arm) - to be measured next session with
the reader placed in the folded arm. No sanctioned family covers an
intentionally folded reader; if the natural spelling is found it is ordinary C
and needs no annotation, otherwise the disposition is unchanged.

- [s44] Floor re-audited at 3 (127/127) on the HEAD src/ings.c:820 chassis; U4 6 (127/125); no FAKE constructs; src/ings.c restored.
- [s44] Frontier item 1 (copy in the guard block before the two-step) F1a/F1b = 6: cse2 keeps p canonical (q's last use inside the extended block), the copy is dead before flow. Closed.
- [s44] Frontier item 2 (roles swapped) F2 = 6; F2b (loop-2 guard reads q) = 12: real post-loop reader folds the copy into the add and seats q in a1. Closed.
- [s44] NEW MECHANISM (instrument M1, cheat, never a candidate): a loop-body read of q folded to a constant by combine after flow keeps the preheader copy (added_sets_2 -> unrecognised PARALLEL, combine.c:1458) with zero reader bytes, and global.c allocates q as live across the loop (stale basic_block_live_at_start). Refutes E-s43-5's deleter enumeration.
- [s44] M5 (instrument, cheat) = 12 at 127/127: target's exact instruction stream with both copies; residual is allocation ORDER (q pri 9230 before sh 5555 / lnk 2500 in that geometry). global.c:615 priority formula quoted; U4 geometry has sh 11428, lnk 8333.
- [s44] Instrument spellings that do NOT work: ashiftrt fold (M2), constant-condition jump fold (M3), single-expression and-chain (M4, fold-const reassociation), lbu-sourced fold with a named cross-block idx (M7, seat rotation).
- [s44] Artifacts: tmp/grind/func_80017848/s44/{body_*.c, D_*.txt, B_*.txt, raw_*.txt, obj_*.o, src_*.c, T.txt, iF2b/, iM1/, iM2/, iM3/, iM4/, iM5/, iM6/, iM7/, cells.ps1, post.sh, apply_win.py, norm.py, idump.sh, mk*.py, bank.py, ledger.py, idiom*.py}.

- [s44] BASE (candidate.c body) re-measured 3 at 127/127 and U4 6 at 127/125 on the HEAD src/ings.c:820 chassis; src/ings.c restored (git diff --quiet passes).

- [s44] F1a/F1b (frontier 1) = 6: a copy in the guard block is canonicalised away by cse2 (cse.c:826-855) because q's last reference is inside the guard+preheader extended block; F2 = 6, F2b = 12 (frontier 2): a real post-loop reader folds the copy into the add and seats q in a1.

- [s44] Instrument M1 (cheat, never a candidate) proves a combine-folded loop-body reader keeps the preheader copy alive (combine.c:1458 added_sets_2, no two-insn split path) with zero reader bytes; global.c allocates q live across the loop from flow's stale basic_block_live_at_start.

- [s44] Instrument M5 (cheat) = 12 at 127/127: target's exact instruction stream with both copies; residual is a pure seat permutation decided by global.c:615 priority order (q 9230 allocated before sh 5555 and lnk 2500 in that geometry; U4 geometry has sh 11428, lnk 8333).

- [s44] combine does not byte-free-fold: an ashiftrt feeding a jump (M2), a constant-condition conditional jump (M3), or a single-expression and-chain that fold-const reassociates (M4); a named cross-block idx (M7) becomes a global pseudo and rotates every seat.

- [s44] Sibling func_8005E54C has no candidate.c and shares no block with this function; nothing to transplant.

- [s44] Repo-wide idiom scan (s44/idiom2.py): no other function, matched or not, carries the addu a3,a0,zero / addu a0,a1,a3 preheader pair; the two matched functions with a kept copy-then-add (func_800200DC, func_80037F40) are hard-register copies (call return / incoming parameter), a different mechanism.


## s45 (object-model, 2026-09-06)

Chassis re-measured first: BASE (candidate.c body over the src/ings.c:820
INCLUDE_ASM anchor) = **3** at 127/127. `python3 tools/fake_ablate.py --func
func_80017848 --file ings --candidate memory/grind/func_80017848/candidate.c`
reports "no FAKE-annotated constructs found" (s45/fake_ablate.txt) - no form of
this function has ever carried a FAKE construct, so ablation is vacuous. Kill
re-audit: the closest banked instance kill to the target that touches the
object model is s30 cell B (struct-typed record/link over the BASE shape,
rejected/s30_struct_typed_record_candidate_shape_costs_7.c); re-measured on the
HEAD chassis as cell R30B = **7** at 127/127, tying s30 exactly. src/ings.c is
restored after every cell (git status clean).

### OBJECT MODEL: per-symbol verdicts (owner directive, object-model audit)

This function references NO global symbol. Its entire data model is reached
through the first parameter `ctx` ($a0 -> $s2) and one callee,
`math_Distance3D`. The audit therefore covers the object `ctx` points at and
the two arrays it owns, with the evidence sources the directive names.

1. **`ctx` (parameter, 52-byte object block) - MATCHES.** Evidence: census
   `named_syms.txt:437-440` ("Object table helpers ... index g_file_data_buf
   with stride 0x34"); the matched sibling `func_80017D84` (src/ings.c:824-843)
   allocates one such block `p` and writes `+0` handle (s32), `+4` record count
   (u16), `+6` link count (`*(s16 *)(p + 6) = 0`), `+8` s32, `+0xC` = heap base
   `c`, `+0x10` = `c + (nrec << 6)`; the only caller `func_80017A44`
   (asm/funcs/func_80017A44.s:197) passes that block in $a0 with $a2/$a3 = two
   `lh` slot indices. Declared shape in the candidate: `u8 *ctx` + literal
   offsets. Corrected declaration measured: `typedef struct { s32 handle; u16
   nrec; s16 nlink; s32 unk8; u8 *recs; u8 *links; } ObjCtx;` with every
   `*(T *)(ctx + off)` replaced by a member access, tail written `ctx->nlink`
   naturally (cell O1, s45/body_O1.c) = **3 at 127/127 and `cmp obj_O1.o
   obj_BASE.o` IDENTICAL**. Typing the context is byte-neutral.
2. **`ctx+0x6` link counter - MATCHES, natural `s16`.** Evidence in this
   function's own asm: `lh $v1,6($s2)` where the value is widened for the link
   address (`sll 4`), `lhu` where it is truncated into the `sb` stores and in
   the `lhu/addiu/sh` increment. That split is exactly GCC 2.7.2 MIPS codegen
   for a plain `s16` field: `movhi` from memory is `lhu`, `extendhisi2` is
   `lh`. The candidate's `*(u16 *)` spellings and O1's `ctx->nlink` produce the
   same bytes (O1 identical object).
3. **`ctx+0xC` record array, stride 0x40 - declared shape MISMATCH (raw
   `u8 *`), measured: typed declaration COSTS.** Evidence for the shape: own asm
   `sll 6` indexing; sibling init loop in `func_80017A44` (`addiu $s0,$s0,0x40`,
   stores at +0/+4/+8 (pos, fed to `math_Distance3D` as `s32 *`), +0xC/+0x10/
   +0x14 zero, +0x18 from an `lh` (the sign-tested field), +0x1C and +0x20 zero
   = the two counts, +0x24 / +0x2C the two 8-byte index lists); `func_80017D84`
   sizes the array as `nrec << 6`. Corrected declaration: `typedef struct {
   s32 pos[3]; s32 unkC[3]; s32 f18; s32 na; s32 nb; u8 la[8]; u8 lb[8]; s32
   unk34[3]; } ObjRec;` (0x40). Measured, all on the HEAD chassis:
   - R30B (s30 B re-audit: typed records/links, raw ctx, BASE shape) = **7**
     at 127/127;
   - O2 (typed ctx + records + links, BASE shape, typed tail) = **37** at
     127/126;
   - O3 (full typing over the symmetric U4 chassis) = **43** at 127/122;
   - O4 (the natural typed form: one record pointer per loop, `for (i = 0;
     i < rec->na; i++)`, `&` top guard) = **47** at 127/118.
   Mechanism (dumps s45/iO2, s45/iR30B, script s45/loads12.py): the six
   `ctx->recs` loads (insns 29/64/167/229/283/308) survive every RTL pass in
   both O2 and R30B, so the typed form does NOT change what cse keeps at the
   preheader; O2's extra +30 is global.c seating both the guard load (29) and
   the preheader load (64) in $v1, after which reorg.c `redundant_insn`
   deletes the preheader load (insn 64 present in .sched2, ABSENT in .dbr)
   and copies `sll $v0,$s4,6` into the second guard's delay slot - the same
   reorg mechanism as E-s42-2, triggered by a seat coincidence the typed tail
   induces (`addu $v0,$v0,$s0` operand flip). The natural typed forms O3/O4
   are 5 and 9 instructions SHORT because holding one typed record pointer
   lets cse share it across the guards, the loops and the tail, whereas the
   target reloads `0xC($s2)` in every block (7 loads). The matched sibling
   `func_80017D84` reaches its bytes with the same raw `u8 *` + literal-offset
   idiom on this very object, so the raw idiom is the evidence-consistent
   spelling of the shipped source, not a mismatch to correct.
4. **`ctx+0x10` link array, stride 0x10 - MATCHES modulo spelling.** Evidence:
   own asm `sll 4`; fields +0 dist (sw), +4 read `lhu` (loop 1) / +6 read `lh`
   (loop 2), +8 `dist*3`, +0xC owner; the tail writes +4/+6 as ONE packed `sw`
   of `(slot_a << 16) | slot_b` (`sll/or/sw`), so the shipped source stores the
   pair as an s32 while the loops read the halves. Corrected declaration
   `typedef struct { s32 dist; u16 b; s16 a; s32 dist3; s32 owner; } ObjLink;`
   measured only in combination (R30B/O2/O3/O4 above; s30 cell D = 7 with the
   typed link reads inside the loop bodies only). No residual instruction
   involves a link field; both idioms are byte-exact there.
5. **`math_Distance3D(s32 *, s32 *)` on record +0 - MATCHES** (tail call
   sequence identical in every cell).
6. **`g_file_data_buf` - NOT REFERENCED** by this function (the block pointer
   arrives as a parameter); out of scope.

### PREMISES the floor-3 argument rests on (explicit, attackable)

- P1. The whole residual is the two preheader reg-reg copies `addu $a3,$a0,
  $zero` and their consumers (base add reading $a3; loop-1 exit `lw` vs BASE's
  `move`) - E-s42-1. Object-model changes move SEATS and cse canonicals
  (R30B/O2) but never create a surviving copy on their own: O2's
  `addu $a3,$v1,$zero` survives only because `p = q` reads it, the same device
  BASE already spends.
- P2. Target reads `0xC($s2)` freshly in every block (guard block, loop-1
  preheader, loop-1 exit, loop-2 base, tail x3 = 7 loads). Any source that
  holds one typed record pointer across blocks lets cse fold them (O3 -5,
  O4 -9 instructions), so the shipped source re-reads the pointer per use or
  per block, which is what the raw idiom and BASE do.
- P3. The copy destination must be live past the base add at flow time with
  no byte-producing reader (E-s44-3). The object model cannot supply that
  reader: a natural in-loop read of the record pointer or of `&recs[slot_a]`
  is cse-merged into `base` (same value) before flow (R30B/O2 base =
  `addu $a0,$a1,$zero` copy of the guard sum).
- P4. The link half-word fields are read as u16/s16 and written as one packed
  s32; byte-exact in both idioms, carries no residual.
- P5. The counter at `ctx+6` is a natural `s16` (movhi/extendhisi2 split);
  carries no residual.

### Facts

- [s45] Floor re-audited at 3 (127/127) on the HEAD src/ings.c:820 chassis; fake_ablate vacuous (no FAKE constructs ever); src/ings.c restored.
- [s45] OBJECT MODEL audit complete (above): ctx typed = byte-identical object (O1); typed records cost 7/37/43/47 (R30B/O2/O3/O4); no global symbol is referenced; the raw u8 * idiom is the matched sibling func_80017D84's idiom on the same object.
- [s45] Pass attribution for O2 vs R30B (+30): identical RTL through .lreg; global.c seats guard load and preheader load both in v1; reorg.c redundant_insn deletes the preheader load (insn 64 in .sched2, absent in .dbr). Not an object-model effect - a seat coincidence.
- [s45] Artifacts: tmp/grind/func_80017848/s45/{body_BASE.c, body_R30B.c, body_O1.c .. body_O4.c, obj_*.o, src_*.c, B_*.txt, D_*.txt, raw_*.txt, T.txt, iO2/, iR30B/, loads12.py, fake_ablate.txt, cells.ps1, post.sh, idump.sh, apply_win.py, norm.py, bank.py, ledger.py}.

- [s45] OBJECT MODEL: no global symbol is referenced; ctx is the 52-byte object block of matched sibling func_80017D84 (src/ings.c:824-843; census named_syms.txt:437 stride 0x34) - MATCHES, typed struct byte-identical (O1); ctx+6 s16 link counter - MATCHES (lh/lhu split = movhi vs extendhisi2 on a plain s16); ctx+0xC 0x40-byte record array (sibling func_80017A44 init loop addiu s0,s0,0x40; fields pos[3]/unkC[3]/f18/na/nb/la[8]/lb[8]) - declared raw u8 *, typed declaration MEASURED 7/37/43/47 (R30B/O2/O3/O4), raw idiom is the matched sibling's idiom on this object; ctx+0x10 0x10-byte link array (dist, u16 b, s16 a, dist3, owner; tail packs a/b as one sw) - MATCHES modulo spelling, no residual instruction touches a link field; math_Distance3D on record +0 - MATCHES; g_file_data_buf - NOT REFERENCED.

- [s45] Premises of the floor-3 argument (evidence.md s45 P1-P5): the residual is the two preheader copies and their consumers (E-s42-1) and object-model changes move seats/cse canonicals but never create a surviving copy; target reloads 0xC(s2) in every block (7 loads) so the source re-reads per block; the copy destination needs a byte-free reader live at flow (E-s44-3) which no natural typed re-read supplies (cse merges it into base); link half-word read/packed write and the s16 counter carry no residual.

- [s45] Pass attribution O2 vs R30B (+30): RTL identical through .lreg (loads 29/64/167/229/283/308 all survive); global.c seats insn 29 and 64 both in v1; reorg.c redundant_insn deletes insn 64 (present in .sched2, absent in .dbr). A seat coincidence, not an object-model effect.

- [s45] Floor re-audited at 3 (127/127) on the HEAD src/ings.c:820 chassis; fake_ablate vacuous; src/ings.c restored (git status shows only ledger files).

- [s45] Owner directive (2026-09-06 scratch-TU forensics of the preheader copy geometry) was executed in s37 and extended in s38-s44 per the ledger; acknowledged, not re-run.

### E-s46-1 - Floor re-audit (escalation / disposition session)
- candidate.c applied over the HEAD src/ings.c:820 INCLUDE_ASM anchor: sandbox
  --disable all = 3 at 127/127, scorable, rules_dropped 0
  (tmp/grind/func_80017848/s46/sandbox_base.txt). Floor flat at 3 since s9;
  45 prior sessions over 9 distinct modalities (recon 1, structural 10,
  permuter 4, forensics 10, rederive 7, synthesis 5, escalation 6, solver 1,
  object-model 1); permuter campaigns total >= 180,472 iterations (evidence.md
  s13/s14 telemetry); 267 rejected forms banked.
- src/ings.c restored to HEAD (git checkout) after measurement; no candidate
  change this session.
### E-s46-2 - Owner directive 2026-09-06 status
- Executed at s37 (H-s37-1, class kill, combine.c:1458) with artifacts still on
  disk in tmp/grind/func_80017848/s37/mini/ (run.sh reproduces the build with
  the exact CPP_FLAGS/CC_FLAGS; m4_iso_loop / m5_iso_noloop carry the promoted
  copy geometry into .cse2 and lose it in .combine). The dispatch audit's
  "DIRECTIVE NOT YET IN LEDGER" is a false positive; acknowledged here.
### E-s46-3 - Gate (a) scan_hand_coded re-run
- tier=LOW score=0/8, 127 insns, no S1/S2/S6 signal
  (tmp/grind/func_80017848/s46/scan_hand_coded.txt). Unchanged from s17.
### E-s46-4 - Gate (b) SOTN-master precedent census (negative)
- docs/reference/sotn-construct-index.md (2746 lines, commit aa53500, 1911 files)
  has 0 hits for register-asm pins / `__asm__` / `asm volatile` in PSX sources,
  and 0 hits for preheader / reg-reg copy / move-vs-load constructs. The
  residual (E-s42-1: target copies the loop-1 record-pointer LOAD into a3 in
  loop 2's preheader and reloads 0xC(s2) in loop 1's exit tail; BASE has the
  copy and the load swapped) has no closing C construct: the ledger's only
  three copy-survival routes are a second use (+1 insn, s17 escape #1), a
  block boundary (incompatible order, escape #8) or a byte-free flow-time
  reader (E-s44-3), and every byte-free reader measured is a fabricated dead
  conditional or an unused local (forbidden families), none of which SOTN
  master ships for GCC 2.7.2.
### E-s46-5 - Frontier item 3 scratch measurement
- f3_s32 / f3_s32_gt / f3_u8 under project flags (tmp/grind/func_80017848/s46/
  mini/): s32 holder keeps the compare through RTL (front end does not fold)
  but jump converts it to a store-flag before combine and it MATERIALISES as
  `nor`/`slt` in the final assembly; u8 holder is front-end folded to a
  constant (even the lbu disappears). No byte-free reader arises from this
  shape in isolation.

- [s46] E-s46-1: candidate.c over HEAD src/ings.c:820 = 3 at 127/127 scorable, rules_dropped 0 (tmp/grind/func_80017848/s46/sandbox_base.txt); src/ings.c restored to HEAD afterwards.

- [s46] E-s46-2: owner directive 2026-09-06 executed at s37 (H-s37-1 class kill, combine.c:1458); artifacts tmp/grind/func_80017848/s37/mini/ (run.sh, m0..m6, .cse2/.combine).

- [s46] E-s46-3: scan_hand_coded --single func_80017848 = tier LOW 0/8, no S1/S2/S6 (s46/scan_hand_coded.txt).

- [s46] E-s46-4: docs/reference/sotn-construct-index.md census: 0 PSX hits for register-asm/__asm__/asm volatile, 0 hits for preheader/reg-reg copy/move-vs-load constructs; gate (b) fails.

- [s46] E-s46-5: frontier item 3 scratch TUs: s32 holder compare survives to assembly as nor/slt; u8 holder front-end folded.

- [s46] Exhaustion: floor flat at 3 since s9 across 45 sessions and 9 modalities (structural 10, forensics 10, rederive 7, escalation 6, synthesis 5, permuter 4, recon/solver/object-model 1 each); >= 180,472 permuter iterations; 8 class + 53 instance kills; 267 rejected forms.

### E-s47-1 - Floor re-audit (solver session)
- candidate.c applied over the HEAD src/ings.c:820 INCLUDE_ASM anchor: sandbox
  --disable all = 3 at 127/127, scorable, rules_dropped 0
  (tmp/grind/func_80017848/s47/sb_BASE.txt). Floor flat at 3 since s9.
  src/ings.c restored with `git checkout` after every cell; tree clean.
### E-s47-2 - The residual's seat belongs to a LOCAL allocno (NEW)
- The divergent instruction is ours[60] `addu a0,a1,v0` vs tgt[60]
  `addu a0,a1,a3`; the value is reg 113, set by .lreg insn 162
  (set (reg:SI 113) (mem:SI (plus (reg/v:SI 72) (const_int 12)))) and consumed
  by insn 164 (set (reg/v:SI 81) (plus (reg/v:SI 85) (reg:SI 113))).
  local_extract.py places it at blk=13 qty 0, birth=2 death=6, refs=2, got=$v0.
  It is absent from global.c's 15-pseudo model (extract.py), which is why
  goal_from_tgt.py derives an EMPTY goal and reports the substitution as
  AMBIGUOUS across 33 $v0-holding pseudos. This is a DIFFERENT cause from
  s31's empty goal (cell E's merged p/links pseudo) and is a property of the
  BASE chassis. Artifacts: s47/goal_BASE.txt, s47/BASE_model.json,
  s47/local_extract.txt, s47/ings.local.json.
### E-s47-3 - inverse.py LOCAL returns a validated negative at depth 3 and 4
- inverse.py local ings.local.json --func func_80017848 --block 13
  --goal {"0": 7} --depth {3,4}: atom space 24 single perturbations over 6
  classes, bounds refs delta +4/-2 with birth/death enumerated exhaustively;
  NEGATIVE RESULT at both depths. Tool verdict verbatim: "the flip is not
  produced by refs / live span / birth order / conflicts / preferences /
  calls-crossed at all, so no C spelling that only moves those will ever close
  it." Artifacts: s47/inverse_local_d3.txt, s47/inverse_local_d4.txt.
- Forward fidelity on this chassis is exact (simulate.py: dispositions 15/15
  match, sort order MATCH), so the negative is about the FORM, not the model.
- COMPOSED CONCLUSION: reaching $a3 requires reg 113 to be a GLOBAL allocno
  live across loop 2, which is the same precondition as the missing
  `addu a3,a0,zero` copy (combine.c:1458 added_sets_2). The three-instruction
  residual is ONE fact - the byte-free flow-time reader of E-s44-3 - now
  typed by the solver instead of argued by hand.
### E-s47-4 - Owner sibling-transplant directive: executed and measured
- func_80017D84 (src/ings.c:824 on main) CONFIRMS and EXTENDS s45's object
  model: it constructs the block and writes *(s32*)(p+0xC) = c and
  *(s32*)(p+0x10) = c + (*(s16*)(p+4) << 6), i.e. the LINK array base at
  +0x10 is the RECORD array base at +0xC plus (record-count << 6). New fact,
  no zero-byte spelling: transplanted as cell D3 it costs sll+lh+addu where
  target has one `lw a2,16(s2)` -> 33 at 128.
- func_80016E60 (src/ings.c:436 on main, MATCHED) carries two sanctioned
  constructs with mechanism annotations - a pointer-alias pass-through local
  (combine i2/i3 merge leaving a copy at the LATER position) and a
  do { } while (0) wrap (flow.c loop-depth weighting -> global.c allocno
  priority). The do-while(0) transplant onto loop 2's preheader base
  assignment is cell D1 = 15 at 128. The pointer-alias transplant is the
  already-banked "named local for the loop-2 base addend", C-INERT since s11
  and re-confirmed by cell D2 = 3 at 127 this session. Neither lever can
  reach reg 113's seat because reg 113 is a LOCAL allocno and both annotated
  mechanisms act on global.c priority / combine position (E-s47-2/3).
- main (src/ings.c, COMPLETED-C) contains no loop-preheader copy geometry of
  this shape; nothing transplantable.
### E-s47-5 - Kill re-audit
- Q1 (s42/body_Q1.c, target's exact 127-instruction stream) = 14 at 127/127,
  unchanged (s47/sb_Q1.txt). fake_ablate on candidate.c: no FAKE-annotated
  constructs, nothing to ablate.

- [s47] Floor re-audit: candidate.c over the HEAD src/ings.c:820 INCLUDE_ASM anchor = 3 at 127/127, scorable, rules_dropped 0 (tmp/grind/func_80017848/s47/sb_BASE.txt). src/ings.c restored with git checkout after every cell; only metrics/events.jsonl and the new rejected/ files are dirty.

- [s47] NEW: the residual's only renamable seat is reg 113, a LOCAL allocno - .lreg insn 162 (set (reg:SI 113) (mem (plus (reg/v:SI 72) (const_int 12)))) feeding insn 164 (set (reg/v:SI 81) (plus (reg/v:SI 85) (reg:SI 113))); local_extract.py places it at blk 13 qty 0, birth 2, death 6, refs 2, got $v0. It is absent from global.c's 15-pseudo model, which is why goal_from_tgt.py derives an EMPTY goal and calls the substitution AMBIGUOUS across 33 $v0-holding pseudos. This is a different cause from s31's empty goal and is a property of the BASE chassis.

- [s47] inverse.py's LOCAL backend (local-alloc.c block_alloc model) returns a validated NEGATIVE for goal qty0 -> $a3 at depth 3 AND depth 4, atom space 24 single perturbations over 6 classes, bounds refs delta +4/-2 with birth/death enumerated exhaustively. Forward fidelity on this chassis is exact (simulate.py 15/15, sort order MATCH), so the negative is about the FORM, not the model.

- [s47] Composed conclusion: reaching $a3 for that value requires promoting reg 113 to a GLOBAL allocno live across loop 2, which is exactly the precondition the missing `addu a3,a0,zero` copy needs (combine.c:1458 added_sets_2). The three residual instructions are therefore ONE fact and it is the E-s44-3 byte-free-flow-time-reader wall - previously an argument, now a solver verdict.

- [s47] Owner directive executed: func_80017D84 (src/ings.c:824 on main) confirms s45's object model and adds a genuinely new relation - the link array base at ctx+0x10 equals the record array base at ctx+0xC plus (record-count << 6), because the constructor writes it that way. Transplanted (cell D3) it costs sll+lh+addu against target's single lw a2,16(s2): 33 at 128.

- [s47] Owner directive executed: func_80016E60 (src/ings.c:436 on main, MATCHED) contributes a do-while(0) wrap (cell D1 = 15 at 128) and a pointer-alias pass-through local (cell D2 = 3 at 127, byte-inert). Neither reaches reg 113's seat, and E-s47-2/3 says why: both annotated mechanisms act on global.c allocno priority or combine position, and reg 113 is a local allocno. main carries no comparable geometry.

- [s47] Kill re-audit: Q1 = 14 at 127/127 (unchanged since s34); fake_ablate finds no FAKE-annotated construct in candidate.c.

### E-s48-1 - Floor re-audit (forensics session)
- candidate.c applied over the HEAD src/ings.c:820 INCLUDE_ASM anchor: sandbox
  --disable all = 3 at 127/127, scorable, rules_dropped 0
  (tmp/grind/func_80017848/s48/sb_BASE.txt). Floor flat at 3 since s9.
  src/ings.c restored with `git checkout` after every cell; tree clean apart
  from metrics/events.jsonl and the new rejected/ file.

### E-s48-2 - FRONTIER ITEM 3 CLOSED: reg 113 carries NO suggestion at all, and
###             $a3 is neither suggested-away nor excluded - it simply loses an
###             ASCENDING regno scan to $v0
- s47's frontier item 3 asked whether local-alloc's SUGGESTED-REGISTER pass
  (qty_phys_copy_sugg / qty_phys_sugg) is what seats reg 113, and whether $a3 is
  even in find_free_reg's scan order at that point or is excluded by a live hard
  reg. Both halves are now answered from the instrumented cc1's BB2_SUGG_DEBUG
  stream (`local_extract.py ings --func func_80017848 --suggest`):
    blk=13 qty=0 reg1=113 birth=2 death=6 refs=2 size=1 mode=4 minclass=1
           altclass=0 calls=0 chgsize=0 ncopysugg=0 nsugg=0 copysugg=[] sugg=[]
  ncopysugg = nsugg = 0, so reg 113 has NO copy-suggested and NO arithmetic-
  suggested register; the suggested pass never runs for it (block_alloc only
  calls find_free_reg with just_try_suggested=1 for qtys that carry a
  suggestion). Correspondingly blk 13 appears ONLY on `main` lines in the QTYDBG
  stream and never on a `sugg` line, and there is exactly ONE SUGGDBG-FFR call
  for it, with jts=0.
- That single find_free_reg call scans:
    SUGGDBG-FFR qty=0 class=1 mode=4 jts=0 acc=0 born=2 dead=6
                used={0,1,26..67}  first_used={0,1,26..67}
  i.e. regnos 2..25 - every general-purpose register including $v0(2), $v1(3),
  $a0(4), $a1(5), $a2(6) and $a3(7) - are ALL FREE over the whole live span
  [2,6). $a3 is therefore in the scan order and is NOT excluded by any live hard
  reg, by the class mask, by an eliminable, or by a suggestion set.
- MECHANISM (read end to end this session, tools/gcc-2.7.2/local-alloc.c):
  find_free_reg builds `used` at local-alloc.c:2164-2190 as
  fixed_reg_set  U  (union over ins in [born,dead) of regs_live_at[ins])  U
  complement(reg_class_contents[class])  U  eliminables, then copies it to
  `first_used` at :2205 (unmodified when just_try_suggested is 0). The scan at
  local-alloc.c:2249-2255 is

      for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
    #ifdef REG_ALLOC_ORDER
        int regno = reg_alloc_order[i];
    #else
        int regno = i;
    #endif

  and the MIPS back end does NOT define REG_ALLOC_ORDER (grep over
  tools/gcc-2.7.2/config/mips/ returns nothing; the only REG_ALLOC_ORDER
  references in the tree are the #ifdef sites in global.c, local-alloc.c,
  regclass.c, reload1.c and stupid.c). So the scan is plain ASCENDING regno from
  0 and returns the FIRST free register - $v0, regno 2.
- CONSEQUENCE (the new, quantified precondition): for the LOCAL pass to seat this
  quantity in $a3 (regno 7), regnos 2,3,4,5,6 - $v0,$v1,$a0,$a1,$a2 - must ALL be
  set in `first_used` over [2,6), i.e. FIVE hard registers must be live across
  loop 2's preheader at that point. `regs_live_at` at local-alloc time contains
  only (a) hard regs referenced by the RTL itself and (b) post_mark_life marks
  from qtys ALREADY allocated in the same basic block; blk 13 contains exactly
  one quantity (ord=0), so route (b) contributes nothing and there is no call in
  the preheader to make route (a) contribute. This replaces s47's "the mechanism
  is outside the current model" with a named mechanism and a concrete price.
- Artifacts: tmp/grind/func_80017848/s48/local_extract_suggest.txt,
  s48/sugg_blk13.json, s48/ings.sugg.json.

### E-s48-3 - FRONTIER ITEM 2 MEASURED: a real second use DOES promote reg 113
###             out of local-alloc, but the promoted allocno takes $v1 and cse
###             deletes target's tail reload - 17 at 126
- Cell A (s48/body_A.c, banked as
  rejected/s48_l2_addend_named_local_reused_as_distance3d_arg_promotes_reg113_but_cse_deletes_tail_reload_costs_17.c)
  is exactly the probe s47's frontier item 2 named: loop 2's base addend becomes
  a named local `recs2 = *(u8 **)(ctx + 0xC);` placed UNCONDITIONALLY before loop
  2's guard (so it is defined on the skip path), used inside the guard as
  `base = (u8 *)(sh2 + (s32)recs2)`, and whose ONLY extra use is
  math_Distance3D's first argument. sandbox --disable all = 17 at 126 build
  insns / 127 target insns (s48/sb_A.txt) - ONE INSTRUCTION SHORT.
- The promotion half WORKS and is now proven, not argued: re-running
  local_extract.py on cell A, block 13 has DISAPPEARED ENTIRELY from the
  local-alloc quantity table (s48/local_A.txt shows blk 9,10,12,14,15,16 and no
  blk 13). The value is no longer a local quantity; it is a global allocno. This
  is the precondition E-s47-3 named, and ordinary C reaches it.
- The seat half FAILS, and differently from E-s47-3's prediction. objdump of the
  cell-A object shows the promoted value in $v1 (`lw v1,12(s2)` hoisted above
  loop 1, then `addu v0,a1,v1` reused in six later blocks), not in $a3, and cse
  then propagates it into every subsequent ctx+0xC consumer - which is why the
  body comes out at 126 instructions instead of 127. E-s47-3 predicted $a3 as
  "first eligible in its pass-0 conflict set"; that prediction was made for a
  short allocno whose range ends just after loop 2, whereas any allocno created
  by a REAL second use necessarily reaches that use, and the resulting long range
  gets a different conflict set and a different seat.
- COMPOSED CONCLUSION (supersedes the loose reading of E-s47-3): promoting reg
  113 to a global allocno is NECESSARY but NOT SUFFICIENT for target's $a3 seat,
  and it is not free. Every C-level second use of the loop-2 preheader load is
  SUBSTITUTIVE rather than additive - it does not add an instruction, it lets
  cse delete one of the reloads target performs in its tail (target re-reads
  0xC(s2) at the math_Distance3D args and again at rec_a/rec_b). The banked s9
  T-series priced this at 19 through the candidate's `p` across a JOIN; re-priced
  on the loop-2 addend as frontier item 2 asked, it is 17 at 126. The residual is
  still the E-s44-3 byte-free-flow-time-reader requirement, but the requirement
  is now sharper: the reader must promote the load's value WITHOUT giving cse a
  register equivalence it can forward to the tail reloads.

- [s48] E-s48-1: candidate.c over the HEAD src/ings.c:820 anchor = 3 at 127/127, scorable, rules_dropped 0 (tmp/grind/func_80017848/s48/sb_BASE.txt); src restored after every cell.
- [s48] E-s48-2: frontier item 3 closed with the BB2_SUGG_DEBUG dump. reg 113 (blk 13 qty 0) has ncopysugg=0 nsugg=0 - no suggestion of any kind - so the suggested-register pass never runs for it. Its single find_free_reg call (jts=0, born=2, dead=6) scans used=first_used={0,1,26..67}: every GPR 2..25 is free, so $a3 is in the scan order and is NOT excluded. It loses because MIPS defines no REG_ALLOC_ORDER and local-alloc.c:2249-2255 therefore scans ascending regno and returns the first free one, $v0. To seat $a3 locally, regnos 2..6 ($v0,$v1,$a0,$a1,$a2) must all be live over [2,6) - five hard registers across loop 2's preheader - and blk 13 has only one quantity and no call, so neither post_mark_life nor RTL hard-reg references can supply them.
- [s48] E-s48-3: frontier item 2 measured as cell A (named loop-2 base addend whose only extra use is math_Distance3D's first argument, read unconditionally before loop 2's guard) = 17 at 126/127. The promotion works and is dump-proven: blk 13 vanishes from local_extract.py's quantity table, so the value became a global allocno. The seat does not: objdump shows the promoted value in $v1 hoisted above loop 1, and cse forwards it into every later ctx+0xC consumer, deleting one of the reloads target performs in its tail. Every C-level second use of that load is substitutive, not additive.

- [s48] Floor re-audit: candidate.c over the HEAD src/ings.c:820 INCLUDE_ASM anchor = 3 at 127/127, scorable, rules_dropped 0 (tmp/grind/func_80017848/s48/sb_BASE.txt). src/ings.c restored with git checkout after every cell; the only dirt at end of session is metrics/events.jsonl and the new rejected/ file.

- [s48] reg 113 (the loop-2 preheader load of ctx+0xC, .lreg insn 162) is blk 13 qty 0 with ncopysugg=0 and nsugg=0 - it carries NO copy-suggested and NO arithmetic-suggested register, so local-alloc's suggested-register pass never runs for it. It appears only on QTYDBG 'main' lines, never on a 'sugg' line, and generates exactly one SUGGDBG-FFR call, with just_try_suggested=0.

- [s48] That find_free_reg call scans used = first_used = {0,1,26..67} over born=2 dead=6: regnos 2..25 - every GPR including $v0, $v1, $a0, $a1, $a2 and $a3 - are all free. $a3 is in the scan order and is excluded by nothing.

- [s48] The MIPS back end defines no REG_ALLOC_ORDER, so the scan at tools/gcc-2.7.2/local-alloc.c:2249-2255 walks regno ascending from 0 and returns the first free register - $v0, regno 2. The $v0 seat is a scan artefact, not a preference.

- [s48] New quantified precondition for a LOCAL-pass $a3 seat: regnos 2,3,4,5,6 must all be set in first_used over [2,6), i.e. five hard registers live across loop 2's preheader. find_free_reg builds that set at local-alloc.c:2164-2190 from fixed_reg_set, the union of regs_live_at over the span, the class complement and the eliminables; blk 13 holds one quantity so post_mark_life of an earlier-allocated qty contributes nothing, and there is no call in the preheader so no RTL hard-reg reference contributes either.

- [s48] Frontier item 2 measured as cell A = 17 at 126 build insns / 127 target insns (s48/sb_A.txt). The promotion half is dump-proven: blk 13 vanishes from local_extract.py's quantity table on cell A's source (s48/local_A.txt), so a real second use does convert reg 113 from a local quantity into a global allocno.

- [s48] The seat half fails: objdump shows cell A's promoted value in $v1, hoisted above loop 1, not in $a3 - E-s47-3's $a3 prediction holds only for a short allocno ending just after loop 2, and no real second use produces such a range.

- [s48] Every C-level second use of the loop-2 preheader load is SUBSTITUTIVE rather than additive: it does not add an instruction, it lets cse forward the register equivalence into target's tail re-reads of 0xC(s2) and delete one of them, which is why cell A is one instruction SHORT of target rather than one long.

### E-s49-1 - Floor re-audit + closest-kill re-audit (forensics session)
- `memory/grind/func_80017848/candidate.c` applied over the HEAD
  `src/ings.c:820` INCLUDE_ASM anchor: `sandbox --disable all` = **3 at
  127/127**, scorable, rules_dropped 0 (`s49/sb_BASE.txt`). Floor flat at 3
  since s9; the chassis has not moved since s48.
- KILL RE-AUDIT of the newest / structurally closest instance kill (s48 cell A,
  the named loop-2 base addend whose only extra use is math_Distance3D's first
  argument): re-measured on this chassis = **17 at 126/127** (`s49/sb_A.txt`),
  identical to s48. `tools/fake_ablate.py --func func_80017848 --file ings
  --candidate s49/body_A.c` reports "no FAKE-annotated constructs found;
  nothing to ablate", so the kill was not measured under a FAKE carrier. The
  kill stands unchanged.
- src/ings.c was restored with `git checkout` after every cell; end-of-session
  dirt is metrics/events.jsonl plus the new rejected/ file.

### E-s49-2 - FRONTIER ITEM 3 CLOSED: reload NEVER revisits an allocation in
###             func_80017848 - zero spills, zero retry_global_alloc
- s48's frontier item 3 named reload spill-retry as the one remaining
  unmodelled surface behind s47's inverse.py negative. Measured with the
  instrumented cc1's BB2_RELOAD_DEBUG stream over the whole TU
  (`bash tools/ra_solver/reload_harvest.sh ings`, run under WSL - the Windows
  Git-Bash shell cannot exec tools/gcc-2.7.2/cc1 ("Exec format error") and
  silently produces an empty log, which is how this probe first appeared to
  "pass" with retries=0; always run it through `bash tools/wsl.sh`):
    RELOADDBG order func=func_80017848 prr=8,9,10,...
    RELOADDBG  uses=8:0,9:0,...,7:238,6:241,5:244,4:257,2:333,...
    RELOADDBG  bad_spill_regs: 0 1 2 4 5 6 7 26 27 28 29 30 31
    RELOADDBG needs func=func_80017848 pass=1 new_bb_needs=0 changed=0
  and NOTHING else. For func_80017848 the stream carries **zero**
  `new_spill_reg`, **zero** `spill_hard_reg`, **zero** `kickout` and **zero**
  `RETRYDBG` (retry_global_alloc) events; reload converges on pass 1 with
  `changed=0`. The only spill_hard_reg events in the entire ings TU belong to
  disp_CalcFov and func_80016A8C.
- CONSEQUENCE: reload cannot be the mechanism that seats (or could re-seat) the
  loop-2 preheader value. Composed with E-s48-2 (reg 113 carries no copy- and
  no arithmetic-suggestion, so the suggested-register pass never runs for it)
  and its SUGGDBG-QTY line (size=1, mode=4 SImode, chgsize=0, so qty_size is
  not a factor), **all three candidates s47's inverse.py negative listed as
  "unmodelled" are now individually eliminated**: the solver's negative result
  for reg 113 -> $a3 is complete on this chassis, not a modelling gap.
- Artifacts: `s49/ings_BASE.reload.log`, `s49/reload_func80017848.txt`.

### E-s49-3 - FRONTIER ITEM 2 PRICED AND KILLED: the ascending-scan seat model
###             is calibrated INSIDE this function, and a LOCAL-pass $a3 seat
###             costs five extra definitions in a two-instruction block
- s48 left the local-pass route open as "five hard registers live across loop
  2's preheader is the whole lever". The per-quantity `got` column of
  `local_extract.py ings --func func_80017848` (`s49/local_extract_BASE.txt`)
  calibrates the walk without building a cell, in this very function, in the
  tail block 16:
    reg142 (birth 36 death 40)  0 earlier-allocated overlapping qtys -> got 2 ($v0)
    reg140 (birth 28 death 32)  1 (reg88, 16-30)                     -> got 3 ($v1)
    reg136 (birth 18 death 24)  1 (reg88, 16-30)                     -> got 3 ($v1)
    reg76  (birth 20 death 40)  2 (reg88 at $v0; reg140/reg136 at $v1,
                                   so used = {2,3})                  -> got 4 ($a0)
  i.e. the seat advances EXACTLY one regno per already-allocated overlapping
  quantity in the same block, which is the ascending scan of
  local-alloc.c:2249-2255 over the `used` set built from post_mark_life at
  local-alloc.c:2164-2190. (reg146, live across the math_Distance3D call, gets
  $s0 = 16 instead - the call-clobbered exclusion; reg128 gets $a1 through the
  suggested pass, not the scan.)
- Applying the calibrated walk to blk 13: reg 113 sits at `got=2` with ONE
  quantity in the block, so reaching $a3 (regno 7) needs FIVE earlier-allocated
  overlapping quantities there. A block-local quantity exists only if some
  instruction IN blk 13 defines it and nothing outside the block reads it -
  a value used in loop 2's body or in the tail spans blocks and becomes a
  global allocno instead, which is precisely what E-s48-3 measured (cell A's
  second use made blk 13 vanish from the local table entirely).
- TARGET'S LOOP-2 PREHEADER CONTAINS EXACTLY TWO ARITHMETIC INSTRUCTIONS
  (`s49/T.txt` lines 59-61: `addu a3,a0,zero` / `lw a2,16(s2)` /
  `addu a0,a1,a3`, the middle load belonging to the links pointer), and BASE
  emits the same two (`lw v0,12(s2)` / `addu a0,a1,v0`). Five additional
  block-local definitions therefore price at **+5 instructions over target's
  127** before any seat is even considered. The LOCAL-pass route to $a3 is
  priced dead on this chassis; the only route left is promotion to a global
  allocno, which E-s48-3 measured at $v1 / 126 insns.
- Artifacts: `s49/local_extract_BASE.txt`, `s49/T.txt`.

### E-s49-4 - NEW: cell C is the FIRST form that keeps 127 instructions, promotes
###             reg 113 out of local-alloc AND materialises a surviving copy in
###             loop 2's preheader - but the copy is of BASE, not of the record
###             pointer (5 at 127/127)
- Cell C (`s49/body_C.c`, banked as
  `rejected/s49_l2_exit_test_via_named_addend_buys_a_preheader_copy_of_base_not_recordptr_costs_5.c`)
  is frontier item 1's own next_probe, spelled exactly as the frontier asked:
  a named local `recs2 = *(u8 **)(ctx + 0xC);` read unconditionally before loop
  2's guard, used as loop 2's base addend, and given a SECOND use that is
  consumed strictly inside loop 2 and dead before the link stores - the loop's
  exit test written `while (i < *(s32 *)(sh2 + (s32)recs2 + 0x20));` instead of
  `while (i < *(s32 *)(base + 0x20));`. Guard, tail and everything else are BASE.
- MEASURED: `sandbox --disable all` = **5 at 127/127** (`s49/sb_C.txt`) - two
  score points worse than BASE but, unlike every previous promoting form, NOT
  short an instruction.
- The promotion is dump-proven: `local_extract.py` on cell C's source lists
  blocks 2,3,7,9,10,12,14,15,16 and **no blk 13** (`s49/local_C.txt`), so the
  loop-2 preheader value left the local-alloc quantity table exactly as in cell
  A - but here WITHOUT cse deleting a tail reload, because the second use is an
  address inside loop 2 that cse folds back to `base` rather than a fresh
  ctx+0xC consumer in the tail.
- What the extra score bought (normalised diff `s49/T.txt` vs `s49/B_C.txt`):
  the recs2 load lands in the slot target fills with `sll zero,zero,0`
  (so it is instruction-free), and the loop-2 preheader now emits
      addu a0,a1,a0      ; base = sh2 + recs2
      lw   a2,16(s2)
      addu a1,a0,zero    ; COPY, kept alive by the exit test's out-of-block use
  against target's
      addu a3,a0,zero    ; COPY of the record pointer
      lw   a2,16(s2)
      addu a0,a1,a3      ; base = sh2 + copy
  i.e. a REAL surviving copy in the right block, in the wrong order and of the
  wrong value, plus the consequent `lw v0,32(a1)` vs `lw v0,32(a0)` in the exit
  test. Residual (a) (loop-1 exit tail `addu a0,a3,zero` vs target's
  `lw a0,12(s2)`) is untouched.
- MECHANISM (named, and it is the same one BASE already uses for loop 1): a copy
  survives combine here because its destination has an OUT-OF-BLOCK use - the
  `can_combine_p` escape s17 enumerated as the out-of-block-use path - which is
  exactly how BASE buys loop 1's `addu a3,a0,zero` through the `p = q;` read in
  the exit tail (E-s42-4). Cell C proves the same device works for loop 2 at
  ZERO instruction cost, whereas loop 1's version costs the tail move that IS
  residual (a).
- CONSEQUENCE / sharpened frontier: the device makes a copy of whichever pseudo
  carries the out-of-block use. Target's copy is of the RECORD POINTER, used
  ONCE in-block by the base add, with no later reader anywhere in the function
  (T.txt line 59 defines a3, line 61 is its only reader). Every non-foldable
  out-of-block use of the record pointer that C can spell lives after the loop
  nest, and E-s48-3 measured that class as substitutive (it deletes one of
  target's seven ctx+0xC reloads, 126 insns); every use INSIDE loop 2 is an
  address that cse folds to `base`, which is why cell C copies base instead.
  The open question is no longer "can a copy survive in loop 2's preheader" -
  it is what keeps TARGET's record-pointer pseudo live past the base add when
  it has no reader at all.

- [s49] E-s49-1: BASE re-audited at 3 (127/127) on the HEAD chassis; s48 cell A re-audited at 17 (126/127) with fake_ablate reporting no FAKE-annotated constructs, so that kill stands.
- [s49] E-s49-2: BB2_RELOAD_DEBUG over the ings TU shows func_80017848 with zero new_spill_reg, zero spill_hard_reg, zero kickout and zero retry_global_alloc events, converging on reload pass 1 with changed=0. Reload spill-retry - the last surface s47's inverse.py negative listed as unmodelled - is eliminated; the solver negative for reg 113 -> $a3 is complete on this chassis.
- [s49] E-s49-2 tooling note: tools/ra_solver/reload_harvest.sh (and anything else invoking tools/gcc-2.7.2/cc1) MUST be run through `bash tools/wsl.sh '...'`; the Windows Git-Bash shell reports "Exec format error" on the Linux cc1 and writes an EMPTY log whose retries=0 looks like a real negative.
- [s49] E-s49-3: local_extract.py's got column calibrates the ascending-scan seat model inside func_80017848 itself - blk 16 shows 0/1/1/2 earlier-allocated overlapping quantities mapping to $v0/$v1/$v1/$a0 for reg142/reg140/reg136/reg76. reg 113 in blk 13 therefore needs FIVE earlier-allocated overlapping quantities to reach $a3, each requiring its own defining instruction in a preheader that target builds from exactly two instructions: the local-pass route prices at +5 insns over 127.
- [s49] E-s49-4: cell C (loop-2 exit test re-reading the count through the named base addend `recs2` instead of through `base`) = 5 at 127/127 - the first form that both promotes the preheader value out of local-alloc (blk 13 gone from the quantity table) and keeps target's instruction count. Its recs2 load fills target's nop slot and it materialises a REAL surviving preheader copy, but of `base` (`addu a1,a0,zero` after the add) rather than of the record pointer (`addu a3,a0,zero` before it), because cse folds every in-loop use of recs2 back to base.
- [s49] The copy-survival device is now named and symmetric: a preheader copy survives combine when its destination carries an OUT-OF-BLOCK use. BASE already buys loop 1's copy that way through `p = q;` in the exit tail - which is exactly what costs residual (a) - and cell C buys loop 2's copy the same way at zero instruction cost. The device copies whichever pseudo carries the out-of-block use, and target's copy is of a pseudo with NO reader after the base add.

- [s49] BASE (memory/grind/func_80017848/candidate.c over the HEAD src/ings.c:820 INCLUDE_ASM anchor) = 3 at 127/127, scorable, rules_dropped 0 (s49/sb_BASE.txt); src/ings.c restored with git checkout after every cell.

- [s49] The s48 cell-A kill re-audits identically on this chassis (17 at 126/127, s49/sb_A.txt) and fake_ablate finds no FAKE-annotated construct in it, so the kill was not measured under a FAKE carrier.

- [s49] BB2_RELOAD_DEBUG over the ings TU: func_80017848 carries zero new_spill_reg, zero spill_hard_reg, zero kickout and zero retry_global_alloc events and converges on reload pass 1 with changed=0. Reload never revisits an allocation in this function.

- [s49] With E-s48-2 (no copy- or arithmetic-suggestion for reg 113, qty size=1 mode=4 chgsize=0), all three surfaces s47's inverse.py negative called unmodelled are now individually eliminated - the solver negative for reg 113 -> $a3 is complete on this chassis.

- [s49] TOOLING: tools/ra_solver/reload_harvest.sh (and anything invoking tools/gcc-2.7.2/cc1) must be run through bash tools/wsl.sh; Windows Git-Bash reports 'Exec format error' on the Linux cc1 and writes an EMPTY log whose retries=0 looks like a real negative.

- [s49] local_extract.py's got column calibrates the ascending-scan seat model inside func_80017848 itself: in blk 16, reg142 with 0 earlier-allocated overlapping qtys gets $v0, reg140 and reg136 with 1 each get $v1, and reg76 with 2 distinct earlier seats gets $a0 - one regno per earlier-allocated overlapping quantity.

- [s49] Reg 113 (blk 13, one quantity) therefore needs FIVE earlier-allocated overlapping quantities to reach $a3, each requiring its own defining instruction in a preheader that target builds from exactly two arithmetic instructions: the LOCAL-pass route prices at +5 insns over target's 127.

- [s49] Cell C (loop-2 exit test re-reading the count through the named base addend recs2 instead of through base) = 5 at 127/127 - the first form that both promotes the preheader value out of local-alloc (blk 13 absent from the quantity table, s49/local_C.txt) and keeps target's instruction count.

- [s49] Cell C's recs2 load fills the slot target fills with sll zero,zero,0 (instruction-free) and materialises a REAL surviving preheader copy, addu a1,a0,zero after the base add, against target's addu a3,a0,zero before it.

- [s49] The copy-survival device is now named and symmetric: a preheader copy survives combine when its destination carries an OUT-OF-BLOCK use. BASE already buys loop 1's copy that way through p = q; in the exit tail - which is exactly what costs residual (a) - and cell C buys loop 2's copy the same way at zero instruction cost.

- [s49] The device copies whichever pseudo carries the out-of-block use. cse folds every in-loop use of the record-pointer local back to base, so in-loop readers yield a copy of base; every non-foldable reader of the record pointer lives after the loop nest and is substitutive (E-s48-3, 126 insns). Target's copy is of a pseudo (T.txt line 59 -> line 61) with no reader at all after the base add.

## s50 (2026-09-07, rederive) — the 3-insn structural residual, pinned and then solved on a new chassis

Chassis re-audit: `sandbox func_80017848 --disable all` with `candidate.c` applied at the
`src/ings.c:820` INCLUDE_ASM anchor = **3** (127 target insns / 127 build insns, scorable).
Unchanged from s42–s49.

**E-s50-1 — the residual is exactly three instructions and they are ONE swap.**
Normalised target-vs-build diff (`tmp/grind/func_80017848/s50/{T.txt,B_C.txt}`, produced by
`s50/dis.sh` + `s50/norm.py`; norm.py's branch-target rule was fixed this session to map
decimal objdump targets to LBL) shows the ONLY divergence is at the loop-1 exit tail and
loop-2 preheader:

| position | target | candidate.c |
|---|---|---|
| loop-1 exit tail | `lw a0,12(s2)` (reload of ctx+0xC) | `addu a0,a3,zero` (move, from `p = q;`) |
| loop-2 preheader | `addu a3,a0,zero` (copy of p) | `lw v0,12(s2)` (fresh load) |
| loop-2 preheader | `addu a0,a1,a3` | `addu a0,a1,v0` |

Everything else — prologue, both guards, both loop bodies, the join geometry, the whole
math_Distance3D/link/store tail — is byte-identical. The target's loop-1 and loop-2
preheaders are literally the same five instructions modulo the record offset
(`addu v1,zero,zero / addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3 / addu v0,a0,v1`),
so the closing form must produce the SAME copy device twice.

**E-s50-2 — residual (a) is independently winnable and costs exactly loop-1's copy (cell F1).**
F1 = `candidate.c` with the loop-1 exit-tail `p = q;` replaced by `p = *(u8 **)(ctx + 0xC);`
measures **4 at 126/127**. The tail becomes target's `lw a0,12(s2)` (exact), and loop-1's
preheader copy disappears (`addu a0,a1,a0` in place of `addu a3,a0,zero` + `addu a0,a1,a3`).
This is the first direct proof that the tail move and loop-1's copy are a SINGLE purchase:
`p = q;` is `q`'s only out-of-block reader, and it is what keeps `q` live across loop 1.
Banked `rejected/s50_tail_reload_only_loop1_copy_dies_costs_4.c`.

**E-s50-3 — the loop exit test as the copy buyer (cells E1/E2) produces copies but in the
wrong direction.** E1 (both loops: `q = *(u8**)(ctx+0xC)` in the preheader + exit test
rewritten to `while (i < *(s32 *)(sh + (s32)q + OFF));` + tail reload) = **10 at 128/127**;
E2 (same with `q = p;`) = **12 at 126/127**. E1 confirms the device does keep a use-once
preheader copy alive in BOTH loops and wins residual (a), but cse gives the exit-test
expression its OWN pseudo and the surviving copy runs base→temp (`addu a1,a1,a0` /
`addu a0,a1,zero`, exit test on the copy) instead of target's pointer→base
(`addu a3,a0,zero` / `addu a0,a1,a3`, exit test on base). E2 additionally lets cse fold
loop-2's base add into the guard add (`addu a0,v0,a0` in the guard block), collapsing the
preheader to a bare copy.

**E-s50-4 — THE NEW MECHANISM (cell M1): the join is a cse EBB boundary, so two same-valued
pointer pseudos can coexist there, and loop-2's base add is a FREE out-of-block reader for
loop-1's copy.** M1 = F1 plus (i) `q = p;` seeded before the loop-1 guard so `q` is defined on
the skip path, (ii) loop-1's preheader keeps `q = *(u8 **)(ctx + 0xC);` (cse turns it into the
copy), (iii) loop-2's GUARD reads the reloaded `p` while loop-2's BASE ADD reads `q`.
M1 measures **14 at 126/127** and, critically, **loop-1's preheader copy `addu a3,a0,zero`
and the loop-1 exit-tail `lw a0,12(s2)` BOTH byte-match the target**. The only structural
miss is loop-2's own copy (126 vs 127 insns); the rest of the 14 points is a seat rotation
(sh lands in a2 and lnk in a1, target has a1/a2). The mechanism is that the join block has
two predecessors, so cse's value table is empty there and it never proves `q == p`; the
guard's `sh2 + p` and the preheader's `sh2 + q` therefore stay two distinct adds — which is
exactly the target's shape (`addu v0,a1,a0` in the guard, `addu a0,a1,a3` in the preheader,
never shared). Banked `rejected/s50_q_carried_across_join_loop2_base_from_q_costs_14.c`;
the q-undefined-on-skip-path variant M1c = 12 at 126 (`q` seats in t0),
`rejected/s50_q_undefined_on_skip_path_variant_costs_12.c`.

**E-s50-5 — cell M2 is the first form that carries BOTH preheader copies and the tail reload
at 127/127.** M2 = M1 plus a second copy `r = q;` in loop-2's preheader, with `base` built
from `r` and `r` re-used as math_Distance3D's first argument (the free out-of-block reader
that buys loop-2's copy, paid for by dropping one of the two uses of the post-loop reload).
M2 measures **17 at 127/127**. Its emitted geometry:

    loop1: addu a3,a0,zero / lw a1,16(s2) / addu a0,a2,a3      (target: a3,a0 / lw a2 / a0,a1,a3)
    tail:  lw a0,12(s2)                                        (target: identical)
    loop2: lw a1,16(s2) / addu t0,a3,zero / addu a0,a2,t0       (target: addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3)
    call:  sll s0 / sll s1 / lw a1,12(s2) / addu a0,t0,s0       (target: sll s0 / lw a1,12(s2) / sll s1 / addu a0,a1,s0)

Every remaining difference is a REGISTER SEAT or a copy SOURCE, not a missing/extra
instruction: (1) sh and lnk are rotated (a2/a1 instead of a1/a2), (2) loop-2's copy is
sourced from `q`/t0 rather than `p`/a3, (3) the call's first argument comes from `r` rather
than the reload. Banked
`rejected/s50_STRUCTURALLY_EXACT_127of127_both_copies_seat_residual_costs_17.c`.

**E-s50-6 — the M2 sub-cells that lose the count.** M3 = M2 with loop-2's copy sourced from
`p` instead of `q` (`r = p;`) = **16 at 126/127** — sourcing from the reload lets GCC drop the
post-loop reload entirely. M4 = M2 with `r = q;` moved BEFORE the `lnk` load (to chase the
target's copy-then-`lw a2` order) = **17 at 127/127**, byte-identical score to M2: sched
reorders the pair regardless, so statement order is not the lever for that pair.
Banked `rejected/s50_loop2_copy_from_p_drops_call_reload_costs_16.c` and
`rejected/s50_M2_copy_before_lnk_load_sched_reorders_costs_17.c`.

**E-s50-7 — owner directive (completed in-TU siblings) executed.** Read the on-main bodies of
`func_80017D84` (src/ings.c:824-843), `func_80016E60` (src/ings.c:436+) and `main`
(src/ings.c:576+). Transplantable spellings found and used this session: `main`'s
read-a-local-before-any-assignment idiom (`func_80016A8C(..., env, idx)` with `env`/`idx`
never assigned) is the precedent for cell M1c's skip-path-undefined `q`; `func_80017D84`
confirms the 52-byte object model already banked at s45 (s16 count at +6, `c` at +0xC,
`c + (count << 6)` at +0x10) and uses a pointer-increment `for` with a `break` rather than an
indexed loop — that geometry does NOT fit this function (the target recomputes `base + i`
in the branch delay slot every iteration, which the indexed form already reproduces exactly).
`func_80016E60` contributes only FAKE-annotated do-while(0)/pointer-alias levers, both of
which s43 already measured on this function (X1/X2/X3/X4, all 6 or 10). No further
transplant surface in the TU.

- [s50] BASE re-audit: candidate.c at the src/ings.c:820 anchor measures 3 at 127/127 on the HEAD chassis, unchanged from s42-s49.

- [s50] The 3-insn residual is now pinned exactly (s50/T.txt vs s50/B_C.txt): loop-1 exit tail target `lw a0,12(s2)` vs build `addu a0,a3,zero`; loop-2 preheader target `addu a3,a0,zero` + `addu a0,a1,a3` vs build `lw v0,12(s2)` + `addu a0,a1,v0`. Nothing else in the function differs.

- [s50] The target's loop-1 and loop-2 preheaders are the SAME five instructions modulo the record offset (`addu v1,zero,zero / addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3 / addu v0,a0,v1`), so the closing form must produce the same copy device twice.

- [s50] F1 (tail reload only) = 4 at 126: wins residual (a) outright and costs exactly loop-1's copy - the tail move and loop-1's copy are one purchase.

- [s50] M1 (q carried across the cse EBB join, loop-2 guard on p / base on q) = 14 at 126, with loop-1's copy and the loop-1 exit-tail reload BOTH byte-matching the target.

- [s50] M2 (M1 + `r = q;` in loop-2's preheader read by math_Distance3D's first argument) = 17 at 127/127 - both preheader copies and the tail reload present simultaneously for the first time; the entire remaining residual is register seats plus two copy sources.

- [s50] The seat rotation in M1/M2 is sh in a2 and lnk in a1 where the target has a1/a2; BASE already seats sh in a1, so the rotation is caused by the added q/r allocnos, not by the loop bodies.

- [s50] Owner directive executed: the three completed in-TU siblings were read on main. func_80017D84 (src/ings.c:824-843) confirms the s45 52-byte object model and uses a pointer-increment for-with-break geometry that does NOT fit this target (which recomputes `base + i` in the branch delay slot each iteration). main (src/ings.c:576) supplies the read-a-never-assigned-local idiom used by cell M1c. func_80016E60 (src/ings.c:436) contributes only FAKE do-while(0) / pointer-alias levers, all already measured on this function at s43 (X1-X4 = 6 or 10). No further transplant surface in the TU.

- [s50] tools/grinder/dump.ps1 was not needed this session: the pass attribution (cse EBB boundary at the join) was established directly from the emitted stream, where the target's guard add and preheader add of the same value are provably unshared.

## s51 (2026-09-07, rederive)

- [s51] BASE re-audit: `memory/grind/func_80017848/candidate.c` at the HEAD src/ings.c:820
  anchor measures **3 at 127/127**, unchanged from s42-s50. `tools/fake_ablate.py --func
  func_80017848 --file ings --candidate memory/grind/func_80017848/candidate.c` reports
  "no FAKE-annotated constructs found; nothing to ablate" — the banked floor is carried by
  ordinary C with no FAKE carrier occupying any pseudo, so the s42-s50 instance kills were
  not measured under a FAKE mask. Mandatory kill re-audit satisfied.

- [s51] Closest-form re-audit: cell **M2** (s50's structurally-exact 127/127 form) re-measures
  **17 at 127/127** on the HEAD chassis, exactly as s50 recorded. The kill stands.

**E-s51-1 — THE SESSION'S RESULT: M2's 17 was carrying a self-inflicted 7 points. Cell M7 =
M2 with loop-2's redundant `lnk` local deleted measures 10 at 127/127.** s50's M2 added an
explicit `lnk = *(u8 **)(ctx + 0x10);` local to loop 2's preheader, which BASE never had
(BASE reads `*(u8 **)(ctx + 0x10)` inline in loop 2's body). That extra local lengthens lnk's
live range, lifts its global.c priority above `sh`, and rotates the a1/a2 seats in BOTH loops.
Deleting it and restoring BASE's inline read drops the score from **17 to 10** with the
structure untouched: M7 still carries both preheader copies and the loop-1 exit-tail reload at
127/127. Banked `rejected/s51_M7_loop2_lnk_local_removed_seat_and_call_residual_costs_10.c`.
This is the first movement on the M-branch since it opened and it retires frontier item 1 of
s50 ("why does sh seat in a2 and lnk in a1") with a one-line C answer: it was the lnk local,
not the added q/r allocnos.

**E-s51-2 — the 10 is a TWO-WAY TRADE-OFF, and both halves are 2 points each.** M7 (with the
pre-guard `q = p;`) and M8 (= M7 with `q = p;` deleted, so q/r are undefined on their skip
paths) both measure **10 at 127/127**, but their residuals are DISJOINT:
  * M7/M19 residual: loop-1 preheader is byte-exact (`addu a3,a0,zero / lw a2,16(s2) /
    addu a0,a1,a3` — correct a3 seat AND correct order), but reorg duplicates
    `addu v1,zero,zero` (i = 0) into the loop-1 guard's `bltz` delay slot where the target
    has a nop, and emits a second copy of it ahead of loop-1's preheader (2 points).
  * M8 residual: the `i = 0` placement is BYTE-EXACT in both loops (target's `blez` delay
    slot), but the two copy destinations seat in **t0/t1 instead of a3** and loop-1's
    `lw a2,16(s2)` is emitted before the copy instead of between it and the base add
    (2 points).
The mechanism for M8's t0/t1 is that with `q = p;` deleted, `q` (and `r`) have no definition on
their loop-skip paths, so flow treats them as live-in at function entry, where they conflict
with the incoming parameter hard registers a0-a3 — leaving t0 as the first free seat in the
allocation order. The pre-guard `q = p;` removes that entry-liveness and buys the a3 seat, but
it also changes the block that precedes the loop-1 guard, which is what lets reorg fill the
`bltz` delay slot. Banked
`rejected/s51_M8_no_preguard_q_i0_slot_correct_copies_seat_t0t1_costs_10.c`.

**E-s51-3 — the trade-off is not a statement-placement artefact.** Five placements of the two
statements were measured and every one that keeps the pre-guard `q = p;` reproduces M7's
residual EXACTLY, and every one that drops it reproduces M8's: M19 (`i = 0;` moved to sit
between the loop-1 count load and `q = p;`) = 10 with a diff byte-identical to M7's
(`tmp/grind/func_80017848/s51/B_M19.txt`); P5 (`i = 0;` after `q = p;`) = 10; M20 (`q = p;`
hoisted above `sh = slot_a << 6;`) = 16; M12 (`q = p;` between sh and `i = 0;`) = 13;
M17/M18 (`i = 0;` moved INSIDE both guarded blocks with the guards rewritten as
`if (count > 0)`, which is the target's own `blez`-against-zero shape) = 35 / 27 — that
rewrite costs an instruction and is dead. The 10 is a property of whether `q` is defined on
the loop-skip path, not of where the statements sit.

**E-s51-4 — loop-2's copy has no byte-free reader among the target's own tail instructions.**
Six readers for `r` (loop-2's preheader copy) were measured this session, and every one either
deletes a reload (dropping to 126 insns) or forces an extra live pseudo through the call block:
  * P1 — no reader at all (`r` fresh-read, call restored to BASE's form): **4 at 126**; the
    copy is deleted outright, confirming the copy exists only because of its reader.
  * N1 — `r` read by math_Distance3D's SECOND argument: **18 at 127**.
  * N2 — `r` read by `rec_a`'s address add: **25 at 127**.
  * P2 — loop-2's guard reads `q` and `r = p;` (copy sourced from the post-loop reload):
    **12 at 126**.
  * P3 — loop-2's guard reads `q`, `r` a fresh read: **29 at 124**.
  * P4 — loop-2's guard reads `q`, copy still `r = q;`, call reader kept: **34 at 123**.
The reason is structural and is now pinned from T.txt: the target reloads `*(u8 **)(ctx + 0xC)`
SIX times (T.txt lines 14, 28, 52, 77, 94, 104) and every post-loop-2 address add
(`addu a0,a1,s0` at 79, `addu s0,s0,v0` at 96, `addu s1,s1,v0` at 106) reads one of those fresh
reloads, never the a3 copy. So the target's loop-2 `addu a3,a0,zero` is dead in the emitted
stream with NO downstream reader of any kind — the same flow-stale-liveness situation s44
identified, not a real C-level second use. M7's call-argument reader is therefore the wrong
shape by construction; it is merely the cheapest wrong shape found so far.

**E-s51-5 — hoisting `r`'s definition out of loop-2's guarded block does not buy a3.** M10
(`r = q;` moved above loop-2's guard so r is defined on all paths and q dies exactly where r
is born, which would let them share a3) = **24 at 127**; M13 (M10 + `q = p;` hoisted above
`i = 0;`) = **24**; M14 (M13 with `r = p;`) = **16 at 124**. Moving the copy out of the
preheader emits it in the join block ahead of the `blez`, which is not where the target has it.

- [s51] BASE (memory/grind/func_80017848/candidate.c) re-audits at 3 (127 target / 127 build) on the HEAD src/ings.c:820 anchor; the honest floor is unchanged at 3.

- [s51] tools/fake_ablate.py reports NO FAKE-annotated constructs in candidate.c, so every s42-s50 instance kill on this function was measured without a FAKE carrier occupying any pseudo. The mandatory flat-floor kill re-audit is satisfied and found nothing void.

- [s51] Cell M7 (= s50's M2 with loop-2's redundant `lnk` local deleted) = 10 at 127/127 - the M-branch's first movement since it opened at s50, down from 17.

- [s51] M7's loop-1 preheader is BYTE-EXACT (`addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3`), i.e. the correct a3 seat and the correct copy/load/add order, while still carrying loop-2's copy and the loop-1 exit-tail reload.

- [s51] M7 and M8 both score 10 at 127/127 with DISJOINT residuals: M7 wins the a3 seat and loses the `i = 0` delay-slot placement (2 points); M8 wins the `i = 0` placement in both blez delay slots and loses the seats to t0/t1 (2 points). The discriminator is a single statement, the pre-guard `q = p;`.

- [s51] M8's t0/t1 seats are caused by q and r having no definition on their loop-skip paths, which makes flow treat them as live-in at function entry where they conflict with the incoming a0-a3 parameter registers.

- [s51] The target reloads *(u8 **)(ctx + 0xC) six times (T.txt lines 14, 28, 52, 77, 94, 104) and every post-loop-2 address add (`addu a0,a1,s0` at 79, `addu s0,s0,v0` at 96, `addu s1,s1,v0` at 106) reads a FRESH reload, never the a3 copy - so the target's loop-2 `addu a3,a0,zero` has no downstream reader at all in the emitted stream.

- [s51] P1 (M7 with loop-2's copy given no reader) = 4 at 126: the copy is deleted outright, confirming the copy in every M-branch form exists only because of its C-level reader, and that the reader is what costs the remaining points.

- [s51] The remaining M-branch residual after M7 is: loop-2's preheader copy source/dest (3 insns), loop-2's lnk seat (1), the call block's operand and load order (3), plus whichever 2-point half of the M7/M8 trade-off is taken.

- [s52] KILL RE-AUDIT on the HEAD chassis (anchor src/ings.c:820, no FAKE construct in any body): rejected/s11_l1tail_fresh_read_loses_l1_copy_costs_4.c = 4 at 126/127, rejected/s12_symmetric_fresh_reload_tail_costs_4.c = 4 at 126/127, rejected/s12_symmetric_no_q_fresh_reload_tail_costs_4.c = 4 at 126/127. All three still hold at their banked values; the s11/s12 kills are re-validated, not void.

- [s52] BASE's 3-insn residual is a TRANSPOSITION, not a missing insn. Normalised objdump diff (tmp/grind/func_80017848/s52/T.txt vs B.txt) shows exactly three differing lines: target has `lw a0,12(s2)` in loop-2's GUARD block and `addu a3,a0,zero` + `addu a0,a1,a3` in loop-2's PREHEADER; BASE has `addu a0,a3,zero` (the loop-1 exit tail `p = q;`) in the guard block and `lw v0,12(s2)` + `addu a0,a1,v0` in the preheader. Same instruction count, opposite placement of the load and the copy.

- [s52] MECHANISM for the load-vs-copy asymmetry between the two loops: cse's extended-block reference count for the MEMORY *(u8 **)(ctx + 0xC). Loop 1's guard block LOADS that memory into `p`, so the preheader's second inline read is the second reference in the same cse extended block and cse rewrites it into a reg-reg copy (matching s34's dump of insn 83). Loop 2's guard block in BASE reads through the register `p` and never touches the memory, so the preheader's read is the FIRST reference in that EBB and stays a real `lw`. The guard add is recomputed rather than reused in BOTH loops because its destination register is immediately clobbered by the following count load, which invalidates cse's equivalence for the sum but not for the pointer.

- [s52] FIRST MATERIALISATION OF LOOP-2'S PREHEADER COPY IN 52 SESSIONS. Cell G (tmp/grind/func_80017848/s52/body_G.c) reuses `slots` - the unconditional top-guard local - as loop-2's base addend (`slots = p;` as the first statement inside loop-2's guard) and gives it a post-loop second use at rec_a. Build emits `addu s5,a0,zero` in loop-2's preheader at 127/127 (score 22). The copy is therefore REACHABLE from ordinary C; the entire cost is the carrier's live range, not the copy.

- [s52] Cell G's 22 points decompose into two carrier costs, both visible in the diff: (a) `slots` is live from function entry to rec_a, so global.c seats it in the callee-saved s5 and the prologue grows a save/restore pair and re-orders (ra moves to 60(sp), s6 appears); (b) because cse knows `slots == p` inside the guard block, the guard add and the base add COLLAPSE - build emits `addu a1,v0,a0` once and then `addu a0,a1,zero`, where the target emits two independent adds.

- [s52] Carrier cost table, all measured on the HEAD chassis at src/ings.c:820 with no FAKE construct present. slots-reuse family: F (both math_Distance3D args via slots) = 17 at 126, G (rec_a) = 22 at 127, H (rec_a + rec_b) = 26 at 124, I (call arg0) = 26 at 127. Fresh local `r = p;` in loop-2's preheader with a `p = r;` loop-2 exit tail: J4 (no post-loop use) = 8 at 125, J2 (call arg0) = 17 at 123, J1 (rec_a) = 50 at 127, J3 (rec_a + rec_b) = 50 at 126. Fresh READ `r = *(u8 **)(ctx + 0xC);` instead of a copy: A = 14 at 125, B = 21 at 124, C = 38 at 128. Loop-2 guard given its own read `p2` on top of loop-1's `p = q;` tail: N1 = 32 at 129, N2 = 17 at 126, N3 = 36 at 126, N4 (no r reader) = 33 at 124. Fully symmetric with `q` reused across both loops and the top-guard read feeding it: R = 13 at 124.

- [s52] The LOOP-BODY (out-of-block, inside the guarded region) carrier is dead in its cheapest spelling. Cell T spells loop-2's do-while condition as `i < *(s32 *)(sh2 + (s32)r + 0x20)` so that `r` has a use in the latch block rather than after the loop. It produces NO pointer copy: cse collapses the guard add and the base add and emits a copy of the BASE instead (`addu a1,a0,zero`), 7 at 126/127.

- [s52] OWNER DIRECTIVE EXECUTED. The three newly-visible COMPLETED-C siblings in src/ings.c were read on main: func_80017D84 (src/ings.c:824-846), func_80016E60 (src/ings.c:436-565) and main (src/ings.c:576-685). func_80017D84 confirms and extends the s45 object model - the 52-byte record's ctx+0xC is the node-array base, ctx+0x10 is `ctx+0xC + (*(s16 *)(ctx+4) << 6)` i.e. the link array immediately following the 64-byte node records, and ctx+6 is the link counter that func_80017848 increments. Neither sibling contains a preheader copy or a two-loop copy geometry to transplant: func_80017D84's only loop is a walking-pointer `for (i = 0; i < 8; i++) { ...; p += 0x34; }` with a break and a post-loop read of `i` (a form already killed for this function at s18 R3 = 49), func_80016E60's loops are a `while (1)` dispatch with no pointer preheader, and main has no analogous geometry. The directive is discharged with a negative result: no sibling spelling applies to loop-2's preheader.

- [s52] BASE (memory/grind/func_80017848/candidate.c) re-audited at 3 with 127 target / 127 build insns on the HEAD chassis at the src/ings.c:820 INCLUDE_ASM anchor.

- [s52] The 3-insn residual is a TRANSPOSITION, not a missing instruction: the target places lw a0,12(s2) in loop-2's GUARD block and addu a3,a0,zero + addu a0,a1,a3 in loop-2's PREHEADER, while BASE places the copy (addu a0,a3,zero, i.e. the loop-1 exit tail p = q;) in the guard block and the load (lw v0,12(s2)) + addu a0,a1,v0 in the preheader. Same count, opposite placement.

- [s52] MECHANISM for the load-vs-copy asymmetry between the two loops: cse's extended-block reference count for the MEMORY *(u8 **)(ctx + 0xC). Loop 1's guard block LOADS that memory into p, so the preheader's second inline read is the second reference in the same cse extended block and cse rewrites it into a reg-reg copy (consistent with s34's dump of insn 83). Loop 2's guard block in BASE reads through the register p and never touches the memory, so the preheader's read is the FIRST reference in that EBB and stays a real lw.

- [s52] The guard add is recomputed rather than reused in BOTH loops because its destination register is immediately clobbered by the following count load, which invalidates cse's equivalence for the sum but not for the pointer - this is why the target emits two independent adds and why any form that lets cse equate the carrier with p collapses them.

- [s52] Cell G is the first form in 52 sessions that emits loop-2's preheader copy (addu s5,a0,zero) at 127/127. The copy is therefore reachable from ordinary C; the whole remaining cost is the carrier's live range.

- [s52] Full carrier cost table on the HEAD chassis with no FAKE construct present: slots-reuse F=17@126, G=22@127, H=26@124, I=26@127; fresh r + p=r tail J4=8@125, J2=17@123, J1=50@127, J3=50@126; fresh read instead of a copy A=14@125, B=21@124, C=38@128; loop-2 guard with its own p2 read on top of loop-1's tail N1=32@129, N2=17@126, N3=36@126, N4=33@124; fully symmetric with q reused and the top-guard read feeding it R=13@124; exit-test carrier T=7@126.

- [s52] cse.c:826 make_regs_eqv (already banked at s32) and the local-alloc.c:700 optimize_reg_copy_1 re-pointing path (called from local-alloc.c:1007 only when the copy's SRC has no REG_DEAD note) are the two routines that decide whether the surviving copy is read by the base add; s52 adds that the copy's SURVIVAL is decided one step earlier, by whether the destination has a reference outside the preheader block at all.

- [s52] OWNER DIRECTIVE DISCHARGED (negative): func_80017D84, func_80016E60 and main were read on main; none carries a transplantable preheader/copy geometry, and func_80017D84 only confirms the object model (ctx+0x10 == ctx+0xC + (*(s16 *)(ctx+4) << 6); ctx+6 is the link counter).

## s53 (2026-09-07, structural)

- [s53] KILL RE-AUDIT / BASE re-audit on the HEAD chassis (anchor src/ings.c:820,
  no FAKE construct in any body): `memory/grind/func_80017848/candidate.c` = **3 at
  127/127**; s52's two closest cells re-measure at their banked values —
  `body_J4` = 8 at 125/127 and `body_G` = 22 at 127/127. The s52 instance kills
  stand; nothing was void.

**E-s53-1 — THE SESSION'S RESULT: the function is SYMMETRIC and the whole residual
is ONE construct emitted twice.** Cell **S1/S2** (`tmp/grind/func_80017848/s53/body_S1.c`,
`body_S2.c`) writes loop 2 as a literal mirror of loop 1: its own pre-guard fresh read
`p2 = *(u8 **)(ctx + 0xC);`, its own guard temp `t2 = sh2 + (s32)p2; t2 = *(s32 *)(t2 + 0x20);`,
and its own preheader fresh read `q2 = *(u8 **)(ctx + 0xC);` feeding `base = sh2 + (s32)q2;`.
S1 (with the now-dead `p = q;` / `p2 = q2;` exit tails) and S2 (without them) BOTH measure
**12 at 125/127**, and the normalised diff (`s53/B_S2.txt` vs `s53/T.txt`) is confined to the
two loop preheaders and is IDENTICAL in both loops:

    target loop N:  lw a0,12(s2) / sll a1,s4,6 / addu v0,a1,a0 / lw v0,<cnt>(v0) / blez /
                    addu v1,zero,zero / addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3
    S2     loop N:  lw a1,12(s2) / sll a0,s4,6 / addu v0,a0,a1 / lw v0,<cnt>(v0) / blez /
                    addu v1,zero,zero /                lw a2,16(s2) / addu a0,a0,a1

Every other instruction in the function — prologue, top guard, both loop bodies, both
latches, the call block, the link-record stores, rec_a/rec_b and the epilogue — is
byte-identical. This RETIRES the framing every session since s34 has worked under (that
loop 1 and loop 2 need DIFFERENT spellings and that the residual is a load/copy
TRANSPOSITION between the loop-2 guard block and the loop-2 preheader). It is not a
transposition: it is ONE missing instruction per loop, the same instruction, plus the
a0/a1 seat swap that follows from it.

**E-s53-2 — the missing construct's PASS ATTRIBUTION, read from the dumps, not guessed.**
`pwsh tools/grinder/dump.ps1 func_80017848` was run with S2 in place
(`tmp/grind/func_80017848/dumps/`). In `ings.cse` and `ings.combine` the loop-1 preheader is:

    (insn 64  (set (reg/v:SI 79) (mem:SI (plus (reg/v:SI 72) (const_int 12)))))   ; p  = ctx->0xC
    (insn 66  (set (reg/v:SI 86) (ashift (reg/v:SI 74) (const_int 6))))           ; sh
    (insn 86  (set (reg/v:SI 77) (mem:SI (plus (reg/v:SI 72) (const_int 16)))))   ; lnk
    (insn 89  (set (reg/v:SI 83) (plus (reg/v:SI 86) (reg/v:SI 79))))             ; base = sh + p
              ... REG_DEAD (reg 86), REG_DEAD (reg 79)

There is **no `(set (reg) (reg))` copy insn anywhere in either preheader**: cse does not
turn the preheader's second `*(u8 **)(ctx + 0xC)` into a reg-reg move that survives — it
propagates the guard's pseudo straight into the base add and the copy never reaches
local-alloc. So the target's `addu a3,a0,zero` is NOT a local-alloc/reload artefact and NOT
an optimize_reg_copy_1 survival question (the model s52 recorded); the copy has to exist as
a distinct pseudo at combine time, which requires **the copy's destination to have more than
one use** — with a single use combine merges `(set qN pN)` into the consuming `plus` and the
insn disappears.

**E-s53-3 — the two-use model CONFIRMED by construction.** Cell **V1** (= S2 with loop 2's
preheader read replaced by `q2 = q;`, so loop-1's `q` now has TWO uses: loop-1's base add and
loop-2's preheader) **materialises loop-1's preheader copy** — the build emits
`addu t0,a0,zero / addu a0,a1,t0` where S2 emitted only `addu a0,a0,a1`. V1 = **10 at 126/127**
(loop 2 now has no copy of its own, because `q2 = q` is itself the copy and its dest has a
single use). This is the first direct demonstration on this function that the preheader copy
is bought by USE COUNT at combine time and by nothing else.

**E-s53-4 — NEW BEST ON THE M-BRANCH: cell Z1 = 9 at 127/127.** Z1 = V1 + loop 2's do-while
latch recomputed from its own copy dest (`} while (i < *(s32 *)(sh2 + (s32)q2 + 0x20));`).
The second use restores the missing insn and the count returns to 127/127. Score **9**,
beating s51's M7/M8 (10) and s50's M2 (17). Banked
`rejected/s53_chain_plus_loop2_latch_via_q2_BEST_M_BRANCH_costs_9.c`.
Z1's remaining residual is entirely the two preheaders: loop 1's copy is `addu t0,a0,zero`
(wrong seat, and emitted AFTER `lw a2,16(s2)` instead of before), and loop 2's surviving copy
is a copy of **base** emitted AFTER the base add (`addu a0,a0,t0 / lw a2,16(s2) / addu a1,a0,zero`)
because cse folds `sh2 + q2 + 0x20` to `base + 0x20` and the second use therefore attaches to
base, not to the pointer.

**E-s53-5 — the latch-reader family always buys a BASE copy, never a POINTER copy.** Every
cell whose second use of the copy dest is an address expression (latch or body index) folds
through cse to `base`, so the surviving copy is `base -> tmp` placed after the base add:
U1 (both latches via q/q2) = **13 at 127/127**, U2 (both body indices via q/q2) = **17 at 127/127**,
U3 (loop-1 latch only) = 14 at 126, AA (latches via the GUARD pointer p/p2) = 11 at 126,
AB = 13 at 127, AC (V1 + loop-2 latch via p2) = 10 at 127, AD (Z1 + loop-1 latch via q) = 11 at 128.
Banked `rejected/s53_both_latches_via_copy_dest_base_copy_after_add_costs_13.c` and
`rejected/s53_both_bodies_indexed_via_copy_dest_costs_17.c`.

**E-s53-6 — post-loop second uses and `slots`-sourced copies are dead on the symmetric chassis.**
BA (V1 + `rec_a` built from q2) = 21 at 127, BB (V1 + math_Distance3D's first argument from q2)
= 13 at 127, BC (V1 + `rec_b` from q2) = 21 at 126, BD (S2 + rec_a from q2 + call arg from q)
= 21 at 127; Y1 (both preheader copies sourced from the top-guard local `slots`) = 15 at 125,
Y2 (loop 2 only) = 15 at 125; W1 (no `base` local at all, every address written inline from
`sh + q`) = 12 at 125; Z3 (V1 + loop-2 body index via q2) = 12 at 127; Z4 (V1 + `p = q2;` tail
read by rec_a) = 32 at 128; AE/AF/AG (Z1 + `q` pre-initialised before loop-1's guard from p /
a fresh read / slots) = 11 / 11 / 12.

- [s53] BASE re-audits at 3 (127 target / 127 build) on the HEAD chassis at the src/ings.c:820 INCLUDE_ASM anchor; the honest floor is unchanged at 3.
- [s53] The mirror-symmetric chassis (S2) matches the target EVERYWHERE except one instruction per loop preheader; the 52-session "loop-2 transposition" framing is retired.
- [s53] cse propagates the preheader's second `*(u8 **)(ctx + 0xC)` read directly into the base add (dumps/ings.cse insn 89, dumps/ings.combine insn 89): no reg-reg copy insn ever reaches local-alloc, so the missing `addu a3,a0,zero` is a COMBINE-time use-count question, not an allocator question.
- [s53] Giving the copy dest a second use materialises the copy (V1 = 10 at 126, Z1 = 9 at 127/127 - the best M-branch score in 53 sessions).
- [s53] Every second use spelled as an ADDRESS (latch, body index) is folded by cse to `base`, so the copy that survives is a copy of base placed after the base add rather than a copy of the pointer placed before it. That is the entire remaining shape gap.

- [s53] BASE (memory/grind/func_80017848/candidate.c) re-audits at 3 with 127 target / 127 build insns on the HEAD chassis at the src/ings.c:820 INCLUDE_ASM anchor; the honest floor is unchanged at 3.

- [s53] s52's closest cells re-measure unchanged on the HEAD chassis: J4 = 8 at 125/127, G = 22 at 127/127.

- [s53] Cell S2 (loop 2 written as a literal mirror of loop 1: pre-guard read p2, guard temp t2, preheader read q2 feeding base = sh2 + q2) = 12 at 125/127, and its ENTIRE diff against the target is the two loop preheaders, identically in both loops - the target's addu a3,a0,zero missing plus the a0/a1 guard seat swap. Everything else in the function is byte-identical.

- [s53] The 52-session framing that loop 2's residual is a load-vs-copy TRANSPOSITION between its guard block and its preheader is retired: on the symmetric chassis it is one missing instruction per loop, the same instruction twice.

- [s53] PASS ATTRIBUTION READ FROM DUMPS (tmp/grind/func_80017848/dumps/, generated with S2 in place): neither ings.cse nor ings.combine contains any reg-to-reg set in either loop preheader. cse propagates the guard's pointer pseudo straight into the base add (loop 1 insn 89, loop 2 insn 165), both operands carrying REG_DEAD in .combine.

- [s53] Consequence: the missing copy is a COMBINE-time use-count question. A single-use reg-reg move is merged into the consuming plus and disappears; the copy survives only when its destination has two or more uses. This supersedes s52's local-alloc.c:700 optimize_reg_copy_1 attribution for this construct.

- [s53] Model confirmed by construction: V1 (loop 2's preheader spelled q2 = q, giving loop-1's copy dest two uses) = 10 at 126/127 and emits addu t0,a0,zero / addu a0,a1,t0 in loop 1's preheader.

- [s53] Cell Z1 (V1 + loop 2's do-while bound recomputed from q2) = 9 at 127/127 - the best M-branch score in 53 sessions (previous best 10 at s51, 17 at s50).

- [s53] Z1's residual is entirely the two preheaders: loop 1's copy sits in t0 and after lw a2,16(s2) instead of a3 and before it; loop 2's surviving copy is a copy of BASE placed after the base add, because cse folds sh2 + q2 + 0x20 to base + 0x20.

- [s53] Every address-shaped second use folds the same way and buys a base copy, not a pointer copy: U1 = 13@127, U2 = 17@127, U3 = 14@126, AA = 11@126, AB = 13@127, AC = 10@127, AD = 11@128, Z3 = 12@127.

- [s53] Post-loop second uses and slots-sourced copies are dead on this chassis: BA = 21@127, BB = 13@127, BC = 21@126, BD = 21@127, Y1 = 15@125, Y2 = 15@125, Z4 = 32@128, W1 = 12@125, AE/AF/AG = 11/11/12.

- [s53] src/ings.c was restored to HEAD (INCLUDE_ASM) at the end of the session; the working tree carries no C for this function.

## s54 (2026-09-07, synthesis)

- **E-s54-1.** Chassis re-audit. `memory/grind/func_80017848/candidate.c` (BASE) applied at the
  HEAD `src/ings.c:820` INCLUDE_ASM anchor re-measures **3 at 127/127** with
  `sandbox func_80017848 --disable all`. `python3 tools/fake_ablate.py --func func_80017848
  --file ings --candidate memory/grind/func_80017848/candidate.c` reports **no FAKE-annotated
  constructs**. Every s52/s53 instance kill therefore still stands on this chassis.

- **E-s54-2 (the residual, stated exactly for the first time).** BASE's 3 differing instructions
  are a pure TRANSPOSITION inside loop 2. Normalised diff (`tmp/grind/func_80017848/s54/T.txt`
  vs `s54/B.txt`), target on the left:

        loop-2 guard      lw   a0,12(s2)        ->   addu a0,a3,zero
        loop-2 preheader  addu a3,a0,zero       ->   lw   v0,12(s2)
        loop-2 base add   addu a0,a1,a3         ->   addu a0,a1,v0

  Both sides emit exactly one load and one reg-reg copy per loop; the target puts the LOAD in
  the guard and the COPY in the preheader, BASE puts the COPY in the guard and the LOAD in the
  preheader. Loop 1, the prologue, both loop bodies, both latches, the call block, the link
  stores, rec_a/rec_b and the epilogue are byte-identical at 127/127.

- **E-s54-3 (pass attribution, read from dumps, supersedes every earlier guess).**
  `pwsh tools/grinder/dump.ps1 func_80017848` with BASE in place;
  `tmp/grind/func_80017848/dumps/ings.combine` line 6767 onward. The three RTL facts that
  explain the whole residual:
  1. Loop 1's surviving preheader copy is **insn 83 `(set (reg/v 80) (reg/v 79))`**. It survives
     because reg 80's SECOND use is **insn 141 `(set (reg/v 79) (reg/v 80))`** — the `p = q;`
     exit tail. Critically, loop-1's base add **insn 89 `(set (reg/v 81) (plus (reg/v 84)
     (reg/v 79)))` reads the copy's SOURCE, not its destination**: cse propagated reg 79 into
     the base add and the copy is kept alive purely by the tail.
  2. Loop 2's preheader is **insn 162 `(set (reg 113) (mem (plus (reg/v 72) (const_int 12))))`**
     — a REAL load, not a copy. reg 79 carries a REG_DEAD note at loop-2's guard add
     (insn 151), and the ctx+0xC memory equivalence does not survive loop 1's blocks, so cse has
     nothing to fold the preheader load against. reg 113 is exactly the "LOCAL allocno, blk 13,
     refs 2" the s47 solver typed without being able to name its source statement.
  3. BASE's wrong `addu a0,a3,zero` in loop-2's GUARD slot is **insn 141 itself** — the `p = q;`
     join copy, materialised at the merge block, which is loop-2's guard block. So loop-1's copy
     and loop-2's wrong guard instruction are the SAME device seen at its two ends: the tail buys
     loop 1's copy and pays for it with loop 2's guard load.

- **E-s54-4 (why the 2-use model cannot be the target's own mechanism).** In the target, `a3` is
  written in loop-1's preheader (T.txt:35), read once (T.txt:37), re-written in loop-2's
  preheader (T.txt:59) and read once (T.txt:61); it is dead everywhere else in all 127
  instructions (only other mention is the prologue's `addu s3,a3,zero` moving the slot_b
  parameter). Under the combine rule measured at s53 and re-confirmed here (a copy whose
  destination has a single use is merged into the consuming `plus`), a C carrier with two uses
  cannot be what produces the target's copies. BASE's loop-1 byte match is produced by a
  DIFFERENT RTL shape (copy dest not used by the base add at all) that happens to allocate to
  the target's seats.

- **E-s54-5 (NEW load-bearing device identified: the two-step guard temp).** Loop 1's
  `t = sh + (s32)p; t = *(s32 *)(t + 0x1C);` reuses ONE variable for the address and the loaded
  value. The second store clobbers the pseudo holding `sh + p` (insn 72 -> insn 75 both write
  reg 86), which destroys cse's record of `sh + p` and forces the preheader to RECOMPUTE the
  base add (insn 89). Cells that spell the guard as a single expression
  (`if (i < *(s32 *)(sh2 + (s32)p2 + 0x20))`) let cse keep the guard's address pseudo alive, and
  the preheader's base add is then folded away and replaced by a copy of BASE emitted AFTER it:
  cell XM emits `addu a3,v0,a1` in the guard and `addu a1,a3,zero` in the preheader
  (`tmp/grind/func_80017848/s54/body_XM.c`, 17 at 126). The two-step temp is present in BASE's
  loop 1 and in S2's loop 2, and absent from the whole s52 N-family — which explains their
  scores retrospectively.

- **E-s54-6 (the `while` chassis is dead on measurement).** Writing both loops as
  `while (i < *(s32 *)(sh + (s32) * (u8 **)(ctx + 0xC) + 0x1C)) { ... }` lets jump.c's
  `duplicate_loop_exit_test` (tools/gcc-2.7.2/jump.c:2163) synthesise the guard from the exit
  test — and it does produce a preheader copy (`addu a2,v0,zero`). But loop.c then hoists the
  latch's bound out of the loop entirely, so the build emits 120 instructions where the target
  reloads `lw v0,28(a0)` on every iteration. Cell WA = 29 at 120
  (`rejected/s54_while_loops_duplicate_loop_exit_test_hoists_latch_costs_29.c`). The target's
  in-loop bound reload is only reproduced by the hand-written `if (guard) do { } while (cond);`
  chassis BASE already uses.

- **E-s54-7 (preheader statement ORDER is byte-neutral).** Giving loop 2 explicit named
  intermediates in loop-1's order (`q2 = *(u8 **)(ctx + 0xC); lnk2 = *(u8 **)(ctx + 0x10);
  base = (u8 *)(sh2 + (s32)q2);`) measures **3 at 127/127** — byte-identical to BASE. Combined
  with S2 (whose loop 1 has the same ordering yet loses its copy because it has no tail), this
  kills the "combine's `all_adjacent` / lnk-load-between-the-copy-and-the-add" theory outright:
  the gate is combine's `added_sets_2 = ! dead_or_set_p (i3, i2dest)`
  (tools/gcc-2.7.2/combine.c:1453), not instruction adjacency.

- **E-s54-8 (the loop-2 carrier has no affordable consumer; six new cells).** Every spelling that
  gives loop-2's preheader copy destination a second use outside the guarded region routes that
  use through `*(u8 **)(ctx + 0xC)` values the target RELOADS after the loops (BASE .combine
  insns 222, 270 and 293 are three separate `(mem (plus (reg 72) (const_int 12)))` loads for the
  call, rec_a and rec_b). Consuming the carrier at any of them deletes that reload and drops the
  build to 126 instructions: M = 17, M2 = 17, M3 = 16, N2 = 17 (re-measured on this chassis),
  XM = 17, F2 = 17, F2b = 17 — all at 126. Consuming it through an ADDRESS instead keeps 127/128
  but cse folds the address to `base`, so the surviving copy is a copy of base emitted after the
  base add: ZD (latch bound via q2) = 5 at 128, ZE (body index via q2) = 6 at 128.

- [s54] BASE (memory/grind/func_80017848/candidate.c) re-measures 3 at 127/127 on the HEAD chassis; tools/fake_ablate.py reports no FAKE-annotated construct in it.

- [s54] The 3-instruction residual is a pure transposition inside loop 2: target emits `lw a0,12(s2)` (guard) + `addu a3,a0,zero` (preheader) + `addu a0,a1,a3`; BASE emits `addu a0,a3,zero` (guard) + `lw v0,12(s2)` (preheader) + `addu a0,a1,v0`. One load and one copy per loop on both sides.

- [s54] tmp/grind/func_80017848/dumps/ings.combine (line 6767 on, BASE applied): insn 83 = (set (reg/v 80) (reg/v 79)) is loop-1's preheader copy; insn 89 = (set (reg/v 81) (plus (reg/v 84) (reg/v 79))) is the base add and reads the copy's SOURCE; insn 141 = (set (reg/v 79) (reg/v 80)) is the `p = q;` tail that keeps the copy alive; insn 162 = (set (reg 113) (mem (plus (reg/v 72) 12))) is loop-2's preheader, a real load.

- [s54] reg 113 in that dump is the 'LOCAL allocno, blk 13, refs 2' the s47 solver typed but could not attribute to a source statement; it is loop-2's preheader ctx+0xC load.

- [s54] In the target, a3 is written in loop-1's preheader (T.txt:35), read once (T.txt:37), rewritten in loop-2's preheader (T.txt:59) and read once (T.txt:61), and is dead everywhere else in all 127 instructions apart from the prologue's `addu s3,a3,zero`. Under the measured combine use-count rule a single-use copy is always merged, so a two-use C carrier cannot be what produces the target's copies; BASE's loop-1 byte match comes from a different RTL shape (copy dest not used by the base add) that lands on the same seats.

- [s54] The two-step guard temp `t = sh + (s32)p; t = *(s32 *)(t + 0x1C);` is load-bearing: combine insns 72 and 75 both write reg 86, destroying cse's record of (plus sh p) and forcing the preheader to recompute the base add. Single-expression guards let cse fold the preheader add into the guard address (cell XM emits `addu a3,v0,a1` then `addu a1,a3,zero`).

- [s54] jump.c:2163 duplicate_loop_exit_test does synthesise both the guard and a preheader copy from a `while` loop, but loop.c then hoists the latch bound out of the loop, so the while-chassis builds 120 instructions against the target's 127 (cell WA = 29).

- [s54] Preheader statement order is byte-neutral (cell Z = 3, identical to BASE); combined with s53's S2 (same ordering, no tail, copy lost), combine's adjacency test is excluded and the use-count gate at tools/gcc-2.7.2/combine.c:1453 is confirmed as the decider.

- [s54] BASE's .combine carries three separate post-loop ctx+0xC loads (insn 222 for math_Distance3D, insn 270 for rec_a, insn 293 for rec_b); routing loop-2's carrier into any of them deletes that load and lands the build at 126 instructions (M/M2/M3/F2/F2b/N2/XM all 16-17).

- [s54] Nine new bodies banked to memory/grind/func_80017848/rejected/ this session (313 -> 326 forms), each named for the measured cost.

## s55 (2026-09-07, escalation modality — the exhaustion premise is FALSIFIED by measurement)

- **E-s55-0 (chassis + kill re-audit).** `memory/grind/func_80017848/candidate.c` (BASE)
  re-measures **3 at 127/127** on the HEAD `src/ings.c:820` INCLUDE_ASM anchor. The dispatch
  brief reported "measurement unavailable"; the ledger's floor of 3 is correct.

- **E-s55-1 (the s54 pass attribution was WRONG, corrected from dumps).** s54 recorded that
  loop-1's base add "reads the copy's SOURCE rather than its destination, and the operand was
  rewritten from reg 79 to reg 80 AFTER combine", with local-alloc named only as a candidate.
  Reading the four dumps at the same insn uids settles it:
    - `ings.combine:6974` insn 89 = `(set (reg/v 81) (plus (reg/v 84) (reg/v 79)))` — reads 79
    - `ings.flow:9027`    insn 89 = identical — still reads 79
    - `ings.lreg:8889`    insn 89 = `(set (reg/v 81) (plus (reg/v 84) (reg/v 80)))` — reads 80,
       and the `REG_DEAD (reg 79)` note has moved back onto the copy at insn 83
  The rewrite therefore happens **inside local-alloc, between `flow` and `lreg`**, and the
  routine is **`optimize_reg_copy_1` (tools/gcc-2.7.2/local-alloc.c:700)**, called from
  `update_equiv_regs` at local-alloc.c:1007. Its contract is exactly this shape: "INSN is a copy
  from SRC to DEST, both registers, and SRC does not die in INSN. Search forward to see if SRC
  dies before either it or DEST is modified... If so, we can replace SRC with DEST and let SRC
  die in INSN." Loop 1 satisfies it (79 dies at insn 89, nothing sets 79 or 80 in between, the
  scan has not yet hit `NOTE_INSN_LOOP_BEG` at insn 91), so `addu a3,a0,zero` / `addu a0,a1,a3`
  is emitted. **This is a mechanism-level correction, not a new spelling: `optimize_reg_copy_1`
  is what turns a surviving preheader copy into the target's exact two-instruction pair.**

- **E-s55-2 (NEW DEVICE, un-tried in 54 sessions: block combine with `use_crosses_set_p`).**
  `tools/gcc-2.7.2/combine.c:914-917` refuses a combination when
  `(! all_adjacent && use_crosses_set_p (src, INSN_CUID (insn)))` — i.e. when the copy and its
  consumer are NOT adjacent AND the copy's SOURCE register is set by an insn in between. The
  function's own loop preheaders supply that intervening insn for free: the target emits exactly
  one instruction between the copy and the base add, `lw a2,16(s2)` (the ctx+0x10 "lnk" load).
  Spelling that load as a write to the SAME C variable that holds the ctx+0xC pointer makes the
  clobber real, and **the preheader copy materialises in BOTH loops for the first time in 55
  sessions** — cell V3 (`tmp/grind/func_80017848/s55/body_V3.c`), 14 at 127/127.
  Every previous session's framing ("the gate is the copy destination's use count",
  s54 / combine.c:1453) is thereby shown to be only ONE of the two gates; the source-clobber gate
  is cheaper because the clobbering insn already exists in the target.

- **E-s55-3 (the residual class CHANGED: `inverse_compose classify` now says RA, not
  instructions).** With V3 applied,
  `python3 tools/ra_solver/inverse_compose.py classify ings func_80017848
   --target-object build/src/ings.o --ours-object tmp/sandbox/func_80017848/ings.o`
  reports **`FIRST DIVERGENCE: RA — same instructions, different registers`** (127 honest / 127
  target). For 54 sessions this function's residual was a structural transposition that no C
  form could produce; it is now a seat permutation, which is the ra_solver's declared domain.
  Artifact: `tmp/grind/func_80017848/s55/ra/`.

- **E-s55-4 (seat topology read off the target: the pointer variable IS the base variable).**
  In the target the ctx+0xC pointer and the loop base occupy the SAME hard register `a0`
  (`lw a0,12(s2)` … `addu a0,a1,a3`), while lnk has its own seat `a2`, the copy dest is `a3` and
  the index is `v1`. V3 reused the pointer variable for LNK, which pins lnk and the pointer to
  one seat (`a1`) and shifts every other seat — the whole 14-point residual. Reusing the pointer
  variable for the BASE instead reproduces the target's seat map.

- **E-s55-5 (cell U1 — EVERY REGISTER IN THE FUNCTION IS NOW TARGET-EXACT).**
  `tmp/grind/func_80017848/s55/body_U1.c` measures **4 at 125/127**, and the complete normalised
  objdump diff against the target is:

        loop 1:   -addu a3,a0,zero          loop 2:   -addu a3,a0,zero
                   lw a2,16(s2)                        lw a2,16(s2)
                  -addu a0,a1,a3                      -addu a0,a1,a3
                  +addu a0,a1,a0                      +addu a0,a1,a0

  Nothing else differs anywhere in 125 instructions — not one register, not one offset, not one
  ordering. The residual is now literally **two missing `addu a3,a0,zero` copies**, one per loop,
  and the two consuming adds each read `a0` instead of `a3`. This is the closest any form has come
  in 55 sessions; the previous best (BASE, 3) is numerically lower only because the engine metric
  charges the two absent instructions plus their two consumers.

- **E-s55-6 (why U1 loses the copies, stated as a predicate).** In U1 the copy's source `q` is set
  by the base add ITSELF (`q = (u8 *)(sh + (s32)q_old)`), not by an insn *between* the copy and
  the add, so `use_crosses_set_p` is false at the time `try_combine` runs (combine's
  `reg_last_set` is only updated for insns already scanned past, and i3 has not been scanned).
  combine therefore merges the copy into the add and the copy is deleted. The two gates are
  currently mutually exclusive on this chassis: the clobber that keeps the copy (V3) costs the
  seat map, and the variable identity that wins the seat map (U1) removes the clobber.

- **E-s55-7 (what was swept and stayed flat).** Sharing/splitting the four loop variables
  (q, p, base, t) across the two loops is a 16-cell lattice, W00..W15: every cell measures 14 at
  127/127 except the four that share BOTH `p` and `t` (W10/W11/W14/W15 = 36). Birth-order
  permutations of the V3 clobber (Y1 `i=0` after the read, Y2 read before `sh`) are byte-neutral
  at 14; routing the base or the pointer through the top-guard variable `slots` is worse
  (Y3 = 17, Y5 = 17, Y4 = 19 at 126). On the U1 chassis, moving the lnk load above the copy
  (U2 = 4), using `slots` as the copy dest (U5/U6 = 4), sharing the copy dest across both loops
  (U7 = 4), re-reading ctx+0xC between the copy and the add (U10 = 4 — cse folds the re-read away,
  so it is not a clobber) and dropping the reuse entirely for a separate base variable (U8 = 10)
  all leave the copies merged at 125.

- **E-s55-8 (bearing on the endgame-lock gates).** Gate (a) is unchanged and still FAILS
  (`scan_hand_coded` LOW 0/8, quoted in the 2026-08-18 entry; nothing in this session's evidence
  bears on hand-written asm — the opposite, since a pure-C form now reproduces 125 of 127
  instructions with every register exact). Gate (b) is not reached: this session introduces NO
  coercion or spelling family needing a precedent. U1 and V3 are ordinary C — named locals,
  ordinary loads, one variable holding one pointer at a time. `tools/fake_ablate.py` finds no
  FAKE construct in either. **The premise the escalation modality rests on — that the pure-C
  levers are exhausted — is falsified by E-s55-2/3/5: an un-tried lever moved the residual from a
  structural transposition no C form could reach to a two-instruction combine question with a
  named predicate at combine.c:914.**

- [s55] BASE re-measures 3 at 127/127 on the HEAD chassis (the dispatch brief's chassis measurement was unavailable; the ledger number is correct).

- [s55] PASS ATTRIBUTION CORRECTED: loop-1's copy/add pair is produced by optimize_reg_copy_1 (tools/gcc-2.7.2/local-alloc.c:700, called at local-alloc.c:1007), not by combine - insn 89 reads reg 79 at ings.combine:6974 and ings.flow:9027 and reg 80 at ings.lreg:8889.

- [s55] NEW LEVER: combine.c:914's '(! all_adjacent && use_crosses_set_p (src, INSN_CUID (insn)))' is a second, cheaper survival gate for the preheader copy; the target's own 'lw a2,16(s2)' lnk load is the free intervening insn that can supply the set.

- [s55] Cell V3 (clobber spelled as the lnk load writing the pointer variable) = 14 at 127/127 and MATERIALISES the preheader copy in BOTH loops - the first time in 55 sessions that loop 2 has a copy at all.

- [s55] tools/ra_solver/inverse_compose.py classify with --target-object build/src/ings.o now reports 'FIRST DIVERGENCE: RA - same instructions, different registers' for this function; the residual class changed from structural to seat.

- [s55] Cell U1 (pointer variable reused as the loop base, lnk separate) = 4 at 125/127 with EVERY register in all 125 emitted instructions matching the target; the entire diff is the two missing 'addu a3,a0,zero' copies and their two consumers reading a0 instead of a3.

- [s55] The two gates are mutually exclusive as spelled so far: V3's clobber buys the copies and loses the seat map; U1's variable identity buys the seat map and loses the clobber (its only set of the source is the base add itself, which combine does not count as a set 'between').

- [s55] Endgame-lock gate (a) still fails and is unaffected (scan_hand_coded LOW 0/8 per the 2026-08-18 entry); a pure-C form now reproduces 125 of 127 instructions with every register exact, which is further evidence against hand-written asm. Gate (b) is not reached: U1 and V3 introduce no coercion or spelling family - they are ordinary named locals and ordinary loads, and tools/fake_ablate.py finds no FAKE construct in either.

- [s55] Twelve new bodies banked; memory/grind/func_80017848/rejected/ is now 333 forms, and the U1 body is saved as candidate_alt_s55_u1_seat_exact_two_copies_missing_4.c.

- [s55] src/ings.c was restored to its HEAD INCLUDE_ASM state at end of session; the tree carries no source edits.

## s56 (2026-09-15, rederive - forced sibling transplant from func_8005BA8C)

### E-s56-1 - Chassis re-audit + sibling transplant
- src/ings.c at HEAD 36c413969 carries `INCLUDE_ASM("asm/funcs", func_80017848);` at line 820; s56 tooling (apply.py / run.ps1 / dis.py / norm.py copied from s54) in tmp/grind/func_80017848/s56/. BASE (candidate.c body) = 3 at 127/127, U1 = 4 at 125/127, both identical to s55.
- func_8005BA8C's closed ledger (git show 526e3facc^:memory/grind/func_8005BA8C/evidence.md, CROSS-KNOWLEDGE line) records that this function shares NO code block with it ("max overlap 0.120 ... nothing to transplant"). Its H8 lever (entry-cursor copy of the pointer param) transplanted onto K2 as cell K4 = 8 at 127/127: the s2 prologue pair moves from slots 2-3 to 9-10, seat residual unchanged. KILLED (instance). Owner directive executed.

### E-s56-2 - NEW MECHANISM, MEASURED: combine.c:914 fed by a set that combine later deletes
- tools/gcc-2.7.2/combine.c:9804 is the only writer of reg_last_set[]; try_combine never clears it when i2 becomes a NOTE, and use_crosses_set_p (combine.c:10107-10131) compares INSN_CUID (reg_last_set[regno]) > from_cuid, which a deleted insn still satisfies. So `p = q; q = *(u8 **)(ctx + 0x10); lnk = q; q = (u8 *)(sh + (s32)p);` yields: load merged into `lnk = q` (one insn, `lw a2,16(s2)`), copy `p = q` kept (gate 914 at the base add), base add reads p, q's ranges exactly U1's.
- Cell K1 = 6 at 127/127 (s56/diff_K1.txt): both preheader copies present, only the copy-dest seat differs (v0 vs a3), 4 diff lines, all seat. RA solver: ra_K1.json / sim_K1.txt, 14 global pseudos, sort MATCH, 14/14; the copy dests are NOT among them (local quantities).
- optimize_reg_copy_2 (local-alloc.c:874) does not fire on this shape (it needs the exact pair `dest = src; ...; src = dest` with no set of src between), and optimize_reg_copy_1 is not called because q is dead at the copy (local-alloc.c:1005-1015 read this session).

### E-s56-3 - K2: one copy-dest variable for both loops = 4 at 127/127 (best-structured form on file)
- Same 4 seat-only diff lines as K1 (s56/diff_K2.txt). RA solver sim_K2.txt: pseudo 78 (p) is now a GLOBAL allocno: `pri=13333 calls=0 hard_conf=[3, 29] someone=[] best=2 prefs=[2]`; sort order MATCH, 15/15 dispositions. The prefs {v0} entry's source was not traced (candidate: hard_reg_preferences via set_preference on a set whose first source operand is a local-alloc'd pseudo in v0; read it from the .greg dump next session, it is harmless here because v0 is also the scan result).
- Banked: memory/grind/func_80017848/candidate_alt_s56_k2_combine914_clobber_both_copies_seat_v0_4.c.

### E-s56-4 - Why v0, and what a3 needs (global.c read end to end this session)
- find_reg (global.c:952-1135): pass 0 excludes hard_reg_conflicts, the complement of regs_used_so_far (all call-used regs are pre-seeded, global.c:340-372) and regs_someone_prefers (prune_preferences: full prefs of LOWER-priority conflicting allocnos); the scan is ascending regno (no REG_ALLOC_ORDER); after best_reg, an unused hard_reg_copy_preferences entry of a compatible class overrides it.
- global_conflicts (global.c:755-790) processes REG_DEAD notes BEFORE note_stores, so a copy dest born where its source dies never conflicts with the source, and a pseudo dying at the base add never conflicts with base. Hence in K1/K2 p conflicts with neither a0 holder. For a3 the copy dest must overlap base (live past the add) AND a loop-body v0 temporary - the E-s44-3 byte-free-reader requirement, unchanged - or hold an a3 copy preference (set_preference global.c:1671: only hard regs or local-alloc'd pseudos give preferences).
- A short GLOBAL copy dest of priority above slot_b's allocno (3970) would also find a3 in regs_someone_prefers (slot_b prefers a3 from its incoming-argument copy) and land in t0 even with v0..a2 excluded; a3 therefore also needs priority below 3970 (live length >= 6 at 2 refs), which a live-through-the-loop range satisfies automatically.
- Post-flow deleters enumerated (grep this session): combine (merge/fold), local-alloc optimize_reg_copy_2 (turns a copy PAIR into self-moves), update_equiv_regs (deletes a REG_EQUIV init insn for a twice-referenced pseudo, local-alloc.c:1090-1111), reload (no-op moves), sched2 (no-op moves, sched.c:4955-4975), jump2 (same-reg moves and find_equiv_reg-redundant moves, jump.c:437-490; dead code), reorg (redundant_insn), final (no-op moves). USE insns for pseudos exist only under obey_regdecls (-O0), stmt.c:3271/3499, function.c:5177/5346.
- Reload's find_equiv_reg copy route (reload1.c:5835-5853 emits `move reloadreg, equivreg` instead of a load when a MEM or unallocated-pseudo input's value is already in a hard reg; reload's first spill reg would be a3 if a3 had zero uses, reload1.c:3756-3790) was examined and closed: global.c allocates every allocno here (find_reg fails only when a class is exhausted), regclass never returns NO_REGS as prefclass (regclass.c:935-960 scans classes ALL_REGS-1..1), and no insn carries a MEM operand needing a register after combine.

### E-s56-5 - Artifacts
- tmp/grind/func_80017848/s56/: body_{BASE,U1,K1,K2,K3,K4}.c, diff_*.txt, raw_*.txt, B_*.txt, T.txt, ra_K1.json, ra_K2.json, sim_K1.txt, sim_K2.txt, run.ps1, dis.py, apply.py, ra.sh, bank.py.

- [s56] E-s56-2: tools/gcc-2.7.2/combine.c:9804 is the only writer of reg_last_set[]; a merged-away i2 keeps its CUID, so use_crosses_set_p (combine.c:10107-10131) still reports a crossing set - the combine.c:914 gate is satisfiable at zero instruction cost by the lnk load written into the pointer variable and copied out.

- [s56] E-s56-3: cell K2 = 4 at 127/127 with both copies present and no reader; the residual is exactly two register seats (copy dest v0 vs a3, once per loop), RA solver 15/15 exact.

- [s56] E-s56-4: global_conflicts processes REG_DEAD before note_stores (global.c:755-790), so a copy dest born where its source dies and dying where base is born conflicts with neither a0 holder; target's a3 requires the copy dest to be live past the base add (conflicting with base and a loop-body v0 temporary) with priority below slot_b's 3970 (else a3 is someone-preferred and t0 results), or an a3 copy preference from a hard-reg move (set_preference global.c:1671).

- [s56] E-s56-4: post-flow deleters enumerated (combine merge/fold, optimize_reg_copy_2 self-copies, update_equiv_regs init deletion local-alloc.c:1090-1111, reload / sched2 (sched.c:4955-4975) / jump2 (jump.c:437-490) / final no-op moves, jump2 find_equiv_reg-redundant moves, reorg redundant_insn); pseudo USE insns exist only under obey_regdecls (stmt.c:3271/3499). The reload find_equiv_reg copy route (reload1.c:5835-5853) is closed: global.c allocates every allocno here and regclass never returns NO_REGS (regclass.c:935-960).

- [s56] E-s56-1: func_8005BA8C's closed ledger records zero shared blocks with this function; its H8 lever measured 8 on K2 (prologue pair moves).

## s57 (2026-09-15, structural - the a3 seat under global.c, read end to end)

### E-s57-1 - Chassis re-audit + directive
- src/ings.c at HEAD 44087a2cd carries `INCLUDE_ASM("asm/funcs", func_80017848);` at line 820; s57 tooling (apply.py / run.ps1 / dis.py / norm.py / gen.py / bank.py, copied from s56 with paths rewritten) in tmp/grind/func_80017848/s57/. K2 = 4 at 127/127 (s57/diff_K2.txt), identical to s56. src/ings.c restored to the HEAD anchor at session end.
- The queue directive (auto-return on func_8005BA8C's completion) was executed in s56 (cell K4, 8 at 127/127); acknowledged here.

### E-s57-2 - The v0 preference on the copy dest is an expand_preferences merge (s56 frontier item 2 closed)
- global.c:829-870 expand_preferences: for each single_set with an allocno dest, every REG_DEAD allocno not conflicting with the dest exchanges hard_reg_preferences and hard_reg_full_preferences with it; copy preferences are exchanged only when the dying reg is the SET_SRC. set_preference (global.c:1671-1740) records nothing for `p = q` (two global pseudos); base gets v0 from the loop body's `(set v0-local (plus base i))` (first source operand of a PLUS, copy=0), and p receives it at the add where p dies. `.greg` for K2: `;; 78 preferences: 2`, `;; 78 conflicts: 72 73 74 75 78 80 82 85 86 89 29` (no 79/81).
- prune_preferences (global.c:882-935): regs_someone_prefers[a] = union of hard_reg_full_preferences of LOWER-priority conflicting allocnos, MINUS a's own full preferences (when sizes are equal). So sh's v0 preference (from the guard's `(set v0-local (plus sh q))`) never excludes v0 for p. prune also strips call-used regs from the preferences of every call-crossing allocno (:900), which is why the a0/a1/a2/a3 incoming-argument preferences of ctx/arg1/slot_a/slot_b (all seated in s-regs because they cross the call) can never reach anyone's someone set.

### E-s57-3 - find_reg's preference override, read exactly
- global.c:1097-1115: after the pass-0/1 scan, the LOWEST hard reg in hard_reg_copy_preferences not in `used` and of a compatible class replaces best_reg; :1133-1160 does the same for hard_reg_preferences (non-copy). So an a3 preference wins only if every lower preferred reg (v0 here) is in `used`.
- global.c:340-372: regs_used_so_far is pre-seeded with every call-used reg (`regs_ever_live[i] || call_used_regs[i]`), so the "never allocate a register for the first time in pass 0" rule excludes nothing in v0..t9.

### E-s57-4 - Cell A (param reuse for the a3 copy preference) = 4, and why (combine)
- Cell A `.greg`: `;; 15 regs to allocate: 88 99 111 78 80 75 72 77 90 84 85 79 81 74 73`; `;; 75 conflicts: 72 73 74 75 79 81 84 85 88 90 29`; `;; 75 preferences: 2`; dispositions `75 in 2`, `90 in 19`. Pseudo 75 (the slot_b parameter, now the copy dest) has NO a0/a1/a2 hard conflicts (by the time `(set 75 a3)` executes the a0-a2 argument copies are done) and NO a3 preference: combine merged `(set 75 (reg a3))` into `(set 90 (reg 75))` -> `(set 90 (reg a3))`. combine.c:860-945 can_combine_p: the only hard-reg-source gate is `! HARD_REGNO_MODE_OK` or SMALL_REGISTER_CLASSES (undefined on MIPS); the insns are in the entry block (flow's LOG_LINKS are per block, reg_next_use reset per block) and nothing sets a3 between them (use_crosses_set_p). Any second reader of 75's entry value would read a3 in the bytes (target reads a3 only at the two base adds).
- Cell C (entry copy only) = 4 byte-identical to K2: the folded `(set b a3)` occupies the same prologue slot as the original last argument copy. s56 K4's prologue move for ctx is therefore a first-argument artefact, not a property of the entry-copy lever.

### E-s57-5 - Cell B (copy before the guard temporaries) = 4 at 125
- cse follows the conditional branch's fall-through into the single-predecessor preheader (cse.c cse_end_of_basic_block path following), canonicalises p's only use to q, the copy dies before flow: both preheader copies vanish, `addu a0,a1,a0`. This is the same fold s2-s5 measured for every explicit second read; it also closes the "copy dest born before the guard" route to a v0/a0 conflict without a different block structure.

### E-s57-6 - Routes to a3 for a [copy, add] copy dest, all closed (source citations)
- Scan: needs v0,v1,a0,a1,a2 in `used` (global.c:985-1078). a0: no conflict (global_conflicts death-before-store, :755-790; q dies at the copy, base is born at the add - K2 `.greg` confirms 78 does not conflict with 79/81), no someone (E-s57-2). a1/a2: sh (pri 3750) and lnk (2500) are allocated after p (13333: log2(4)*4/6) so they never enter hard_reg_conflicts; p would need priority < 2500, i.e. live length >= 33 at 4 refs (allocno_compare global.c:636-650, priority = floor_log2(refs)*refs/live_length*10000; live length is flow's REG_LIVE_LENGTH, flow.c:1685/2087, not updated by combine). v0: never someone-excluded (p prefers v0 itself), and no v0 local overlaps [copy, add] in the target's preheader.
- Preference override: needs an a3 event; closed by E-s57-4 (class kill, predicate global.c:1671).
- local-alloc (K1 shape, p local): find_free_reg `used` = fixed/call-used base + regs_live_at over the range + class complement (local-alloc.c:2164-2171); regs_live is seeded from the hard-reg word of basic_block_live_at_start (:1160-1162) and grows only with hard regs and already-allocated local quantities; the target's preheader has no other local quantity -> v0.
- reload copy route: reload1.c:5843-5853 reloads an UNALLOCATED pseudo input by copying from a hard reg that already holds its value (find_equiv_reg); the reload reg comes from potential_reload_regs, which lists unused call-used regs first (reload1.c:3766-3772) - a3 is the only unused a-reg here, so this route would print EXACTLY `addu a3,a0,zero` / `addu a0,a1,a3`. It needs reg_renumber[p] < 0: global.c creates an allocno for every pseudo with reg_n_refs != 0 and reg_live_length != -1 (:414-431; -1 only from flow.c:1240/1260 setjmp; the `-2` parameter marker at :583 has no setter in this tree) and find_reg fails only when the preferred and alternate classes are exhausted (:586-598). reg_n_refs is zeroed only when the pseudo leaves every insn (combine.c:2313/2336; local-alloc.c:1104 after validate_replace_rtx, which cannot substitute a MEM into addsi3's arith_operand). reload's equiv-init deletion (reload1.c:1955-1966) is the only byte-free way to lose the copy's own set. Closed on every expanded-copy chassis; reopened only if a spelling leaves the add's operand as a MEM or an unallocated pseudo at reload time.
- jump1 deletes `p = p` self-assignments before flow (jump.c:437-446, pointer-equal SET_SRC/SET_DEST from expand_assignment, and the true_regnum same-reg test at :456), so a self-copy cannot extend p's flow-time range.

### E-s57-7 - Artifacts
- tmp/grind/func_80017848/s57/: body_{K2,BASE,A,B,C}.c, diff_{K2,A,B,C}.txt, raw_*.txt, B_*.txt, T.txt, ings_HEAD.c, gen.py, apply.py, run.ps1, dis.py, norm.py, bank.py, hyp_s57.md, ev_s57.md, outcome_s57.json; tmp/grind/func_80017848/dumps/ings.{greg,lreg,combine,...} (last generated with body_A applied).

- [s57] E-s57-2: the copy dest's v0 preference is expand_preferences (global.c:829) merging base's v0 preference at the add; prune_preferences (global.c:920-928) never lets someone_prefers exclude a reg the allocno itself fully prefers, and strips call-used prefs from call-crossing allocnos (:900), so the s-reg-seated parameters can never someone-exclude a0..a3 for anyone.
- [s57] E-s57-4: cell A = 4 at 127/127; combine folds the incoming `(set 75 a3)` into the entry copy's single use (combine.c:860-945, no SMALL_REGISTER_CLASSES on MIPS), so the parameter pseudo never carries the a3 preference; cell C (entry copy alone) is byte-neutral.
- [s57] E-s57-6: every global.c/local-alloc route to a3 for a [copy, add]-ranged copy dest is closed by source reading; reload's find_equiv_reg copy into the first spill reg (a3, reload1.c:3766/5843) prints the target's exact pair but needs an unallocated pseudo, which global.c never leaves for a GR_REGS allocno (global.c:414-431, :586-598).

- [s57] E-s57-2: the copy dest's v0 preference is an expand_preferences merge from base (global.c:829); prune_preferences removes an allocno's own full preferences from its someone set (global.c:920-928) and strips call-used prefs from call-crossing allocnos (global.c:900), so the s-reg parameters can never someone-exclude a0..a3 for anyone.

- [s57] E-s57-3: find_reg's copy-preference and hard-preference overrides (global.c:1097-1115, :1133-1160) pick the LOWEST preferred reg not in `used`; regs_used_so_far pre-seeds every call-used reg (global.c:340-372).

- [s57] E-s57-4: cell A = 4 at 127/127, cell C = 4 byte-identical to K2; combine folds the incoming `(set 75 a3)` into the entry copy's single use (combine.c:860-945, no SMALL_REGISTER_CLASSES on MIPS; LOG_LINKS are per block), so a parameter pseudo reused as the copy dest never carries the a3 preference.

- [s57] E-s57-5: cell B = 4 at 125/127; cse path-following into the single-predecessor preheader folds a copy dest born before the guard temporaries into q.

- [s57] E-s57-6: for a copy dest with range [copy, add], a0 can never be in the pass-0 `used` set (no conflict by death-before-store, global.c:755-790; no a0 someone-preference survives pruning), a1/a2 need p's priority below lnk's 2500 (live length >= 33 at 4 refs; allocno_compare global.c:636-650, REG_LIVE_LENGTH from flow.c:1685/2087, not updated by combine), and v0 cannot be someone-excluded; local-alloc's `used` is hard regs live in the block only (local-alloc.c:1160-1162, :2164-2171).

- [s57] E-s57-6: reload's find_equiv_reg copy route (reload1.c:5843-5853) with the first spill reg a3 (reload1.c:3766-3772, unused call-used regs first; a3 is the only unused a-reg here) would print exactly `addu a3,a0,zero` / `addu a0,a1,a3`, but needs reg_renumber[p] < 0, and global.c allocates every GR_REGS allocno it creates (global.c:414-431 allocno creation, :586-598 find_reg fails only on class exhaustion; reg_n_refs is zeroed only when the pseudo leaves every insn, combine.c:2313/2336, local-alloc.c:1104).

- [s57] jump1 deletes `p = p` self-assignments before flow (jump.c:437-456), so a self-copy cannot extend the copy dest's flow-time live range.
