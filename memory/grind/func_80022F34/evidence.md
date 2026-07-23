# Evidence bank — func_80022F34

- WIP rejected_form: {'form': 'inline the call args (drop idx1/val1 intermediates)', 'score': 11, 'reason': 'not a cheat; no change to the callee-saved promotion'}

- WIP rejected_form: {'form': 'switch directly on D_800A38DC (drop val temp)', 'score': 11, 'reason': 'not a cheat; no change'}

- WIP rejected_form: {'form': 'do/while loop instead of goto', 'score': 26, 'reason': 'not a cheat; worse'}

- == imported from memory/wip notes.md ==
# func_80022F34 — WIP (callee-saved over-promotion / frame plateau)

## TL;DR (2026-06-14)
HEAD carries 11 regfix rules: 10 frame-offset substs (prologue idx 0-7 +
epilogue idx 59-63, shifting the frame down) + 1 `insert "nop" @ 36`. Unpinned
masked floor = **11** and NO pure-C form found this session beats it. Two
distinct issues:

1. **maspsx .L-label load-delay nop** (the `insert nop @ 36`): store-value
   consumer variant `lhu v0,0(s2)` -> `.L` switch-merge label -> `sh v0,8(a0)`.
   This is a LEGITIMATE pure-C retirement path via
   [[maspsx-label-nop-gate]] (add func to `maspsx_label_nop_funcs.txt`, retire
   drops the rule). NOT the blocker.

2. **Callee-saved over-promotion (the real blocker):** compiling pure C, GCC
   2.7.2 allocates SEVEN callee-saved regs (s0,s1,s2,s3,s4,s5,ra) and a -48
   frame; the TARGET uses only FOUR (s0,s1,s2,ra) and a -32 frame. GCC promotes
   3 extra long-lived values (the &D_801027BC base, val1, an index temp) to
   s3/s4/s5 where target keeps them caller-saved. The 10 frame-offset substs +
   the register renames are correcting this. No pure-C restructuring tried this
   session changes the promotion.

## Ruled out this session (all >= floor 11, none a cheat)
- inline the call args (drop named idx1/val1 intermediates) — floor 11
- switch directly on D_800A38DC (drop `val` temp) — floor 11
- do/while loop instead of goto — floor 26 (worse)

## Why blocked, not matched
COMPLETED-C needs 0 rules + 0 pins + SHA1 == oracle. Frame-padding to coerce
the size is FORBIDDEN ([[dead-vars-local-array]]) and anyway the divergence is
GCC over-promoting to callee-saved (frame too BIG, not too small) — no
"subtract a local" lever exists. The maspsx nop is retirable but the
callee-saved over-promotion is a register-allocation plateau.

## Next hypotheses
- Directed permuter from baseline targeting the s3/s4/s5 demotion (register
  allocation is exactly the permuter's domain; caveat: high-cascade-adjacent
  file code6cac.c).
- ALLOCDBG / -da RTL dump to identify WHICH values GCC promotes to s3/s4/s5 and
  whether a lifetime-shortening restructure can keep them caller-saved.
- If the promotion is immovable: canonical-asm authorization (the rules encode
  a register-allocation the compiler won't produce).


## s1 (recon, 2026-07-23) — THE PRIOR "s3/s4/s5 OVER-PROMOTION" DIAGNOSIS IS FALSE

- [s1] BASELINE re-measured at HEAD: sandbox --disable all = 11, build_insns 69,
  target_insns 70, rules_dropped 11, canonical verdict C. Floor confirmed 11.

- [s1] **THE WIP/LEDGER CLAIM (GCC allocates 7 callee-saved s0-s5,ra, -48 frame,
  over-promotes 3 values to s3/s4/s5) IS WRONG for HEAD's source.** The honest
  cheat-stripped build uses ONLY s0,s1,s2,ra with a -0x28 (40) frame. The BODY
  is BYTE-IDENTICAL to target. The entire 11-distance decomposes as:
  10 diffs = frame is 8 BYTES TOO BIG (build vars=8 vs target vars=0; affects the
  sp adjust x2 + all 8 save/restore offsets), and 1 diff = the maspsx .L-label
  load-delay nop (lhu v0,0(s2); .L80022FD0: nop; sh v0,8(a0) — store-value
  consumer, retirable via maspsx_label_nop_funcs.txt).

- [s1] **THE +8 IS A PHANTOM FRAME SLOT (reload stale-ref bug — see
  func_80037540 s3 mechanism).** cc1 -da lreg dump: "Register 100 ... block 10;
  ST_REGS or none; pointer." reg 100 = the &D_801027BC[idx1*5] ADDRESS temp
  (insn 102: reg100 = symbol_base + idx1*20; insn 104: val1 = *reg100). combine
  FOLDS these into per-access `lw a1,%lo(D_801027BC)(at)` addressing (matching
  target) but leaves reg 100 stranded via a stray `(use (reg:SI 100))` emitted at
  code_label 85 (the SWITCH MERGE label). Stale reg_n_refs -> ST_REGS ->
  alter_reg gives it a frame slot nothing references -> vars=8.

- [s1] **THIS IS THE INVERSE of the sibling wall (func_80049A2C / func_80037540):
  they must ADD a phantom +8 slot (hard, owner-gated); WE must REMOVE one
  (legitimate clean-C, not a coercion).**

- [s1] Instrument: standalone reproducer tmp/grind/func_80022F34/s1/probe_base.c
  + reuse of tmp/grind/func_80037540/s2/build.sh prints cc1's `.frame` comment
  (# vars=V) — the direct gradient on the phantom (the sandbox score cannot
  separate wrong-frame from wrong-codegen).

- [s1] KILLED (all vars=8, phantom persists): idx1/idx2 declared s32; val s16;
  (s32)-cast the *5 multiply; separate pointer var for the a0 reload; val1
  computed AFTER idx2 (late a1 order); if-else chain instead of switch;
  base pointer declared INSIDE the loop (vF) or per-use (vG). None touch the
  strand. Reordering/inlining consistent with the banked "inline args -> 11".

- [s1] **vD (function-scope loop-invariant `s32 *b = &D_801027BC;` shared by both
  accesses) KILLS the phantom: vars=0 — BUT promotes b to a 5th callee-saved reg
  (regs=5/0), which the target lacks.** Banked
  rejected/hoisted-base-ptr-kills-phantom-but-promotes-5th-reg.c. This proves the
  phantom is 100% dissolvable in pure C; the tension is phantom-vs-extra-reg.

- [s1] **vH (m2c-faithful, FULLY INLINED: both call args inline `(&D_801027BC)[..]`,
  a0-reload as double-indirection `*(u8**)a0` in arg3, NO named temps, goto loop)
  -> vars=0, regs=4/0, frame 0x20 — EXACTLY the target frame, phantom GONE, no
  extra reg.** Applied to src + full-TU sandbox --disable all = 11 with the
  PROLOGUE/EPILOGUE now BYTE-MATCHING target (confirmed by objdump). Saved as
  memory/grind/func_80022F34/candidate.c. m2c reconstruction (mipsel-gcc)
  corroborates this shape (inlines both D_801027BC accesses into the call, a0
  reload as temp_a0->unk0->unk4A double-indirection).

- [s1] **vH TRADES the phantom for a base-CSE.** With the frame now correct, the
  remaining 11 are ENTIRELY in the D_801027BC access region: GCC CSEs
  &D_801027BC into a shared base (la $a1; addu ...,$a1; lw ...,0($n)) while
  target re-materializes %hi/%lo(D_801027BC) PER ACCESS; plus the a0 self-reload
  lands in $v0 (lw v0,0(a0); lh a0,74(v0)) vs target's in-place $a0. Standalone
  vH ALSO CSEs (la $5) — so this is not full-TU register pressure, it's the
  inlined form's CSE visibility. CORE TENSION: per-access %lo fold <=> phantom
  strand (HEAD/probe_base); CSE'd base <=> no phantom but wrong body (vH). Target
  has BOTH per-access %lo AND no phantom — a form not yet found.

- [s1] do/while loop (m2c's loop form) is STRICTLY WORSE: vI -> regs=6/0
  (promotes more). Keep the goto loop. (Matches banked "do/while -> 26".)

- [s1] src REVERTED to HEAD this session: HEAD's 10 frame-offset regfix substs
  were correcting the OLD phantom frame; with vH the frame is already correct, so
  leaving vH + the stale substs would CORRUPT the oracle build. The DONE path for
  vH must retire those 10 substs + allowlist the nop (queue done/retire — outside
  a grind session's surface). candidate.c preserves vH for the next session.

- [s1] Artifacts: tmp/grind/func_80022F34/s1/{probe_base.c, vD_baseptr.c, vH_m2c.c,
  build_now.txt, target_now.txt, dumps/base.i.lreg, dumps/base.i.combine,
  dumps/base.i.greg}. rejected/named-temps-per-access-lo-but-phantom-frame.c,
  rejected/hoisted-base-ptr-kills-phantom-but-promotes-5th-reg.c.

- [s1] Baseline sandbox --disable all = 11 (build 69 / target 70 insns), verdict C, 11 rules_dropped.

- [s1] Honest cheat-stripped build uses s0,s1,s2,ra + -0x28 frame; body byte-identical to target. Floor = frame 8-bytes-too-big (10 diffs) + 1 maspsx .L-label load-delay nop.

- [s1] The +8 is a reload stale-ref phantom slot; phantom pseudo = reg 100 (the &D_801027BC[idx1*5] address), stranded by a stray (use) combine emits at code_label 85 (switch merge).

- [s1] This is the INVERSE of siblings func_80049A2C/func_80037540: they must ADD a phantom slot (owner-gated); this function must REMOVE one (legitimate clean-C).

- [s1] KILLED (all vars=8): idx s32; val s16; cast multiply; separate reload ptr; late val1; if-else instead of switch; base ptr inside loop / per-use.

- [s1] vD (function-scope shared base ptr) kills the phantom (vars=0) but promotes it to a 5th callee-saved reg the target lacks.

- [s1] vH (m2c-faithful fully-inlined, goto loop) = vars=0 / regs=4/0 / frame 0x20 = exact target frame; saved as candidate.c. m2c reconstruction corroborates the inlined shape.

- [s1] vH residual 11 = base-CSE (la $a1 shared base vs target's per-access %hi/%lo) + a0 self-reload in $v0 vs target $a0. Standalone vH also CSEs, so it's the inlined form's CSE visibility, not full-TU pressure.

- [s1] CORE TENSION: per-access %lo fold <=> phantom strand; CSE'd base <=> no phantom but wrong body. Target has BOTH per-access %lo AND no phantom.

- [s1] do/while loop is strictly worse (regs=6/0); keep the goto loop.

- [s1] src reverted to HEAD: HEAD's 10 frame-offset regfix substs correct the OLD phantom frame; with vH the frame is already correct so those substs would corrupt the oracle. DONE path = vH + retire the 10 substs + maspsx_label_nop_funcs.txt for the nop.

- [s1] The permuter is a VALID lever here (unlike the siblings) because vH's residual is NOT sp-offset-only; the frame is correct so the stack-normalized scorer sees the real reg/CSE diffs.

## s2 (structural, 2026-07-23) — per-access<->phantom coupling MAPPED across ~24 forms; floor firmly 11 on the structural axis

- [s2] Baseline reconfirmed: HEAD (base named-temp form) sandbox --disable all = 11, build_insns 69, target 70, 11 rules_dropped.

- [s2] **base's BODY is BYTE-PERFECT vs target** (2x per-access `lw D_801027BC(idx)` + in-place a0 reload `lw $4,0($4)` in $a0). Its ENTIRE 11 = the +8 phantom frame slot (10 diffs: sp-adjust x2 + 4 saves + 4 restores at wrong offsets) + 1 maspsx nop. Removing the phantom (keeping the body) would drop the floor to 1 (the nop, retirable via maspsx_label_nop_funcs.txt on the DONE path).

- [s2] **The phantom (reg100 strand) and per-access are COUPLED through the val1 sub-block + lifetime.** Measured .frame(vars=)/per-access(count of `lw D_801027BC`) across ~24 structural forms:
  * val1 loaded AFTER switch, BEFORE reload, in a sub-block (base/vFLAT/vFLAT2/vARR/vQ/vPTR): 2x per-access + vars=8 (STRAND). Right instruction ORDER.
  * val1 loaded AFTER reload/idx2 (vW/vMIRROR/vK1/vU/vY/vP/vH/vSPLIT): CSE'd `la $5` base (0-1x per-access) + vars=0. Frame byte-matches target.
  * val1 loaded BEFORE the switch (vPRESW): 2x per-access + vars=0 — the ONLY form with BOTH — but hoisting reorders the whole chain and pushes a0 off $a0 => real sandbox = 28. Rejected.

- [s2] **The reg100 strand mechanism (combine note-relocation wart).** cse2 has reg95=symbol_ref(D_801027BC), reg99=idx1*20, reg102: reg100=reg99+reg95, reg104: val1=*reg100. combine folds reg104 into `lw val1,D_801027BC(reg99)` (per-access) and DELETES reg100's def, but strands a bare `(insn 163 (use (reg:SI 100)) REG_DEAD)` placed UPSTREAM of reg100's own def — right after switch-merge code_label 85, before the val1 sub-block's NOTE_INSN_BLOCK_BEG (note 87). reg100 is thus in greg's "6 regs to allocate: 100 80 75 72 74 73" list, gets NO hard reg (absent from Register dispositions), => alter_reg reserves an unreferenced slot => vars=8. Only val1 (long lifetime, loaded early in a sub-block, USED in the outer-scope call) strands; val2 (loaded late in the same block, consumed in-block) folds cleanly.

- [s2] **Frame-correct CSE form == base's floor.** vMIRROR (target-order, both CSE, vars=0) and vSPLIT (val2 per-access + val1 CSE, vars=0) BOTH = sandbox 11, build_insns 64. The CSE'd base + a0-reload-in-$v0 + nop sum to 11 — no better than base's phantom. Neither regime is < 11.

- [s2] The per-access difference is a genuine fork cse2 divergence: standalone AND full-TU our-fork cse2 keeps &D_801027BC in one reg (`la`); cc1psx re-materializes %hi/%lo per load. base gets per-access only via register-pressure SEPARATION (val1's `la` reg reused across the long gap), NOT a cse2 barrier — and that separation is exactly what strands reg100.

- [s2] KILLED forms not previously banked: val1-before-switch (vars0+per-access, reorder=28); val1 in if-body no-subblock (vSPLIT/vSPLIT2/vSPLIT3 => CSE); flat single-block (vFLAT/vFLAT2 => strand; vFLAT3 => CSE); array-typed D_801027BC (vARR => strand, decl type irrelevant); explicit displaced pointer p1=&(&D)[idx1*5] (vPTR => strand); named scaled-index k1 (vK1/vMIRROR => CSE); idx1-before-switch val1-after (vIDXSW => val1 CSE).

- [s2] Artifacts: tmp/grind/func_80022F34/s2/{probe.sh, show.sh, _prelude.h, base.c, vH.c, vP.c vU.c vW.c vY.c vK1.c vMIRROR.c vSPLIT*.c vFLAT*.c vPRESW.c vARR.c vPTR.c + their .s}. rejected/val1-before-switch-per-access-vars0-but-reorder-28.c, rejected/frame-correct-cse-base-still-11.c.

- [s2] Baseline sandbox --disable all = 11 (build 69 / target 70), 11 rules_dropped, verdict C.

- [s2] base's body is BYTE-PERFECT vs target (2x per-access `lw D_801027BC(idx)` + in-place a0 reload `lw $4,0($4)` in $a0); the whole 11 = +8 phantom frame slot (10 diffs) + 1 maspsx nop.

- [s2] The phantom = reg100 (val1's folded address) stranded by combine placing a bare `(use reg:SI 100)` upstream of its def at switch-merge code_label 85 (before the val1 sub-block's NOTE_INSN_BLOCK_BEG); reg100 gets no hard reg -> unreferenced frame slot -> vars=8.

- [s2] per-access and phantom are COUPLED via val1's sub-block lifetime: val1 loaded early-in-subblock => per-access + strand (base); val1 loaded late => cse2 shares `la` base => vars=0 but not per-access (vH/vMIRROR); val1 before switch => vars=0 + per-access but gross reorder => sandbox 28.

- [s2] All non-reordered structural regimes floor at exactly 11: base (phantom body) 11, vMIRROR/vSPLIT (frame-correct CSE) 11.

- [s2] Only val1 strands (long lifetime, used in outer-scope call); val2 (loaded late, consumed in-block) folds clean => the strand is a lifetime/scope-crossing artifact, not inherent to the D_801027BC access.

- [s2] Type of D_801027BC (scalar+& vs array[]) is irrelevant to the strand (vARR strands identically); explicit displaced pointer (vPTR) strands too.

- [s2] The per-access gap is a genuine fork cse2 divergence: standalone AND full-TU our-fork cse2 keeps &D_801027BC in one reg (`la`); cc1psx re-materializes %hi/%lo per load. base gets per-access only via register-pressure separation, which is exactly what strands reg100.
