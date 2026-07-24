/* REJECTED (s2) — two-object epilogue: cols b,c via a 119C-anchored pointer,
 * col a via the 1198 symbol expression. Score 2 (build 38), does NOT close.
 *
 *   ofs = i*12;   (reused ofs -> index in v1, s2 register-flip lever)
 *   p = (s32 *)((u8 *)&D_800F119C + ofs);   // "data pair" object base at 119C
 *   p[1] = 0;                                // col c (11A0) -> 4(v0)
 *   p[0] = 0;                                // col b (119C) -> 0(v0)
 *   *(s32 *)((u8 *)&D_800F1198 + ofs) = 0;   // col a (1198, the flag)
 *
 * WHY IT MATTERS (kills frontier hyp #1 — the semantic flag/data object split):
 *   This realizes "col a = flag view (1198 symbol+index); cols b,c = data view
 *   (row/base pointer)". col a's store now MATCHES target byte-for-byte
 *   (lui at,%hi(1198); addu at,at,v1; sw %lo(1198)(at)). The ONLY residual 2
 *   diffs are the b,c pointer ANCHOR: this form anchors b,c at 119C (v0=&119C+idx,
 *   disp 0/4); target anchors b,c at 1198 (v0=&1198+idx, disp 4/8 — see
 *   asm/funcs/func_80062020.s .L80062084). Same addresses, different base+disp.
 *
 * WHY IT CANNOT CLOSE LEGITIMATELY:
 *   To make the b,c pointer base &1198 (target), you must write p=&1198+ofs and
 *   use p[1],p[2] for b,c. But then p[0] == col a's address, and matching col a's
 *   SEPARATE %hi/%lo recompute requires spelling col a as *(&1198+ofs) apart from
 *   p -> the SAME-LVALUE dual-spelling (see epilogue-dual-spelling-col0.c), a
 *   codegen steer. Target's b,c "data view" is anchored at the FLAG's address
 *   (1198), which proves b,c are NOT a separate object from col a; the layout is
 *   one interleaved struct-row {flag@1198, b@119C, c@11A0} stride 12. So no
 *   genuine object separation reproduces target's partial CSE.
 *
 * BORDERLINE-CHEAT NOTE: even at score 2 this form's 119C anchor has no semantic
 * purpose under the true single-row model (a human would anchor at the row base
 * 1198); the anchor choice is codegen-motivated (fails 6-test #3/#4). Not adopted
 * as the floor. The clean, unambiguous floor is the single-object pointer form
 * (candidate.c, score 4).
 */
