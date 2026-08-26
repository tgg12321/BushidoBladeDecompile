/* CANDIDATE - func_800871D4 (_SsVmKeyOffNow) - session 4 (2026-08-26)
   sandbox --disable all: score = 0   (target_insns = build_insns = 52)
   FULL BUILD verified this session: verify-oracle build_sha1 ==
   62efab4f73f992798c43e8c730aa43baa10bb4fa (build_matches true), with ZERO
   regfix/asmfix rules and ZERO inline asm in this function.

   WHAT CLOSED IT (the s3 frontier was aiming at the wrong lever):
   s3 left the function at score 3 on a 53-instruction chassis, having flipped
   the two mask locals' register seats ($a1/$a2) by hoisting `mask_lo = 0` into
   block 0 - which cost one instruction, because sched2 sinks that store to
   just before the `beqz` and reorg.c then takes it for the branch delay slot,
   displacing the target's shared `addiu $v0,$zero,1`.  s3's ra_solver analysis
   framed the remaining choice as `refs_up pseudo 76: 3->4` or
   `refs_down pseudo 75: 3->2`.  Both were unnecessary.

   The seat flip is reachable through the FOURTH inverse.py family that s3
   recorded but declared unreachable: live_shrink of the else-arm mask.  Its
   live range is dominated by its block-3 contribution - it stays live from the
   join all the way down to `or $a0,$a0,$a1`, 18 RTL insns in, purely because
   the source finishes the whole D_801078D8 read-modify-write group before it
   starts the D_801078DA one.  Interleaving the two groups (both loads, then
   both ORs, then both stores, then both masked write-backs) is an ordinary,
   semantically identical statement ordering that moves the else-arm mask's
   death ~5 RTL insns earlier.  That drops its live_length below the then-arm
   mask's, so global.c's allocno_compare (floor_log2(n_refs)*n_refs*size /
   live_length) sorts it FIRST and it takes $a1 - the target's seat - with no
   block-0 store and therefore no extra instruction.  Measured on the s3
   score-6 chassis: score 6 -> 0, build_insns 52 == target_insns 52.
   Partial interleaves are NOT enough (moving only the DA `or` up, or only the
   DA `or` above the D_800F1B10 load, both stay at score 6): the DA store and
   the D_800F1B12 write-back must move up too.

   The two `& 0xFFFF` masks and the in-place `vc -= 0x10` are inherited from s3
   and remain load-bearing (see self_vet.md): dropping the masks gives score 6 /
   53 insns, and a `u16 vc` narrow local instead gives 55 insns (extra
   PROMOTE_MODE truncations).  The `u16 raw` + `u32 vc = raw & 0xFFFF` spelling
   is the same shape already shipping in accepted BB2 C two Sony-library
   functions away - SpuGetVoiceVolume, src/main.c:2884 and src/main.c:2891.

   Naming: the function is Sony LIBSND `_SsVmKeyOffNow`; D_801078D8/DA are the
   pending key-off masks for voices 0-15 / 16-23 and D_800F1B10/12 the matching
   key-on masks, which is why each key-off bit is cleared from the key-on word.
   The symbol is deliberately NOT renamed (queue keys and this ledger use it). */
void func_800871D4(s32 arg0)
{
    u16 raw;
    u32 vc;
    s32 mask_lo;
    s32 mask_hi;
    s32 off;
    u16 keyoff_lo;
    u16 keyoff_hi;
    u16 keyon_lo;
    u16 keyon_hi;

    raw = D_8010280A;
    vc = raw & 0xFFFF;
    if (vc < 0x10U) {
        mask_lo = 1 << vc;
        mask_hi = 0;
    } else {
        mask_lo = 0;
        vc -= 0x10;
        mask_hi = 1 << vc;
        vc = raw & 0xFFFF;
    }
    off = ((((vc * 8) - vc) * 4) - vc) * 2;
    *((s8 *)((u8 *)&D_800F4E35 + off)) = 0;
    keyoff_lo = D_801078D8;
    keyoff_hi = D_801078DA;
    *((s16 *)((u8 *)&D_800F4E1C + off)) = 0;
    *((s16 *)((u8 *)&D_800F4E18 + off)) = 0;
    keyon_lo = D_800F1B10;
    keyon_hi = D_800F1B12;
    keyoff_lo = keyoff_lo | mask_lo;
    keyoff_hi = keyoff_hi | mask_hi;
    D_801078D8 = keyoff_lo;
    D_801078DA = keyoff_hi;
    D_800F1B10 = keyon_lo & ~keyoff_lo;
    D_800F1B12 = keyon_hi & ~keyoff_hi;
}
