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
