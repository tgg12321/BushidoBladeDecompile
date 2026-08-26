/* REJECTED (s7, 2026-08-26) -- semantically fine, but the WRONG RECONSTRUCTION, and
 * the reason six sessions stalled. Every form s1-s6 spelled the 0x3C-byte sector->dest
 * copy as an explicit C pointer loop over a 16-byte `Quad` struct plus a 12-byte
 * `Triple` tail:
 *     copyloop: *dst_q = *src; src++; dst_q++;
 *               if (src != copy_end) goto copyloop;
 *               *(Triple *)dst_q = *(Triple *)src;
 * That makes `copy_end` a C-level pseudo whose single use sits inside a C loop, so its
 * reg_n_refs is loop-depth-weighted like every other local -- and the only way to
 * un-weight it is to spell the two loops asymmetrically (structured outer + goto inner),
 * which is exactly the construct the 2026-08-26 00:52 layer-1 cheat review FAILed.
 * The copy loop is not in the source: it is GCC's mips block-move expander output
 * (config/mips/mips.c block_move_loop, mips.c:2222). Write the copy as ONE aggregate
 * assignment (`*(CamRot *)dest = *(CamRot *)&sp_buf[0x10];`) and the asymmetry is
 * produced by the compiler -- block_move_loop emits its label with emit_label(), never
 * NOTE_INSN_LOOP_BEG, so flow.c's loop_depth never rises inside it. See candidate.c.
 */
