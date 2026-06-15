/* func_80045878 (text1a_c.c) — WIP NOTE. No floor reduction yet; keep HEAD body.
 *
 * HEAD matches via 10 regfix rules: 1 lost-codegen insert_after
 * "addiu $19,$18,3" @30 (a SECOND `s3=a0+3` GCC folded away) + a 9-rule tail
 * subst cluster that rewrites the final store block to target's
 * "v0=s1 base copy + v1 scratch" form.
 *
 * Two coupled gaps toward COMPLETED-C:
 *   A. lost-codegen: target recomputes `addiu $s3,$s2,3` (= a0+3) TWICE; the
 *      source's `s3=a0+3` (top) + `s3 = a0 - -3` (else) fold to one. build is
 *      1 insn SHORT (107 vs target 108).
 *   B. tail store cluster: target copies `s1`->$v0 (addu v0,s1,zero) and does
 *      ALL final stores through $v0, with $v1 holding a0+3 then 0x8000:
 *        addu  v0,s1,zero
 *        addiu v1,s2,3 ; sh v1,0x16(v0)       (s1[11]=a0+3)
 *        ori   v1,zero,0x8000
 *        sh s2,4(v0); sh s5,8(v0); sh s2,0x14(v0); sh s2,0x10(v0)
 *        sw v1,0x18(v0)
 *      A single-set `s16 *p = s1;` alias does NOT materialize the copy (GCC
 *      copy-props it; s1 isn't used after) — sandbox stayed 10 / 107 insns.
 */
