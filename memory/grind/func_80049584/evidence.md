# evidence — func_80049584 (src/text1b.c)

## Session 1 (recon modality, 2026-08-17) — closed at honest distance 0

### Starting state
- `canonical` verdict **C** (pure-C distance 26 <= 50; 0 asm insns; 99 target insns).
- `sandbox --disable all` floor **26** on the inherited body.
- The inherited body was a pin-laden m2c transcription: five
  `register T x asm("$N")` pins ($18/$17/$16/$5/$6), an unused `s32 dummy[2]`
  frame-padding array, and a trailing `(void) var_s2;`. All score-inert (the
  sandbox strips them), i.e. the 26 was already the honest floor.

### Function semantics (read off asm/funcs/func_80049584.s + the inherited body)
1. Loop A (0x3A iterations): copy `D_800EF980[k]` into `D_80099C50[k]`; if the
   two entries' SIGNS differ at any k, clear a `unchanged` flag. The sign test
   compiles as the classic GCC-2.7.2 `nor rX,zero,rY; srl rX,rX,31` pair, i.e.
   the C is a comparison of two `>= 0` boolean VALUES, not a branch chain:
   `if ((v >= 0) != ((*dst) >= 0))`.
2. Loop B (0x3A iterations): re-rank `D_800EF980` in place — every non-negative
   entry is overwritten with a densely increasing counter (`bltz` skip).
3. `step = (D_800A33EC == 0) ? 0x10 : 8`.
4. Normalize the pair (`D_800A33E8`, `D_800A33EA`) into an ordered (lo, hi):
   the `-1` sentinel cases short-circuit to a constant total
   (0x24, or 0x88 when `D_800A33EC == 0`) via a `goto end`.
5. Otherwise `total = func_8004954C(step, lo, hi)` (the already-COMPLETED-C
   sibling immediately above it in text1b.c).
6. If `D_800A324C != total`, store it and clear `unchanged`; if `unchanged` is
   clear, run the three-call refresh
   (`func_80046020(); func_80045B68(D_800A33EC, total, D_800EF980, arg0); func_8003E120();`).

### Measured floor ladder (this session, all `sandbox --disable all`)
| form | floor |
|---|---|
| inherited pin/dummy body | 26 |
| clean natural-C rewrite (no pins, no dummy array, separate `total`, loop B reusing loop A's pointer) | 23 |
| + loop counter and call result share ONE variable | 16 |
| + loop B walks its OWN pointer variable (`p`) instead of reusing `src` | **0** |
| (control) merged-counter form BUT with a separate `total` again | 12 |

### The three facts the residual decomposed into
- **F1 — frame size.** Target reserves `sp,-0x28` with saves at 0x18/0x1C/0x20/0x24;
  the clean rewrite produced `sp,-0x20` with saves at 0x10..0x1C, i.e. 8 bytes of
  locals the clean C did not allocate. This is what the inherited body's
  `s32 dummy[2]` was faking. It is NOT an independent phantom-slot problem:
  the 8 bytes appeared on their own the moment the loop counter became
  live across the `jal` (see F2) — one extra call-saved reg has to be saved,
  which is exactly the delta. No frame-coercion construct was needed.
- **F2 — the loop counter lives in `$s0` in target.** In target BOTH loop
  counters and the final total are `$s0`. A pseudo is only eligible for a
  call-saved hard reg when it crosses a CALL; neither loop counter crosses one.
  Therefore the counter can only be `$s0` if it IS the same pseudo as the
  call result. Confirmed by measurement in both directions: merging drops
  23 -> 16, and re-splitting the merged form costs 0 -> 12 with the entire
  12-insn residual being `i` moving from `$s0` to `$a2` plus the resulting
  prologue reschedule. This is a forced conclusion, not a preference.
- **F3 — loop B uses a DISTINCT pointer variable.** Target's loop-A source
  pointer is `$a2` and its loop-B pointer is `$v1` — two different hard regs,
  so two different pseudos. Reusing one C variable for both walks collapses
  them into one allocno and mis-assigns the whole `a1/a2/v1/a0` cluster
  (16 insns). Giving loop B its own `s16 *p` closed the remaining 16 to 0.

### Final form
Pure C. Zero regfix/asmfix rules, zero `register asm()` pins, zero inline asm,
zero volatile, zero dead stores, zero unused declarations, no frame coercion.
The ONLY non-obvious construct is F2's single-variable reuse (loop counter +
computed total), which is the frozen SOTN-sanctioned "variable reuse for
codegen control" family and carries a `/* FAKE: ... */` annotation naming the
mechanism. See self_vet.md.

## Session 2 (recon modality, 2026-08-17) — re-verification after a validator discard

Session 1's work was DISCARDED by the driver validator on a self_vet FORMAT
defect only, not on merit: the `SCOPE:` quote was wrapped across two physical
lines, and `tools/grinder/grindlib.py:51` matches `SCOPE:` with a single-line
anchored regex (`^\s*SCOPE\s*:\s*["“](.+?)["”]\s*$`), so it counted 0 quoted
scope sentences against 1 `FAMILY:` block. The src/ edits were reverted with the
discard, so this session re-applied `candidate.c` to `src/text1b.c` verbatim and
re-measured.

- `sandbox func_80049584 --disable all` → **score 0**, target_insns 99,
  build_insns 99, rules_dropped 0. Honest floor 0 re-confirmed with the edits
  live in `src/text1b.c` (captured at `tmp/grind/func_80049584/s1/sandbox_final.json`).
- `self_vet.md` rewritten: the scope sentence is now a single physical line, and
  the precedent is a hard `file:line` citation
  (`.claude/rules/no-new-park-categories.md:171`).
  `grindlib.validate_self_vet` now returns `(True, '')`.
- No new C forms were tried this session and none were needed; session 1's H1–H4
  findings stand unchanged and are not re-derived here.

**Lesson for future sessions on any function:** the `SCOPE:` line in self_vet.md
must be ONE physical line with the opening and closing quote on it. Wrapping it
for readability silently fails the mechanical validator and costs an entire
session.
