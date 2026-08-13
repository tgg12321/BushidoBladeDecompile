# HYPOTHESES — func_80078654

## RESOLVED

### H1 — CONFIRMED (floor 23 -> 19, insn count 115 -> 116)
**Statement.** The `v` / `v_copy` pair plus the inline `move` asm in the
inherited body is not just a cheat, it is the WRONG shape. The target's
fade-value clamp is a single SHORT-typed local: `sll $v0,$v0,16; bgez` is
GCC 2.7.2's canonical test of `(s16)x < 0`, and the `move $v1,$v0` /
`move $v0,$v1` pair around it is just GCC preserving the value across the
destructive `sll`.

**Mechanism.** A `short` local forces the sign test to be done on the
low half-word (shift-left-16 then test the sign bit) rather than a direct
`bltz`, and forces GCC to copy the un-shifted value to a second pseudo
because the `sll` clobbers its source register. The `move $v0,$v1` restore
then gets duplicated into the branch delay slot and the else-arm.

**Probe.** Replaced
```c
s32 v_copy;
v = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
__asm__ volatile("move %0, %1" : "=r"(v_copy) : "r"(v));
if ((s32)(v << 16) < 0) { v_copy = 0; }
s.r = (s.g_ = (s.b_ = (u8) v_copy));
```
with
```c
s16 sv;
sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
if (sv < 0) { sv = 0; }
s.r = (s.g_ = (s.b_ = (u8) sv));
```
(and deleted the now-unused `s32 v;`).

**Result.** Sandbox `--disable all` 23 -> 19. Build insn count 115 -> 116,
exactly matching target. The entire clamp region (target insns 38-46) now
matches byte-for-byte. **CONFIRMED — this is in candidate.c.** As a bonus
the function no longer contains any inline asm at all.

### H2 — KILLED (floor 23 -> 38, +5 insns)
**Statement.** Rewriting the trailing `goto`-loop as a real `while` loop
restores NOTE_INSN_LOOP_BEG/END, which makes flow.c weight the loop
pointer's `reg_n_refs` by loop_depth and should flip the $s0/$s1 inversion.

**Mechanism.** `flow.c:2081,2329,2515,2725` do `reg_n_refs[regno] +=
loop_depth`, and `basic_block_loop_depth` is derived purely from the
front-end's loop notes (flow.c:456,471), which a goto-loop never emits.

**Result.** KILLED, twice over. (a) loop.c ROTATES the `while` loop: it
duplicates the exit test to the top and deletes the target's entry
`j .L800787F8`, giving 121 insns vs 116, a fourth live callee-save ($s3),
and frame growth 88 -> 96. (b) It did not flip the allocation anyway — the
loop pointer still landed in $s1. Form banked at
`rejected/while-loop-triggers-loop-rotation.c`.

### H3 — KILLED analytically (do not re-run)
**Statement.** Some loop-depth ref weighting `w` exists at which var_s0
outranks arg0.

**Result.** KILLED. With `arg0 = 7 + 6w` refs and `var_s0 = 1 + 4w` refs
(their out-of-loop / in-loop split, read off the target bytes) and live
lengths 98 vs 91, `floor_log2(n)*n/len` keeps arg0 ~1.6-2.0x ahead of
var_s0 for every `w >= 1`. Ref weighting is not a route to the flip.

## LIVE FRONTIER

### F1 — the residual is a dataflow-shape problem, not an ordering problem
The 19 remaining points are ONE two-way callee-save inversion whose cause is
measured exactly (evidence.md): allocno priorities 3979 (arg0: 13 refs /
98 live) vs 1098 (var_s0: 5 refs / 91 live), where the target needs the
second to sort first. The solved bounds say a flip needs var_s0 at >=13
refs or arg0 at <=5 refs, and BOTH ref counts are pinned by the target's own
emitted memory accesses. So the original C's dataflow differed.
**Next probe.** Hunt for a C shape that changes the PSEUDO PARTITION rather
than the statement order: e.g. does the original hold the `D_800A3610` base
and the walk pointer in ONE variable (so the walk pointer inherits the
base's early refs and long live range)? Does it reach `arg0[3]`/`arg0[5]`
through a shape that costs fewer `reg_n_refs` on the parameter pseudo?
Drive each variant with `BB2_ALLOC_DEBUG=1` on `tools/gcc-2.7.2/cc1` and
read the `pri=` numbers directly — that is a far finer gradient than the
sandbox score, which only reports the binary outcome of the sort.

### F2 — sibling census
`func_80078654` is one of a family in text1b_b.c that build the same
`S78654` descriptor and call the same
`func_8007352C`/`func_8006E480`/`SetDrawMode`/`AddPrim` quartet (see
`func_80078824`, `func_800788B0`, and the neighbours around src/text1b_b.c
line 900-1100). **Next probe.** Find a SIBLING in this cluster that is
already COMPLETED-C and holds both a parameter pointer and a walk pointer in
callee-saves, and diff its `.greg` / `BB2_ALLOC_DEBUG` numbers against ours.
A matched sibling is direct evidence of the dataflow shape the original
author used, and it costs one cc1 run.

### F3 — permuter on the now-clean base
The base is materially better than it was (19, exact insn count, zero inline
asm), which is a much better seed than any prior session had. **Next probe.**
`tools/permuter_campaign.py` with a single-function `target.o` built from
`asm/funcs/func_80078654.s` + `tools/decomp-permuter/prelude.inc` (drop the
`.set gp=64` line for r3000) so the function sits at offset 0 and the score
is the real weighted diff. Honour the fresh-seed stopping rule
([[permuter-fresh-seed-discipline]]) and WAIT for it in-turn
(`permuter_campaign.py wait --dir <ws>`) — never end a turn on a live
campaign.

## [s1] The fade-value clamp's v/v_copy pair plus the inline __asm__ move is the wrong shape; the target's clamp is a single SHORT-typed local, whose sign test GCC 2.7.2 emits as sll-16 + bgez with a copy around the destructive sll.
- mechanism: A `short` local forces the sign test onto the low half-word (shift-left-16 then test the sign bit) instead of a direct bltz, and forces GCC to preserve the un-shifted value in a second pseudo because the sll clobbers its source; the restoring move is then duplicated into the branch delay slot and the else-arm by jump2/reorg.
- probe: Replaced `s32 v_copy; ...; __asm__ volatile("move %0, %1" : "=r"(v_copy) : "r"(v)); if ((s32)(v << 16) < 0) v_copy = 0;` with `s16 sv; sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15); if (sv < 0) sv = 0;` and deleted the now-unused `s32 v;`. Measured with `sandbox func_80078654 --disable all` plus a normalized target-vs-build disassembly differ.
- result: Sandbox floor 23 -> 19; build instruction count 115 -> 116, exactly matching target; the whole clamp region (target insns 38-46) now matches instruction-for-instruction; the function now contains zero inline asm.
- verdict: CONFIRMED

## [s1] Rewriting the trailing goto-loop as a real `while` loop restores the front-end loop notes, so flow.c weights the walk pointer's reg_n_refs by loop_depth and flips the $s0/$s1 allocation inversion.
- mechanism: flow.c:2081,2329,2515,2725 do `reg_n_refs[regno] += loop_depth`, and basic_block_loop_depth is derived solely from NOTE_INSN_LOOP_BEG/END (flow.c:456,471), which a goto-spelled loop never emits — so today every in-loop reference is counted at weight 1.
- probe: Rewrote `goto check; loop: ...; check: if (var_s0[1] != -1) goto loop;` as `while (var_s0[1] != -1) { ... }` and re-ran the sandbox and the disassembly differ.
- result: Floor 23 -> 38 (worse). loop.c ROTATES the while-loop: it duplicates the exit test to the top (lw v1,4(sN); li v0,-1; beq) and deletes the target's entry `j .L800787F8`, giving 121 insns vs 116, a fourth live callee-save ($s3), and frame growth 88 -> 96 bytes. The allocation inversion was not fixed either — the walk pointer still landed in $s1. Banked as rejected/while-loop-triggers-loop-rotation.c.
- verdict: KILLED

## [s1] Some loop-depth reference weighting w exists at which the walk pointer outranks the arg0 pointer in allocno_compare, so the inversion is reachable by making GCC see the loop.
- mechanism: allocno_compare (tools/gcc-2.7.2/global.c) sorts by floor_log2(n_refs)*n_refs/live_length*10000*size and find_reg then hands out callee-saves in ascending hard-reg order, so the higher-priority allocno takes $s0.
- probe: Read the two pointers' in-loop / out-of-loop reference split directly off the target bytes (arg0 = 7 + 6w, var_s0 = 1 + 4w) with measured live lengths 98 and 91, and evaluated allocno_compare's formula over all w >= 1.
- result: arg0 stays 1.6x to 2.0x ahead of var_s0 at every weight. Reference weighting is arithmetically incapable of producing the flip. Do not re-run any loop-note / loop-depth variant for this purpose.
- verdict: KILLED
