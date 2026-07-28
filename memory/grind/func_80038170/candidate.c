/* Candidate: func_80038170 (code6cac_c_mid.c) — post-Judge-ruling session (2026-07-28)
 * s3 (structural, 2026-07-28): src at HEAD had again reverted to the cheat form;
 * this candidate was re-applied verbatim and re-measured: sandbox --disable all
 * = 1 (141/141, rules_dropped 1) — identical to s2. New exhaustion probe: the
 * one-symbol pair rebased on D_8008F19D ([s3*2-1]/[s3*2]) also scores 1 (addend
 * artifact moves to the other word; banked rejected/d8008f19d-rebased-pair.c) —
 * NO source spelling reaches sandbox 0 pre-integration. OWNER-ESCALATION filed
 * (docs/grind/decisions.md 2026-07-28 func_80038170 entry): the driver candidate
 * gate (sandbox 0 BEFORE retire) is circular for this function; owner must
 * retire regfix.txt:1250 + the prologue_config.json entry, rebuild, queue done.
 * s2 (structural, 2026-07-28): src at HEAD had REVERTED to the old cheat form
 * (pins + dummy-asm + two-symbol spelling); this candidate was re-applied
 * verbatim to src and re-measured: sandbox --disable all = 1 (141/141), word
 * diff vs the oracle stream = jal reloc placeholder only, normalized diff vs
 * build/ reference = the 2 linker-identical reloc-spelling words + 4 j-words
 * that are pure section-offset artifacts of the stale reference (sandbox
 * strips 29 cheat-asm instances file-wide, shifting all offsets 4 bytes).
 * Artifacts: tmp/grind/func_80038170/s2/. Form UNCHANGED from the Judge-
 * sanctioned spelling below; layer-1 reviewer PASS carried over.
 *
 * IN PLACE in src/code6cac_c_mid.c. Engine sandbox floor: 1 (stale-reference
 * reloc artifact ONLY — see below). TRUE byte distance to oracle: 0 (proven at
 * word level: 141/141 insns match the oracle stream; the sole .o-text diff is
 * the linker-identical reloc spelling D_8008F19C+1 vs D_8008F19D, hi 0x8009 /
 * lo 0xF19D both ways).
 *
 * Form = the Judge-sanctioned spelling (BINDING ruling): decl `s32 s1, s2, s3;`
 * unchanged, separate statements `s3 = 0; s2 = 0; s1 = 0;`. Layer-1 reviewer:
 * PASS on the whole body.
 *
 * NEW LEVER THIS SESSION (required — the Judge spelling alone does NOT reach
 * byte parity, a fact no prior session had measured): the loop-counter init
 * must be a standalone `i = 0;` BEFORE `mask = D_80106A50;` (for-init clause
 * empty). With `for (i = 0; ...)` the scheduler emits `move a3,zero` AFTER the
 * li a0,1/lui/lw mask cluster and displaces `sw ra` — 5 words off target
 * (engine masked metric hides this as score 1; raw word diff exposes it).
 * With the standalone init the stream matches target exactly. Layer-1
 * cheat-reviewer PASS: ordinary live-statement order, same accepted family as
 * store-before-jal / hoist-call-arg-local levers.
 *
 * Phantom +8 frame stays closed by the one-table pair
 * (&D_8008F19C)[s3*2+0]/[+1] (compiler stack temp, phantom-frame-slots-gcc272;
 * reviewer PASS prior session). Two-symbol spelling would kill the temp — the
 * reloc artifact is inherent to the correct source and vanishes when build/
 * regenerates from this src.
 *
 * INTEGRATION (driver surface, forbidden to grind sessions): retire
 * regfix.txt:1250 reorder @9-13 + tools/prologue_config.json func_80038170
 * entry — BOTH now actively MANGLE the correct natural output (rules-applied
 * sandbox score 4 vs clean 1, measured this session; a full build with them
 * active would break the oracle). Then rebuild, sandbox reads 0, SHA1==oracle,
 * FINAL CALL.
 */

void func_80038170(u8 *out) {
    s32 s1, s2, s3;
    s32 i;
    s32 mask;
    s32 bit;

    s3 = 0;
    s2 = 0;
    s1 = 0;
    i = 0;
    mask = D_80106A50;

    for (; i < 0x1B; i++) {
        bit = 1 << i;
        if (mask & bit) {
            s32 v = (&D_8008F204)[i];
            switch (v) {
                case 0: s1++; break;
                case 1: s2++; break;
                case 2: s3++; break;
            }
        }
    }

    i = 0x3F;
    out[0] = 0x53;
    out[1] = 0x43;
    out[2] = 0x11;
    out[3] = 0x01;

    {
        u8 *p = out + 0x3F;
        do {
            p[4] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    func_80079194(out + 4, &D_8008F1C0);

    out[0x22] = (&D_8008F1A8)[s1 * 2 + 0];
    out[0x23] = (&D_8008F1A8)[s1 * 2 + 1];
    out[0x3C] = (&D_8008F1A8)[s2 * 2 + 0];
    out[0x3D] = (&D_8008F1A8)[s2 * 2 + 1];

    if (s3 > 0) {
        out[0x40] = D_800A3200;
        out[0x41] = D_800A3201;
        out[0x42] = (&D_8008F19C)[s3 * 2 + 0];
        out[0x43] = (&D_8008F19C)[s3 * 2 + 1];
    }

    i = 0x1B;
    {
        u8 *p = out + 0x1B;
        do {
            p[0x44] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    i = 0;
    {
        u16 *src = (u16 *)&D_800109EC;
        u8 *dst = out;
        do {
            *(u16 *)(dst + 0x60) = *src;
            src++;
            i++;
            dst += 2;
        } while (i < 0x10);
    }

    i = 0;
    {
        u8 *outer_src = (u8 *)&D_80010A2C;
        u8 *outer_dst = out;
        do {
            s32 j = 0;
            u16 *dst = (u16 *)(outer_dst + 0x80);
            u16 *src = (u16 *)outer_src;
            do {
                *dst = *src;
                src++;
                j++;
                dst++;
            } while (j < 0x40);
            outer_src += 0x80;
            i++;
            outer_dst += 0x80;
        } while (i <= 0);
    }
}
