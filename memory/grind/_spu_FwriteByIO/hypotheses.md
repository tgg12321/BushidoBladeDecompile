# Hypothesis ledger - _spu_FwriteByIO

## Session 1 (2026-09-10, recon)

H1 (CONFIRMED). The function is a verbatim-linked Sony PsyQ libspu object and a
    psyz/SOTN transplant reproduces it almost exactly.
    Mechanism: same compiler family (GCC 2.7.2), same source.
    Probe: transplanted psyz `spu.c:111` shape onto BB2's `_spu_RXX` MMIO idiom.
    Result: 115 -> 12, then -> 3.  113/115 instructions byte-identical.

H2 (KILLED, instance). Adding two unused `s32` locals (`wt0`, `wt1`) to the
    transplant body reserves the target's 8 extra frame bytes.
    Mechanism claimed: declared locals count toward `get_frame_size()`.
    Probe: added the two decls, read `.frame ... # vars=`.
    Result: vars=0, sandbox still 12 - GCC 2.7.2 drops unreferenced non-volatile
    scalars before frame layout.
    kill_scope: instance.  measured_on: HEAD 66e2f1e6e, current CC_FLAGS, no FAKE
    constructs present.

H3 (KILLED, instance). Marking those two locals `volatile` is the way to reserve
    the 8 bytes honestly.
    Mechanism claimed: volatile forces a memory home, so the slot survives.
    Probe: `volatile s32 wt0, wt1;`; read `.frame`, then `sandbox --disable all`,
    then read the sandbox's own preprocessed source.
    Result: the REAL compile does give vars=8 and the right frame, but the sandbox
    blanks both declaration lines (engine/volatile_cheats.py:388
    `find_volatile_unused_locals`) so the honest score stayed 12; the construct is
    additionally outside the owner-gated `_SANCTIONED_UNWRITTEN_PADS` allowlist
    (engine/volatile_cheats.py:746).  Moot in any case: H4 gets the same frame
    with ordinary C.
    kill_scope: instance.  measured_on: HEAD 66e2f1e6e, current CC_FLAGS, the two
    volatile pads present as the only FAKE-class construct.

H4 (CONFIRMED). The target's 8 phantom frame bytes come from the fifo loop being
    spelled as a `for` with its own `i = 0` init clause - no dead declaration is
    involved.
    Mechanism: GCC 2.7.2 allocates a stack temp while expanding that loop shape
    and register-allocates it away, but `get_frame_size()` has already counted it
    (the [[phantom-frame-slots-gcc272]] artifact, confirmed here on a second
    function).
    Probe: 7-way spelling sweep of that one loop with everything else held fixed,
    reading `.frame ... # vars=` each time (evidence E4).
    Result: `for (i = 0; ...)` -> vars=8; do-while / while / `for (; ...)` /
    `i = 0` hoisted out of the guard -> vars=0.  Floor 12 -> 3.

## Live frontier (for session 2)

F1. Move the loop-invariant `lui/lw %hi/%lo(_spu_RXX)` BELOW the fifo loop's guard
    branch, so the target's `blez` + `addu $v1,$zero,$zero` delay-slot pair is
    emitted and the load-delay nop disappears.  That is the whole 2-word residual.
    Attribute the mover with a DUMP FIRST, not a guess:
    `pwsh tools/grinder/dump.ps1 _spu_FwriteByIO`, then read `.loop` (did loop.c
    hoist the `_spu_RXX` load into the preheader as an invariant?) and
    `.sched`/`.reorg` (is it a scheduler sink or a delay-slot-fill decision?).
    The `b = _spu_RXX` local's position inside vs outside the `if (num > 0)` guard
    is already known to move the score (3 vs 4), so the emission point IS
    source-controllable - this is a spelling question, not a wall.

F2. If F1's dump shows loop.c is the mover: the guard `if (num > 0)` and the `for`
    share the same exit test, so jump.c's `duplicate_loop_exit_test` may be
    reshaping the preheader.  Probe spellings that keep the `for (i = 0; ...)`
    init clause (required for vars=8, H4) while changing what the preheader holds
    - e.g. deriving `b` from something the guard already computed, or reading the
    fifo register through a `volatile u16 *` pointer local initialised inside the
    guard.  The already-matched `_spu_init` uses exactly that idiom at
    src/main.c:1721 (`vp = (volatile u16 *)_spu_RXX;`), and its ledger records
    that `loop_has_volatile` blocks MEM invariant hoisting in 2.7.2 loop.c, so the
    base load must be a source-level local.

F3. Cheap parallel check: `num` / `i` / `size` type sweep (`s32` vs `u32` vs
    `unsigned long`) on candidate.c.  The target's `sltiu $v0,$s1,0x41` and
    `slt $v0,$v1,$s0` already match, so this is low-probability - but it is one
    compile each with the tmp/grind/_spu_FwriteByIO/s1/try.py harness, so it costs
    a single turn.  Use the harness, not the sandbox.

## [s1] _spu_FwriteByIO is the verbatim-linked Sony PsyQ libspu spu.c object, so a psyz/SOTN transplant rewritten in BB2's established `*(volatile u16 *)(_spu_RXX + off)` MMIO idiom reproduces it.
- mechanism: Same compiler family (GCC 2.7.2) compiling the same Sony source; _spu_init (src/main.c:1592) already matched with this exact register-file idiom, so the declaration side was pre-solved.
- probe: Replaced INCLUDE_ASM at src/main.c:1754 with the transplant; `sandbox _spu_FwriteByIO --disable all`, plus an instruction-by-instruction objdump-vs-asm/funcs comparison.
- result: 115 -> 12 on the first cut, and the 12 residual diffs were exactly the frame words (addiu sp -0x30, five sw, five lw, addiu sp 0x30). Every one of the 115 opcodes, both timeout loops, both printf arms and the epilogue were already in the target's order. Further spelling work took it to 3.
- verdict: CONFIRMED

## [s1] Two unused plain `s32` locals (wt0, wt1) added to the transplant body reserve the target's 8 extra frame bytes.
- mechanism: Claimed: declared locals are counted by get_frame_size() during mips.c compute_frame_size, so two words of declarations widen vars= from 0 to 8.
- probe: Added `s32 wt0; s32 wt1;` at the top of the body and read cc1's own `.frame $sp,N,$31 # vars= N` comment for _spu_FwriteByIO (harness tmp/grind/_spu_FwriteByIO/s1/frame.sh + try.py), then re-ran the sandbox.
- result: vars= 0, frame $sp,40 unchanged, sandbox still 12. GCC 2.7.2 drops unreferenced non-volatile scalars before frame layout, so plain dead scalars reserve nothing here.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 66e2f1e6e, CC_FLAGS -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float; no FAKE constructs present in the measured form.

## [s1] Marking those two locals `volatile` (Sony's vestigial WASTE_TIME macro locals, as banked bit-exact by Closer phase-3 session 11) is the way to reserve the 8 bytes and reach an honest zero.
- mechanism: Claimed: volatile forces a memory home for the scalar, so the slot survives DCE and is counted by get_frame_size().
- probe: Applied `volatile s32 wt0; volatile s32 wt1;`, read the `.frame` comment, ran `sandbox --disable all`, then read the sandbox's own copy of the preprocessed source at tmp/sandbox/_spu_FwriteByIO/src/main.c.
- result: The real cc1 compile does give `.frame $sp,48 # vars= 8` (the target frame), but the sandbox blanks both declaration lines before compiling — engine/volatile_cheats.py:388 find_volatile_unused_locals — so the honest score stayed at 12. The construct is additionally outside the per-function owner-gated allowlist _SANCTIONED_UNWRITTEN_PADS at engine/volatile_cheats.py:746. Moot regardless: the next hypothesis reaches the same frame with ordinary C, so no ruling request is warranted for this construct on this function.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 66e2f1e6e, same CC_FLAGS, with the two volatile unused pads present as the only FAKE-class construct in the body.

## [s1] The target's 8 bytes of never-touched frame locals come from the fifo loop being spelled as a `for` whose init clause is the loop's own `i = 0`, with no dead declaration anywhere.
- mechanism: GCC 2.7.2 assigns a stack temp while expanding that loop shape and later register-allocates it away, but get_frame_size() has already counted the slot — the phantom-frame-slot artifact previously byte-verified on tslLineG5Init (src/code6cac_c2.c:1267), now confirmed on a second, independent function.
- probe: Seven-way spelling sweep of that one loop with the entire rest of the body held fixed, reading cc1's `.frame ... # vars=` after each compile (one compile per spelling, no sandbox runs), then a sandbox run on the winner.
- result: do-while, while, `for (; i < num; i += 2)` with the init hoisted out, and `i = 0` moved inside the guard all give vars= 0. All three spellings that keep `for (i = 0; i < num; i += 2)` give vars= 8 and the target's `.frame $sp,48`. psyz spells this loop exactly that way, so the phantom bytes are ordinary Sony source. Sandbox floor dropped 12 -> 3.
- verdict: CONFIRMED
