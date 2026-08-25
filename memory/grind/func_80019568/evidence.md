# Evidence bank — func_80019568

## [s1] 2026-08-25 — recon (fresh derivation; rule-era chassis discarded)

Function map (from asm/funcs/func_80019568.s, 141 insns, frame 0x40, one callee-save s0=arg0):
- Frame struct at sp+0x10..0x37: `s16 output[4]` @0x10, `s32 voice_mask` @0x18, 3 unknown
  words @0x1C-0x27, `s32 packets[4]` @0x28 (loaded from D_800FF580/584/5A4/5A8).
- Loop 1 (2 iters): per 8-byte packet record, byte tests + a switch through
  **jtbl_80010068** (8 entries); computes `bits`; `voice_mask = ((u32)voice_mask>>16)|(bits<<16)`.
  Register file: a2=output walker (offsets 0/4), a3=packet walker (offsets 0..3),
  t0=i (slti t0,2), t1=voice_mask, t2=const 4, t3=jtbl base.
- `func_8001B138(&sp.voice_mask)` with the mask store in the jal delay slot.
- Conditional block (D_800A3834==1 && arg0==0): switch on lh D_800A38DC through
  **jtbl_80010088** (7 entries); two `|= 0x08000800` sites gated on D_8010278E / D_8010278C.
- `func_8003A728(&sp.output)`; copy loop (2 iters) sp.output[0..1]->D_80102788/8A,
  sp.output[2..3]->D_8010278C/8E via two dest walkers (a0,a1) + one src walker + i in t0.
- Tail: old=D_80102790; D_80102790=mask; D_80102794=mask&~old; D_8010279C=~mask;
  D_80102798=~mask&old. Target materializes &D_80102790 fully (lui+addiu, then lw 0(v0)/
  sw 0(v0)) and stores D_80102790 EARLY (before the nor/and chain); store order 9C, 94, 98.

Measurements (sandbox func_80019568 --disable all; target_insns 141):
- Committed rule-era body (pins + hardcoded asm + 5 regfix): honest floor **42**
  (47 cheat insns stripped; stripped object is structurally broken — extra callee-save s1,
  split output[2] pointer, packet lbu through sp+43 base). Rule-era chassis carries no
  evidence about the original structure; discarded per the 2026-08-19 chassis warning.
- v1 clean C (explicit walking pointers out++/pk+=8 + counter i, no cheats): **49**.
  loop.c strength-reduced every (biv+const) deref into separate walking givs
  (out[2]->own walker sp+20; pk[1..3]->combined walker anchored sp+43 offsets -2..0),
  hoisted the `li 1`, and cse FOLDED the output[0] store->reload (no lhu).
- v2 clean C (indexed accesses `sp.output[i]`, `sp.output[i+2]`, `packets[i*8+k]`,
  do-while i<2): **34**, build 141 == target 141. Combined givs now EXACTLY match target
  addressing (a2 offsets 0/4; a3 offsets 0..3) AND the lhu store->reload survives
  (indexed cse lookup does not fold it — v1's pointer form did fold it). Residual: loop.c
  ELIMINATED biv i ("Final biv value for 75, calculated. biv 75 can be eliminated",
  dumps/code6cac.loop:885-886) — exit test rewritten to `slt a2,limit` with hoisted
  limit sp+20 (an out-class giv, mult 2 add reg191); knock-on renames everywhere
  (mask t0 vs t1, loop-2 counter a2 vs t0).
- s16 i probe: **74** (build 159) — KILLED; sign-extensions everywhere. MIPS does not
  neutralize this via PROMOTE_MODE in a helpful way.
- v2 + DIAGNOSTIC exit test `while (i + 1 < 3)`: **28**, build 143. maybe_eliminate_biv_1's
  REG case returns 0 on the biv inside the PLUS -> elimination blocked -> i survives in t0
  and the ENTIRE register file snaps to target (t0/t1/t2/t3, loop-2 counter t0). combine
  does NOT fold (lt (plus i 1) 3) -> (lt i 2) (simplify_comparison only shifts constants
  across PLUS for equality compares, not signed <), so the diagnostic leaves
  `addiu v0,t0,1; slti v0,v0,3` (+2 insns). THIS SPELLING IS A PROBE, NOT A CANDIDATE:
  it exists only to defeat the eliminator and does not byte-match anyway.

Residual inventory at 28 (from tmp/grind/func_80019568/s1 normalized diff; LO-vs-0 and
split-immediate lines are normalizer artifacts, not real):
1. Exit-test spelling (+2 insns, diagnostic construct) — need an honest form that leaves
   `slti v0,t0,2` while blocking biv elimination (frontier F1).
2. `li 1` for `sp.output[i+2] = 1` hoisted out of the loop (loop.c movables:
   "Insn 87: regno 109 (life 1), move-insn savings 1 moved" — dumps/code6cac.loop:861)
   vs target keeps `addiu v0,zero,1` inside, filling the lhu load-delay slot; the store
   `sh ,4(a2)` sits early (between lhu and addiu -1) in target, late in build (F2).
3. bits/scratch v0-v1 rename cluster in loop 1 (lhu into v1 vs v0; nor v1 vs v0;
   srl/sll operand roles swapped). Likely coupled to F2's slot pressure.
4. Tail block: build accesses D_80102790 via %lo forms and stores it late; target
   materializes the full address (lui+addiu; lw/sw through 0(v0)) and stores early.
   Rule-era body used a pointer local (pointer-alias FAKE family) for this; an honest
   producer is unproven (F3). Note D_80102788..D_8010279C is a plausible single aggregate
   (cf. .claude/rules/split-scalars-hide-aggregate.md) but no independent evidence yet;
   func_800194F4/func_80019534 write the same cluster as scalars.
5. jtbl_80010088 addressing: build emits `lw v0,32(at)` off the FIRST jtbl's %hi/%lo
   (tables adjacent in TU rodata) where target references %lo(jtbl_80010088) directly.
   May be masked by the scorer or may be a real point — not yet isolated.

GCC-internals facts established (read from tools/gcc-2.7.2/loop.c):
- Biv elimination gate: strength_reduce:4035 `all_reduced == 1 && bl->eliminable &&
  maybe_eliminate_biv(...)`. A biv with NO derived givs survives ("Cannot eliminate biv
  75: biv used in insn 400" — this is exactly why the copy-loop counter survives, its
  arrays walk as independent bivs).
- Compare rewrite eligibility (loop.c:6127+): giv needs CONSTANT positive mult_val,
  `!ignore && !maybe_dead && always_computable && mode == biv mode`.
- Worthwhile test loop.c:3824: `v->lifetime * threshold * benefit < insn_count` -> giv
  not reduced -> all_reduced=0 -> biv survives. (Alternate honest blocking axis: make a
  giv fail this; untested.)
- final_biv_value (unroll.c:3020) needs loop_n_iterations != 0, no early exits,
  invariant init; falls back to reg_dead_after_loop.

Artifacts: tmp/grind/func_80019568/s1/{objdump.txt,norm_diff.py,dump.sh},
tmp/grind/func_80019568/dumps/code6cac.loop (v2-state dump; NB dump.ps1 reports
pre-existing parse errors at src/code6cac.c:661/977 for later functions but our function's
dumps are complete).

- [s1] Honest floors this session: committed chassis 42; v1 clean pointers 49; v2 clean indexed 34 (build 141==target); v2 + diagnostic elimination blocker 28 (build 143)

- [s1] Full residual inventory at 28 and GCC loop.c internals (elimination gate, compare-rewrite eligibility, worthwhile test, final_biv_value conditions) banked in memory/grind/func_80019568/evidence.md

- [s1] s16-counter form banked to memory/grind/func_80019568/rejected/s16-loop-counter.c

- [s1] src/code6cac.c reverted to HEAD (5 regfix rules are index-calibrated to the committed body; probe bodies live in candidate.c)

- [s1] Owner directive (RULES-TO-ZERO, 2026-08-24) acknowledged and executed: this session began the pure-C route; jtbl coupling confirmed (jtbl_80010068 8 entries, jtbl_80010088 7 entries, both emitted by the C switches)

## [s2] 2026-08-25 - structural (F1 SOLVED, F2 SOLVED; floor 34 -> 20)

Chassis re-measured at session start: s1's v2 body re-applied reproduced **34** exactly
(build 141 == target 141), so every s1 conclusion is chassis-valid.

### F1 SOLVED - biv `i` now survives loop.c elimination, honestly.

Mechanism, read out of tools/gcc-2.7.2/loop.c (not guessed):
- record_giv (loop.c:4386-4389): **DEST_ADDR givs are always_computable = 1
  unconditionally** ("INSN merely uses the value of the giv"), and their mode is
  GET_MODE(*location) = SImode. So ANY indexed memory access inside the loop yields a
  giv that passes every eligibility test at maybe_eliminate_biv_1's COMPARE case
  (loop.c:6160-6190). s1's v2 therefore could never keep the biv while its givs were
  reduced: giv 207 (mult 2, add reg191) rewrote `slti i,2` into `slt a2,limit` with
  limit = sp+20. The "every giv site is inside a conditional arm" angle in s1's F1(a)
  is a DEAD LEAD - conditional placement only clears always_computable for DEST_REG
  givs, never for DEST_ADDR ones.
- The gate that actually decides is one level up: strength_reduce:4034
  `if (all_reduced == 1 && bl->eliminable && maybe_eliminate_biv (...))`.
  all_reduced is cleared by the worthwhile test at loop.c:3824
  `v->lifetime * threshold * benefit < insn_count`, where benefit has already had
  `benefit -= add_cost * bl->biv_count` applied (loop.c:3804).
- combine_givs (loop.c:5517) does `g1->benefit += g2->benefit` for the leader, and the
  per-giv loop SKIPS combined members (`if (v->ignore || v->same) continue;`, loop.c:3782).
  So only giv-group LEADERS are ever tested. A DEST_REG "mult N add 0" leader with NO
  other giv combined into it has benefit 2; add_cost measures 2 and biv_count is 1, so its
  net benefit is exactly 0 -> "not worth while, 0 vs <insn_count>" -> ignore=1 ->
  all_reduced=0 -> biv elimination is skipped ENTIRELY.

The honest C that produces that: hoist a per-iteration record pointer to the top of the
loop body, so each scale group has exactly ONE "add 0" intermediate instead of several:

    do {
        u8  *p = &packets[i * 8];
        s16 *o = &sp.output[i];
        ...  p[0] p[1] p[2] p[3]   o[0] o[2]  ...
        i++;
    } while (i < 2);

Measured: 34 -> **20** (build 142). The .loop dump confirms the predicted mechanism
verbatim: "giv of insn 51 not worth while, 0 vs 51." / "giv of insn 45 not worth while,
0 vs 51." and NO "biv 75 was eliminated" line; the add-reg givs still reduce to reg
167/168 = the a2/a3 walkers. Emitted exit test: `slt $8,2` (= target `slti v0,t0,2`).
The whole register file snaps to target with no pins: i->t0, mask->t1, const4->t2,
jtbl->t3, a2 = sp+16 walker with offsets 0/4, a3 = sp+40 walker with offsets 0..3.
The ignored i*2/i*8 intermediates cost ZERO insns - they are dead once the address givs
reduce, and DCE removes them.

### F2 SOLVED - the `li 1` stays inside the loop.

Mechanism: scan_loop's movable-candidate test (loop.c:702-716) requires
`invariant_p(src) && (n_times_set[regno] == 1 || consec_sets_invariant_p(...))`.
The desirability test at loop.c:1631 is `threshold * savings * m->lifetime >= insn_count`
with threshold = 2*(1 + n_non_fixed_regs) (loop.c:532) - so once a movable EXISTS it is
always hoisted here; the only lever is to stop the movable from being created.
Two NON-consecutive sets of the same pseudo inside the loop do exactly that
(n_times_set==2, and consec_sets_invariant_p fails because the sets are not adjacent).

Spelled as reuse of the `voice` local:

    voice = p[1] >> 4;
    o[0] = voice;
    voice = 1;
    o[2] = voice;

Measured: build_insns 142 -> **141 == target**, score still 20, and the emitted if-arm is
now structurally identical to target (`li v1,1; sh v1,4(a2)` sitting between the lhu and
the addiu -1, i.e. filling the load-delay slot) - only a v0<->v1 seat swap remains.
FAMILY FLAG: this is the variable-reuse family (defeat-licm-hoist-var-reuse.md +
staged-value-reused-variable.md, FAKE-gated). It is NOT yet cleared for submission. An
honest non-reuse spelling that also gives the pseudo two non-consecutive sets has not been
found; that is now frontier F2b.

### Residual inventory at 20 (tmp/grind/func_80019568/s2/nd.py against asm/funcs)
R1 (~6 insns) if-arm v0<->v1 swap. Target: `srl v0,v0,4; sh v0,0(a2); lhu v1,0(a2);
   addiu v0,zero,1; sh v0,4(a2); addiu v1,v1,-1; sll v1,16; sra v1,16`.
   Ours: same shape with v0/v1 exchanged for `voice` and the lhu temp. Target ties
   `voice` to the lbu temp's seat; ours ties the lhu temp to it. Pure local-alloc seat
   assignment - candidate for tools/ra_solver (inverse_compose.py classify).
R2 (~13 insns) tail block. Target: `lui v0,%hi(D_80102790); addiu v0,v0,%lo; lw v1,24(sp);
   lw a0,0(v0); sw v1,0(v0); nor; and; nor; sw 9C; and; sw 94; sw 98`. Ours emits two
   independent %hi/%lo accesses for the load and the store and sinks the store.
R3 masked-by-scorer, not real: move-vs-addu spellings, %hi/%lo reloc addends, and
   `lw v0,32(at)` (jtbl_80010088 addressed off the first table's %hi in the same TU rodata).

### Facts banked
- Pointer-walking the arrays (out++/pk += 8) with a separate counter is DEAD as a match
  route: measured 46 (build 146). It gives THREE extra walkers - `sp+20` for o[2], and a
  SECOND packet walker anchored `sp+43` with offsets -2..0 - because with a pointer biv
  the base access MEM(biv) is not a giv while MEM(biv+k) is, so the +k form reduces into
  its own register. Only the indexed spelling combines all offsets into one walker.
  Banked: rejected/pointer-walkers-split-givs.c.
- Computing `voice2` BEFORE the `o[2]` store scores 17 (build 138) but is a FALSE minimum:
  with no intervening store cse folds the `sh -> lhu` reload away, deleting target's
  `lhu v1,0(a2); addiu v1,v1,-1; sll; sra` chain (3 insns short of target). The o[2]
  store between the o[0] store and the reload is load-bearing.
  Banked: rejected/voice2-early-cse-folds-lhu-reload.c.
- Tail statement order (90, 9C, 94, 98 to match target emission order) is INERT: still 20.
  The scheduler picks the store order; the residual is entirely the address materialization.
  Banked: rejected/tail-store-order-9c-before-94-inert.c.
- Sibling func_800194F4 writes D_80102788/8A and D_80102790/94/98/9C with plain per-symbol
  `lui at,%hi / sw ,%lo(at)` - what a scalar declaration produces - and target's own tail
  stores to 94/98/9C are likewise plain %lo stores off $at. Only D_80102790 gets a
  register-held address. INDEPENDENT EVIDENCE AGAINST s1's aggregate hypothesis for F3.

- [s2] Honest floor 34 -> 20 (structurally correct form, build_insns 141 == target 141)
- [s2] src/code6cac.c reverted to HEAD at end of session; the s2 form lives in candidate.c

- [s2] Chassis re-verified: s1's v2 body reproduces 34 exactly (build 141 == target 141), so all s1 conclusions remain chassis-valid.

- [s2] loop.c:4386-4389 -- DEST_ADDR givs are always_computable=1 regardless of conditional placement, and their mode is Pmode/SImode. This forecloses s1's F1(a) angle (respelling giv sites so none is always_computable) for any indexed loop.

- [s2] loop.c:3782 (worthwhile loop skips v->same/v->ignore), 3804 (benefit -= add_cost*biv_count), 3824 (worthwhile test), 4034 (all_reduced==1 && bl->eliminable gate), 5517 (combine_givs accumulates benefit into the leader) -- together: a giv-group LEADER with nothing combined into it and benefit 2 nets 0 and clears all_reduced, which is the ONLY practical honest lever against biv elimination in this loop.

- [s2] loop.c:702-716 + 1631 + 532 -- a scan_loop movable, once created, is always hoisted here (threshold = 2*(1+n_non_fixed_regs) dwarfs insn_count); the only lever is to prevent creation, which two non-consecutive sets of the same pseudo do.

- [s2] Honest floor progression this session: 34 (s1 v2 re-measured) -> 20 (v4, record pointers only, ORDINARY C with no family claim, build 142) -> 20 (v5, + voice reuse, build 141 == target 141) -> 17 (v7, false minimum, build 138).

- [s2] Residual at 20 is exactly two clusters: R1 (~6 insns) a v0<->v1 local-alloc seat swap in the if-arm, and R2 (~13 insns) the tail block's &D_80102790 address materialization. Everything else in the objdump diff is scorer-masked (move-vs-addu, %hi/%lo addends, and lw v0,32(at) for jtbl_80010088 addressed off the first table's %hi).

- [s2] Owner RULES-TO-ZERO directive: this session continued the pure-C route (no regfix/asmfix touched, src reverted to HEAD at end); the 5 rules retire when this reaches COMPLETED-C.

- [s2] Two independent GCC passes were named from dumps, not guessed: loop.c strength_reduce/scan_loop for both wins; the dumps are in tmp/grind/func_80019568/dumps/.

## [s3] 2026-08-25 - structural (SOLVED: honest floor 20 -> 0, build_insns 141 == target 141)

Chassis re-measured at session start: s2 candidate.c re-applied to src/code6cac.c
reproduced **20** exactly (build 141 == target 141), so every s2 conclusion is
chassis-valid and was spent, not re-derived.

### F2b SOLVED - and it was worth far more than the family question it was raised to answer.

s2 frontier F2b asked for an honest, non-variable-reuse spelling that gives the
1-holding pseudo two NON-consecutive sets. The spelling is a real per-slot flag local
written in BOTH arms and stored inside each arm:

    s32 enable;
    if (rec[0] == 0) { o[0] = rec[1] >> 4; enable = 1; o[2] = enable; ... }
    else             { o[0] = 4;           enable = 0; o[2] = enable; bits = 0; }

Measured: 20 -> **12** (build 141). It does TWO things, not one:
 (a) it keeps the `li 1` inside the loop (the F2 win, same loop.c:702-716 mechanism as
     the s2 variable-reuse spelling), and
 (b) it ALSO closes R1, the v0<->v1 seat swap in the if-arm that s2 banked as a pure
     local-alloc question needing tools/ra_solver. With `enable` a distinct pseudo from
     the shifted voice id, local-alloc gives `voice` the lbu temp seat ($v0) and the
     lhu reload $v1, exactly like target; the s2 reuse spelling forced the mirror image.
     R1 therefore never needed the RA solver - it was a consequence of the wrong
     C-level variable partition, not of an allocator tie.

Ablations (all with the s3 tail in place, so they isolate this construct):
 - bare `o[2] = 1;` / `o[2] = 0;` (no local at all): **8**, build 142 - the `li 1` is
   hoisted out of the loop again (+1 insn). rejected/bare-literal-o2-li-hoisted-8.c
 - s2 variable-reuse spelling (`voice = 1; o[2] = voice;`): **8**, build 141 - keeps
   the li inside but re-introduces the v0<->v1 swap.
   rejected/voice-reuse-instead-of-flag-8.c
 - the flag local: **0**.

INDEPENDENT EVIDENCE THAT THE ORIGINAL SOURCE HELD THIS VALUE IN A VARIABLE: in the same
loop the literal 4 is written bare in two arms (case 4/6 and the invalid arm) and target
HOISTS it into $t2 in the prologue (loop.c combine_movables merges the two equal-constant
movables). The 1 is not hoisted. The only thing GCC 2.7.2 can see that differs between
those two constants is n_times_set on the pseudo - i.e. whether a declared variable
carried the value. So target own bytes say the original wrote the 4 as a literal and
the o[2] value through a variable.

### F3 SOLVED - the tail &D_80102790 is a read-modify-write pointer handle.

Two honest tail spellings were measured first:
 - drop `old_mask` entirely and read D_80102790 three times, storing it last:
   **11**, build 141. cse folds the three reads to one `lw`, but every access is still a
   (mem (symbol_ref)) so the address is never materialised.
   rejected/tail-direct-multiread-no-la-11.c
 - (s2, already banked) statement reordering to target emission order: INERT at 20.
Then the RMW pointer:

    s32 *p = &D_80102790;
    old_mask = *p;
    *p = sp.voice_mask;

Measured: 12 -> **0**, build 141 == target 141. This produces target
`lui $v0,%hi(D_80102790); addiu $v0,$v0,%lo(D_80102790); lw $a0,0($v0); sw $v1,0($v0)`
- one `la` shared by the load and the store - and pulls the D_80102790 store EARLY,
ahead of the nor/and chain, which is where target has it. Mechanism: on MIPS a MEM whose
address is a bare symbol_ref is already a legitimate address, so no pass ever cse-s that
address into a register; only a pointer VALUE forces the la pair. This is the exact
zero-displacement pointer-RMW shape user-sanctioned on 2026-06-10
(.claude/rules/pointer-rmw-global-sanctioned.md).

### Cleanups verified codegen-neutral at 0
 - The s2 candidate carried a DEAD `s16 *output;` declaration. Removed; floor stayed 0.
   No dead local is load-bearing in this function.
 - Loop-local packet pointer renamed p -> rec (it shadowed the tail `s32 *p`);
   function-scope `packets` renamed pk. Names are codegen-neutral; re-measured 0.

### Final residual: NONE
tmp/grind/func_80019568/s3/v9.od + the s2 normalizer showed the ONLY remaining diff
classes before the tail fix were scorer-masked artifacts (move-vs-addu spellings,
%hi/%lo reloc addends, and `lw v0,32(at)` for jtbl_80010088 addressed off the first
table %hi in the same TU rodata). After the tail fix the sandbox score is 0.

- [s3] Honest floor progression this session: 20 (s2 candidate re-measured) -> 12 (flag local) -> 11 (tail multi-read, dead end) -> 0 (tail pointer RMW). build_insns 141 == target 141 throughout the winning line.
- [s3] R1 (the v0<->v1 local-alloc seat swap s2 flagged for tools/ra_solver) was NOT an allocator tie: it fell out for free once the o[2] value got its own C variable instead of borrowing `voice`. No solver run was needed.
- [s3] The `li 1` hoist and the v0/v1 seats are ONE C-level question (which variable holds the o[2] value), not two independent residuals.
- [s3] The dead `s16 *output;` local inherited from the s2 candidate is NOT load-bearing - removing it left the floor at 0.
- [s3] Owner RULES-TO-ZERO directive: the pure-C route reached distance 0; the 5 regfix rules for func_80019568 are now retirable by the operator/driver (`retire func_80019568`) - this session touched no rule file.
- [s3] Self-vet written to memory/grind/func_80019568/self_vet.md: C1 (record pointers) ordinary C, C2 (`enable` flag) claimed under named-local-fake-exception with FAKE, C3 (`s32 *p` RMW) claimed under pointer-rmw-global-sanctioned + pointer-alias-fake-exception with FAKE.
