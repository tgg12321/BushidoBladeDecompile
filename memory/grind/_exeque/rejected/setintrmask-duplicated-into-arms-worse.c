/* _exeque — s6 (structural) rejected form.
 * Restructured the post-loop `SetIntrMask(D_8009BF84);` (currently
 * unconditional, before the final guard `if`) by duplicating it into BOTH
 * arms of the final guard if/else instead — moving it inside the `if`
 * block (right before the inner guard) and adding an `else` arm that
 * calls it alone:
 *
 *   if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
 *       s32 *p = &D_8009BE7C;
 *       SetIntrMask(D_8009BF84);
 *       if (*p != 0 && D_8009BE80 != 0) { *p = 0; callback(); }
 *   } else {
 *       SetIntrMask(D_8009BF84);
 *   }
 *
 * Tried as a candidate application of the sanctioned
 * duplicated-statement-into-arms family (moving the call adjacent to the
 * final block to see if it changes the local scheduling that produces the
 * jalr-delay-slot residual). On the s4/s5 floor-2/187 do-while(0)-wrapped
 * chassis (candidate.c unchanged otherwise).
 *
 * Measured: sandbox _exeque --disable all score 2 -> 18 (MUCH WORSE — the
 * worst of the three structural probes this session). Moving the
 * SetIntrMask call disturbs a much larger region than the target
 * jalr-delay-slot residual; not a local, isolated effect. Reverted; not
 * adopted. Never submitted for FAKE annotation since it does not close
 * anything.
 * kill_scope: instance — this exact if/else duplication + relocation, on
 * the s4 chassis, both do-while(0) FAKE wraps present, no other construct
 * changed.
 */
