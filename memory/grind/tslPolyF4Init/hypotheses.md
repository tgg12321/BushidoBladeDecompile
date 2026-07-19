# Hypothesis ledger — tslPolyF4Init

## KILLED

### H-s1: `count = 3;` first (before idx/saved/base/elem) fixes s0 save-slot position
- **Mechanism proposed:** move s0's first-write to the top → GCC's
  save_restore_insns emits `sw s0` early in the prologue matching target's
  4th-slot placement.
- **Probe:** swapped assignment order so count=3 is first, then idx, base,
  saved, elem.
- **Result:** sandbox 8 → **13**. Regressed. First-write ordering of s0
  alone does not drive prologue save ordering the way this hypothesis
  supposed; forcing s0-first PENALIZES the other saves' positions worse
  than the s0 win.
- **Verdict:** KILLED. Do not re-propose plain "reorder assignments" without
  addressing WHY the sibling first-arg (a1) triggers its save first in
  target. Rejected form: `rejected/count-eq-3-first.c`.

## FRONTIER (build from here)

### H-s2: `saved = g_cd_callback_a;` FIRST (before any arg-touching)
- **Mechanism:** target's earliest `sw` after ra-reservation is `sw s1`, but
  s5 (saved) is loaded at slot 15. Yet a1 (→ s1) is not touched in current
  C until inside the loop's `if (a1 != 0)`. If `saved = g_cd_callback_a`
  is moved to top AND expressed via an idiom that references a1/a2 as a
  side effect, s1/s2 save-slot placement might drift toward target.
- **Probe:** try `saved = g_cd_callback_a; ... /* args used later */`.
  Alternative: eliminate `base` local (`elem = &g_cd_sector_buf[idx];`
  like the sibling) to reduce pseudo count and see if RA-scheduling picks
  a different sw insertion order.

### H-s3: `elem = &g_cd_sector_buf[idx]` (drop `base` intermediate)
- **Mechanism:** current source uses `base = g_cd_sector_buf; elem = base +
  idx;` — two named pseudos. Sibling uses one `elem = &g_cd_sector_buf[idx]`.
  Fewer pseudos → different first-use sequence → possibly different
  prologue interleaving. Sibling reaches its match with this shape (though
  paired with the cheat), so shape alone is worth testing without the pin.
- **Probe:** rewrite as `elem = &g_cd_sector_buf[a0 & 0xFF];` (also removes
  the `idx` first-write, forcing andi to appear where target has it —
  between `move s4,a0` and `count = 3`).

### H-s4: Loop-entry restructure — hoist first iteration's setup
- **Mechanism:** target puts `sw ra` LAST (slot 21) but our build also does.
  What if the loop label placement / `goto loop` shape steers sched1's
  choice for where callee-save sw's go? An `while (count-- != -1)` header
  vs the current `loop:` label could change basic-block boundaries and
  thus save-insertion decisions.
- **Probe:** rewrite `loop: ... if (count != -1) goto loop;` as
  `do { ... } while (--count != -1);` — measure whether the body
  restructure alters the prologue save ordering. NB: check that this
  doesn't introduce a do-while(0) or a semantics change; the guard is
  `count = 3; --count == 2, 1, 0, -1` — first-iter unaffected.

## NOT-TO-PROPOSE (from cheat siblings)

- `register s32 result asm("s7"); result = 0;` at top — [[register-asm-pins]]
  forbidden.
- `unsigned long long new_var2 = new_var; count = new_var2;` dummy chain —
  dead-scalar-coercion + un-annotated fake, forbidden per
  [[named-local-fake-exception]] prerequisites (would need dual review +
  documented lever-exhaustion at minimum).
- `s32 buf[N]` or `(void)&local` frame coercion — [[dead-vars-local-array]]
  banned.

## [s1] Moving `count = 3;` to the first local assignment (before idx/saved/base/elem) will front-load s0's save-slot to match target's 4th-position placement.
- mechanism: GCC save_restore_insns emits sw N-save at the point the pseudo becomes live; earlier first-write => earlier sw.
- probe: Swap assignment order so count=3 is first; sandbox tslPolyF4Init --disable all.
- result: sandbox 8 -> 13 (regressed by 5). Reordering s0 alone worsens the other save-slot placements more than it improves s0's.
- verdict: KILLED

## [s2] H-s3: dropping `base` local (`elem = &g_cd_sector_buf[idx];`) reduces pseudo count and may re-interleave prologue saves toward target
- mechanism: Fewer named pseudos => different first-use graph; sibling shape without cheat pin.
- probe: Removed `base` decl, wrote `elem = &g_cd_sector_buf[idx];` in place of the base+add.
- result: sandbox 8 -> 13 (regressed by 5). Sibling shape without the register-asm pin + dead-scalar coercion does not reach match; fewer pseudos worsens save-slot placement here.
- verdict: KILLED

## [s2] H-s4: `do { ... } while (--count != -1);` restructure changes basic-block boundaries so save-restore_insns land in target order
- mechanism: Loop shape affects NOTE_INSN_LOOP_BEG emission and basic-block boundaries.
- probe: Rewrote loop as do-while with continue-equivalent via goto next; sandbox.
- result: sandbox 8 -> 17 AND insn count 81 -> 83. do-while emits an extra loop-preservation cycle; not a save-order lever, adds instructions.
- verdict: KILLED

## [s2] H-s5 (new): placing `count = 3;` BETWEEN `saved` and `idx` (`saved; count=3; idx; base; elem;`) makes count's pseudo the winner for hard reg s0
- mechanism: In this position count's first-write in the RTL precedes idx's, and its allocno ranks above a1's in global.c's priority sort; a1 shifts from s0 to s1, count from s1 to s0 — matches target's s0=count/s1=a1 assignment. The 4 sw of s0 -> position 4 in the prologue interleave matches target (s0 was 7th in old build, is 4th in target).
- probe: Wrote `saved = g_cd_callback_a; count = 3; idx = a0 & 0xFF; base = g_cd_sector_buf; elem = base + idx;` in that order; sandbox.
- result: sandbox 8 -> 4. Confirmed via objdump: build now has s0=count / s1=a1 / s2=a2 / s3=idx / s4=a0 / s5=saved / s6=elem — identical to target allocation. Remaining 4-insn diff is the arg-copy interleave at prologue top (build starts with sw s4/move s4; target starts with sw s1/move s1/sw s2/move s2 THEN sw s4/move s4).
- verdict: CONFIRMED

## [s2] H-s5b: swapping the local declaration order (`saved` decl first) further shifts allocation
- mechanism: Decl order affects LUID and secondary allocno tiebreaks.
- probe: Moved `s32 saved;` to be the first local declaration (init order unchanged).
- result: sandbox stayed at 4 - neutral. Declaration order has no observable effect once init order is fixed.
- verdict: KILLED

## [s2] H-s5c: `base = g_cd_sector_buf;` before `idx = a0 & 0xFF;` (base written before touching a0) shifts allocation further
- mechanism: Loading the global before the a0-touch might delay s4's first-use to after s5/s6.
- probe: Swapped `base = ...` to precede `idx = a0 & 0xFF;`.
- result: sandbox 4 -> 8 (regressed). Splits s5/s6 first-uses in a bad way; the count-before-idx placement is the sweet spot.
- verdict: KILLED

## [s2] H-s5d: `count = 3;` FIRST (before `saved`) further front-loads count
- mechanism: Increases count's priority even more relative to a1.
- probe: Moved `count = 3;` to first statement.
- result: sandbox 4 -> 13 (severely regressed). Duplicates the H-s1 finding: count-alone-first hurts every other save slot; the sweet spot is count SECOND, after saved's global load.
- verdict: KILLED

## [s2] H-s5e: split-init `count = 4; --count;` increases count's ref count enough to lock in its priority over a1
- mechanism: Extra ref via decrement.
- probe: Wrote `count = 4; --count;` above idx; sandbox.
- result: sandbox stayed at 4 - neutral. GCC folds the decrement away (constant propagation) so refs unchanged.
- verdict: KILLED
