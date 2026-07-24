# Evidence bank — func_8001924C

## s1 recon (2026-07-23) — baseline + phantom-frame-slot map, structural TWIN identified

- [s1] BASELINE: `sandbox func_8001924C --disable all` = **12**, target_insns=49,
  build_insns=49, rules_dropped=0, cheat_asm_stripped=185. Canonical verdict C.
  All 49 instructions match in shape; the 12 diffs are purely sp-relative offset
  shifts from a single +8-byte frame-size delta. Body is byte-identical to target.

- [s1] FRAME MECHANISM (measured, minrepro tmp/grind/func_8001924C/s1/minrepro.c,
  cc1 exact project flags -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls):
  * pad[2] present -> `.frame $sp,48,$31  # vars= 8, regs= 5/0, args= 16, extra= 0`
    == target frame 0x30 (asm/funcs/func_8001924C.s:2 `addiu $sp,$sp,-0x30`).
  * pad removed    -> `.frame $sp,40,$31  # vars= 0` = honest floor form (frame 0x28).
  So `s32 pad[2]` is the SOLE producer of the 8-byte locals slot. Frame equation
  (mips.c compute_frame_size, o32): 0x30 = ALIGN8(vars) + ALIGN8(16 args) +
  ALIGN8(20 gp_regs = 5*4) -> vars in [1..8]. Target locals region = sp+0x10..0x17
  (8 bytes), NEVER written and NEVER read (target has zero `sw ...,0x10/0x14($sp)`;
  the 5 saves are at 0x18/0x1C/0x20/0x24/0x28).

- [s1] HEAD SHIPS THE CHEAT: src/code6cac.c:611 currently carries `s32 pad[2];`
  (FORBIDDEN unused-local-array frame coercion, [[dead-vars-local-array]] /
  [[inline-asm-policy]] expanded catalog). It makes the full build oracle-green but
  the honest pure-C distance is 12. INCOMPLETE. 2026-07-01 written-never-read
  carve-out does NOT apply (target writes none of the 8 hole bytes -> no
  oracle-backed dead-store evidence). rejected/pad2-unused-local-array-frame-coercion.c

- [s1] **STRUCTURAL TWIN = func_80049A2C (OWNER-GATED 2026-07-20).** Identical
  species: target frame 0x30, saves s0/s1/s2/s3/ra at 0x18..0x28, 8-byte zero-store
  phantom hole. The twin was exhausted across s1..s5:
  * cc1 -da greg proof: phantom-slot mechanism does NOT fire (dummy-in vs dummy-out
    greg passes ZERO delta; all 7 pseudos hard-reg allocated). +8 slot is 100%
    attributable to the aggregate declaration. Aggregate-only.
  * phantom-injection H1/H2/H3, scalar-widening, scalar-dummy, struct-aggregate H8
    all KILLED.
  * scan_hand_coded LOW 0/8 -> canonical-asm route UNAVAILABLE.
  * OWNER-ESCALATION filed docs/grind/decisions.md:954; disposition owner-gated
    (endgame-lock-disposition species).

- [s1] CANONICAL-ASM ROUTE for func_8001924C: `scan_hand_coded --single
  func_8001924C` = LOW, S1..S8 ALL negative (0 multu pairs, no empty branches,
  49 insns/5 spills/7 regs, max load burst 2, no sibling cluster, no BIOS jtbl,
  all $sN saved, no redundant mask). No hand-coded evidence — path (b) of
  [[endgame-lock-disposition]] (canonical-asm ONLY with hand-coded evidence)
  is UNAVAILABLE, same as the twin.

- [s1] KILLED (measured, minrepro himode variant) — naive HImode-bitwise phantom
  injection. `s16 fa=*(s16*)(s0+2); s16 fb=1; if((fa&fb)&1)` -> `.frame vars= 0`
  (no phantom slot). fb=1 constant folds `(fa&1)&1`->`fa&1`; sign-ext combined away
  before any stack temp reserved; also changes lbu->lh (diverges). The witness form
  (tslLineG5Init `(v1 & ~mask) & 1`) requires TWO non-constant s16 GLOBAL loads AND
  a real emitted store GATED by the bitwise expr. func_8001924C's target body has
  NO conditional store to gate (both arms just make calls at 49 total insns), so the
  witness shape cannot be replicated without emitting instructions the target lacks.
  rejected/himode-bitwise-phantom-inject-vars0.c

## s2 structural (2026-07-23) — BREAKTHROUGH: phantom fires in pure C; sandbox 0

- [s2] **H1 (own greg dump) KILLED as an owner-gate premise — but in the GOOD
  direction.** cc1 -da on nopad (honest, vars=0) vs minrepro (pad, vars=8):
  Register-dispositions are BYTE-IDENTICAL (both: 5 regs 72-76 all hard-allocated,
  same conflicts, Hard regs used 2 4 5 16 17 18 19 31). So the pad-array +8 is
  aggregate-only AND no pseudo is denied a reg in the do-while form — confirming
  s1's read. BUT this does NOT close the axis, because a DIFFERENT structural form
  makes a live pseudo go unallocated (below). Artifacts: s2/nopad.c.greg, s2/minrepro.c.greg.

- [s2] **The phantom-frame mechanism DOES fire for func_8001924C in pure C.**
  Rewriting the loop as a for/while loop makes cc1 emit vars=8 (frame 0x30, == target)
  with NO pad array. Mechanism (measured, s2/v5.c.loop + .flow): loop.c generates the
  loop-entry guard by duplicating the exit test into a pseudo —
  `(insn 111  reg101 = (i < arg1))  ; slt`, `(jump 112 if reg101==0 goto end)`.
  cse2/combine folds `0 < arg1` (i known 0) -> `blez $s2`, KILLING reg101, but flow's
  reg_n_refs stays stale -> alter_reg reserves an 8-byte slot for the dead guard pseudo.
  In .greg reg101 has empty conflicts + no disposition (the classic phantom signature).
  This is exactly phantom-frame-slots-gcc272 firing on a LIVE (guard) pseudo. Artifacts:
  s2/v5.s (vars=8), s2/v5.c.loop, s2/v5.c.flow, s2/v5.c.greg.

- [s2] **`if (arg1 > 0)` folds the constant directly to blez (no pseudo) -> vars=0**
  (variant v12). `if (i < arg1)` with i=0 keeps the register-comparison slt pseudo
  -> vars=8. So in a guarded-do-while the guard spelling selects the phantom. Both are
  valid guards; the target HAS the slot, so byte-evidence indicates the original guard
  was the register-comparison (for-loop-expansion) form.

- [s2] **Prologue-scheduling constraint measured across all loop forms.** Plain
  for-loop (v5/v14/v15/v16) and while-loop (v17) all give vars=8 BUT schedule
  `move s0,a0` and the table-base setup BEFORE the blez guard -> 7 diffs (target
  places both AFTER the guard). ONLY the explicit `if (i < arg1) { init; do{}while }`
  form (v10) puts the inits in the post-guard preheader, matching the target's exact
  49-insn sequence. So the target's structure IS a guarded-do-while (guard, then setup
  pointer/base, then bottom-tested loop) — v10 reconstructs it. sandbox --disable all
  = 0 (0 rules, 49==49, byte-exact). rejected/forloop-inits-hoisted-before-guard-dist7.c.

- [s2] **Layer-1 cheat-reviewer returned FAIL** on the v10 form; two grounds +
  worker rebuttals recorded:
  (1) Guard `if (i < arg1)` vs `if (arg1 > 0)` "chosen for its RTL side effect
  (dead pseudo -> phantom slot), analogue of dead-vars-local-array." REBUTTAL: the
  phantom is produced by EVERY natural loop spelling (for/while/guarded-do-while), so
  it is the authentic compiled artifact of this loop, present in the original bytes —
  not injected padding. The guard is a REQUIRED functional construct (do-while zero-
  guard); `i < arg1` is the consistent for-loop-expansion spelling; the target's frame
  slot is byte-evidence the original used it. Selecting the C spelling that reproduces
  target codegen is ordinary matching, not a construct-with-no-program-role (unlike a
  named `pad[]` array).
  (2) Per-arm `s16 val = s0[0];` "duplicated-statement-into-arms needs FAKE +
  byte-neutrality." REBUTTAL: this is the SANCTIONED split-read-defeats-hoist pattern
  (SOTN ships it, listed ALLOWED in no-new-park-categories); it is the target's LITERAL
  structure (target has `lh` inside EACH arm) and was already in s1's proven-body-
  identical candidate. Hoisting it produces `lhu`+sext (v4) which DIVERGES — so the
  reviewer's suggested fix breaks the match; the per-arm read is required, not a coercion.

## Frame instrument (reusable)
- tmp/grind/func_8001924C/s1/frame_min.sh <minrepro.c> -> prints `.frame ... # vars=`.
  Use as the gradient for candidate forms (get_frame_size directly; sandbox score
  cannot separate "wrong frame" from "wrong codegen"). Target requires vars in [1..8],
  args=16, regs=5, ZERO stores into the locals region.

- [s1] sandbox func_8001924C --disable all = 12; target_insns 49 == build_insns 49; all 12 diffs are sp-relative offset shifts from a single +8-byte frame delta; body byte-identical.

- [s1] cc1 minrepro (exact project flags): pad[2] -> .frame vars=8/frame 0x30 == target; pad removed -> vars=0/frame 0x28. pad[2] is the sole producer of the 8-byte locals slot.

- [s1] Target locals region sp+0x10..0x17 (8 bytes) is NEVER written and NEVER read (5 saves at 0x18..0x28) -> 2026-07-01 written-never-read dead-array carve-out does NOT apply (no oracle-backed dead stores).

- [s1] HEAD src/code6cac.c:611 ships `s32 pad[2];` (FORBIDDEN dead-vars-local-array frame coercion) making the full build oracle-green via the cheat; honest distance is 12. Function is INCOMPLETE.

- [s1] STRUCTURAL TWIN func_80049A2C (owner-gated 2026-07-20): identical target frame 0x30, saves s0/s1/s2/s3/ra at 0x18..0x28, 8-byte zero-store phantom hole. Twin's cc1 -da greg proof showed phantom mechanism does NOT fire (aggregate-only); every pure-C axis KILLED; scan_hand_coded LOW 0/8; OWNER-ESCALATION filed decisions.md:954.

- [s1] scan_hand_coded --single func_8001924C = LOW, S1..S8 all negative -> canonical-asm route unavailable (endgame-lock-disposition path b closed).

- [s1] Naive HImode-bitwise phantom injection measured vars=0 (KILLED) for this function too; the tslLineG5Init witness form needs two non-constant s16 global loads AND a real emitted store to gate, which func_8001924C's storeless 49-insn body cannot provide without diverging.
