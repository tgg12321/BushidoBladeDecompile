/* REJECTED (s3, structural) — the idx25 addu operand order is NOT reachable by
 * ANY structural lever (declaration order, block-local split, fresh-local copy,
 * statement re-association, type narrowing).
 *
 * PROOF (cc1 -da RTL dump, tmp/grind/cpu_check_tubazeri_2/s3/full.i.{rtl,combine,greg}):
 *   The pointer add `s2 = (u8*)a0 + v0` is generated at initial RTL as
 *     (insn 77 (set (reg 77) (plus (reg 72 = a0) (reg 75 = v0=s1<<1))))
 *   i.e. base(a0)-FIRST. This spelled order is carried UNCHANGED through
 *   jump -> cse -> loop -> combine -> greg to the final asm:
 *     .greg: (insn 77 (set (reg s2) (plus (reg s0=a0) (reg v0))))  -> addu s2,s0,v0
 *   Combine does NOT canonicalize commutative (plus reg reg) by pseudo regno:
 *   reg72(a0) < reg75(v0) yet a0 STILL emits first. The order is fixed at
 *   tree-lowering (c-typeck pointer_int_sum builds PLUS_EXPR(ptr,int) = base
 *   first), BEFORE register allocation / scheduling / declaration order can act.
 *   Target wants (plus v0 a0) = index-first (addu s2,v0,s0).
 *
 * MEASURED base-first (score 1) this session, in addition to s1/s2's 5 pointer
 * spellings + inline form:
 *   - fresh-local copy of a0 consumed only by the add, materialized after the
 *     shift (the form below)                          -> score 1
 *   - inline shift with no named v0: (u8*)a0 + (s1<<1) -> score 1
 *
 * The ONLY distance-0 form is integer-domain `v0 + (s32)a0` (offset-first,
 * source-order-preserved) — reviewer-FAILED as a commutative-operand-order
 * coercion (rejected/int-cast-operand-swap.c). Structural axis is DEAD.
 * Residual = 1 insn, operand-order-only => endgame-lock candidate.
 */
do_sll:
    v0 = s1 << 1;
    {
        s32 *base = a0;
        s2 = (s32 *)((u8 *)base + v0);   /* still (plus base v0) base-first */
    }
    s3 = *(s16 *)((u8 *)s2 + 0x332);
