# Hypothesis ledger — func_80086014

Floor at end of session 1: **10** (from 16). Target 27 insns, we build 25.
Register allocation is already exact; the whole residual is the 8-byte phantom
frame (2 insns) plus two scheduling placements. See `evidence.md` for the
measurements behind everything below.

## Resolved this session

### KILLED — H0a: the phantom frame comes from ordinary scalar locals
Mechanism claimed: [[phantom-frame-slots-gcc272]] — GCC 2.7.2 allocates a stack
temp for a computation it later register-allocates away; `get_frame_size()`
counts it though no store is emitted. Its documented minimal trigger is two
HImode (`s16`) locals feeding an HImode bitwise expression.
Probe: 10 scalar-local spellings compiled with the project's exact cc1 flags,
reading cc1's `.frame` comment as the gradient.
Result: **all ten `vars= 0`**, all emitting an identical 16-insn body; two
confirmed byte-identical through the full pipeline. The documented trigger —
including its literal two-HImode-locals shape — does **not** reproduce in this
function's context, so the trigger is context-dependent and not transplantable.
Do not re-run this axis. Banked: `rejected/local-scalar-spellings-frame-inert.c`.

### KILLED — H0b: the guard folds into a combine leftover
Mechanism claimed: the matched phantom-frame witnesses in `src/config.c` all
guard on a **memory-loaded HImode** bound, and the Judge-ruled comment at
`src/config.c:539` attributes a sibling's phantom slot to "the combine-leftover
of the folded guard".
Probe: guard rewritten against a memory-loaded `s16` bound; also a two-condition
guard (`idx >= 0 && idx < 0x18`), and a `switch` on the guard.
Result: `vars= 0` for all three. Our guard compares a register param against a
**literal constant**, so there is no folded memory operand to leave a leftover.
This axis is closed for this function.

### CONFIRMED — H0c: a >=4-byte local aggregate reproduces target's frame
`union { s16 h[2]; s32 w; }` yields `vars= 8, regs= 0/0, args= 0` with **zero
`$sp` traffic** — target's exact phantom-frame signature — from pure C with no
dead declarations. Threshold is 4 bytes (a 2-byte union gives `vars= 0`), and
zero traffic requires the aggregate collapse to a single pseudo (the `s32 w`
view does; a field-wise 2-field struct spills instead, `sp_traffic=3`).
Rejected as a *net* win only because the SImode collapse forces a pack/unpack
round-trip costing 4-5 insns when we need to add exactly 2. Full gradient table:
`rejected/union-word-view-pack-unpack-tax.c`.

## Live frontier

### H1 — a >=4-byte aggregate that is already SImode-shaped (no pack/unpack)
**Mechanism.** H0c proved the frame is reachable and priced the failure
precisely: the tax is the mode punning between the aggregate's SImode pseudo and
the two HImode halves the table wants. An aggregate whose *natural* use in this
function is already word-shaped would pay nothing. Then frame 8 costs exactly
the 2 instructions target has.
**Next probe.** Aggregates whose members are `s32`/pointer rather than paired
`s16`, so there is nothing to pack: e.g. a 4-byte aggregate carrying the byte
offset or the slot pointer (`struct { Slot16 *p; } s;` / `union { s32 off; u8
b[4]; }`) genuinely consumed by the address arithmetic; or a 4-byte aggregate
consumed by the flag read/modify/write path (the flag is already widened to a
word by `lbu`, so an aggregate holding it may forward with no punning — note the
round-3 `union {u8 b[4]; s32 w;}` flag variant reported `sp_traffic=0` but
`vars= 0`, so it needs its size/liveness tuned, not the idea abandoned).
Gradient: the `run_probe.sh` harness reports `vars=`, `sp_traffic=`, and insn
count for ~10 variants per run — keep using it rather than the sandbox score,
since it separates "wrong frame" from "wrong codegen".
**Hard constraint.** The aggregate must be live and load-bearing. Solving this
by declaring an unused/unwritten aggregate is the forbidden dead-local-array
frame coercion ([[dead-vars-local-array]]); its narrow 2026-07-01 carve-out
requires the target bytes to contain the corresponding dead stores, and this
target has **zero** `$sp` traffic, so that gate fails on its face.

### H2 — resolve `func_80047E5C`, a matched witness with our exact signature
**Mechanism.** Not a codegen hypothesis but the cheapest high-value lookup left.
The census found 21 already-matched pure-C functions with phantom frames;
`func_80047E5C` is matched, frame 8, and **exactly 27 instructions** — the same
size as our target. Its C body is a working pure-C recipe for the thing we are
missing, in this tree, under this toolchain.
**Next probe.** It is not greppable in `src/` by name (`asm/funcs/` is a
reference dump; these compile under named symbols, and `engine diagnose` says
"not found in any build/src/*.o — run `engine build` first"). So: run
`engine build`, then map `0x80047E5C` to its owning object/symbol via the build
map or `objdump` over `build/src/*.o`, and read that C body. Same for the
11-insn `func_8007DEE4`. Cross-referencing `bb2.ld` may be faster.

### H3 — the two scheduling placements (`move v0,zero`, and the `lbu` hoist)
**Mechanism.** cc1's sched1 hoists the flag `lbu` above the first `sh` because
the store to `D_80102A7A` and the load from `D_800F65E0` are distinct symbols
and provably non-aliasing, so the scheduler launches the load early for latency.
Target issues it *after* that store. Likewise our `move v0,zero` floats to the
top of the taken block while target emits it right after the `lbu`.
**Next probe.** Make the flag read genuinely dependent on address arithmetic
only available after the first store, so a real data dependence — not a barrier
— pins the order: derive both the pair address and the flag address from one
walked/advanced pointer. [[walking-pointer-serializes-parallel-loads]] is the
documented shape for exactly this symptom (parallel independent loads/stores
getting reordered; fixed by making the addressing serial).
**Priority note.** Second-order: worth ~2-3 of the 10, less than H1/H2, and it
may fall out for free once the frame shape is right, since a frame-carrying
aggregate changes the dependence graph. Do **not** reach for a memory clobber or
`volatile` — both are cheats, and both were in the inherited body that scored
*worse*.

## Standing notes for future sessions

- The inherited pins/barrier/volatile were not merely inert, they cost 6 points.
  If a future session is tempted by a pin here: RA is already exact without one.
- `func_80086130` (the ×129 twin, `src/main.c:1033`) has the identical 8-byte
  phantom frame and is also queued and also pin-laden. Whatever resolves H1
  should resolve both — check before assuming per-function work.

## [s1] func_80086014's inherited register pins, memory barrier and volatile local were score-inert scaffolding that could be deleted for free.
- mechanism: The cheat-invisible sandbox strips register asm() pins and __asm__ barriers before scoring, so they cannot move the honest floor. But a 'volatile' local is NOT stripped as a pin - it survives as ordinary volatile semantics and forces the value through memory.
- probe: Replaced the whole body (5 register asm("$N") pins + __asm__ volatile("" ::: "memory") + a volatile local) with the naive pure-C spelling; ran `sandbox func_80086014 --disable all` and objdumped the cheat-stripped main.o before and after.
- result: Not free - strictly BETTER. Floor 16 -> 10; built insns 31 -> 25 (target 27). The stripped volatile local had been forcing six sw/lw spills at 0(sp). The clean form also reproduces target's register allocation instruction-for-instruction, including the non-obvious `move a3,a1` param-save in the bnez delay slot (GCC computes the byte offset idx*16 into $a1, clobbering the param). Register allocation is therefore NOT part of the remaining residual.
- verdict: CONFIRMED

## [s1] Target's 8-byte phantom frame (addiu $sp,-8 with zero $sp traffic) comes from ordinary scalar locals that GCC 2.7.2 reserves frame bytes for and then register-allocates away.
- mechanism: The documented [[phantom-frame-slots-gcc272]] artifact: GCC 2.7.2 allocates a stack temp for a computation it later register-allocates away; get_frame_size() counts it but no store is emitted. Its recorded minimal trigger is two HImode (s16) locals feeding an HImode bitwise expression.
- probe: Built a 10-variant grid in one TU (tmp/grind/func_80086014/s1/frame_probe.c) compiled with the project's exact cc1 flags (-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w) and read cc1's own .frame comment, where `vars=` IS get_frame_size() - a direct frame gradient that separates 'wrong frame' from 'wrong codegen', unlike the sandbox score. Variants: plain s32/u8/s16/u16 locals, the literal two-HImode-locals-feeding-a-bitwise-or trigger shape, staged params, byte-offset pointer spelling, shared return local, wider s32 params with (s16) casts.
- result: All ten reported `vars= 0, regs= 0/0, args= 0` and all emitted an IDENTICAL 16-instruction body; two were additionally confirmed byte-identical through the full pipeline (both score 10, both 25 built insns). The documented minimal trigger does not reproduce in this function's context - it is context-dependent, not transplantable. The local-scalar spelling axis has no gradient here at all.
- verdict: KILLED

## [s1] The phantom frame is a combine leftover of a folded narrow guard, as it is for the matched phantom-frame witnesses in src/config.c.
- mechanism: The two readable matched witnesses (md_option_reset_8004001C / _80040068 / _800400B0, src/config.c:516-538) all bound their loop on a memory-loaded HImode value, and the Judge-ruled comment at src/config.c:539 attributes a sibling's phantom slot to 'the combine-leftover of the folded guard (phantom slot sp+20)'.
- probe: Rewrote the guard three ways in the frame harness: against a memory-loaded s16 bound, as a folded two-condition range test (idx >= 0 && idx < 0x18), and as a switch on the guard expression.
- result: vars= 0 for all three. func_80086014's guard compares a register PARAM against a LITERAL CONSTANT ((u16)idx < 0x18), so there is no folded memory operand that could leave a leftover slot behind. Consistent with the witness evidence rather than contrary to it, and closed for this function.
- verdict: KILLED

## [s1] A local aggregate of >=4 bytes reproduces target's exact phantom-frame signature from pure C.
- mechanism: GCC 2.7.2 calls assign_stack_local for a local aggregate at expand time, so get_frame_size() counts it (vars=4, rounded to 8 by MIPS_STACK_ALIGN). If every access to that aggregate is subsequently register-forwarded, the slot is never loaded or stored - yielding addiu $sp,-8 / addiu $sp,8 with zero $sp traffic, exactly target's shape, with no dead declarations and no cheats.
- probe: Two further harness rounds (18 more variants) over struct / union / array locals with both aggregate-assignment and field-wise stores, reporting vars=, $sp-traffic count and insn count per variant.
- result: CONFIRMED and priced. `union { s16 h[2]; s32 w; }` with field-wise stores gives vars= 8, regs= 0/0, args= 0 with sp_traffic=0 - target's exact frame signature - AND keeps the two separate `sh` stores at the right addresses, with cc1 emitting the frame `subu` into the bnez delay slot where prologue_fix hoists it to the top just as target has it. Two sub-findings: the aggregate must be >=4 bytes (a 2-byte union gives vars= 0), and zero traffic requires collapse to ONE pseudo (the s32 view forwards; a field-wise 2-field struct keeps two HImode subregs and spills, sp_traffic=3). Rejected only as a NET win: the same SImode collapse forces a mode-punning pack/unpack round-trip (andi/sll/or ... sra) costing 4-5 instructions when we need to add exactly 2.
- verdict: CONFIRMED
