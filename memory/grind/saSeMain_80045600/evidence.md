# Evidence bank — saSeMain_80045600

- [s1] BASELINE: canonical verdict C (pure-C distance 4, total 37). `sandbox --disable all` = 4,
  target_insns 37 == build_insns 37. The ENTIRE residual is the STACK FRAME: target
  `addiu $sp,-0x20` / `sw $ra,0x18($sp)` vs build `-0x18` / `0x10($sp)` (4 insns: prologue pair +
  epilogue pair). All 33 body insns byte-exact including all register choices.

- [s1] FRAME EQUATION (mips.c compute_frame_size, ALIGN8): target 0x20 = ALIGN8(vars) + ALIGN8(args=16)
  + ALIGN8(gp_regs=4:ra) → ALIGN8(vars)=8 → **get_frame_size ∈ [1..8]**. args=24 (5-arg call)
  alternative also yields 0x20/ra@0x18 but REQUIRES a 5th-arg `sw 16($sp)` the target lacks
  (only call is `jal InitFadePanel`, no stack stores) — ruled out. Target writes NOTHING in the vars
  region [0x10,0x18) → written-aggregate route emits stores target lacks; unwritten tail / volatile pad
  are forbidden families. The ONLY clean route is the reload stale-ref PHANTOM slot.

- [s1] CHEAT REMOVAL (free cleanup): prior src form carried `volatile s32 sp_pad;` (unused volatile
  local) + `register s32 old_a0 asm("v0")` pin. Both removed; floor UNCHANGED at 4 → both were inert
  paperwork; body registers (incl. v0/v1 in the found block) match naturally from the plain C.

- [s1] PHANTOM ENTRY CONDITION PRESENT: target has a sign-extending narrow load — `lh $v0,0x0($a3)`
  (the record-id compare) — unlike func_80037540 (all-lw, phantom unreachable there).

- [s1] POSITIVE CONTROL FIRES: grafting the faithful tslLineG5Init trigger (two s16 locals, HImode
  bitwise `(v1 & ~mask) & 1`, two low-bit consumers) into the found block → **vars=8, frame 0x20**
  (tmp/grind/saSeMain_80045600/s1/v1_poscontrol_witness_graft.func.s). The phantom IS reachable in this
  TU/function shape — but the graft costs ~+11 emitted insns (nor/and/andi + second consumer chain).

- [s1] KILLED probes (all vars=0, probe harness tmp/grind/saSeMain_80045600/s1/probe.py — compiles a
  spliced copy of text1a_c.c with exact Makefile cpp+cc1 flags, greps the function's `.frame vars=`):
  * H1 single `s16 cur` local (lh into HImode pseudo, one SImode compare use): extension folds into the
    load via the well-behaved mem-fold path, refs decremented, NO orphan. Byte-neutral (kept in candidate).
  * v2 HImode sub-compare `s16 delta = cur - a0; if (delta==0)`: vars=0 AND +2 insns (subu+beqz).
  * v3 two SImode compare uses of cur (loop + found join): CSE unifies the extension pre-flow; vars=0.
  * v4 `s16 a0` param: promoted subreg, NO pseudo exists to orphan (matches func_80037540 s3 finding).
  * v11/v12/v13 XOR-compare family `(s16)(cur ^ want) == 0` etc.: combine does NOT fold
    (eq (xor a b) 0)→(eq a b); the xor is EMITTED (+2 insns), vars=0. The hoped-for total-fold path
    does not exist via xor.

- [s1] NEW WITNESSES — zero-cost phantom exists in-tree in THIS shape-family (both COMPLETED-C, both
  frame-slack 8, from the func_80037540 s2 census tmp/grind/func_80037540/s2/bb2_frame_slack2.txt):
  * **func_800493E4** (src/text1b.c:790): `u8 temp_v1 = D_80099CC8[idx];` — lbu into QImode local with
    TWO uses of DIFFERENT kinds: a compare (`!= 0xFF`) AND an array INDEX (`D_800EF980[temp_v1] = 1`).
    Zero extra insns. NB: CONTRADICTS func_80037540-s3's "lhu/lbu (zero-extend) → no phantom" — that
    was measured in a different usage shape; the zero-extend redundant-AND fold evidently also orphans
    when the consumer shape is right (dual-use compare + index is the working hypothesis).
  * **func_80030900** (src/code6cac_b.c:2511): `*(s16 *)(a0+0x332)` passed DIRECTLY as SImode call arg
    + `(s16)(*(u16 *)(a0+0x330) - 1)` truncating RMW store. Zero extra insns.
  → Zero-cost riders EXIST; the firing condition is finer-grained than "HImode bitwise" — it depends on
  the CONSUMER shape of the narrow-loaded value. saSeMain's single consumer (SImode equality compare
  against an s32 param) takes the clean mem-fold path and does not orphan.

- [s1] INSTRUMENT CAVEAT (inherited, re-flagged): decomp-permuter's default scorer NORMALIZES
  sp-relative offsets (stack_differences=False) — this function's entire gap is invisible to it
  (func_80037540 s4 measured a 15-off form scoring blind=0). Any permuter campaign here MUST re-score
  with the engine sandbox or --stack-diffs.

- [s1] Artifacts: tmp/grind/saSeMain_80045600/s1/{probe.py, run_probes.sh, diff.sh, build.dis,
  v0_baseline.c/.s/.func.s, v1_poscontrol_witness_graft.c/.s/.func.s, v2/v3/v4/v11/v12/v13 .c/.s/.func.s,
  pristine.s, t_pristine.sh, dbg.py}. NB probe.py compiles with cc1 exit 33 (pre-existing benign
  conflicting-decl diagnostics in text1a_c.c — the Makefile pipeline ignores cc1's exit status; the .s
  is complete). First probe run wrote stray v*.s at repo root (relative -o with cwd=ROOT) — cleaned up,
  probe.py fixed to resolve paths.

- [s1] Baseline: canonical C distance 4; sandbox --disable all = 4 with 37/37 insns; ALL 33 body insns byte-exact — the residual is exactly the 4 frame insns (target frame 0x20/ra@0x18 vs build 0x18/ra@0x10), i.e. get_frame_size must be in [1..8] vs our 0

- [s1] Removed both src cheats (unused volatile s32 sp_pad local; register s32 old_a0 asm("v0") pin) — floor unchanged at 4, so they were inert; src form is now cheat-free

- [s1] Target has the phantom entry condition (sign-extending lh of the record id) unlike func_80037540; positive control graft measures vars=8 in this exact TU/shape

- [s1] NEW zero-cost witnesses in-tree (both COMPLETED-C, frame-slack 8): func_800493E4 (u8 local from lbu, dual use compare+array-index) and func_80030900 (lh passed directly as call arg; u16 truncating RMW) — zero-cost phantom riders exist and the firing condition is consumer-shape-dependent, finer than 'HImode bitwise'; also contradicts the inherited 'lbu/lhu never fire' claim (shape-specific)

- [s1] Built reusable probe harness tmp/grind/saSeMain_80045600/s1/probe.py: splices a candidate body into text1a_c.c, compiles with exact Makefile flags, reports the function's .frame vars= and emitted body — direct gradient the sandbox score cannot give (frame vs codegen)

- [s1] Permuter caveat re-flagged: default scorer normalizes sp offsets, so this function's entire gap is invisible to it — campaigns must score via engine sandbox or --stack-diffs
