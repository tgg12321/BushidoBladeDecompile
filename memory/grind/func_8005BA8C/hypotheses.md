# Hypothesis ledger - func_8005BA8C

## s1 (recon, 2026-09-15)

- H1 CONFIRMED - Loop 1 is an index loop `for (i = 0; i < 3; i++)` over D_8009AD18 whose counter is reused by loops 2 and 3; loop.c eliminates the biv, emits its final value (`addiu $sN,$zero,3`) at the loop end, and cse2 consumes it for the `count == 3` compare. Probe: v1 (pointer for-loop) 104 -> v2 (pointer do-while + separate n=3) 95 -> v4 (index loop, i reused, count==3) 67. Result: the `addiu 3` lands at the loop exit exactly as in the target; the loop body, bound and hoist order match.
- H2 CONFIRMED - `u8 count = 3; if (arg2 == arg3) count = 2;` is the count spelling. Probe: v2 (u8) 95 vs v3 (s32 + explicit u8 casts) 96 on the same chassis; on v4 the andi/raw pattern is byte-identical at all six use sites.
- H3 CONFIRMED - The 0x38 spill slot is a `base = (s32)hdr` local copy assigned after func_80062020 and consumed only by the return; on v4 the frame becomes 0x80 (target) and the return computes `dst - base`. Residual: global.c spills arg3 (pseudo 75, pri 625, allocated last) instead of base (pseudo 76, pri 1250, gets $s7) - a consequence of the seat cascade in H5, not of the spelling.
- H4 KILLED (instance) - Explicit pointer locals for D_800EFC38/D_800EFB38 (func_8005BDF0's spelling) change nothing: v5 == v4 byte-for-byte (score 67). measured_on: v4 chassis, no FAKE. Do not re-try as a lever for the loop-1 hoist order; it is already correct.
- H5 OPEN (frontier #1) - The entire remaining residual (67) is one global.c order flip: pseudo 212 (loop-3 ent-pointer giv, pri 10000) must be allocated before pseudo 80 (i, pri 10114). inverse.py says depth-1 reachable via i refs 22->21, i live length +2, 212 refs 7->8, or 212 live length -2. Candidate C levers to measure next (ordinary C first): (a) shorten i's ref count by one plain ref - e.g. loop 1 using a DIFFERENT counter variable whose final value feeds the compare (then i is born at loop 2: fewer refs but shorter live range - measure, the two effects oppose); (b) lengthen i's live range by 2 insns - e.g. give i a use after loop 3; (c) raise 212's refs by one - a second read of `loc.ent[i].off` in the loop-3 body outside the inner loop (e.g. a named `off` local read once and used for both the snd_VabFakeOpen delta and the store). Each probe: apply on v4, sandbox, check `.greg` order for 80 vs 212, and confirm the cascade (dst -> $s3, arg3 -> $fp, base spilled).
- H6 OPEN (frontier #2) - If H5's flip alone does not cascade to the target seats, the next divergence to check in .greg is dst (82, pri 7500) needing $s3 by preference onto hdr (72): requires hdr to be dead at `dst = hdr` - spell the running pointer as the PARAM itself (advanced in place, `base` the saved copy) so no copy exists at all.

## [s1] Loop 1 is `for (i = 0; i < 3; i++)` with the same i reused in loops 2/3; loop.c biv final value + cse2 produce the `addiu $s2,$zero,3` consumed by the count==3 compare
- mechanism: loop.c strength_reduce eliminates the biv in favour of the D_8009AD18 pointer giv and emits the biv final value at loop end because the pseudo is referenced after the loop; cse2 follows the LABEL_NUSES==1 taken path of `bne $s7,$fp` into the join block and replaces the compare's constant temp with the equivalent reg
- probe: v1 pointer for-loop (104) -> v2 pointer do-while + n=3 (95) -> v4 index loop with i reused (67); dumps in tmp/grind/func_8005BA8C/dumps
- result: v4 places the addiu 3 at the loop exit exactly as the target; loop 1 body/bound/hoist order byte-identical modulo seat names
- verdict: CONFIRMED

## [s1] count is `u8 count = 3; if (arg2 == arg3) count = 2;`
- mechanism: one zero_extendqisi2 CSE'd across the ==3 compare, the loop-2 guard and the loop-2 bound; QImode pseudo compared raw at loop 3; fresh andi at the ==2 tail
- probe: v2 (u8) 95 vs v3 (s32 count with (u8) casts) 96 on the same chassis
- result: u8 spelling reproduces all six use sites byte-identically on v4
- verdict: CONFIRMED

## [s1] The 0x38 frame slot is a `base = (s32)hdr` local copy assigned after func_80062020, consumed only by the return
- mechanism: spilled pseudo (global.c gives it no hard reg) stored from $s3 at its set, reloaded into $t0 at the return; a store cannot be scheduled across the preceding call so the set sits between func_80062020 and the bne
- probe: v4 adds `base = (s32)hdr;` after the call and returns dst - base
- result: frame becomes 0x80 (target); on our chassis global.c spills arg3 (pri 625) instead of base (pri 1250) because of the H5 cascade
- verdict: CONFIRMED

## [s1] Explicit pointer locals for D_800EFC38 / D_800EFB38 (the func_8005BDF0 spelling) change the loop-1 hoist order or seats on this chassis
- mechanism: LICM hoist order follows the RTL order of the invariant address loads
- probe: v5 = v4 + `u32 *tblA = D_800EFC38; u32 *tblB = D_800EFB38;` used in loop 1
- result: v5 is byte-identical to v4 (score 67, same .o); the hoist order EFC38, EFB38, AD18 already matches the target in v4
- verdict: KILLED
- kill_scope: instance
- measured_on: v4 chassis (index loop 1, u8 count, base copy), no FAKE constructs, HEAD 205b551bb -mel -msoft-float
