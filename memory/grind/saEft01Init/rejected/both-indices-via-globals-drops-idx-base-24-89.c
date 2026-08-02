/* REJECTED (grind s17, escalation modality) — 24 / 89 against the 7 / 91 floor.
 *
 * The completion of the index-global axis: BOTH index reads respelled through
 * the two separate byte globals D_800A1494 / D_800A1495, so the hoisted base
 * pointer `u8 *idx_1494` disappears from the function entirely.
 *
 * RESULT: 24 / 89.  The build is now TWO instructions SHORT of target's 91 —
 * the `lui`/`addiu` pair that materialises the index base in the prologue is
 * gone, which is direct positive evidence that target DOES hoist that base
 * (i.e. the original source did hold the two indices in one array/pointer,
 * not in two independent scalars).  The callee-save map rotates with it.
 *
 * This is the exact shape s1's H1 measured for the TABLE bases (85 insns vs
 * 91) reproduced for the INDEX base.  Both halves of the global-spelling axis
 * are now closed by measurement; the three hoisted bases are structurally
 * required by target's instruction count, not merely score-preferred.
 *
 * DO NOT RE-PROPOSE.
 */
        arg4 = tbl_125c[D_800A1494];
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4,
                     tbl_125c[D_800A1495]);
