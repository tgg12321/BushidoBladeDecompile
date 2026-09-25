/*
 * REJECTED (session 7) — the ONLY construct in the frozen gcc-2.7.2 MIPS o32
 * backend that emits a call with a ZERO outgoing-argument area, and it cannot
 * produce motion_Close's bytes.
 *
 * WHY IT WAS PROBED. Frontier F9 observed that H1 (the 16-byte outgoing-arg
 * area that makes a pure-C frame >= 32 where the target's is 16) rested on a
 * CENSUS of the oracle build, not on the backend. A census cannot rule out a C
 * construct that simply never occurs in BB2. So: enumerate every `emit_call_insn`
 * site in the compiler and check whether any of them can leave
 * `current_function_outgoing_args_size` at zero, since `mips.c:4466`
 * (`compute_frame_size`) takes that variable as the sole input to the frame's arg
 * area.
 *
 * THE ENUMERATION. calls.c:387/398/403 (emit_call_1 — shared by expand_call and
 * emit_library_call, both of which MAX args_size.constant up to
 * reg_parm_stack_space and store it into cfoas at calls.c:1245+1400,
 * 2393-2401, 2743-2751); integrate.c:1807, unroll.c:1948, loop.c:1801/1845
 * (copies of an already-expanded call insn; integrate.c:1358 additionally
 * propagates the inlinee's OUTGOING_ARGS_SIZE into the caller by MAX);
 * expr.c:8361/8382 inside `expand_builtin_apply` — the ONLY site that never
 * touches cfoas.
 *
 * On MIPS `reg_parm_stack_space` is the compile-time constant 16 for every
 * fndecl: REG_PARM_STACK_SPACE (mips.h:1822) = MAX_ARGS_IN_REGISTERS(4) *
 * UNITS_PER_WORD(4) - FIRST_PARM_OFFSET, and FIRST_PARM_OFFSET is the `#else`
 * arm of an `#if 0` block (mips.h:1801-1812), i.e. literally 0.
 * MAYBE_REG_PARM_STACK_SPACE and FINAL_REG_PARM_STACK_SPACE are undefined for
 * MIPS, so the two zeroing escape hatches in calls.c are dead code, and
 * OUTGOING_REG_PARM_STACK_SPACE IS defined (mips.h:1830), so the `#ifndef`
 * subtraction at calls.c:1253 that would take the 16 back off is dead too.
 *
 * THE FORM (the hole, written as C):
 */

extern void callee(void);

void probe(void) {
    __builtin_apply((void (*)())callee, __builtin_apply_args(), 16);
}

/*
 * MEASURED (tmp/grind/motion_Close/s7/f9probe.sh, canonical cc1 flags
 * -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel;
 * emitted asm in tmp/grind/motion_Close/s7/out/p3_apply.s):
 *
 *     .frame  $fp,72,$31    # vars= 56, regs= 3/0, args= 0, extra= 0
 *     .mask   0xc0010000,-8
 *     34 emitted instructions
 *
 * So `args= 0` IS reachable — and it is worthless. It buys the zero arg area at
 * the price of a FRAME POINTER, a 72-byte frame, 56 bytes of vars, $fp in the
 * save mask and 34 instructions, against the target's 16-byte frame, no frame
 * pointer, a {s0,s1,ra} mask and 26 instructions. `expand_builtin_apply` also
 * emits a dynamic argument-block push plus an emit_block_move of the incoming
 * argument area and a reload of $a0-$a3, none of which appears anywhere in the
 * target. And `__builtin_apply` is a GCC extension no 1998 PsyQ crt0 contains.
 *
 * THE CONTROLS, all measured in the same run (f9probe.sh / f9probe2.sh):
 *   motion_Close-shaped indirect call through a table   .frame $sp,32  args=16
 *   plain direct call                                   .frame $sp,24  args=16
 *   UNPROTOTYPED call                                   .frame $sp,24  args=16
 *   call to an __attribute__((const)) function          .frame $sp,24  args=16
 *   call inlined from a static callee                   .frame $sp,24  args=16
 *   call inside a loop                                  .frame $sp,24  args=16
 *   leaf function, no call at all                       .frame $sp,0   args=0
 *
 * The last row is the point: args=0 is available exactly when there is no call.
 * F9's own suggested escapes — "a call through a pointer with a prototype gcc
 * treats specially", "-mno-abicalls interactions" — are dead on measurement
 * (the unprototyped and const-attributed probes, and every probe above is
 * already compiled with -mno-abicalls, which is the canonical build flag).
 *
 * VERDICT: F9 KILLED. H1 is upgraded from "no function in this build has a
 * sub-16 arg area" to "the backend cannot emit a call without one" — the
 * "no C input to this compiler produces these bytes" form. Do not re-probe the
 * frame axis.
 */
