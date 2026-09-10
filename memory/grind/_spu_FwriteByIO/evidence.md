# Evidence bank - _spu_FwriteByIO

## Session 1 (2026-09-10, recon)

OBJECT MODEL: every symbol the DATA MODEL section flagged was checked against
the emitted relocations of a build whose 113/115 instructions are already
byte-identical to `asm/funcs/_spu_FwriteByIO.s` (measured this session,
sandbox `--disable all` = 3).  Per-symbol verdict:
  - `_spu_RXX` @0x800A2CDC - decl `extern s32 _spu_RXX;` (src/main.c:75, also
    m2c_context.h).  MATCHES.  Every access is spelled
    `*(volatile u16 *)(_spu_RXX + <off>)`, exactly the idiom the already-matched
    `_spu_init` (src/main.c:1592-1752) uses ~30 times.  The target's
    `lui/lw %hi/%lo(_spu_RXX)` + `lhu/sh <off>($reg)` pairs are reproduced
    one-for-one; no aggregate/union re-declaration is needed or wanted.
  - `_spu_tsa` @0x800A2CF4 - decl `extern u16 _spu_tsa;` (src/main.c:87; not in
    include/*.h, which is why the auto-scan flagged it).  MATCHES: the target's
    `lhu %lo(_spu_tsa)` requires the u16 type; an s32 would emit `lw`.
  - `D_800163D8` @0x800163D8 ("SPU:T/O [%s]\n") - defined `const char
    D_800163D8[16]` at src/main.c:1583, no header decl (flag reason).  MATCHES:
    both `printf(&D_800163D8, ...)` call sites reproduce the target's
    `lui/addiu %hi/%lo(D_800163D8)` pair, the same spelling `_spu_init` already
    uses at src/main.c:1611.
  - `D_800163F8` @0x800163F8 ("wait (wrdy H -> L)") and `D_8001640C` @0x8001640C
    ("wait (dmaf clear/W)") - defined `const char [20]` at src/main.c:1585-1586;
    no address row / no header decl (flag reason).  MATCHES: each is the second
    printf argument of one of the two timeout arms and reproduces the target's
    `lui/addiu` pair at the right index.
  No MISMATCH and no MISMATCH-unmeasured: the declaration side of this function
  is fully settled and is NOT where the residual lives.

FLOOR: 115 (HEAD, INCLUDE_ASM) -> 3 (this session; memory/grind/_spu_FwriteByIO/candidate.c).

E1. The whole function transplants from PsyQ libspu `spu.c::_spu_FwriteByIO`.
    A first-cut body (do-while fifo loop + nested if/do-while timeout loops,
    banked as rejected/do-while-fifo-frame-vars0-score12.c) already reproduces
    ALL 115 target instructions in the right order EXCEPT the 12 frame words:
    build `.frame $sp,40 vars=0 regs=5 args=16`, target `$sp,48` with 8 bytes
    of locals nothing ever touches (target saves start at 0x18, ours at 0x10).
    Measured sandbox 12; the 12 diffs are exactly `addiu sp,-0x30`, 5 `sw`,
    5 `lw`, `addiu sp,0x30`.

E2. The frame gap is a `get_frame_size()` (`vars=`) gap, and cc1 prints it.
    `.frame ... # vars= N` on the function's own label is a direct, one-compile
    gradient on the frame, far cheaper than a sandbox run.  Harness this session:
    tmp/grind/_spu_FwriteByIO/s1/frame.sh (cpp | cc1 with the Makefile flags,
    dumps main.s) + try.py (splices a body file into a pristine src/main.c copy,
    compiles, prints the `.frame` line).  Target needs vars=8.

E3. KILLED - the "dead local reserves the frame" route is BOTH unnecessary and
    mechanically inert here.  `s32 wt0; s32 wt1;` (plain, unused) -> vars=0, so
    non-volatile dead scalars reserve nothing at all.  `volatile s32 wt0, wt1;`
    DOES give vars=8 and a real-build byte match - but `engine/volatile_cheats.py`
    `find_volatile_unused_locals` (engine/volatile_cheats.py:388) BLANKS the two
    declaration lines before the sandbox compiles, so the honest score stayed 12
    (verified by reading tmp/sandbox/_spu_FwriteByIO/src/main.c, where both lines
    are whitespace).  The construct is also outside `_SANCTIONED_UNWRITTEN_PADS`
    (engine/volatile_cheats.py:746), a per-function owner-gated allowlist.
    Superseded by E4 - do NOT spend a ruling request on it.

E4. THE FRAME LEVER IS THE `for` STATEMENT, and it is ordinary C.  Holding the
    rest of the body fixed and varying ONLY the fifo loop's spelling:
      i = 0; if (num > 0) { b = _spu_RXX; do { ... } while (i < num); }  -> vars=0
      i = 0; if (num > 0) { b = _spu_RXX; for (; i < num; i += 2) ... }  -> vars=0
      i = 0; if (num > 0) { b = _spu_RXX; while (i < num) { ... } }      -> vars=0
      if (num > 0) { b = _spu_RXX; i = 0; do { ... } while (i < num); }  -> vars=0
      if (num > 0) { b = _spu_RXX; for (i = 0; i < num; i += 2) ... }    -> vars=8
      b = _spu_RXX; for (i = 0; i < num; i += 2) ...   (unguarded)       -> vars=8
      for (i = 0; i < num; i += 2) { *(volatile u16*)(_spu_RXX+0x1A8)=...} -> vars=8
    So the slot appears iff the loop is spelled as a `for` whose INIT clause is
    the loop's own `i = 0`; a `for` with an empty init clause does not do it, and
    neither does hoisting `i = 0` out of the guard.  psyz spells this loop exactly
    that way, so the target's 8 phantom bytes are ordinary Sony source, not a dead
    declaration.  This CONFIRMS [[phantom-frame-slots-gcc272]] on a second,
    independent function.

E5. Best form this session = memory/grind/_spu_FwriteByIO/candidate.c, sandbox 3.
    It is E1's body with the fifo loop respelled
    `if (num > 0) { b = _spu_RXX; for (i = 0; i < num; i += 2) { ... } }`.
    Frame is now exactly right and 113/115 instructions are byte-identical.

E6. THE ENTIRE RESIDUAL IS THE FIFO-LOOP PREHEADER (2 extra emitted insns).
      target: blez $s0,.L80088A54 / addu $v1,$zero,$zero (delay slot) /
              lui $a0,%hi(_spu_RXX) / lw $a0,%lo(_spu_RXX)($a0)
      ours:   lui $a0,%hi / lw $a0,%lo / nop / blez $s0 / move $v1,$zero (delay)
    i.e. cc1 emits the loop-invariant `_spu_RXX` load ABOVE the guard branch, so
    (a) maspsx must insert a load-delay nop after the `lw`, and (b) the `i = 0`
    that the target uses to fill the branch delay slot is pushed after the branch.
    Two extra words, and nothing else in the function differs.  Intermediate forms
    bank the ordering evidence: with the load hoisted OUT of the `if`
    (`b = _spu_RXX; for (...)`, rejected/for-with-hoisted-base-score4.c) the score
    is 4; psyz-verbatim (no `b` local at all, rejected/psyz-verbatim-score8.c) is 8.

E7. Chassis note: everything above was measured on HEAD 66e2f1e6e with the current
    CC_FLAGS (`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls
    -fno-builtin -w -mel -msoft-float`), with no FAKE constructs present in any
    measured form except where H3 explicitly says otherwise.  src/main.c was
    reverted to HEAD at end of session (candidate lives only in memory/grind/).

- [s1] OBJECT MODEL: all five flagged symbols MATCH, verified against a build whose 113/115 instructions are byte-identical to asm/funcs/_spu_FwriteByIO.s. _spu_RXX @0x800A2CDC (`extern s32`, src/main.c:75) MATCHES — every access spelled `*(volatile u16 *)(_spu_RXX + off)`, the same idiom the already-matched _spu_init uses ~30 times; the target's lui/lw %hi/%lo(_spu_RXX) + lhu/sh off($reg) pairs reproduce one-for-one and no aggregate/union re-declaration is needed. _spu_tsa @0x800A2CF4 (`extern u16`, src/main.c:87) MATCHES — the target's `lhu %lo(_spu_tsa)` requires u16; s32 would emit lw. D_800163D8 @0x800163D8 (`const char[16]`, src/main.c:1583) MATCHES — both printf call sites reproduce the target's lui/addiu %hi/%lo pair. D_800163F8 @0x800163F8 and D_8001640C @0x8001640C (`const char[20]`, src/main.c:1585-1586) MATCH — each is the second printf argument of one timeout arm and lands its lui/addiu pair at the right index. No MISMATCH, no MISMATCH-unmeasured: the declaration side of this function is settled and is not where the residual lives.

- [s1] FLOOR 115 -> 3. Best form banked at memory/grind/_spu_FwriteByIO/candidate.c; src/main.c reverted to HEAD at end of session.

- [s1] cc1 prints get_frame_size() itself as `.frame $sp,N,$31 # vars= N` on the function's own label. That is a direct one-compile gradient on the frame and is strictly better than the sandbox score for frame questions, which cannot separate 'wrong frame' from 'wrong codegen'. Harness reusable by the next session: tmp/grind/_spu_FwriteByIO/s1/frame.sh (cpp | cc1 with the Makefile flags) and tmp/grind/_spu_FwriteByIO/s1/try.py (splice a body file into a pristine src/main.c, compile, print the .frame line) — about 10 seconds per spelling versus a full sandbox run.

- [s1] The first-cut transplant (rejected/do-while-fifo-frame-vars0-score12.c) already emitted all 115 target opcodes in the target's order and differed ONLY in the 12 frame words: build `.frame $sp,40 vars=0 regs=5/0 args=16` versus the target's $sp,48 with 8 bytes of locals nothing ever reads or writes (target saves start at 0x18, ours at 0x10).

- [s1] The engine's sandbox does not merely score cheat-asm: engine/volatile_cheats.py:388 find_volatile_unused_locals silently BLANKS `volatile <scalar> name;` declarations that have no other reference in the body, before the sandbox compiles. Confirmed by reading tmp/sandbox/_spu_FwriteByIO/src/main.c, where both pad lines came back as whitespace while src/main.c still carried them. Any future session tempted by an unwritten volatile pad on any function should expect the honest floor to be unchanged, exactly as [[unannotated-fake-inflates-honest-floor]] describes.

- [s1] THE FRAME LEVER, measured (all other code held fixed, one compile each): `i=0; if (num>0) { b=_spu_RXX; do {...} while (i<num); }` -> vars=0 | `i=0; if (num>0) { b=_spu_RXX; for (; i<num; i+=2) ... }` -> vars=0 | `i=0; if (num>0) { b=_spu_RXX; while (i<num) {...} }` -> vars=0 | `if (num>0) { b=_spu_RXX; i=0; do {...} while (i<num); }` -> vars=0 | `if (num>0) { b=_spu_RXX; for (i=0; i<num; i+=2) ... }` -> vars=8 | `b=_spu_RXX; for (i=0; i<num; i+=2) ...` -> vars=8 | `for (i=0; i<num; i+=2) { *(volatile u16*)(_spu_RXX+0x1A8) = *cur++; }` -> vars=8. The slot appears iff the loop is a `for` carrying its own `i = 0` init clause.

- [s1] THE ENTIRE REMAINING RESIDUAL IS THE FIFO-LOOP PREHEADER, 2 extra emitted words. Target: `blez $s0,.L80088A54` / `addu $v1,$zero,$zero` in the delay slot / `lui $a0,%hi(_spu_RXX)` / `lw $a0,%lo(_spu_RXX)($a0)`. Ours: `lui $a0,%hi` / `lw $a0,%lo` / `nop` / `blez $s0` / `move $v1,$zero` in the delay slot. cc1 emits the loop-invariant _spu_RXX load ABOVE the guard branch, which both forces a maspsx load-delay nop after the lw and pushes the `i = 0` out of the branch delay slot. Nothing else in the function differs.

- [s1] The preheader emission point is source-controllable, which is why this is a spelling question and not a wall: with `b = _spu_RXX` hoisted OUT of the `if (num > 0)` guard the score is 4 (rejected/for-with-hoisted-base-score4.c); with no `b` local at all, i.e. psyz verbatim, it is 8 (rejected/psyz-verbatim-score8.c); with `b` assigned inside the guard it is 3 (candidate.c).

- [s1] Applying candidate.c to src/main.c also means deleting the redundant forward declaration `extern void _spu_FwriteByIO(s32, s32);` (HEAD src/main.c:1898). Both call sites (src/main.c:1719 and src/main.c:1905) pass s32 values into the `(u8 *addr, u32 size)` prototype and compiled cleanly all session; neither call site's own bytes were disturbed.

- [s1] Cross-knowledge reconciled: memory/closer/phase3-progress.md:349 records _spu_FwriteByIO 'BIT-EXACT 132/132' from the 2026-07-10 DispUpdateStatusMessage splice era, achieved with the volatile sp0/sp4 pads. That claim is real but its construct is stripped by today's sandbox (see above) and its splice context is gone — _spu_FiDMA and _spu_Fr_ are now separate matched C at src/main.c:1769 and :1794. The honest route found this session supersedes it and needs no policy exception.


## Session 2 (2026-09-10, structural) - FLOOR 3 -> 0, BYTES PROVEN

E8. THE RESIDUAL WAS TWO SEPARATE THINGS, not one, and the s1 frontier's framing
    ("get the loop-invariant load below the guard branch") was only half of it.
    Disassembling the s1 candidate's own sandbox object
    (tmp/grind/_spu_FwriteByIO/s2/build.dis, insns at 0x2da4-0x2db8) showed our
    build emitted TWO `blez $s0` branches where the target emits one:
      ours:   blez $s0,.L / nop (delay)            <- source `if (num > 0)` guard
              lui $a0,%hi(_spu_RXX) / lw %lo
              blez $s0,.L / move $v1,$zero (delay) <- loop-entry test
      target: blez $s0,.L80088A54 / addu $v1,$zero,$zero (delay)
              lui $a0,%hi(_spu_RXX) / lw %lo(_spu_RXX)($a0)
    The two extra words are the FIRST branch and its `nop`, not a misplaced load
    plus a load-delay nop. Cause: the source-level `if (num > 0)` guard is
    redundant with the loop-entry test GCC already emits for
    `for (i = 0; i < num; i += 2)`, and 2.7.2 does not merge them.

E9. DELETING THE GUARD FIXES THE BRANCH HALF, AND ONLY THAT HALF. With the guard
    and the `b = _spu_RXX` base local both removed
    (rejected/flat-cast-no-licm-hoist-score8.c) the build is 115/115 instructions
    with a single `blez` whose delay slot carries `move $v1,$zero` - the target's
    shape - but sandbox = 8, because the `lui/lw %hi/%lo(_spu_RXX)` is now emitted
    INSIDE the loop and reloaded every iteration instead of sitting in the
    preheader. So the base load genuinely does need loop.c to hoist it, and s1 was
    right that the load placement is the second half of the residual.

E10. THE HOIST IS BLOCKED BY GCC'S ALIAS TEST, AND THE GATE IS MEM_IN_STRUCT_P.
    Read of the compiler source, not a guess: loop.c's invariant_p
    (tools/gcc-2.7.2/loop.c:2760-2783) rejects a MEM if any recorded loop store is
    `true_dependence`-conflicting with it; true_dependence
    (tools/gcc-2.7.2/sched.c:817-840) returns 0 when the STORE is MEM_IN_STRUCT_P
    at a varying (register) address in a non-QImode mode while the LOAD is
    non-MEM_IN_STRUCT_P at a fixed (symbol) address. Our FIFO store is at a
    register address (varying, HImode) and the `_spu_RXX` load is at a SYMBOL_REF
    (fixed) - so the entire question is whether the store carries MEM_IN_STRUCT_P.
    A flat `*(volatile u16 *)(_spu_RXX + 0x1A8)` cast is an INDIRECT_REF of
    non-aggregate type, so expand_expr leaves MEM_IN_STRUCT_P clear and the hoist
    is refused. A COMPONENT_REF - a struct member access - sets it, and the hoist
    happens.

E11. PASS ATTRIBUTION, DUMP-CONFIRMED (tmp/grind/_spu_FwriteByIO/dumps/, produced
    by `pwsh tools/grinder/dump.ps1 _spu_FwriteByIO` with the final body applied).
    The mover is loop.c LICM, not jump.c and not reorg.c:
      main.cse  (pre-loop):  insn 67 `(set (reg:SI 88) (mem:SI (symbol_ref "_spu_RXX")))`
                             sits INSIDE the loop, after code_label 63 / note 65.
      main.loop (post-loop): that insn is gone from the loop body and reappears as
                             insn 278, emitted between the loop-entry `jump_insn 273`
                             and `NOTE_INSN_LOOP_BEG` note 56 - i.e. in the preheader,
                             exactly the target's position.
    The store prints as `(mem/s/v:HI (plus:SI (reg:SI 88) (const_int 424)))` - the
    `/s` is MEM_IN_STRUCT_P and the `/v` is the MMIO volatile. Recorded so no future
    session re-guesses this: reorg.c never moved anything here; the `i = 0` in the
    delay slot is an ordinary backward fill of the loop-entry branch.

E12. THE HONEST DATA MODEL IS THE FIX, AND IT IS THE ORIGINAL ONE. `_spu_RXX`
    (0x800A2CDC) holds the SPU register-file base 0x1F801C00, so `_spu_RXX + 0x1A6`
    is the SPU transfer/control register block at 0x1F801DA6: five consecutive
    16-bit hardware registers - sound-RAM transfer address (0x1DA6), sound-RAM data
    FIFO (0x1DA8), SPUCNT (0x1DAA), sound-RAM transfer control (0x1DAC), SPUSTAT
    (0x1DAE). Sony's own libspu reaches them through `union SpuUnion *_spu_RXX`
    with the SPUR()/SPUW() field macros (Xeeynamo/psyz decomp/src/libspu/spu.c),
    which is a COMPONENT_REF and therefore MEM_IN_STRUCT_P. Typing the block as
    that struct and routing all nine SPU accesses in the function through it gives
    sandbox 0, 115/115.

E13. FLOOR 3 -> 0, AND THE BYTES ARE PROVEN ON THE FULL BUILD.
    `sandbox _spu_FwriteByIO --disable all` = {"score": 0, "target_insns": 115,
    "build_insns": 115, "rules_dropped": 0, "cheat_asm_stripped": 11 (the file's
    other, unrelated cheat-asm; none in this function)}. `verify-oracle` =
    {"ok": true, "build_sha1": "62efab4f73f992798c43e8c730aa43baa10bb4fa",
    "build_matches": true} - identical to the locked original SHA1. The body is in
    src/main.c now and banked at memory/grind/_spu_FwriteByIO/candidate.c; the
    self-vet is at memory/grind/_spu_FwriteByIO/self_vet.md.

E14. SIDE FINDING worth carrying to other functions: an array-index spelling on a
    cast pointer, `((volatile u16 *)_spu_RXX)[0xD4] = *cur++;`, ALSO produces the
    hoist (measured this session, one compile, same preheader shape). That was not
    adopted - a magic word index of 0xD4 standing in for byte offset 0x1A8 is worse
    C than naming the register - but it means the LICM gate can be opened by more
    than one spelling, which is useful the next time a base-address reload has to
    be moved into a preheader.

E15. CHASSIS: HEAD 73ce9a57d, CC_FLAGS
    `-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin
    -w -mel -msoft-float`. src/main.c is LEFT WITH THE MATCHING BODY APPLIED at end
    of session (score 0, oracle-verified), unlike s1 which reverted.

- [s2] The s1 frontier's premise was measurably incomplete: the 2-word residual was NOT one misplaced load plus a load-delay nop, it was a REDUNDANT SECOND GUARD BRANCH (`blez $s0` + `nop`) emitted because the source carried `if (num > 0)` in addition to the loop-entry test GCC already emits for `for (i = 0; i < num; i += 2)`. Disassembling the sandbox object (tmp/grind/_spu_FwriteByIO/s2/build.dis) rather than reasoning from the score is what showed it.

- [s2] MECHANISM, read out of the compiler and then dump-confirmed: loop.c invariant_p (tools/gcc-2.7.2/loop.c:2760-2783) refuses to hoist a MEM whose address is fixed when a loop store `true_dependence`-conflicts with it, and true_dependence (tools/gcc-2.7.2/sched.c:817-840) clears that conflict only when the store is MEM_IN_STRUCT_P at a varying non-QImode address. A flat `*(volatile u16 *)(base + k)` cast is an INDIRECT_REF of scalar type and leaves MEM_IN_STRUCT_P clear; a struct member access sets it. This is a general BB2 lever, not a fact about this function.

- [s2] PASS ATTRIBUTION IS SETTLED AND MUST NOT BE RE-GUESSED: the mover is loop.c LICM. main.cse has the `_spu_RXX` load as insn 67 inside the loop; main.loop has it as insn 278 between the loop-entry jump_insn 273 and NOTE_INSN_LOOP_BEG note 56. jump.c duplicate_loop_exit_test and reorg.c were both innocent - the `i = 0` in the branch delay slot is an ordinary backward fill.

- [s2] `_spu_RXX` = SPU register-file base 0x1F801C00, so `_spu_RXX + 0x1A6` = 0x1F801DA6 = the SPU transfer/control register block (trans_addr / trans_fifo / SPUCNT / trans_ctrl / SPUSTAT, five consecutive u16 hardware registers). Sony libspu models it as `union SpuUnion *_spu_RXX` with SPUR()/SPUW(); typing it that way in BB2 is both the original data model and the thing that opens the LICM gate.

- [s2] FLOOR 3 -> 0. sandbox --disable all score 0 (115/115, rules_dropped 0) and full-build verify-oracle SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == locked oracle. Body applied in src/main.c, banked at candidate.c, self-vet at self_vet.md.
