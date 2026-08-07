/* REJECTED (grind s17, escalation modality) — 14 / 92 against the 7 / 91 floor.
 *
 * AXIS: the INDEX globals, not the table globals.  Every prior global-spelling
 * probe in this ledger (s1 H1 "inline-globals-kills-hoisted-base-pointers",
 * s13 "permuter-global-arg5-rotates-callee-save-map-17-92") respelled the
 * TABLE reference D_800A125C.  This form leaves both table bases hoisted and
 * respells only the SECOND INDEX: `tbl_125c[idx_1494[1]]` becomes
 * `tbl_125c[D_800A1495]`, using the separate byte global that src/system.c
 * already declares (`extern u8 D_800A1495;`, line 778) and that this function
 * has never referenced.  This is a genuinely plausible ORIGINAL spelling — two
 * adjacent byte globals rather than one two-element array — so it was worth a
 * measurement even though the ledger warned the axis looked closed.
 *
 * RESULT: 14 / 92.  The second reference to the array base is what gives
 * idx_1494 its 2 in-loop refs (5 after the do{}while(0) loop_depth weighting);
 * dropping it to a single ref collapses that allocno's priority, and the
 * separate %hi/%lo chain for D_800A1495 adds an instruction.  Same failure
 * shape as s13's table-global probe, one register earlier in the chain.
 *
 * DO NOT RE-PROPOSE.  The index-global axis is now measured on both halves
 * (see the sibling rejection file for the both-indices form at 24 / 89).
 */
        arg4 = tbl_125c[idx_1494[0]];
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4,
                     tbl_125c[D_800A1495]);
