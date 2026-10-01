# Hypothesis ledger — func_8006BD28

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: active; INCLUDE_ASM (`src/text1b_tu1c.c:5216`), 103 insns. Banked `preauth_body.c` contains an `__asm__ volatile("" : "=r"(idx) : "0"(idx))` barrier (sandbox strips it) and stale callee names (saMotionSet / initTexPage / ot_Link = func_8006E480 / SetDrawMode / AddPrim on main). Authorization commit 6e0476f0f lists the 37 cheats it needed before going to asm (26 frame-cascade substitutions, inner-loop rewrite, 4-insn prologue reorder, the barrier). The 2026-08-06 "stands" ruling is superseded; de-authorized 2026-10-01.
- CONSTRAINTS: no admissible asm (gate says C); barrier gone; frame pads banned except via the approved pad exception; register pins diagnostic only.
- BLOCKER [I]:
  (A) 8-byte frame skew, ~20-22 of 48: target frame 0x58, locals 0x18-0x2F, only 0x18 (`base`) and 0x20 (`arg0*8`) used as spill slots; natural frame was 0x50. The old "spill at 4 mod 8" diagnosis came from the big-endian bug fixed by `-mel` on 2026-08-04 (0aa210037; `docs/grind/journal.md:8-9`) - those 26 substitutions predate it. Probably one phantom slot still missing.
  (B) inner-loop order, ~15: stores to +0x18/+0x1C/+0x14 precede the flag branch (BDD4-BDE0); has_color branch inverted (store-0 block falls through first). Banked body computes the flag first via an AND chain.
  (C) prologue order / param copies, ~4-6.
- PLAN:
  1. `dossier` + `sandbox --disable all --diff --candidate` (preauth body with main's callee names) to confirm the A/B/C split.
  2. Type arg2 as the existing 0x2C draw descriptor `EnvB` (`src/text1b_tu1c.c:6115`) or `EnvA` (~5853); hoist the typedef above :5216 rather than adding a 4th copy (aggregate-merge-family risk).
  3. Reorder the inner loop: `x=0; y=arg1; ot_idx=8;` then `if (arg0!=0x12 || j==arg3 || j==2) has_color=0; else has_color=1;` then `semi=0; out=D_800A36E4; table=&pairs[j]; D_800A36E4=func_8007352C(env);`.
  4. Frame: walk the phantom-slot producers - the `n` guard (target `beqz s2`), a narrow flag local, a named `header` local (read twice). Measure via `.frame vars=`.
  5. Then spill/reg choice: s7=0x12 and fp=8 are hoisted constants; base/offset spill; address shape `offset + (i*4 + base)` ~ `((s32*)base + i)[arg0*2]`.
- DEPENDS: same file as func_800693CC (land that first). `sdata_funcs.txt:247` already lists it (GP-relative globals).
- ODDS/LANE: multi-session, moderate. Manual deep-dive (or Grinder once restart approved).

## 2026-10-01 laneA — CLOSED to sandbox 0 (candidate.c); details evidence.md
- Plan steps 2-5 resolved: arg2 typed `S_6A880 *` (already declared above; caller retyped, byte-neutral);
  frame skew was allocation, not a missing pad (giv j*8 unreduced + `arg0*8` hoisted -> 0x58 naturally);
  two FAKE constructs with receipts in rejected/ (cells named intermediate; `(sheets + i) + arg0 * 2` grouping).
