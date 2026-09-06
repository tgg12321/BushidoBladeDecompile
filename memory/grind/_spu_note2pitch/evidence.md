# Evidence bank — _spu_note2pitch

## s1 (2026-09-06, recon) — floor 37 -> 2, mechanism fully attributed

OBJECT MODEL: the function touches NO globals (pure leaf arithmetic on four u16
params; asm/funcs/_spu_note2pitch.s has no %hi/%lo and no memory access except
one 0x8($sp) spill). No DATA MODEL signals apply. The only declaration involved
is its own prototype: include/m2c_context.h:1184 is `u16 _spu_note2pitch(u16,
u16, u16, u16);` at HEAD — MATCHES (the four `andi $aN,$aN,0xFFFF` entry
truncations and the `andi $v0,...,0xFFFF` return truncations come from the u16
params / u16 return; present in every measured form). Sole caller
func_8008B488 is INCLUDE_ASM, so the prototype has no caller-side codegen.

- CROSS-KNOWLEDGE: memory/closer/phase3-progress.md:784 and
  memory/grind/_spu_gcSPU/psyz-seed-notes.md:29 — BB2 links the PsyQ 4.0
  curve-scan (0x103B) algorithm; SOTN's s_n2p.c is the table-lookup revision
  and psyz stubs it. No source seed anywhere; self-decomp only. Confirmed.
- The retired-chassis body (retired-chassis-2026-08/body.c, 37 rules) used
  three `volatile s32 spN` frame-coercion locals and a longhand loop; discarded
  as a starting point (forbidden family + wrong structure).
- STRUCTURE (measured, form A, score 37 -> 18): the target is a prefix (cents
  diff -> |diff| -> /1536 quotient+remainder -> 0x1000<<oct or 0x1000>>oct with
  the rem!=0 adjust) followed by the COMPLETED-C sibling `_spu_2pitch`
  (src/main.c, immediately above) INLINED. Marking the sibling GNU89 `inline`
  makes GCC 2.7.2 integrate.c inline it; the sibling still emits out-of-line and
  stays byte-identical (sandbox _spu_2pitch --disable all = 0 with the keyword).
  The inlined copy reproduces the one-mult loop shape, the `upper` spill to
  0x8($sp), and the 16-byte frame. Longhand copies of the same loop (forms
  B/C/D/E) give frame 8 with the spill at 0($sp); the extra 8 bytes only appear
  through the inline path (scratch tests in tmp/grind/_spu_note2pitch/s1/fr
  give vars=16 for every helper body in a trivial caller, but the static inline
  helper inside the real function gave 8 — unresolved detail, moot because the
  `inline _spu_2pitch` path is right by every other measure).
- DIVMOD MOVE (form A, applies memory/reference/divmod-coalesce-reuse-var):
  `rem = absdiff / 1536; oct = rem; rem = absdiff - oct * 1536;` reproduces the
  target's `addu a0,v1` (quotient moved to oct) with the remainder reusing v1.
- ABS IS ONE INSN: the target's `bgez v1,L / move v0,v1 / negu v0,v0` is the
  mips.md `abssi2` output template (tools/gcc-2.7.2/config/mips/mips.md,
  define_insn "abssi2": `%(bgez %1,1f; move %0,%1; subu %0,$0,%0; 1:%)`). It is
  produced by fold-const.c:4861-4888 turning `(A < 0) ? -A : A` (or `(A >= 0)
  ? A : -A`) into ABS_EXPR at tree level; under -fno-builtin `abs()` would be a
  call, so the source used the ternary (or a macro of it). Being a single insn
  the scheduler can place it between the inlinee's first statements and
  `steps = rem >> 5`, which is why it sits mid-block in the target. A statement
  `if (rem < 0) rem = -rem;` is a real branch that splits the block (forms
  B/D/E, score 21/23). Forms F/G (ternary, either spelling): score 2.
- RESIDUAL 2 (forms F/G): order of `andi a2,v0,0xFFFF` (u16 base -> u32 atten
  actual) vs `addiu a0,zero,0x103B` (inlinee's `ratio` init) at the join
  label. Target: andi first. Ours: li first. Pass attribution from
  tmp/grind/_spu_note2pitch/dumps (main.sched, block 7): decided in sched1.
  sched.c adjust_priority (reload_completed==0) boosts a "birthing" insn to
  LAUNCH_PRIORITY; birthing_insn_p (tools/gcc-2.7.2/sched.c:2512-2526) = SET of
  a live REG with reg_n_sets == 1. The andi's dest is integrate.c's parm-copy
  pseudo (created at tools/gcc-2.7.2/integrate.c:1447 because the zero-extend
  temp is not REG_USERVAR_P) — single-set -> boosted -> picked first in the
  backward list scheduler -> emitted LAST (after the li). `ratio` has 3 sets
  (loop updates) -> not boosted. With equal priorities the LUID tie-break in
  rank_for_schedule puts the andi first, as in the target. reg_n_sets is
  counted by flow.c after dead-store deletion, so a dead second assignment does
  not count (and would be a cheat anyway).
- PROOF OF MECHANISM (form L, score 6): `u32 atten = base; pitch =
  _spu_2pitch(atten, ...)` with a second LIVE set of atten (the clamp written
  into atten in both arms) -> the andi/li order MATCHES the target; the only
  residual is the tail move from the contrived second set. Form H
  (`base &= 0xFFFF` on a u32 base, score 3) also fixes the order but masks in
  place (andi a2,a2); the target masks v0 -> a2, so the arms' value and the
  masked value are DIFFERENT pseudos, the second being a multi-set user
  variable. Form K (`pitch = base; pitch = _spu_2pitch(pitch,..)`, score 16):
  the actual overlaps the call target -> integrate.c:1305 copies it -> seats
  rotate. Form M (arms into u32 atten, `base = atten; atten = base;`, score 3):
  same-pseudo andi like H.
- Frame facts: `.frame $sp,16` vars=16 for both the standalone sibling and the
  inlined form (tmp/grind/_spu_note2pitch/dumps/main.s).
- Sandbox JSON reports the metric as "score" (not "distance"); the diff tooling
  for this function is tmp/grind/_spu_note2pitch/s1/run.sh + norm.py (objdump
  of tmp/sandbox/_spu_note2pitch/main.o normalised against the splat listing).

- [s1] OBJECT MODEL: no globals touched; prototype include/m2c_context.h:1184 u16 _spu_note2pitch(u16,u16,u16,u16) MATCHES (entry/return andi truncations reproduced); sole caller func_8008B488 is INCLUDE_ASM.

- [s1] Target = prefix + inlined _spu_2pitch: the sibling must carry the GNU89 inline keyword; measured byte-neutral for the sibling (sandbox _spu_2pitch = 0).

- [s1] Residual pair (andi a2,v0,0xFFFF vs addiu a0,zero,0x103B) is decided in sched1: sched.c adjust_priority boosts birthing insns (birthing_insn_p sched.c:2512-2526 = set of a live reg with reg_n_sets==1); the andi writes integrate.c's single-set parm-copy pseudo (integrate.c:1447, made because the zero-extend temp is not REG_USERVAR_P), ratio's init is multi-set, so the andi is emitted last; equal priorities fall to the LUID tie-break which yields the target order.

- [s1] Form L (u32 atten = base passed as actual, second live set of atten in the clamp) scores 6 with the andi/li order matching: proof the receiver-multi-set lever works; its second set must be found in a byte-neutral spelling.

- [s1] reg_n_sets is counted by flow.c after dead-store deletion: a dead second assignment does not count (and is a cheat family anyway).

- [s1] cc1 -da dumps for form F are in tmp/grind/_spu_note2pitch/dumps (main.sched block 7 shows the tie); scratch frame bisection in tmp/grind/_spu_note2pitch/s1/fr.

## s2 (2026-09-06, structural) — floor 2 -> 0, MATCHED (verify-oracle ok:true)

- The s1 residual (order of `andi $a2,$v0,0xFFFF` vs `addiu $a0,$zero,0x103B` at
  .L8008BBB0) was NOT a scheduling problem to be worked around — it was a
  STRUCTURE signal that s1 mis-read. The target's single `andi` sits AFTER the
  join label, so it truncates the value produced by BOTH arms of the sign
  branch. That means the original source narrowed the octave base to 16 bits
  INSIDE EACH ARM, and jump2 cross-jumping re-merged the two identical `andi`
  tails into one after the join. s1's spelling (one `u16 base` assigned in both
  arms, widened once at the call) puts the truncation after the join in the
  SOURCE, which is a different program shape and left the 2-insn residual.
- MEASURED (HEAD chassis, `inline _spu_2pitch`, no FAKE constructs anywhere):
    vF  s1 candidate (u16 base, widened at the call)                        = 2
    vS  narrowing only the up-shift arm, down arm bare `0x1000 >> oct`      = 4
    vN  `u16 base` per arm + `atten = base;` duplicated into both arms      = 0
    vQ  `atten = (u16)(0x1000 << oct);` / `(u16)(0x1000 >> oct);` per arm   = 0
    vR  vQ with the prefix abs written as a ternary                         = 0
  vQ/vR and vN are the same shape (a u32 receiver written in both arms); the
  cast form is the chosen one because it carries no extra variable.
- MECHANISM (consistent with s1's attribution, now confirmed by the fix): with
  the narrowing in both arms the widened receiver `atten` has TWO static sets at
  sched1 time, so sched.c `birthing_insn_p` (tools/gcc-2.7.2/sched.c:2512-2526,
  `reg_n_sets[i] == 1`) no longer boosts the `andi` to LAUNCH_PRIORITY and the
  pair falls to the LUID tie-break, which is the target order. jump2's
  cross-jump then merges the two arm-local `andi`s back into the single
  post-join instruction.
- DIVMOD: s1's `rem = absdiff / 1536; oct = rem; rem = absdiff - oct * 1536;`
  (the divmod-coalesce-reuse-var reference recipe) is NOT needed on this chassis
  and was DROPPED — the plain `oct = absdiff / 1536; rem = absdiff % 1536;`
  scores 0 by itself. This removes the only variable-reuse construct from the
  body. The remainder MUST be spelled with `%`:
    `oct = absdiff / 1536; rem = absdiff % 1536;`                = 0
    `rem = absdiff % 1536; oct = absdiff / 1536;` (order swapped) = 37
    `oct = absdiff / 1536; rem = absdiff - oct * 1536;`           = 36
    `... rem = absdiff - (oct * 1536);`                           = 36
    s1's quotient-through-rem reuse + `- oct * 1536`              = 0
  Declaring `rem` before `oct` with the `%` form is also 0 (declaration order is
  free here).
- The two ternary abs forms (`(diff < 0) ? -diff : diff` and the call actual
  `(rem < 0) ? -rem : rem`) each materialize a mips.md abssi2 template that is
  present in the target (0x8008BB4C and 0x8008BBC4). `rem` is non-negative on
  every reachable path, so the second one is defensive normalization in the
  original library source — its three instructions are in the shipped bytes.
- FINAL: sandbox _spu_note2pitch = 0, sandbox _spu_2pitch = 0 (the `inline`
  keyword leaves the sibling byte-identical), full `verify-oracle` ok:true.
  Body = memory/grind/_spu_note2pitch/candidate.c, in place in src/main.c.

- [s2] The target's single post-join `andi $a2,$v0,0xFFFF` proves the original narrowed the octave base to u16 in BOTH arms of the sign branch; jump2 cross-jumping merges the two arm-local truncations. Per-arm narrowing gives the receiver two static sets, defeating sched.c birthing_insn_p's reg_n_sets==1 boost, which was the whole 2-insn residual.

- [s2] Post-join narrowing = 2, up-arm-only narrowing = 4, per-arm narrowing = 0. The score-4 measurement is what discriminates: without an identical tail in both arms there is nothing for jump2 to cross-jump.

- [s2] The divmod-coalesce-reuse-var spelling is NOT required here: plain `oct = absdiff / 1536; rem = absdiff % 1536;` = 0. But `%` is load-bearing (`absdiff - oct * 1536` = 36) and so is the quotient-first statement order (`%` before `/` = 37).

- [s2] MATCHED: floor 0, full-build verify-oracle ok:true, pure C, no FAKE constructs, no sanctioned-family claim. self_vet.md written.
