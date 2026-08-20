/* REJECTED s3 — F4 detour variants that mis-place the mflo1 bump. Two
 * measured spellings, both 109 insns (detour folds byte-neutrally) but
 * score 23:
 *
 * (1) multi-set dxs detour:
 *       s32 dxs = dx*dx;  dxs = dxs + (s32)ent;  dxs = dxs - (s32)ent;
 *       dist_sq = (u32)(dxs + dy*dy + dz*dz);
 *     ALLOCDBG: dxs (p114, 6 refs, pri 9230) -> hardreg 65 (LO!). The
 *     mflo-sourced pseudo carries an LO copy-preference; at pri 9230 it
 *     allocates before the other mult temps, LO is still free, find_reg's
 *     copy-preference pass (global.c:1097-1104) hands it LO. Cascade:
 *     ent->$a1, walker->$a2, t0->$a3, t1->$t0. (ent's own bump to 8 refs /
 *     3750 DID leapfrog the walker as predicted.)
 *
 * (2) reversed detour placed directly after the dist_sq sum:
 *       ent = (u8*)((s32)ent + dxs);  ent = (u8*)((s32)ent - dxs);
 *     dxs single-set, 4 refs, len 11 -> pri 7272: STILL above the dys
 *     mult-temp (6666), so dxs again beats it to LO -> hardreg 65 -> score 23.
 *
 * THE RULE EXTRACTED (the s3 win): the bumped mflo1 pseudo's priority must
 * land in the (4687, 6666) window — BELOW the dys mult-temp so that temp
 * claims LO first (their ranges conflict, blocking LO for dxs), and above
 * bumped-ent. Arm-END placement inside `if (dist_sq < 0x400)` gives
 * len 14 -> pri 5714 -> $a1. That form measured SCORE 0 (see candidate.c).
 */
