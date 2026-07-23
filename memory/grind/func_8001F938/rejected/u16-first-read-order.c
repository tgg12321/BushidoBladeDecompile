/* REJECTED (disproven, not a cheat) — session s1 recon.
 * Hypothesis: reading the +0x270 field as u16 FIRST (unconditionally), then s16,
 * might force GCC to keep two loads (since zero->sign derivation costs 2 insns
 * sll;sra vs a 1-insn reload, GCC might reload -> second load).
 * MEASURED: floor 10 (WORSE than the s16-first guarded form). GCC emitted a
 * SINGLE `lhu $v0,0x270` and sign-extended in-register via `sll $v0,$v0,16;
 * ...; slti` for the compare — never a second load. Confirms GCC 2.7.2 CSE keeps
 * exactly one same-address load regardless of read order; the derivation cost
 * asymmetry does NOT trigger a reload here.
 *
 * Only the changed ternary block shown (kind-split applied elsewhere):
 */
    {
        s32 rawu = *((u16 *)(arg0 + 0x270));
        s32 probe = *((s16 *)(arg0 + 0x270));
        s32 raw_or_3 = (probe < 4) ? rawu : 3;
        idx = ((raw_or_3 << 16) >> 15);
    }
