# Hypothesis ledger — file_LoadSectors

## s1 (recon, 2026-07-28)
- H-vars-phantom: "a live-locals spelling induces a phantom vars=8 slot (per
  phantom-frame-slots-gcc272)" — KILLED. 7 probes all vars=0 or wrong frame
  composition; the HImode-bitwise trigger needs narrow global loads and dies with
  calls present. See rejected/phantom-vars-slot-all-probes-vars0.c.
- H-args24-deadcall: "target frame is args=24 from a >=5-arg call expression
  expanded then deleted by RTL cse (outgoing_args_size is monotonic)" — CONFIRMED
  mechanically: sandbox 0 on real TU, byte-identical body, zero residue.
  Classification (sanctioned-family vs cheat) sent to ruling.
- Frontier: (1) ruling on the dead-call construct; (2) if refused — sweep for a
  more plausible-as-original deleted-call spelling (macro-shaped error check),
  same mechanism; (3) SOTN-master census for deleted-call/args-area precedent to
  inform the ruling.

## s1b (recon, 2026-07-28, post-Judge-FAIL)
- H-if0-expands: "GCC 2.7.2 expands if(0)/const-false-guarded call bodies to RTL
  (bumping outgoing_args) before deletion" — CONFIRMED (p2_if0_call5, p2_dbgflag_call5
  byte-identical to the proven sandbox-0 form).
- H-strlit-clean: "a deleted call may carry a format-string literal without residue"
  — KILLED (literal emitted to .rodata even from the deleted arm; p2_if0_strlit.s).
- H-sotn-precedent: "SOTN master contains deleted-call/args-area precedent" — KILLED
  (census negative on mechanism; genre-adjacent if(0) instances only).
- Frontier: owner ruling (escalation filed 2026-07-28). No grind-advanceable axis
  remains: vars axis dead (s1), dead-local family forbidden absent carve-out ruling,
  deleted-call family owner-only (Judge 2026-07-28 08:12).

## [s1] GCC 2.7.2 expands if(0)/const-false-guarded call bodies to RTL (bumping outgoing_args to 24) before jump-opt deletes them, so a macro-shaped disabled-debug spelling reproduces the s1 sandbox-0 result
- mechanism: calls.c expand_call bumps current_function_outgoing_args_size during RTL expansion of the constant-false arm; the bump is monotonic; jump-opt/cse delete the arm with zero residue
- probe: tmp/grind/file_LoadSectors/s1/probe2.sh: p2_if0_call5.c and p2_dbgflag_call5.c through project cc1, diffed vs v_deadcall5.s (the sandbox-0-proven form)
- result: both variants produce cc1 output IDENTICAL to v_deadcall5.s except the .file line: frame 48, args 24, zero relocs/rodata
- verdict: CONFIRMED

## [s1] A deleted call may carry a printf-style format-string literal without residue, making a disabled-debug-print original fully plausible
- mechanism: string constant would need to stay unemitted when the guarded arm is deleted
- probe: p2_if0_strlit.c: if(0)-guarded call with string-literal arg through project cc1
- result: literal IS emitted to .rodata even though the call is deleted; oracle has no such string, so any deleted-call original had scalar-only args; banked rejected/deleted-call-string-literal-rodata-leak.c
- verdict: KILLED

## [s1] SOTN master contains deleted-call/args-area-coercion precedent usable for a new-family sanction request (Judge-mandated census, frontier item 3)
- mechanism: precedent search over the matched SOTN tree for constant-false-guarded or unreachable calls whose effect is frame/args inflation
- probe: grep census over C:/Users/Trenton/Desktop/sotn-decomp/src for if(0)/while(0)/if(false)/unreachable patterns, each hit read in context
- result: NEGATIVE on mechanism: zero >=5-arg dead calls in SOTN master. Genre-adjacent if(0) dead code in matched code: e_hellfire_beast.c:827 FAKE while-loop (US PSX), e_collect.h:290 beta arm with <=2-arg calls, game_handlers.c:1465 empty arm, sel_psp/94D8.c:67 fake local
- verdict: KILLED

## s2 (structural, 2026-08-20)

## [s2] An ORDINARY-C structural spelling of the same logic reaches frame 48 with a byte-identical body, without any dead object — i.e. the s0/s1 "dead object is forced" partition (dead local XOR deleted call) is false
- mechanism: memory/project/phantom-frame-slots-gcc272.md — GCC 2.7.2 allocates a stack temp during RTL expansion of a strength-reduced induction variable; get_frame_size() counts it (cc1 prints it as `vars=` in the `.frame` comment) but the allocator keeps the value in a register, so no store is ever emitted. A `for (i = 0; i < count; i++)` loop triggers it where the hand-unrolled `i = 0; if (count > 0) { do { ... i += 1; } while (i < count); }` does not.
- probe: 13-variant structural sweep of semantically-equivalent spellings (loop shape, narrow-typed locals, named intermediates, block scoping, real >4-word calls) compiled with the project cc1 + exact Makefile CC_FLAGS incl. -mel, read out through the `.frame` comment (tmp/grind/file_LoadSectors/s2/sweep.py); then the winning variant applied to the real TU and scored with `sandbox file_LoadSectors --disable all`.
- result: CONFIRMED, and it is a MATCH. `for` spelling -> `.frame $sp,48,$31 # vars= 8, regs= 6/0, args= 16`, sandbox score 0 (51/51, rules_dropped 0). cc1 output identical to the do-while baseline except the 14 frame-offset instructions and label numbering. Banked candidate.c + self_vet.md.
- verdict: CONFIRMED

## [s2] The args=24 composition is reachable by a REAL (non-deleted) call, making the refused deleted-call family unnecessary
- mechanism: a >4-word outgoing argument list sets current_function_outgoing_args_size to 24 whether or not the call survives
- probe: v_real_5arg_call (5 scalar args) and v_real_double_arg (3 ints + a double, which o32 alignment pushes to sp+16..23) in the s2 sweep
- result: both reach frame 48 via args=24 — and both cost +6 instructions, because a real call MUST store its 5th word at sp+16..23 while the target body has zero references to sp+0..23. Free args=24 requires deletion, i.e. the owner-refused family (b).
- verdict: KILLED

## [s2] Frame 48 alone identifies the right composition
- mechanism: n/a — decoy check
- probe: same sweep, reading the full vars/regs/args triple rather than the frame total
- result: KILLED. 5 of the 8 frame-48 variants reach 48 through regs=7 (an extra callee-save), which puts the saves at the wrong offsets and costs 3-8 extra instructions. The target's 6 saves at sp+0x18..0x2C pin regs=24, so only vars=8 or args=24 are admissible. Read the triple, never the total.
- verdict: KILLED
