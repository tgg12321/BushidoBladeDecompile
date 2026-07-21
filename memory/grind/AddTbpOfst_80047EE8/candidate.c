/* AddTbpOfst_80047EE8 (text1b.c) — s1 candidate, sandbox --disable all = 10.
 *
 * Instruction stream is 53/53 and IDENTICAL to target except the 10
 * frame-offset insns (2x addiu sp: -0x28 vs target -0x48; 8x save/restore
 * offsets shifted by the 32-byte unused vars region). Same terminal species
 * as sibling InitHiraRmd_80047FBC (owner-escalation filed 2026-07-20,
 * awaiting ruling).
 *
 * Levers that produced this (s1, 2026-07-21):
 *  1. SINGLE-WALKER: one pointer variable `p` carries arg0-copy -> add ->
 *     reload -> loop walker (was split cached/p in the old candidate;
 *     merging them: 16 -> 14).
 *  2. LIVE precompute of the call's first arg into `first` inside the loop
 *     (sibling InitHiraRmd's committed lever 1). This flips whole-function
 *     RA so GCC stages arg0 through $s0 (sw s0; move s0,a0; sw s2;
 *     move s2,s0; addu s0,s0,a1) exactly as target: 14 -> 11, 53/53 insns.
 *     `first` is live code (consumed by the call) — not a FAKE construct.
 *  3. FAKE-annotated arg0 = 0 dead store (dead-store-fake-exception family,
 *     sanctioned 2026-07-01) breaks cse2's {arg0,p,saved} canonical-reg
 *     class so the second pointer binds addu s0,s2,v0 (not a0): 11 -> 10.
 *     Same lever the sibling's s6 Judge-PASSed. PREREQUISITE NOT YET
 *     DISCHARGED on this function: pure-C alternatives for THIS residual
 *     must be measured dead here (sibling killed them s2-s9; transfer
 *     argued but unmeasured) before this construct can ship.
 *
 * Remaining gap: the 32-byte unused frame. Sibling s7 evidence (identical
 * species): target has ZERO stores in the vars region, so the written-array
 * carve-out byte-diverges; phantom-frame-slots probes dead; unwritten array
 * forbidden. No sanctioned lever known.
 */
void AddTbpOfst_80047EE8(s32 arg0, s32 arg1)
{
    u32 *p;
    s32 saved;
    s16 new_var;
    s32 count;
    u32 v_off;
    unsigned int new_var2;
    p = (u32 *) arg0;
    saved = (s32) p;
    arg0 = 0; /* FAKE */
    p = (u32 *) ((s32) p + (((s32) (arg1 << 16)) >> 14));
    v_off = *p;
    p = (u32 *) (saved + ((v_off >> 2) << 2));
    count = *(p++);
    if (count != 0)
    {
        count--;
        do
        {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            s32 first;
            word = *p;
            p = (u32 *) (((s32) p) + 4);
            a1v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a2v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a3v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            new_var = a1v;
            new_var2 = word >> 2;
            first = saved + (new_var2 << 2);
            v0v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            efc_buki_draw_zanzou(first, new_var, a2v, a3v, v0v);
        }
        while ((count--) != 0);
    }
}
