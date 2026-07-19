/* REJECTED s12 — F2 header-type-correction on arg0 as ArenaHdr_57CC8 struct.
 * Sandbox --disable all: score=3 (unchanged from candidate baseline).
 * target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395.
 *
 * KILLED by measurement: the type-level change from `u8 *arg0` + byte-offset
 * casts to `ArenaHdr_57CC8 *arg0` + member access is fold-equivalent under
 * GCC 2.7.2 combine.c. The struct member access `arg0->table` compiles to
 * the identical `lw $rD, 0x4(arg0)` as the byte-cast `*(s16**)(arg0+4)`;
 * `arg0->count` compiles to identical `lbu $rD, 0x3(arg0)` as `arg0[3]`.
 * combine.c sees the same addsi3 SET_SRC/SET_DEST structure at insn 124's
 * p2 addu regardless of the source-level type of arg0. Pseudo 86's copy-pref
 * {3} still originates from pseudo 129's v1 propagation.
 *
 * Per [[header-type-correction-from-use-sites]] four-prong bar:
 *  (a) grep-consistent use sites: only ONE C use site (this function itself);
 *      callers are asm — trivially unfalsifiable, does NOT constitute evidence.
 *  (b) OLD type required compensating casts: yes (u8* -> s16** at offset 4).
 *  (c) fix is one extern edit: N/A — struct declared locally in text1b.c.
 *  (d) casts eliminated at every use site: yes in this variant.
 * The type change is byte-neutral at codegen, so even if the four-prong were
 * met the axis provides no measurement gradient toward closure.
 */
typedef struct { u8 flags; u8 _p1; u8 scale_units; u8 count; s16 *table; } ArenaHdr_57CC8;
void func_80057CC8(ArenaHdr_57CC8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s16 *p;
    u16 cy;
    s16 *table;

    prev_idx = arg1 - 1;
    table = arg0->table;
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0->count - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0->count) {
            next_idx = 0;
        }
    }

    p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)arg0->table);
    ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0->scale_units * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
