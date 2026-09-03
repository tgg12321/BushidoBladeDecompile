# Hypothesis ledger — func_8003A728

## s1 (2026-09-02, recon) — floor 38 → 3 (v2.c = candidate.c)

### H1 — `s32 c0lo` (not `u16`) fixes the `or v0,v0,v1` operand order — CONFIRMED (instance)
Mechanism: combine.c:2973 swaps commutative operands when one side becomes a
non-object (`zero_extend (reg:HI)`); an SImode local keeps expansion order.
Measured: draft1 38 → v2a 37.

### H2 — multi-set `s32 t` staging keeps `lh` (not `lhu`) at the `& 0xF` sites — CONFIRMED (instance)
Mechanism: direct `s16` read expands as a HImode move (printed `lhu`); staging
into a multi-set SImode local expands as `extendhisi2` and combine cannot fold it
into the `and` (combine.c:726 `reg_n_sets > 1`). Measured: 37 → 34.

### H3 — u16 low-half loads written into the existing `buf8` local reach the s0 seat — CONFIRMED (instance)
Mechanism: the loads join the call-crossing pseudo; global.c priority order then
gives buf8→s0, param→s1, lower→s2. Measured: 34 → 3. A fresh (non-call-crossing)
temp cannot get s0 under global.c's allocation (caller-saved regs come first in
numeric order); a spelled fresh-temp form was measured implicitly in v2b (inline
reads, 34).

### H4 — the `lbu D_800A3916` must land AFTER the hash `or` via a T-3 stall in sched1 — CONFIRMED (instance, mechanism)
Mechanism: sched.c function-unit hazard forbids load-immediately-before-store;
the target text is only reachable if both the packed store and the `or` are
unready at T-3, i.e. the lbu writes the pseudo both of them read. Spelled as one
scratch variable `packed` carrying packed → hash-low → flag (v3b). Measured: the
order becomes exact; residual moves to RA (24, pure seats).

### H5 — v3b seat rotation (packed a0/hi16 v1 instead of v0/a0) fixable by spelling — OPEN
Tried (KILLED, instance, v3b chassis): `^=` compound steps (v4a, 24); single
`packed = (...) & 0xFFFF` step (v4b, 26). Neither moves the seats.
Next probes, in order:
1. Keep hi16 GLOBAL-allocated (in v2 it wins a0 with packed in v0): e.g. give
   `hi16` a use outside block 1 or declare/compute it earlier so its qty is not
   local-alloc'd; or compute `hi16 >> 16` from `D_800A37C4` directly
   (`packed ^= (s16)D_800A37C4`?) — check whether the target's `sra v1,a0,16`
   still appears (it does in target: `sra $v1,$a0,16`), so hi16 must stay a
   register value read twice.
2. Run the RA solver properly: `tools/ra_solver/extract.py code6cac_c_mid` then
   `inverse.py` on block 1 with goal `{packed: v0, hi16: a0}` (see README §Phase
   6 usage); `classify` already says RA-only.
3. Alternative dependence shapes for H4 that keep v2's allocation: any spelling
   where the flag load writes a pseudo read by BOTH the packed store and the
   `or` — a variable other than `packed` cannot satisfy the store side, so the
   space is: which of the hash steps are in-place on `packed` (v3a/v3b/v4a/v4b
   explored 4 of them; the remaining axis is where `hi16` is born relative to
   the chain — LUID order feeds both sched1 tie-breaks and qty birth order).

### Rejected forms (this session)
- `u16 c0lo` (HImode local) — wrong `or` operand order (H1).
- direct `D_800A36C2 & 0xF` reads — `lhu` instead of `lh` (H2).
- inline `(u16)D_800A3698` / `(u16)D_800A36D0` reads inside the arms — no s0 seat (H3).

## [s1] An SImode (s32) c0lo holding (u16)D_800A36C0 keeps the target's `or v0,v0,v1` operand order in the D_800A38A0!=0 arm; a u16 (HImode) c0lo does not
- mechanism: combine.c:2973 commutative canonicalisation swaps operands once zero_extend(reg:HI) is substituted into the ior; an SImode pseudo keeps expansion order
- probe: draft1 (u16 c0lo) vs v2a (s32 c0lo), sandbox --disable all
- result: 38 -> 37; the or operand order matches
- verdict: CONFIRMED

## [s1] Staging D_800A36C2/D_800A36D2 through a multi-set s32 local t before `& 0xF` yields lh+andi instead of lhu+andi at all three sites
- mechanism: direct s16 read expands as movhi_internal2 (printed lhu) with the and applied to a subreg; the staged form expands extendhisi2 and combine.c:726 (reg_n_sets>1) blocks folding the sign-extend into the and
- probe: v2a vs v2b
- result: 37 -> 34
- verdict: CONFIRMED

## [s1] Writing the (u16)D_800A3698 / (u16)D_800A36D0 low-half loads into the existing call-crossing buf8 local yields the target's buf8->s0, param->s1, lower->s2 allocation and the `lhu s0` in both arms
- mechanism: global.c allocno priority: the reused pseudo's ref count exceeds the param copy's, so it is allocated first and takes s0; a fresh non-call-crossing temp takes the first free caller-saved reg and can never reach s0
- probe: v2b (inline u16 reads) vs v2 (buf8 reuse)
- result: 34 -> 3; only the lbu D_800A3916 placement remains
- verdict: CONFIRMED

## [s1] The target's `or a0; lbu v0; sw D_800A369C; beqz` order arises from a sched1 T-3 stall, which requires the lbu's destination pseudo to be read by both the packed store and the hash or; spelling the hash as in-place steps on `packed` and reading the flag into `packed` (v3b) reproduces the exact order
- mechanism: sched.c actual_hazard_this_instance (sched.c:1200-1240): a load immediately before a store on the memory unit is blocked one cycle; only an empty ready list at T-3 lets the lbu sit at T-4 adjacent to the store; register anti-dependences from the flag load to the store and the or make both unready
- probe: v2 (3) vs v3a (in-place steps only, 5) vs v3b (steps + flag into packed, 24), fdiff of sandbox objects
- result: v3b: instruction order identical to target including the lbu; remaining 24 = register seats only (inverse_compose classify: FIRST DIVERGENCE RA: packed a0 vs v0, hi16 v1 vs a0)
- verdict: CONFIRMED

## [s1] On the v3b chassis, respelling the hash chain as compound `^=` steps (v4a) or as a single `packed = (...) & 0xFFFF` step (v4b) restores the packed->v0 / hi16->a0 seats
- mechanism: different pseudo counts / LUIDs might change local-alloc qty ordering in block 1
- probe: v4a, v4b sandbox --disable all
- result: 24 and 26 — seats unchanged (packed a0, hi16 v1)
- verdict: KILLED
- kill_scope: instance
- measured_on: post-migration main chassis, -mel, no rules; v3b body with buf8 reuse + multi-set t staging + flag-into-packed present, no FAKE annotations


## s2 (2026-09-02, structural) - floor 3 -> 3, shape changed, 24-family explained

### H6 - the v3b/24 seat rotation is caused by `packed` acquiring a second REG_DEAD note - CONFIRMED (instance)
Mechanism: local-alloc.c:472 only creates a local quantity for a pseudo with
`reg_n_deaths == 1`; reusing `packed` for the D_800A3916 flag gives it two disjoint
live ranges, so it is handed to global_alloc, which finds v0 already taken by the
block-1 local temps and assigns a0. Measured: v3b/v7a/x1/x3/x4 all 24; the .lreg slice
shows "Register 74 ... dies in 2 places" with no ";; Register 74 in N" line, and the
.greg dispositions read "74 in 4" (a0), "75 in 3" (v1).

### H7 - reading the flag into a variable OTHER than `packed` restores packed to v0 - CONFIRMED (instance)
Mechanism: with one death `packed` becomes a local quantity and its priority
(floor_log2(12)*12/13) beats every 2-ref temp in block 1, so it is allocated first and
takes v0. Measured: v3b 24 -> v6a 10 (flag into hi16), and 24 -> v7c/w3 3 (fresh `flag`).

### H8 - the ior written in place on hi16 gives the target's `or a0,a0,v0` - CONFIRMED (instance)
Mechanism: the ior's destination is then hi16's own pseudo rather than a fresh temp, so
it keeps hi16's hard register instead of taking the lowest free one. Measured: v8c 6 ->
v9b 5 (the two or/lbu register diffs disappear).

### H9 - the position of `hi16 = D_800A37C4 << 16;` controls the lhu/sll schedule slots - CONFIRMED (instance)
Mechanism: sched1's LUID tie-break (sched.c:2464) on equal-priority ready insns; the
statement's LUID decides whether the load floats up next to `lh v1,0(s1)` and whether the
`sll a0,a0,0x10` lands before `sra v1,v0,0x10`. Measured across five placements on the
v6a chassis: 14 / 10 / 6 / 3 / 14 (v8a / v8b / v8c / w3 / w2).

### H10 - declaration order, unused-local removal, and flag-statement position are inert on this residual - KILLED (instance)
Mechanism hypothesised: pseudo numbering / LUID order feeding local-alloc qty ties.
Measured: v5a/v5c/v5d (decl order, dropping the dead v0/v1 locals) all 24 on the v3b
chassis; z2 (flag statement before the ior) and z3 (no flag local at all) both 3 on the w3
chassis - byte-identical to w3.
- kill_scope: instance
- measured_on: HEAD chassis 9de438a2, -mel, no rules; w3/v3b bodies with buf8 reuse + multi-set t staging, no FAKE annotations

### H11 - on the w3 chassis the last 3 insns are not closed by any of the three ways of making the ior unready at sched1 T-3 - KILLED (instance)
Mechanism: the target text needs a stall at T-3, i.e. insn 58 (the ior) must be unready
there, which requires the flag lbu to write a pseudo that 58 reads - reg 74 (packed) or
reg 75 (hi16). Writing 74 trips local-alloc.c:472 (24). Writing 75 with the ior in place
makes the store read 75 too, pushing the lbu past the store plus a load-delay nop (6-10).
Writing 75 with a fresh ior destination T gets the order exactly right but T and the flag
overlap in the target's own order, so T cannot share a0 (6-10). Measured: x1 24, x3 24,
x4 24, v9a 6, v7b 10, v8c 6, v6a 10, v6c 10, x2 10.
- kill_scope: instance
- measured_on: HEAD chassis 9de438a2, -mel, no rules; w3 body (fresh `flag` local, in-place ior on hi16, hi16 statement between the D_800A3698 store and the first hash step), buf8 reuse + multi-set t staging present, no FAKE annotations

### Rejected forms (this session)
- flag-into-packed (any spelling measured) - local-alloc.c:472 two-deaths gate, 24.
- flag-into-hi16 with a fresh ior destination - seats swapped, 6.
- flag-into-hi16 with the ior in place - lbu past the store + nop, 6-10.
- separate `raw` local for the pre-hash value - 27.

## [s2] The 24-insn seat rotation seen in v3b (and in every other spelling that reads D_800A3916 into the `packed` variable) is caused by `packed` acquiring a second REG_DEAD note, which disqualifies it from local allocation and hands it to global_alloc.
- mechanism: local-alloc.c:472 creates a local-alloc quantity only for pseudos with reg_basic_block>=0 AND reg_n_deaths==1. Reusing `packed` for the flag gives reg 74 two disjoint live ranges (hash value, then flag), so reg_qty stays -1 and packed falls through to global_alloc; by then the block-1 local temps have taken v0/v1, so packed gets a0 while the target has it in v0.
- probe: Sliced .lreg/.greg for func_8003A728 on the v3b state (Register 74 'dies in 2 places', no ';; Register 74 in N' line, greg '74 in 4'), then measured four independent flag-into-packed spellings on two different hi16 statement placements: v3b, v7a, x1, x3, x4.
- result: All five flag-into-packed forms score 24 regardless of hash spelling, declaration order or statement order; the .lreg/.greg slices show the predicate firing exactly as described.
- verdict: CONFIRMED

## [s2] Reading D_800A3916 into a variable other than `packed` leaves `packed` with one death, makes it a local-alloc quantity, and restores its v0 seat.
- mechanism: With one death `packed` becomes a qty; its local-alloc priority floor_log2(12)*12/13 beats every 2-ref block-1 temp, so it is allocated first and takes the lowest free hard reg, v0.
- probe: v3b (flag into packed, 24) vs v6a (flag into hi16) vs v7c/w3 (flag into a fresh `flag` local), sandbox --disable all plus fdiff against the target object.
- result: v3b 24 -> v6a 10 -> w3 3. In w3 every register seat in the function matches the target.
- verdict: CONFIRMED

## [s2] Writing the final ior in place on hi16 (rather than into a fresh temporary) makes its destination inherit hi16's hard register and prints the target's `or a0,a0,v0`.
- mechanism: The ior's SET_DEST is hi16's own pseudo instead of a newly born temp, so it keeps hi16's allocation rather than taking the lowest free caller-saved register.
- probe: v8c (fresh ior destination, 6) vs v9b (in-place ior, 5), then w3; fdiff against the target object.
- result: The two register diffs on the ior and the following lbu disappear (6 -> 5, and 3 in w3).
- verdict: CONFIRMED

## [s2] The source position of the single statement `hi16 = D_800A37C4 << 16;` controls both the schedule slot of its lhu and the slot of its sll relative to the first hash step.
- mechanism: sched1's LUID tie-break in rank_for_schedule (sched.c:2464) on equal-priority ready insns; the statement's LUID decides whether the load floats up next to `lh v1,0(s1)` and whether `sll a0,a0,0x10` lands before `sra v1,v0,0x10`.
- probe: Five placements of that one statement measured on the v6a chassis (v8a, v8b, v8c, w3, w2) plus two load/shift splits (w4, w5, w6, w7).
- result: 14 / 10 / 6 / 3 / 14, and 12 for every load-shift split; it is the strongest single structural lever found in block 1.
- verdict: CONFIRMED

## [s2] Declaration order of the block-1 locals, removal of the two never-used s32 locals inherited from s1's drafts, and moving the flag-read statement relative to the ior leave the measured score unchanged on this residual.
- mechanism: Hypothesised: pseudo numbering and LUID order feeding local-alloc qty ties and sched1 tie-breaks.
- probe: v5a (dead locals dropped), v5c/v5d (hi16 declared before packed) on the v3b chassis; z2 (flag statement before the ior) and z3 (no flag local, inline `if (D_800A3916 != 0)`) on the w3 chassis.
- result: v5a/v5c/v5d all 24 (identical to v3b); z2 and z3 both 3 (byte-identical to w3). The dead-local removal is byte-neutral and has been taken into the candidate as hygiene.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 9de438a2, -mel, no rules; v3b and w3 bodies with the buf8 low-half reuse and the multi-set s32 t staging present, no FAKE annotations

## [s2] The three flag-carrier spellings measured on the w3 chassis - flag into packed, flag into hi16 with the ior in place, and flag into hi16 with a fresh ior destination - each leave the last 3 instructions unclosed, so the sched1 T-3 stall was not reached this session.
- mechanism: The target text needs an empty ready list at sched1 T-3 so the lbu drops to T-4 adjacent to the store. Insn 58 (the ior) is otherwise ready there, and the only insn that can make it unready is the lbu itself via an anti-dependence, which requires the lbu to write one of the two pseudos 58 reads: reg 74 (packed) or reg 75 (hi16). Writing 74 trips local-alloc.c:472. Writing 75 with the ior in place makes the store read 75 as well, so the anti-dependence pushes the lbu past the store and maspsx adds a load-delay nop. Writing 75 with a fresh ior destination T yields the exact target order, but in that order T is born at the ior, the flag at the lbu, and T dies at the store, so T and the flag overlap and T cannot share a0 with hi16.
- probe: x1 / x3 / x4 (flag into packed on the w3 chassis), v9a / v7b (flag into hi16, ior in place), v8c / v6a / v6c / x2 (flag into hi16, fresh ior destination); each measured with sandbox --disable all and fdiff'd against the target object, with the post-sched1 RTL for w3 read out of the .sched dump (insn order 56, 65, 58, 61, 68 with 58's LOG_LINKS 48 / REG_DEP_ANTI 53 / 56).
- result: 24, 24, 24 / 6, 10 / 6, 10, 10, 10 respectively - none reaches 0 and none beats w3's 3.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 9de438a2, -mel, no rules; w3 body (fresh `flag` local, in-place ior on hi16, hi16 statement between the D_800A3698 store and the first hash step) with buf8 reuse and multi-set t staging present, no FAKE annotations

## s3 (2026-09-02, structural) — floor 3 (unchanged); block-1 dependence space closed

### H-s3-1 — the s2 frontier's "fourth flag-carrier arrangement" (or-dest = packed, flag into hi16) gives the T-3 stall AND the a0 seat — KILLED (instance)
Mechanism (as proposed by s2): the trichotomy is exhaustive only for block-1 shapes that route the
stored value through hi16; making the ior write `packed` and the flag land in `hi16` gives the
anti-dependence (58 reads hi16, 65 writes it) without the store reading the carrier.
Probe: y6 = `packed = packed & 0xFFFF; packed = hi16 | packed; D_800A369C = packed;
hi16 = D_800A3916; if (hi16 != 0)`, `sandbox --disable all`.
Result: 10. The anti-dependence is there and the schedule is right, but pseudo 75 now spans the
`sll a0,a0,0x10` (which wants $a0, the target's hi16 seat) and the `beqz` (which wants $v0, the
target's flag seat); one pseudo cannot hold two seats, so block 1 re-rotates. Companion form y5
(or-dest packed, FRESH flag, no anti-dep) = 6 with `or v0,v0,a0` operand order wrong.
Verdict: KILLED. kill_scope instance; measured on HEAD 1c8fa981, -mel, no rules, w3 chassis, no
FAKE constructs present.

### H-s3-2 — an OUTPUT dependence (the ior's dest reused as the flag carrier) empties the T-3 ready list without touching either hash pseudo — KILLED (instance)
Mechanism: sched.c adds a dependence 58 -> 65 for an output (write-after-write) as well as for an
anti (write-after-read), so reusing the ior's own destination for the flag read makes 58 unready at
T-3 while leaving hi16 and packed untouched.
Probe: y1 = `flag = hi16 | packed; D_800A369C = flag; flag = D_800A3916; if (flag != 0)`.
Result: 6, and 201 insns instead of 200. The dependence works, but insn 61 (the store) READS the
same pseudo, so 61 also becomes a dependent of 65 and is pushed below it: the emitted order is
`or a0,a0,v0 / lui at / sw a0,0(at) / lbu a0 / nop / beqz a0` — the load leaves the block tail and
maspsx adds a load-delay nop. Any output-dependence spelling has this property, because the store
must read the ior's destination by definition.
Verdict: KILLED. kill_scope instance; measured on HEAD 1c8fa981, -mel, no rules, w3 chassis, no
FAKE constructs present.

### H-s3-3 — the sched1 T-3 blockage can be dodged by changing what occupies the memory unit at T-2/T-3 (s2 frontier item 1) — KILLED (class)
Mechanism proposed by s2: if the insn at T-2 were not a store, the load-after-store blockage would
not apply and the flag lbu would win T-3 on priority (it carries 0x7f000001 vs the ior's 2).
Probe: instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_SCHED_DEBUG=1 over the w3 source; read the
block-1 trace in the .sched dump and the SCHEDDBG BLOCKAGE/SELBLOCK lines in s3/sched_debug.txt.
Result: the trace prints `;; ready list at T-2: 61 (2), now 61` — the D_800A369C store is the ONLY
insn ready at T-2, so no spelling can put a different insn there without also emitting that insn
between the sw and the beqz (which the target text does not contain). The blockage itself is a
machine-description constant, not a source property: mips.md:153-161 gives the memory unit ready
delay 2 for a load and 1 for a store, the blockage function returns 2 in the load-after-store
direction only, and actual_hazard therefore returns cost 1 for ANY load offered at T-3 after a
store at T-2 (`SCHEDDBG BLOCKAGE unit=0 clock=3 raw_tick=5 adj_tick=4 maxb=3 exec=65 last=61`).
Verdict: KILLED. kill_scope class; predicate tools/gcc-2.7.2/sched.c:2685
(`if ((cost = actual_hazard (insn_unit (insn), insn, clock, 0)) != 0)` — the ready insn is queued
whenever the unit hazard is non-zero). measured on HEAD 1c8fa981, -mel, no rules, w3 chassis, no
FAKE constructs present.

### H-s3-4 — freeing $v0 for the (globally allocated) flag-carrying `packed` by removing the competing block-1 local quantities — KILLED (instance)
Mechanism: local_alloc runs before global_alloc and hands $v0 to the first block-1 local quantity
it processes; if the two `sra` temporaries were multi-set pseudos (two REG_DEAD notes) they would
be excluded from local allocation too, leaving $v0 free for reg 74, whose 12 references make it the
highest-priority allocno in the block.
Probe: on the z0 (v3b) chassis — z10 (one shared named `h` for both sra results), z11 (two distinct
named temps), z13 (`h` also reused as the ior destination), zt3/zt4 (one sra result staged through
the existing multi-block `t` local), zt/zt2 (both staged through `t`).
Result: z10 = z11 = z13 = zt3 = zt4 = 24, byte-identical to z0; combine re-collapses the named
temporaries so the quantity landscape is unchanged. zt = 33 and zt2 = 21 are worse because reusing
`t` for the hash also perturbs the `& 0xF` staging sites in the later arms. z12 (the same shared
temp on the w3 fresh-flag chassis) = 12, i.e. the construct is not even byte-neutral there.
Verdict: KILLED. kill_scope instance; measured on HEAD 1c8fa981, -mel, no rules, z0/w3 chassis, no
FAKE constructs present.

### Standing structural picture after s3
w3 (floor 3) is the no-dependence branch: correct seats everywhere, ior at T-3, lbu at T-4.
z0/v3b (24) is the dependence branch: correct ORDER, and its register geometry is the target's own
(the target uses $v0 for the hash accumulator and for the flag), but the carrier pseudo has two
REG_DEAD notes and so is refused a local-alloc quantity (local-alloc.c:471) and reaches
global_alloc after $v0 is taken. Every arrangement between those two poles is now measured.
The untried axes are (i) changing the block-1 INSN SET so that either the ready list at T-2 or the
local-quantity ordering differs, and (ii) the ra_solver/inverse route: enumerate which
{packed:$v0, hi16:$a0, flag:$v0} configurations are REACHABLE for a twice-dying carrier, instead of
guessing spellings.

## [s3] The s2 frontier's fourth flag-carrier arrangement - ior destination `packed`, flag read into `hi16` - produces the sched1 T-3 anti-dependence AND keeps the a0 seat for the or destination.
- mechanism: sched.c adds a dependence 58 -> 65 when the load writes a pseudo the ior reads (REG_DEP_ANTI); routing the stored value through `packed` instead of `hi16` was supposed to keep insn 61 (the store) from also depending on the load, so the lbu would not be pushed past the store.
- probe: y6 = `packed = packed & 0xFFFF; packed = hi16 | packed; D_800A369C = packed; hi16 = D_800A3916; if (hi16 != 0)` applied to src/code6cac_c_mid.c, `sandbox func_8003A728 --disable all`; companion control y5 (same shape with a FRESH flag local).
- result: y6 = 10, y5 = 6 (w3 baseline 3). The anti-dependence and the schedule are right, but pseudo 75 now has to serve `sll a0,a0,0x10` (the target's hi16 seat, $a0) and the `beqz` (the target's flag seat, $v0) at once, so block 1 re-rotates. Banked as rejected/frontier-4th-arrangement-or-dest-packed-flag-into-hi16-10.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 1c8fa981 chassis, -mel, no rules; w3 body with the buf8 low-half reuse and the multi-set s32 t staging; no FAKE constructs present in the measured diff

## [s3] An OUTPUT dependence - reusing the hash or's own destination local as the flag carrier - empties the T-3 ready list without disturbing either hash pseudo.
- mechanism: sched.c creates a dependence 58 -> 65 for a write-after-write just as it does for a write-after-read, so the ior becomes unready until the flag load is scheduled, while `hi16` and `packed` keep their single live ranges and their local-alloc quantities.
- probe: y1 = `flag = hi16 | packed; D_800A369C = flag; flag = D_800A3916; if (flag != 0)`, `sandbox func_8003A728 --disable all` plus tools/pairdiff.py.
- result: 6, and 201 emitted insns instead of 200. The dependence fires, but insn 61 (the store) reads that same pseudo, so 61 also becomes a dependent of the load and is scheduled below it: the emitted tail is `or a0,a0,v0 / lui at / sw a0,0(at) / lbu a0 / nop / beqz a0`. The store must read the ior's destination by definition, so this coupling is inherent to the output-dependence spelling. Banked as rejected/output-dep-or-dest-reused-for-flag-lbu-past-store-201insns-6.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 1c8fa981 chassis, -mel, no rules; w3 body with the buf8 low-half reuse and the multi-set s32 t staging; no FAKE constructs present in the measured diff

## [s3] The s2 frontier's first item - reach the T-3 slot by changing what occupies the memory unit at T-2/T-3 rather than by making the ior unready - is available to some spelling of block 1.
- mechanism: s2 hypothesised that if the insn at T-2 were not a store, the load-after-store blockage would not apply and the flag lbu (INSN_PRIORITY 0x7f000001, larger LUID) would take T-3 on rank, giving the target text with w3's already-correct seats.
- probe: Ran the instrumented cc1 (tools/gcc-2.7.2/cc1) over the TU with BB2_SCHED_DEBUG=1 (tmp/grind/func_8003A728/s3/dbg.sh) and read the block-1 schedule trace in the .sched dump plus the SCHEDDBG BLOCKAGE / SELBLOCK lines in tmp/grind/func_8003A728/s3/sched_debug.txt.
- result: The trace prints `;; ready list at T-2: 61 (2), now 61` - the D_800A369C store is the only insn ready at T-2, so nothing else can occupy that slot without also being emitted between the sw and the beqz, which the target text does not contain. The blockage is a machine-description constant, not a source property: mips.md:153-161 gives the memory unit a ready delay of 2 for a load and 1 for a store, the blockage function returns 2 only in the load-after-store direction, and the dump shows `SCHEDDBG BLOCKAGE unit=0 clock=3 raw_tick=5 adj_tick=4 maxb=3 exec=65 last=61` -> `SELBLOCK clock=3 insn=65 unit=0 cost=1`. Any load offered at T-3 behind a store at T-2 is queued.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 1c8fa981 chassis, -mel, no rules; w3 body, instrumented cc1 dump; no FAKE constructs present in the measured diff
- predicate_cite: tools/gcc-2.7.2/sched.c:2685

## [s3] Freeing $v0 for the twice-dying flag carrier by removing the competing block-1 local quantities lets global_alloc give reg 74 the target's $v0 seat on the v3b/z0 shape.
- mechanism: local_alloc runs before global_alloc and hands $v0 to the first block-1 local quantity it processes (the two `sra` results); making those temporaries multi-set pseudos gives them two REG_DEAD notes, excluding them from local allocation too, so $v0 would still be free when global_alloc reaches reg 74 - the highest-priority allocno in the block with 12 references.
- probe: On the z0 (v3b) chassis: z10 (one shared named `h` for both sra results), z11 (two distinct named temps), z13 (`h` also reused as the ior destination), zt3/zt4 (one sra result staged through the existing multi-block `t` local), zt/zt2 (both staged through `t`), plus z12 (the same construct on the w3 fresh-flag chassis). All measured with `sandbox --disable all`; z0's .lreg/.greg dumped for the disposition read.
- result: z10 = z11 = z13 = zt3 = zt4 = 24, byte-identical to the z0 control - combine re-collapses the named temporaries so the quantity landscape does not move. zt = 33 and zt2 = 21 are worse (reusing `t` for the hash perturbs the `& 0xF` staging sites in the later arms). z12 = 12, so the construct is not even byte-neutral on the w3 chassis. The dumps confirm the mechanism it was meant to defeat: .lreg says "Register 74 used 12 times across 12 insns in block 1; dies in 2 places" and the .greg dispositions are `74 in 4` ($a0) / `75 in 3` ($v1).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 1c8fa981 chassis, -mel, no rules; z0 (v3b) and w3 bodies with the buf8 low-half reuse and the multi-set s32 t staging; no FAKE constructs present in the measured diff
