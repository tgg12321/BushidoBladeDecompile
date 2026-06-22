/* D_80083418 — pure-C candidate at sandbox-floor 7 (HEAD 13, prior WIP 13).
 *
 * Reduces the honest pure-C distance from 13 (HEAD) → 7 by THREE structural
 * levers, all legitimate (no dead stores, no register-asm pins, no inline asm,
 * no alias renames):
 *
 * (1) INIT ORDER `mask, base, one`.
 *     The 4 regfix `subst $20<->$21 @ 15,18,20,27` rules encode a coupled
 *     register-rotation: target wants one→$s4, mask_const→$s3, base→$s5;
 *     HEAD's source `one = 1; mask_const = ...; base = ...;` produced
 *     one→$s5, base→$s4 (swap). The allocator priority is
 *     `(log2(n_refs)*n_refs)/live_length` (global.c:611) — `base` lives
 *     shorter than `one`, so base wins higher priority and grabs $s4 first.
 *     Moving the `mask_const`/`base` assignments BEFORE `one = 1;` shifts the
 *     scheduler's emission so `one` lands AFTER mask+base in the prologue,
 *     and the global allocator's processing order flips to `mask_const, one,
 *     base` (mask→$s3, one→$s4, base→$s5 — matches target). This retires
 *     all 4 `$20<->$21` substs in pure C.
 *
 * (2) OUTER STRUCTURE `i = 0; if (bits == 0) goto reload; tbl = base;`.
 *     HEAD had `if (bits == 0) goto reload; i = 0; tbl = base;` which lets
 *     GCC see the redundant bits==0 check (since reload re-tested bits!=0)
 *     and fold the branch (96 vs target 97 insns; the `insert beq $17,$0 @ 19`
 *     regfix rule existed to inject the missing branch). Moving `i = 0;`
 *     BEFORE the branch makes GCC keep the check (97 insns matches target)
 *     AND places i=0 in the branch's delay slot in target's exact shape:
 *       beqz s1, .reload
 *       move s0, zero          (delay)
 *       move s2, s5             (tbl=base)
 *     This retires the `insert beq @ 19` regfix.
 *
 * (3) ERROR-LOOP ARRAY INDEX `p[i << 2]`.
 *     HEAD's `*(s32 *)((u8 *)p + (i << 4))` (offset = i*16 bytes) makes GCC
 *     emit `lui p; lw p; sll v1, i, 4; addu v0, v0, v1; lw a2, 0(v0)` — the
 *     addu's destination is v0 (target wants v1). Rewriting as the natural
 *     `s32`-array index `p[i << 2]` (i*sizeof(s32)*4 = i*16) lets GCC see
 *     the access as a stride-4 element index and flip the addu's operand
 *     attribution so the sum lands in v1. Standard C, more idiomatic.
 *
 * REMAINING GAP (floor 7):
 *   - INIT EMISSION ORDER (2 Levenshtein-ops): scheduler emits `mask, base,
 *     one` while target emits `one, mask, base`. All 3 produce identical
 *     bytes IN the right registers — only the issue order differs. Sched1's
 *     ready-queue picks the chained lui+ori/lui+addiu (longer critical path)
 *     before the standalone `addiu $s4, 0, 1`. Lever-attempts to flip:
 *       - decl-order swaps (one_first, etc.) — all unchanged (RA is intrinsic)
 *       - `mask_via_one = (one << 24) - 1` — broke RA, score 14
 *       - inline_base_reread (drop `base` var) — broke LICM, score 15
 *       - `register` qualifier on one/val — no effect
 *     The longest-critical-path scheduler heuristic appears to be the blocker.
 *
 *   - DONE-BLOCK v0/v1 SWAP (5 substs in build/.o): val gets $v1, target wants
 *     $v0. RTL shows pseudo 94 (val, /v global) conflicts with hardreg 2
 *     because local-alloc gives $v0 to pseudo 96 (const 0xFF000000) FIRST
 *     during val's live range. Lever-attempts:
 *       - split val into first_val/second_val locals (so they're local-alloc'd):
 *         floor 18 (95 insns) — scheduler fuses load-delay nops away
 *       - inline `*ctrl` (drop ctrl local): no change
 *       - rewrite check as ~val, xor, subtract: changes insn count
 *       - `register` qualifier: no effect
 *       - separate scope for the volatile read: no change or worse
 *     The const pseudo's local-alloc grab of $v0 is the blocker. Could be
 *     pursued via more aggressive RA levers or instrumented cc1 dumps
 *     (BB2_ALLOC_DEBUG, PRIODBG) — see [[register-alloc-deep-dive]].
 *
 * Pre-edit head-vs-candidate diff snapshot (10 raw differing positions):
 *   - 5 init order positions (16-20) → Levenshtein 2 ops (insert+delete)
 *   - 3 done-block v0/v1 positions (58, 59, 64, 66) + 1 operand reorder (60)
 *     → 5 substs
 *   Total: 7.
 *
 * Oracle still SHA1==expected after these edits (12 regfix rules continue to
 * cover the residual). The candidate is ready to apply as a pure-C reduction
 * of the cheat-honest distance from 13 → 7. Next agent should attack the
 * remaining 7 — likely via the v0/v1 swap (5 ops, biggest single block).
 */
void D_80083418(void) {
    s32 i;
    u32 bits;
    s32 *tbl;
    s32 *base;
    s32 one;
    u32 mask_const;

    bits = ((u32)*(volatile s32 *)D_800A263C >> 24) & 0x7F;
    if (bits == 0) goto done;

    mask_const = 0x00FFFFFF;
    base = D_800A2640;
    one = 1;

outer:
    i = 0;
    if (bits == 0) goto reload;
    tbl = base;
inner:
    if (i >= 7) goto reload;
    if (bits & 1) {
        s32 *ctrl = D_800A263C;
        s32 val = *ctrl;
        val &= (one << (i + 24)) | mask_const;
        *ctrl = val;

        if (*tbl != 0) {
            ((void (*)(void))*tbl)();
        }
    }
    tbl++;
    bits >>= 1;
    i++;
    if (bits != 0) goto inner;
reload:
    bits = ((u32)*(volatile s32 *)D_800A263C >> 24) & 0x7F;
    if (bits != 0) goto outer;

done:
    {
        s32 *ctrl = D_800A263C;
        s32 val = *ctrl;
        if ((val & (s32)0xFF000000u) == (s32)0x80000000u) goto error;
        val = *(volatile s32 *)ctrl;
        if (!(val & 0x8000)) return;
    error:
        debug_printf(&D_80016394, *ctrl);
        i = 0;
        do {
            s32 *p = D_800A2660;
            debug_printf(&D_800163B0, i, p[i << 2]);
            i++;
        } while (i < 7);
    }
}
