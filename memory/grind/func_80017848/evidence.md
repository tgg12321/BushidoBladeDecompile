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
