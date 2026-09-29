# Evidence bank — func_80030D7C

- [s1] [fable-blitz 2026-07-07] Queue: distance 707, verdict ASM-STRUCTURAL, status parked; park_reason itself states scan_hand_coded scores LOW and the asm reads as standard GCC-compiled C. The 1 rule is asmfix.txt:89 `replace_with_asmfile "asm/funcs/func_80030D7C.s"`; src/code6cac_b.c:2682 is an empty stub - the 707 is the whole missing body, not a residual.

- [s1] [fable-blitz 2026-07-07] Function role: per-frame update of the 12-slot, 0x64-stride weapon-entity table at D_80106A78 (thrown/disarmed weapon flight). Same table walked by COMPLETED siblings in the same file: func_80030208 (src:2241, called in this function's epilogue at asm line 748), func_80030524 (src:2366), func_80030D04 (src:2662), coli_hit_body_weapon (src:2381) - the established source idiom is struct-free byte-offset addressing off (u8*)&D_80106A78 / &D_80106A7A, so no struct-definition work is needed.

- [s1] [fable-blitz 2026-07-07] Register frame (asm lines 2-24): 0x50 frame, ALL of s0-s7+fp+ra saved. Live loop values: s0=elem+2 (record ptr), s5=elem+0x2C (pos triple ptr), s6=elem base (u16 counter), fp=loop idx 0..11, s3=0x1F8002B8 scratchpad struct, s7=&Judge (4096-entry sin table, andi 0xFFF/sll 1/lh idiom, +0x400=cos), s4=collision result across calls, s1/s2 region temps, and a 10th value - the constant 0x1F8002C8 - SPILLED to sp+0x18 (stored line 24, reloaded lines 292/318). The spill is natural C (a local pointer initialized pre-loop that loses global-alloc priority); do not try to force it into a register.

- [s1] [fable-blitz 2026-07-07] Region map: R1 gate (asm 26-50): state==-1 -> sb 0xFF @+8 via shared tail j .L8003183C; +6 flag -> skip; backup pos 0x2A/2E/32->0x36/3A/3E; if +0x4E lifetime counter (*s6)++. R2 homing (52-233): state==0x10 && +0x4E && +3==0 && counter>=14 -> single_game_getEnemyCharId, a3=((0x4E-counter)*96)/64 clamped [0,0x42], three 2D sin/cos rotations of the velocity triple 0x42/46/4A routed through scratchpad fields 0x10-0x28, then angular-velocity decay *63/64 on 0x5A/5C/5E; else-branch decay *15/16 (234-261).

- [s1] [fable-blitz 2026-07-07] Region map cont: R3 integrate (262-301): angles 0x52/54/56 += angvels; vel.y += 13 (gravity); scratch 0/4/8 = pos+vel; pos.y -= 8; call func_8005344C(s5, 0x1F8002B8, [sp+0x18], s3+0x30, s3+0x38) - already declared src:79 as 5-arg (5th via 0x10(sp), matches sw at asm line 301). R4 bounce (302-435): hit && func_80054434()!=7: state==0xF -> func_80032854(+4,0xE,...) + kill; else reflect v off s16 normal triple (scratch 0x30/32/34): dot=(v.n)>>11, v-=n*dot>>12 per component; pos=contact (scratch 0x10/14/18); per-state restitution from D_8008E19E[state*7] (s16 stride-0xE table, base D_8008E194+0xA); if +0x4E scale v by coef and s1=vx*vx+vz*vz.

- [s1] [fable-blitz 2026-07-07] Region map cont: R5 SFX funnel (436-560): rng-driven 0x5C tweak; three func_80032854 call sites (a1=0x2F state==0xE path / 0x2C state<0x12 / 0x29 state-0x12<12u) CROSS-JUMPED by jump2 into one call with three entry labels .L80031590/.L8003159C/.L800315A0 (asm 552-560); plus rest-state relaunch (.L800314D8, 503-551): +3==1 -> 0x54+=0x780+rng&0xFF, vel from Judge sin/cos, vy=-150, +3=2. R6 post (561-621): +2=0; +5:1->2; +3:2->3; no-hit path .L800315D8 pos=scratch 0/4/8; rest detect (vy+15<31u && vx+3<7u && vz+3<7u) -> if s4: D_800A38DC==3 ? state=-1 : zero vel/+4E=0/+3=1; else +0x4E=1. R7 random re-hop (622-678, state==0xE,+3==1,!(rng&0x133) - mirror of R5 relaunch with its own NON-merged 0x2F call at 677). R8 easing (679-732): +5==2 -> func_80030D50 on 0x54/0x56 + 0x5C*3/4; else if +2==0 && D_8008E194[state*7]==2 -> ease 0x52/54/56. R9 kill check (733-747): pos.y>=0x3A99 -> state=-1; loop advance s0/s5/s6 += 0x64; tail call func_80030208().

- [s1] [fable-blitz 2026-07-07] m2c produced a COMPLETE valid-syntax reference (tmp/blitz/m2c_80030D7C.c, 375 lines, zero errors) - control flow fully recovered including the block_61/62/78/79/80 funnel nesting that encodes the cross-jump structure. This is the transcription base.

- [s1] [fable-blitz 2026-07-07] Scratchpad dataflow is honest store+reload: intermediates written to 0x10-0x28($s3) are reloaded mid-expression (e.g. sw line 114 -> lw line 127) because interleaved stores through s0 (unknown alias) flush GCC 2.7.2's CSE memory window for the constant-address s3 accesses. Route rotation intermediates through the scratchpad struct in C exactly as the original did; do NOT use locals for them.

- [s1] [fable-blitz 2026-07-07] All divisions are signed power-of-2 with bias diamonds (bgez/addiu 0x3F or 0xF or 0xFFF or 0x7FF/sra): decay is written x*63/64 (sll 6 - x) and x*15/16, restitution scaling /4096, dot /2048. These emit naturally from plain C division on possibly-negative ints; several bias diamonds have the PREVIOUS component's sw filled into their delay slots (e.g. 409-410, 418-419) - consecutive statement order per component produces this via cc1 first-pass sched; keep per-component statement blocks contiguous.

- [s1] [fable-blitz 2026-07-07] Clamp region (80-95): `if(a3<0) a3=0; else if(a3>=0x43) a3=0x42;` followed by idx=s4+0x400 - the addiu $a1,$s4,0x400 appears in a delay slot at 91 AND at .L80030ED4 (94); that is reorg.c duplicate-fill of the post-clamp statement, natural from plain sequential C.

- [s1] [fable-blitz 2026-07-07] Callee typing already present in src/code6cac_b.c: func_8005344C (line 79), func_80032854 (multiple local extern redecls with per-callsite signatures - lines 321/401/2464, an established file idiom), func_80030D50 COMPLETED at line 2675 (the angle-easing helper this function calls 5x), rng_Next, single_game_getEnemyCharId, func_80054434 all known.

- [s1] [fable-blitz 2026-07-07] No WIP checkpoint, no prior grind ledger, no duplicates-leads file. Nothing to import; this recon is the first banked work.

- [s1] [fable-blitz 2026-07-07] Walls, named: (1) R5 jump2 find_cross_jump call-funnel - recipe exists (.claude/rules/cross-jump-call-merge.md); the three call arg-setups must produce byte-identical suffixes (a3=0 + sltu chain). (2) global.c allocation order across 9 callee-saved values - ref-count priority in a 380-insn body; expect a rename plateau phase and budget -da greg dumps (register-alloc-pure-c) rather than pin experiments. (3) R7's SEPARATE non-merged 0x2F call at line 677 vs R5's merged funnel - the C must make R7's suffix NOT identical (it isn't: xor operands differ - lh 0x0(s0) vs the funnel's reload; transcribe faithfully and it stays distinct).

## s2 — manual lane (2026-09-29): first full-body transcription, 709 -> 21

- [s2] candidate.c = full pure-C body (no FAKE constructs). Sandbox --disable all: 21/709, 709 insns,
  frame 0x50 and prologue identical, every block structurally identical. Only two operand-only clusters
  remain (see below). Build: tmp/func_80030D7C/F3.c; m2c at tmp/func_80030D7C/m2c.c.
- [s2] Measured closers (each a separate probe, full TU sandbox):
  - v1 plain transcription 178.
  - `nrm = (s16 *)(scr + 0x30)` as a USER local set in the loop body + the contact pointer spelled
    `scr + 0x10` at both call sites (a compiler temp, not a local) -> 80 -> 55. Mechanism: loop.c
    scan_loop refuses to move a user var set after a conditional jump and used in another block (so
    s1 = s3+0x30 stays in-loop), while the two `scr + 0x10` temps are matching movables, hoisted to the
    preheader and folded to 0x1F8002C8 by cse2 with no REG_EQUIV -> stack slot sp+0x18 (target's spill).
    A `contact` local set before the loop scored 80 (wrong prologue position).
  - av tweak as `*(s16 *)(obj + 0x5E) += (rng_Next() & 1) ? spd / 64 : -spd / 64;` (preexpand_calls
    stops at COND_EXPR, so the old value is loaded into s2 before the call) — target's lh s2.
  - `(amt - ang)` inline (no reassignment of ang) — target's subu v1,a3,s4 fresh temp.
  - `*(s32 *)(obj + 0x50) = 1;` in both relaunch blocks (no flag local): CSE substitutes the byte reg
    (v1 in R5, s1 in R7) for the constant. A shared `flag` local put R5 in s1 (wrong).
  - `s16 state` (not s32): 55 -> 30 (gives the target's `move v1,t0` + andi 0xFFFF shapes).
  - Funnel written with direct `*(s16 *)(obj + 2)` re-reads in every test and arg (F3): 30 -> 21. This is
    what lets jump.c thread_jumps redirect the ny-block's `== 0xE` branch into the funnel's E arm
    (compare regs are non-user temps that die at the branch), and why the E arm reloads state.
    Keeping a `state` local in the funnel (v4/F2) blocks the thread (30).
- [s2] Remaining 21 = two register clusters:
  (1) R2 `ang` (ratan2 result): target s4, ours t0 (+ knock-on t2 for a mflo temp, one sched slot at
      target[68]). BB2_FINDREG_DEBUG=75 (tmp/func_80030D7C/dump.sh): ang does not cross a call, so
      find_reg pass 0 takes the lowest used-so-far non-conflicting reg; t0..t9 are all in the seed
      (call-used regs), conflicts are only v0-a3,s0,s3,sp -> t0. For s4 the pseudo must cross a call:
      s4 is the collision-result pseudo (`hit`, set by func_8005344C, live across func_80054434 /
      rng_Next / func_80032854). One pseudo for both values reproduces s4 by construction.
  (2) R4 restitution `rest`: target a3, ours a1. FINDREG for pseudo 80: conflicts v0,v1,a0,s0,sp only ->
      a1. a3 is `amt` (R2 clamp result). One pseudo holding amt and rest would conflict with R2's
      a1/a2 temps -> a3.
  Neither is a Ruling-11 claim yet: needs the (D) search first.
- [s2] Tbl8008E194.unkA gives `lh %lo(D_8008E194)+10`; target's reloc names D_8008E19E. Same bytes;
  the sandbox shows it inside the rest hunk — confirm at the oracle build.

## s2 (cont.) — Ruling 11 (D) record for `temp` and `work` (2026-09-29, rev. 3 after layer-2 FAIL #2)

History: FAIL #1 — the first record measured other bodies (superseded probes in
probes/superseded/). FAIL #2 — prong (A): `temp`/`work` were declared at function scope while all
their writes are in the for body; both declarations now sit at the top of the loop body (the
reviewer's A_scoped.c, 0/709). The landing also removes undefined_syms_auto.txt's
`D_8008E19E ... retire with func_80030D7C` alias row (aggregate-merge prong (c)).

Reuse spelling = probes/landed_body_L2-0.c (the exact body spliced into src/code6cac_b.c;
sandbox 0/709). One-variable-per-value spelling = probes/onevar_PV2-21.c (21/709), generated from
the landed body by probes/mkpv2.py with each value's local at its innermost scope (Ruling 11
(C)(1)): `ang`/`amt` at the top of the turn block, `hit` at the top of the loop body (where `temp`
was), `rest` at the top of the `if (hit != 0)` block. Only declarations and identifiers differ
(checked by diff). PV2 compiles to the same 709 instructions as the function-scope per-value
body that seeded campaign C (objdump comparison of the two permuter base.o files: identical).

### (D)(1) dumps
d_proof_dumps.txt, built by probes/dproof.sh (-> probes/dump.sh: cpp | tools/gcc-2.7.2/cc1 -O2
-G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -da,
BB2_ALLOC_DEBUG=1, BB2_FINDREG_DEBUG=<pseudo>) on tmp copies of onevar_PV2-21.c and
landed_body_L2-0.c (identical to the banked probes).
- per-value (PV2): pseudo 106 `ang`: .lreg "used 8 times across 68 insns" (crosses no call);
  FINDREG conflicts {2-7,16,19,29}; pass 0 may take 8 -> hardreg 8 (t0). Target: s4.
- per-value: pseudo 81 `hit`: crosses 8 calls; pass 0 has no candidate -> pass 1 -> 20 (s4).
- per-value: pseudo 342 `rest`: 14 insns, conflicts {2,3,4,16,29} -> hardreg 5 (a1). Target: a3.
- per-value: pseudo 107 `amt`: 65 insns, conflicts {2-6,16,29} -> 7 (a3).
- reuse (landed body): pseudo 82 `temp` (heading + collision result): 288 insns, crosses 8 calls,
  conflicts {2-8,16-19,29} -> 20 (s4) for both values.
- reuse: pseudo 81 `work` (turn amount + restitution): 79 insns, conflicts {2-6,16,29} -> 7 (a3)
  for both values.

### (D)(2) mechanism
global.c find_reg: lines 970-975 (allocno_calls_crossed == 0 -> used1 = fixed regs only; else
call_used_reg_set is excluded); lines 999-1001 + 1052-1065 (pass 0 only takes a register already
in regs_used_so_far, in ascending regno order, skipping conflicts). regs_used_so_far is seeded at
global.c:344-372 with every call-used GPR (BB2_ALLOC_DEBUG seed 0-15,24,25,...), so a pseudo that
crosses no call gets the lowest-numbered caller-saved register it does not conflict with.
- temp: the heading's reads are all in the turn block, before any later call of the iteration, so
  a variable holding only the heading crosses no call and pass 0 gives it t0 (its conflicts are
  the turn block's v0-a3 temps). One pseudo shared with the collision result (live across
  func_80054434 / rng_Next / func_80032854) crosses 8 calls -> call-saved only -> s4, the
  target's register for both values.
- work: the restitution factor's live range (its load to the third velocity multiply) touches only
  v0, v1, a0, so alone it gets a1. Shared with the turn amount, the pseudo also conflicts with the
  turn block's a1/a2 temps -> a3, the target's register for both values. In the bounce block a3
  (vx) is live wherever a1 (vx*nx) or a2 (nx) is (target 325-354), so a restitution load placed
  earlier conflicts with a3 as well (probe Rb, on an earlier chassis: t0, 57).

### (D)(3)/(4) measured one-variable-per-value spellings (sandbox --disable all, full TU)
- onevar_PV2 21 (exact per-value spelling of the landed body).
- Ablation on PV2: only `temp` shared (ablate_temp_only_PV2a) 4 = the work cluster; only `work`
  shared (ablate_work_only_PV2b) 17 = the temp cluster; each merge fixes exactly its own cluster.
- Reviewer Q31 proposals (probes/reviewer_q31/, copied from the layer-2 reviewer's
  tmp/rev30d7c/): R1_inner (per-value, every local at innermost scope, incl. `half`) 21;
  R2_inline_rest (no restitution local, table read inline) 71; R3_order (declaration order
  swapped) 21; R4_rest_s16 (`s16 rest`) 24. None reaches the target.
- Structural respellings of the function-scope per-value variant (onevar_PV_struct_*; same
  statements, declarations at function scope): S1 turn-block locals block-scoped 21; S5
  declaration order reversed 21; S6 operand order rest*v 21; S10 nested hit test 21; S4
  `s16 ang` 53; S8 sin/cos locals 46.
- Permuter campaign C (next entry) seeded from the function-scope per-value body, whose code is
  identical to PV2's (see above). Earlier campaigns A/B ran from a superseded chassis; their finds
  were likewise all new reuses of the heading's variable (permuter_B_harvest.txt) —
  corroboration only.
- nrm: `s16 *nrm = scr + 0x30` once-written, read by the reflection (nrm[0..2]) and passed to both
  calls. Using nrm also in the two later ny >= -0x7FF tests: 3 (reuse_nrm_all_N1-3.c); the tests
  stay on scr+0x32. Inline scr+0x30 everywhere (no local): loop.c hoists the compiler temp and cse2
  folds it to a spilled constant (v1 178).
- permuter campaign C (tmp/func_80030D7C/permC, permuter score 735, -j 2, --stack-diffs; mini TU
  byte-identical to the full TU by probes/cmpmini.sh): 15,261 iterations in 26 min, stopped at the
  fresh-seed window, best 75, no 0. Every find below 500 makes `ang` (the heading's variable) carry
  a further value (state, a flag byte, rest, a velocity, a LerpAngle result, -1, ...) or turns it
  into `long long` (225). No counting one-variable-per-value spelling reaches the target.
  Harvest: permuter_C_harvest.txt.

### LANDED 2026-09-29 (s2) — COMPLETED-C
- Match commit c0f4dae1f (src/code6cac_b.c body; undefined_syms_auto.txt D_8008E19E alias row
  retired); queue done 4991cafad. Layer-2: FAIL #1 (record measured other bodies), FAIL #2 (R11
  prong (A): temp/work moved to loop-body scope), PASS on round 3 (sandbox 0/709, PV2 21,
  ablations 4/17, dumps reproduced). check_completion_integrity OK.
