/* REJECTED form — case 34 with D_80099478.variant = 1; BEFORE the s4 load,
 * under the [s6] struct-merge probe (StageState at 0x80099478).
 *
 * Measured 2026-08-25 [s6]: sandbox --disable all = 21 (248/248).
 * Why dead: once the variant store is a /s MEM (COMPONENT_REF), sched.c
 * true_dependence (sched.c:831-839) makes the case-34 `lw v0,0x14(s0)` (a /s
 * varying-address read, s0[5]) conflict with the PRECEDING fixed-address /s
 * store — both exemption clauses need the OTHER ref non-struct — so the load
 * can no longer hoist above the store and ours emits li/sh BEFORE the lw
 * (target: lw first, li in its delay slot, sh after). The mis-order also
 * cascaded into a whole-function C-s2/C-s3 callee-saved seat swap.
 * Fix measured same session: swap the two statements (s4 = ... load/ALIGN4
 * first, variant store LAST) -> 21 dropped to 4 and the seat swap resolved.
 * Store-LAST is the required C order for case 34 under any /s-store object
 * model (mirrors target's lw/li/sh order literally).
 */
    case 34:
        s1 = s2;
        D_80099478.variant = 1;   /* store-first: REJECTED, forces lw after sh */
        s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[5]));
        break;
