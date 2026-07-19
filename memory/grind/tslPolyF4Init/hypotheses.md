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
