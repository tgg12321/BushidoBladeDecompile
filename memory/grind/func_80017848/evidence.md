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
