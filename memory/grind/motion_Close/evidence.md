# Evidence bank — motion_Close

## Session 1 (recon, 2026-08-11)

### Baseline
- `canonical motion_Close` → verdict **C**, asm_insns 0, total 26, distance 21
  ("pure-C distance 21 <= 50 — pure-C target").
- `sandbox motion_Close --disable all` at HEAD → **score 21**, target_insns 26,
  **build_insns 13**, rules_dropped 9, cheat_asm_stripped 18.
- `diagnose` → LARGE, "21 differing insn(s) — deep restructure".
- Nine regfix rules at `regfix.txt:115-124`: a `$2 <-> $8` register swap over
  insns 0-4, an `insert "nop" @ 6`, a `reorder 2,3,5,4 @ 2-5`, and six `subst`
  rules that rewrite save/restore offsets (`$31,8`→`$16,4`; `,4(`→`,8(`;
  `$16,0(`→`$31,0xc(`; `,8(`→`,0xc(`; `,4(`→`,8(`; `,0(`→`,4(`).
  Every one of them is a frame-layout or register-name rewrite. There is no
  rule that adds, removes, or reorders a computation.

### The HEAD baseline of 21 was not a measurement of any C form
The committed body used `register void (**p)(void) asm("s0")`, `register s32
count asm("s1")`, `register void (*f)(void) asm("t0")`, plus
`__asm__ volatile("jalr %0" :: "r"(f) : "ra","memory")` and a hardcoded
`__asm__ volatile("addiu $17, $17, -1" : "=r"(count) : "0"(count))`. The
cheat-stripper removes the two `__asm__` blocks, which ARE the loop body, so
the stripped function collapses to 13 instructions with no call and no loop
work. **The 21 was an artifact of a degenerate stripped function.** Before this
session, no honest pure-C form of motion_Close had ever been measured.

### The target, decoded
`normalized_insns(build/src/ings2.o, "motion_Close")` (the oracle-matching
reference) is a textbook "walk a linker-provided table of function pointers"
loop:

```
 0 lui t0        1 lw t0,0(t0)        <- D_800A2668 guard, hoisted above the frame
 2 addiu sp,sp,-16
 3 sw s0,4(sp)   4 sw s1,8(sp)   5 sw ra,12(sp)
 6 beqz t0,end   7 nop
 8 lui s0        9 addiu s0        <- p     = &D_8008D070
10 lui s1       11 addiu s1        <- count = (s32)&D_00000000
12 beqz s1,end  13 nop
14 lw t0,0(s0)  15 addiu s0,s0,4   <- f = *p; p++
16 jalr t0      17 addiu s1,s1,-1  <- f(); count-- in the call delay slot
18 bnez s1,loop 19 nop
20 lw ra,12  21 lw s1,8  22 lw s0,4  23 addiu sp,sp,16  24 jr  25 nop
```

### The honest pure-C form reproduces the target STRUCTURE exactly
Body measured this session (banked as `candidate.c`):
`void (**p)(void); s32 count; if (D_800A2668 != 0) { p = &D_8008D070;
count = (s32)&D_00000000; if (count != 0) { do { void (*f)(void) = *p; p++;
f(); count--; } while (count != 0); } }`

→ **score 20, build_insns 25** (vs target 26). The CFG, the operation sequence,
the `count--` scheduled into the `jalr` delay slot, and the loop shape are all
identical. The entire 20-instruction "difference" is register names and stack
offsets — there is no missing or extra computation anywhere, and the single
instruction-count delta is the one unfilled delay slot described below.

### H1 (CONFIRMED KILL) — the target frame layout is unreachable from pure C
GCC 2.7.2's MIPS frame layout is `[outgoing args][vars][saved regs]`, with the
highest save (`ra`) at `args + vars + gp_size - 4`. Formula validated against
this session's own build, whose C is known to have zero stack locals:
args 16 + vars 0 + gp_size 12 - 4 = **24**, and the build emits `sw ra,24(sp)`. ✓

Applying it to the target: `ra@12`, gp_size 12 (s0, s1, ra) ⇒ **args + vars = 4**,
i.e. an outgoing-argument area of **ZERO**. But gcc-2.7.2's o32 backend defines
`REG_PARM_STACK_SPACE` = 16, so *every* C-level call forces at least 16 bytes of
outgoing-arg area. Minimum pure-C frame for this function is therefore
16 + 12 = 28 → aligned **32**, which is exactly what the honest form emits.
The target's 16 is not reachable.

Corroborated by census (`tmp/grind/motion_Close/s1/census.py`) over **all 854
call-making functions in the oracle-matching `build/src/*.o`**:

    implied (args+vars) distribution:
      4 -> 2 | 16 -> 538 | 20 -> 1 | 24 -> 101 | 32 -> 30 | 40 -> 21 | ...

**Zero** functions have a sub-16 arg area — except the two at 4, which are
`motion_Close` and its sibling `func_80083794` (motion_Open), in the same file.
Both reach it *only* because their `jalr` is inline asm: an `__asm__` block never
sets `current_function_outgoing_args_size`, so no arg area is reserved. In other
words the frame shape the target has is precisely the shape that the cheat
produces and that compiled C provably cannot. This axis is CLOSED — no future
session should spend time trying to shrink the frame from C.

### H2 (KILLED) — declaration order does not move the s0/s1 role assignment
Target: s0 = p, s1 = count. Honest C: s1 = p, s0 = count (roles swapped), and
the scalar temp is v0 where the target uses t0 (both for the guard load and for
the `jalr` target register). Both forms emit the two address materializations in
source order (p first), so emission order is not the discriminator. Probe:
declare `count` before `p`. Result: **score 20, build_insns 25 — unchanged, no
register moved.** Banked as `rejected/decl-order-swap-no-register-effect.c`.

### Two independent hand-written-asm signals (for a future escalation packet)
- **S-a — no outgoing-arg area despite making a call.** Unique in the corpus
  (2 of 854, and both are this function and its sibling). See H1.
- **S-b — an unfilled branch delay slot that GCC would have filled.** The target
  has `beqz t0 / nop` at insns 6-7 with all three register saves already placed
  above it. The honest C form places only two saves, then `beqz v0` with
  `sw s0,16(sp)` **in the delay slot** — GCC's `reorg.c` fills that slot because
  a pending save is available. The target leaves a `nop` there instead, which is
  what `regfix.txt:117` (`insert "nop" @ 6`) exists to re-create. A compiler does
  not waste that slot; a human hand-writing the prologue does.
- Neighborhood corroboration: `motion_Close` sits among `_start`, `InitHeap`
  (a hand-written `addiu $t2,$zero,0xA0 / jr $t2` BIOS trampoline),
  `func_800836B8` (contains a raw `.word 0x0000410D` syscall), `func_800836C8`
  and `bios_FileRead` — i.e. the PsyQ/BIOS crt0 runtime region. The body is the
  classic crt0 constructor/destructor-table walk, and `motion_Open`
  (`func_80083794`) is its once-only-guarded twin.
- NOTE: `tools/scan_hand_coded.py --single motion_Close` could **not** run —
  it requires `asm/funcs/motion_Close.s`, which does not exist because the
  function is C-routed. An escalation session must extract that file first to
  obtain a signal tier.

### Artifacts
- `tmp/grind/motion_Close/s1/dump.py` — side-by-side normalized-instruction dump
  of `build/src/ings2.o` (target) vs `tmp/sandbox/motion_Close/ings2.o` (build).
- `tmp/grind/motion_Close/s1/census.py` — the 854-function outgoing-arg-area
  census that established H1.

### Tree state at end of session 1
`src/ings2.c` was reverted to HEAD. The honest form is preserved in
`candidate.c` only, so the oracle and the `build/src/ings2.o` reference are
left intact for the next session.

- [s1] canonical motion_Close -> verdict C, asm_insns 0, total 26, distance 21, reason 'pure-C distance 21 <= 50'. The gate routed this function C on SIZE alone, having never seen an honest C measurement.

- [s1] sandbox --disable all at HEAD: score 21, target_insns 26, build_insns 13, rules_dropped 9, cheat_asm_stripped 18. build_insns 13 proves the stripped HEAD function is degenerate (no call, no loop).

- [s1] sandbox --disable all with the honest pure-C body: score 20, build_insns 25. New floor 20.

- [s1] Target decoded (normalized_insns of build/src/ings2.o): lui/lw t0<-D_800A2668; addiu sp,-16; sw s0,4/s1,8/ra,12; beqz t0; nop; lui/addiu s0<-&D_8008D070; lui/addiu s1<-&D_00000000; beqz s1; nop; loop: lw t0,0(s0); addiu s0,s0,4; jalr t0; addiu s1,s1,-1; bnez s1; nop; lw ra,12/s1,8/s0,4; addiu sp,16; jr; nop.

- [s1] All nine regfix rules (regfix.txt:115-124) are frame-layout or register-name rewrites only: a $2<->$8 swap over insns 0-4, an `insert "nop" @ 6`, a `reorder 2,3,5,4 @ 2-5`, and six `subst` rules rewriting save/restore offsets. Not one rule adds, removes, or reorders a computation — consistent with a body whose logic is exactly right and whose frame/registers are not.

- [s1] GCC frame-layout formula validated against a known-C build before being applied to the target: honest form has args 16 + vars 0 + gp_size 12 - 4 = 24 and emits `sw ra,24(sp)`.

- [s1] HAND-ASM SIGNAL S-a: no outgoing-arg area despite making a call — 2 of 854 functions in the corpus, and both are this function and its sibling, both only via inline asm. Impossible from compiled C.

- [s1] HAND-ASM SIGNAL S-b: the target's `beqz t0` at insn 6 has an UNFILLED delay slot (nop at 7) with all three register saves already placed above it. The honest C form places two saves then puts `sw s0,16(sp)` IN that delay slot — GCC's reorg.c fills the slot when a pending save is available. regfix.txt:117 (`insert "nop" @ 6`) exists solely to re-create the human's wasted slot.

- [s1] NEIGHBORHOOD: motion_Close sits among _start, InitHeap (a hand-written `addiu $t2,$zero,0xA0 / jr $t2` BIOS trampoline), func_800836B8 (raw `.word 0x0000410D` syscall), func_800836C8 and bios_FileRead — the PsyQ/BIOS crt0 runtime region. Its body is the classic constructor/destructor-table walk and func_80083794 (motion_Open) is its once-only-guarded twin.

- [s1] tools/scan_hand_coded.py --single motion_Close could NOT run: it requires asm/funcs/motion_Close.s, which does not exist because the function is C-routed. A future escalation session must extract that file first to obtain a signal tier — the canonical-asm gate needs a measured STRONG tier (S1/S2/S6), not an asserted one.

- [s1] Tree state: src/ings2.c reverted to HEAD at end of session (git status shows only metrics/events.jsonl and the untracked ledger dir), so the oracle and the build/src/ings2.o reference are left intact. The honest form lives in memory/grind/motion_Close/candidate.c.
