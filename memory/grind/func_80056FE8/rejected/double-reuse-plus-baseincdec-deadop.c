/* REJECTED (s5 permuter). The ONLY weighted-0 (byte-match) form the s5 campaign
 * produced from the double-reuse score-9 seed: the clean double-reuse chassis
 * PLUS `base++; base--;` — a dead no-op pair.
 *
 * WHY IT REACHES 0: `base++; base--;` adds TWO extra references to `base`,
 * raising its allocation priority above a2local so base wins $a1 and a2local
 * takes $a2 — flipping the exact $a1<->$a2 tiebreak that is the score-9
 * residual (greg-confirmed: base=pseudo77 has 2 refs, a2local=pseudo73 has 5;
 * base must out-ref a2local to get colored first). The dead ++/-- supply the
 * missing refs.
 *
 * WHY REJECTED: `base++; base--;` is a dead-computation coercion with no
 * semantic purpose — a forbidden cheat by any spelling (no-new-park-categories:
 * "necessary only because removing it raises the sandbox score"). This is the
 * SAME dead-op the s4 permuter found on the base-reuse chassis
 * (rejected/base-reuse-reload-reaches43-tail-ra-swap-residual.c); s5 reconfirms
 * it is the permuter's only route to 0 on the strictly-better double-reuse
 * chassis too. The weighted-10 permuter forms (output-10-1/-2) are ALSO cheats:
 * they overwrite `base = <arm load>` inside an if/else arm (corrupting
 * base=a3*40 before the tail `base += var_v0`), which is semantically divergent
 * from target (target computes each arm into a fresh $v0, never touching base).
 * No CLEAN permuter form beat sandbox 9. */
s32 ang_hosei_80056FE8(s32 arg0) {
    s32 a2 = *((s32 *) arg0);
    s32 a3 = *((u8 *) ((*((s32 *) (a2 + 0x58))) + 3));
    s32 base = a3 * 40;
    s32 var_v0;
    if ((*((u8 *) (a2 + 0xA3))) != 0xFF) {
        if ((var_v0 = *((s16 *) (arg0 + 0x5E))) == 0) {
            var_v0 = (*((u8 *) (((s32) (&D_8009A830)) + (*((s16 *) (a2 + 0xE)))))) * 2;
        } else {
            var_v0 = (*((s8 *) (((s32) (&D_8009A838)) + (*((s16 *) (a2 + 0xE)))))) * 8;
        }
    } else {
        var_v0 = (*((u8 *) (((s32) (&D_8009A840)) + (*((s16 *) (a2 + 0x14)))))) * 2;
    }
    {
        base += var_v0;
        var_v0 = *((s32 *) arg0);
        base++;   /* CHEAT: dead-op pair, supplies base's missing 2 refs */
        base--;
        return (base + (*((s16 *) (var_v0 + 0x40A)))) + 0x12C;
    }
}
