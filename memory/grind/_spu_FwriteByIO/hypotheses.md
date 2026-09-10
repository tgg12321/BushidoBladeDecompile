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


## Session 2 (2026-09-10, structural)

H5 (KILLED, instance). The 2-word residual is a single misplaced loop-invariant
    load: getting the `lui/lw %hi/%lo(_spu_RXX)` below the guard branch removes
    both a load-delay nop and an empty delay slot (session 1 F1 framing).
    Mechanism claimed: maspsx inserts a load-delay nop after the `lw`, and the
    `i = 0` cannot reach the branch delay slot while the load is above it.
    Probe: disassembled the s1 candidate own sandbox object
    (tmp/grind/_spu_FwriteByIO/s2/build.dis, 0x2da4-0x2db8) instead of reasoning
    from the score.
    Result: there is no load-delay nop. The two extra words are a SECOND `blez
    $s0` and its `nop` delay slot - the source `if (num > 0)` guard, which 2.7.2
    does not merge with the loop-entry test it emits for the `for`. The load
    placement is a real but separate defect. Superseded by H6+H7.
    kill_scope: instance. measured_on: HEAD 73ce9a57d, CC_FLAGS
    -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin
    -w -mel -msoft-float, no FAKE constructs present (s1 candidate body).

H6 (CONFIRMED). Deleting the redundant source-level `if (num > 0)` guard removes
    one `blez` and its `nop`, leaving the loop-entry test as the only branch with
    `i = 0` filling its delay slot - the target branch shape.
    Mechanism: `for (i = 0; i < num; i += 2)` already gets an entry test from the
    loop optimizer; the hand-written guard duplicates it and GCC 2.7.2 keeps both.
    Probe: removed the guard and the `b` base local, compiled, sandboxed.
    Result: 115 build insns vs 115 target (was 117), single `blez` with `move
    $v1,$zero` in the delay slot. Score 3 -> 8, because removing `b` also un-did
    the base-load placement - the branch half is fixed, the load half regressed.
    Banked as rejected/flat-cast-no-licm-hoist-score8.c.

H7 (CONFIRMED). The `_spu_RXX` base load lands in the FIFO loop preheader iff
    the FIFO store is a struct member access, because MEM_IN_STRUCT_P is what
    clears GCC store-vs-load dependence test.
    Mechanism: invariant_p (tools/gcc-2.7.2/loop.c:2760-2783) rejects the MEM if
    any loop store is true_dependence-conflicting; true_dependence
    (tools/gcc-2.7.2/sched.c:817-840) returns 0 when the store is MEM_IN_STRUCT_P
    at a varying non-QImode address and the load is non-MEM_IN_STRUCT_P at a fixed
    address. A flat `*(volatile u16 *)(_spu_RXX + 0x1A8)` cast is an INDIRECT_REF
    of scalar type -> flag clear -> no hoist. A COMPONENT_REF sets it.
    Probe: typed the SPU transfer/control register block at `_spu_RXX + 0x1A6`
    (0x1F801DA6) as the five-register struct Sony libspu already models it as,
    routed the FIFO store through it, compiled, sandboxed; then ran
    `pwsh tools/grinder/dump.ps1 _spu_FwriteByIO` and read main.cse vs main.loop.
    Result: score 0. Dump confirms loop.c LICM is the mover - insn 67 (the load)
    inside the loop in main.cse becomes insn 278 in main.loop, emitted between the
    loop-entry jump_insn 273 and NOTE_INSN_LOOP_BEG note 56; the store prints
    `(mem/s/v:HI ...)` with `/s` = MEM_IN_STRUCT_P set.

H8 (CONFIRMED). Routing ALL nine SPU register accesses through that struct - not
    just the FIFO store - keeps the match and is the better body.
    Mechanism: the other eight accesses sit in the outer `while` loop and in the
    two timeout loops. The outer loop contains `jal _spu_Fw1ts`, which sets
    unknown_address_altered, so no MEM there is hoistable regardless of the flag;
    the timeout loops re-read SPUSTAT every iteration by construction. So the
    struct changes nothing for them.
    Probe: converted every `*(volatile u16 *)(_spu_RXX + <off>)` in the function to
    `SPU_CTRL-><member>`, compiled, sandboxed, then ran verify-oracle.
    Result: score 0, 115/115, and full-build SHA1
    62efab4f73f992798c43e8c730aa43baa10bb4fa == locked oracle. Adopted: a struct
    used for one of nine accesses to the same register block would be the odd
    construct; used for all nine it is simply the register block type.

H9 (CONFIRMED, side finding). The LICM gate can be opened by more than one
    spelling: `((volatile u16 *)_spu_RXX)[0xD4] = *cur++;` also produces the
    preheader hoist.
    Mechanism: same MEM_IN_STRUCT_P gate reached by a different tree shape.
    Probe: one compile, read the emitted preheader.
    Result: identical `blez / move $3,$0 / lw $4,_spu_RXX / loop-top` shape as the
    struct form. NOT adopted - a magic word index 0xD4 standing in for byte offset
    0x1A8 is worse C than naming the register - but recorded because the next
    function that needs a base-address reload moved into a preheader has two
    spellings to try, not one.

## Live frontier (for session 3)

NONE - the function is MATCHED. sandbox --disable all = 0 (115/115, rules_dropped
0) and `verify-oracle` reports build_sha1 == the locked oracle
62efab4f73f992798c43e8c730aa43baa10bb4fa with the body applied to src/main.c.
The remaining steps are review and integration, which belong to the driver: layer-1
cheat-reviewer on the diff, then the Judge. If either bounces the struct, the
correct next move is a ruling-request asking whether typing an MMIO register block
as a struct (no splat symbol merged, `_spu_RXX` declaration untouched) sits inside
[[mmio-volatile-type-level]] or needs its own grant - NOT a respelling, and NOT the
`[0xD4]` array-index variant, which reaches the same bytes with strictly worse C.
