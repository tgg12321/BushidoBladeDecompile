/* func_80060A68 candidate — session s3 (2026-08-19, structural). HONEST SANDBOX
 * SCORE 0 (build 66 / target 66) measured THIS session with this body applied
 * over src/text1b.c:3321.  `sandbox func_80060A68 --disable all` => 0.
 *
 * WHAT CLOSED IT (the s3 mechanism finding).  s1/s2 had the residual down to a
 * single adjacent swap: our slots 10/11/12 were `lw $2,0xC($3); lw $5,0x10($3);
 * lw $4,0xC($3)` where target has `lw $2,0xC($3); lw $4,0xC($3); lw $5,0x10($3)`.
 * s2 localised the decision to rank_for_schedule but attacked it from the wrong
 * side.  Reading tools/gcc-2.7.2/sched.c:2500-2600 against a fresh -da dump shows
 * the deciding line is NOT the LUID fall-through but `birthing_insn_p`
 * (sched.c:2504-2535), called from `adjust_priority` (sched.c:2540-2592) the
 * moment an insn becomes ready:
 *
 *     if (bb_live_regs[offset] & bit)  return (reg_n_sets[i] == 1);
 *
 * An insn whose destination pseudo has EXACTLY ONE SET, and which kills no
 * register, gets its INSN_PRIORITY raised to `max_priority` — which at that point
 * is the LAUNCH_PRIORITY (0x7f000001, sched.c:187) temporarily held by the insn
 * being scheduled.  GCC 2.7.2 schedules each block BACKWARD, so "picked early"
 * means "emitted late".  Concretely, in the floor-2 body:
 *   - copy2's address load (insn 35) set a fresh single-set pseudo  -> bumped to
 *     0x7f000001 -> picked at T-45 -> emitted LAST of the group (slot 12);
 *   - the staged 0x10 load (insn 25) set the multiply-assigned `temp_a1` ->
 *     reg_n_sets == 2 -> no bump -> sat in the ready list at honest priority 3
 *     for 18 cycles -> picked at T-47 only when alone -> emitted FIRST (slot 11).
 * That is the whole residual.  The fix is to take the bump AWAY from copy1's and
 * copy2's address loads as well, by giving those destinations more than one set,
 * and to place the staged 0x10 read AFTER copy2 so that when the three loads are
 * finally all at honest priority the LUID tie-break (sched.c:2464) orders them
 * the way target wants.  Measured ladder this session: v40 (copy2/copy3 share one
 * scratch, stage after copy2) fixed slot 12 and left 10/11 swapped; v42/v45/v47
 * (copy1 additionally staged through a multiply-set local) => 0.
 *
 * CONSTRUCTS (all three are one family — a real, immediately-read value staged
 * through a multiply-assigned local; zero dead code, no pins, no asm, no
 * volatile): `result` carries copy1's source pointer and then the loaded word;
 * `src` carries copies 2 and 3's source pointer; `temp_a1` carries the 0x10
 * pointer and then the halfword read through it.  Each is FAKE-annotated in
 * place with the sched.c mechanism.  See memory/grind/func_80060A68/self_vet.md
 * for the 6-test vet and the sanctioned-family citations.
 *
 * INTEGRATION (BLOCKING, operator/driver surface — do not skip).  asmfix.txt:109
 * and asmfix.txt:110 splice this function's ENTIRE body in from rule text
 * (delete_between anchored on "^lhu\t\$4,0\(\$3\)$" + a 43-instruction
 * insert_before).  With this C body the first body instruction is `lhu $2,0($3)`,
 * so the delete_between anchor no longer matches and the splice mis-fires,
 * duplicating the body in a full build.  Both rules MUST be retired in the same
 * change as this C (`engine retire func_80060A68`, then `verify-oracle`).  The
 * grind session may not touch asmfix.txt, so this is handed off.
 */
void func_80060A68(void) {
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    s32 outer;
    s32 idx;
    u16 temp2;
    s32 temp_a1;
    s32 result;
    s32 src;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    /* FAKE: copy1's source pointer, and then the word it points at, are staged
       through the function's existing `result` local (its previous value is dead
       here — nothing reads `result` until the dispatch call below overwrites it),
       mechanism: GCC 2.7.2 sched.c adjust_priority -> birthing_insn_p, whose
       LAUNCH_PRIORITY bump is gated on reg_n_sets[regno] == 1, so a fresh
       single-set destination is picked early in the backward list schedule and
       therefore EMITTED late; a multiply-set destination keeps its honest
       priority, lever-exhaustion: memory/grind/func_80060A68/evidence.md
       (s1 v3-v13, s2 v20-v33, s3 v40-v46) */
    result = *(s32 *)(outer + 0xC);
    result = *(s32 *)(result + 0);
    *(s32 *)(outer + 0x20) = result;
    /* FAKE: copies 2 and 3 share one `src` pointer scratch rather than re-reading
       the 0xC field inline, so that copy2's address load also loses the
       birthing_insn_p LAUNCH_PRIORITY bump (same sched.c mechanism as above),
       lever-exhaustion: memory/grind/func_80060A68/evidence.md s2 K7 + s3 v40/v42 */
    src = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x24) = *(s32 *)(src + 4);
    /* FAKE: the 0x10 pointer is staged through temp_a1, which its own next read
       (`temp_a1 = *(u16 *)(temp_a1 + 4)`) consumes and overwrites; same reg_n_sets
       mechanism, and the statement sits AFTER copy2 so that its RTL LUID is above
       copy2's address load, lever-exhaustion: evidence.md s1 K2 / s2 K10 / s3 */
    temp_a1 = *(s32 *)(outer + 0x10);
    src = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x28) = *(s32 *)(src + 8);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(temp_a1 + 4);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
