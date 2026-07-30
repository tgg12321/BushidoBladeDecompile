# Evidence bank — func_80086014

Function: `func_80086014`, `src/main.c:990`. Routed **C** by the canonical gate
(`asm_insns 0`, `total 27`, "pure-C distance 16 <= 50 — pure-C target").
Target: `asm/funcs/func_80086014.s`, 27 instructions, leaf (no `jal`).

## Floor history

| session | modality | floor (`sandbox --disable all`) | note |
|---|---|---|---|
| — | (inherited) | 16 | pin/barrier-laden body, 31 built insns |
| 1 | recon | **10** | clean pure C; 25 built insns vs 27 target |
| 2 | structural | **1 (bytes MATCH)** | single-base `+ 1` pair spelling; 27/27 insns, full-build SHA1 == oracle; the residual 1 is an unlinked-object relocation artifact |

## SESSION 2 RESULT — the function byte-matches from pure C

Body (also in `candidate.c`, in place at `src/main.c:990`):

```c
s32 func_80086014(s16 idx, s16 x, s16 y)
{
    if ((u16)idx < 0x18) {
        D_80102A78[idx * 8 + 1] = y;
        D_80102A78[idx * 8] = x;
        D_800F65E0[idx] |= 3;
        return 0;
    }
    return -1;
}
```

`engine build` with this body: SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` ==
oracle, **MATCH**. The linked disassembly at `0x80086014` reproduces all 27 words
of `asm/funcs/func_80086014.s` (`tmp/grind/func_80086014/s2/linked.txt`). Zero
rules, zero pins, zero inline asm, zero volatile, zero dead code.

**Why the sandbox still prints 1.** The cheat-invisible sandbox scores an
UNLINKED object. Our y-store assembles as `sh $a2,%lo(D_80102A78+2)($at)` — an
immediate of 2 plus a relocation — while the target dump carries the resolved
`%lo(D_80102A7A)` = 0x2A7A. `ld` resolves `D_80102A78+2` to `0x80102A7A`, so the
emitted word is `a4262a7a` in both. One instruction, one point, zero real
difference. Any future session that sees "score 1" here should NOT treat it as
remaining work.

## The mechanism of the phantom frame — SETTLED (session 2)

Breaking on `assign_stack_local` under gdb (`tmp/grind/func_80086014/s2/gdb.sh`,
run against the in-tree witness) gives the definitive backtrace:

```
assign_stack_local (mode=SImode, size=4, align=-1) at function.c:672
alter_reg (i=78, from_reg=-1) at reload1.c:2352
reload (first=..., global=1, ...) at reload1.c:658
global_alloc (file=0x0) at global.c:580
```

So an 8-byte phantom frame is **reload spilling a pseudo that `global_alloc`
never allocated**. In the witness that pseudo (78) does not appear in the `.greg`
"Register dispositions" line at all, and `.lreg` classifies it
`ST_REGS or none; pointer` — because its ONLY surviving reference is a bare
`(use (reg:SI 78))` insn that **combine** created when it folded that pseudo's
address arithmetic into the following memory reference's `%lo` displacement.
A use-only pseudo cannot be given a GR_REGS hard reg, so `alter_reg` gives it a
stack slot; `get_frame_size()` counts 4, `MIPS_STACK_ALIGN` rounds to 8, and no
load or store is ever emitted. **No RTL dump stage (`.rtl` … `.dbr`) contains a
single `virtual-stack-vars` reference** — the frame is invisible in RTL, which is
why session 1 could only find it by reading cc1's `.frame` comment.

The C shape that produces the fold: **two accesses to the SAME base symbol whose
offsets differ by a constant carried INSIDE the variable index expression**
(`arr[v1]` and `arr[v1 + 1]`). The delta must survive to RTL — if it is a
constant field offset that the tree folds (`tbl[idx].y`, a struct-typed table),
combine has nothing to fold and there is no leftover pseudo (measured: all eight
struct-object-model variants in `p7.c` report `vars= 0`).

## Corpus witnesses, resolved (closes session 1's H2)

`build/bb2.map` maps the census's two smallest phantom-frame witnesses:
`0x80047E5C` = `snd_GetFadeCurve` (`src/sound.c:993`) and `0x8007DEE4` =
`bb2_memset_8007DEE4`. `snd_GetFadeCurve` is pure C with only `s32` scalars — it
carries the frame purely from `g_snd_fade_curve[v1]` + `g_snd_fade_curve[v1 + 1]`
captured into named locals. Re-compiled standalone it reports `vars= 8`
(`tmp/grind/func_80086014/s2/p1.c` variant `w1`); with the two multiplies replaced
by adds it reports `vars= 16` (four temps instead of two). This is the evidence
that killed session 1's "aggregates only" reading of the frame axis.

## Frame-axis measurements, session 2 (86 variants over 9 grids)

Everything below reports cc1's own `.frame` comment; `insns` counts the emitted
body INCLUDING the two frame instructions, so the winning signature is
`insns=18, vars=8, sp_traffic=0` against a `vars=0` baseline of 16.

- `p1.c` — witness transplant: `w1` (snd_GetFadeCurve) 8; `w2` (same, adds) 16;
  `w3` (memset loop) 0; `w4` (our baseline) 0; `w5` 0.
- `p2.c` — block nesting / guard inversion / local count on OUR function: all
  eight `vars= 0`. **Brace layout and local count are inert; KILLED.**
- `p3.c` — witness bisect: index from a param still 8 (`b2`), no guard still 8
  (`b3`), one multiply 0 (`b1`), single array read 0 (`b5`), memory-loaded index
  on our body 0 (`b6`).
- `p5.c` — fine bisect: **`d6` = `s32 a = arr[v1]; s32 b = arr[v1+1]; return a+b;`
  gives 8 with only 11 insns** — the minimal trigger. `d7` (one array read, two
  locals) 0. `b4` (same math inline, no locals) 0.
- `p6.c` — pointer/offset/staging locals on our function: all eight 0.
  **Named address locals do NOT substitute for the fold; KILLED.**
- `p7.c` — struct-typed table object model (`Slot D_80102A78[]`, `.x`/`.y`,
  slot pointers, whole-Pair aggregate copy): all `vars= 0` except the two that
  spill for real (`h4`, `h8`: `sp_traffic` 3-4). **Tree-folded field offsets are
  frame-inert; KILLED.**
- `p8.c` — return-value / control-flow shapes (shared return local, `goto end`,
  ternary+comma, else-return, flag pointer, byte-offset pair): all nine 0.
  **KILLED.**
- `p9.c` — the mechanism spelled onto our function: `j1` (`[idx*8+1]` then
  `[idx*8]`) **18 insns / vars=8 / sp_traffic=0 and the body otherwise unchanged**;
  `j2` (x first) 8; `j3` (offset in a local) 8; `j7` (unsigned offset) 8;
  `j8` 8; `j6` 8 but +4 insns; `j4`/`j5` (pointer spellings of the same delta) 0.

- [s2] func_80086014 BYTE-MATCHES from pure C. The single structural change from session 1's floor-10 body: write the y half as element +1 of the SAME base symbol (D_80102A78[idx*8+1]) instead of through the interior splat symbol D_80102A7A. Full build SHA1 == oracle; linked disassembly reproduces all 27 target words; zero rules, zero pins, zero inline asm, zero volatile, zero dead code.

- [s2] The sandbox reports score 1 for the MATCHING body because it scores an unlinked object: %lo(D_80102A78+2) carries immediate 0x0002 plus a relocation where the target dump carries the resolved 0x2A7A. ld resolves them to the same word (a4262a7a). A future session must not treat that 1 as remaining work - check the linked build before grinding.

- [s2] MECHANISM OF THE 8-BYTE PHANTOM FRAME (gdb backtrace on assign_stack_local): it is reload1.c:alter_reg spilling a pseudo that global_alloc never allocated, because combine had folded that pseudo's address arithmetic into the next memory reference's %lo displacement and replaced the insn with a bare (use (reg)). A use-only pseudo is classified 'ST_REGS or none' in .lreg, cannot get a GR_REGS hard reg, gets a spill slot, and get_frame_size() counts it (4 -> 8 via MIPS_STACK_ALIGN) with zero traffic emitted. No RTL dump stage contains any virtual-stack-vars reference, which is why the frame is invisible outside cc1's .frame comment.

- [s2] The C shape that triggers that fold: TWO accesses to the SAME base symbol whose offsets differ by a constant carried INSIDE the variable index expression (arr[v1] and arr[v1+1]). If the delta is a struct field offset the tree folds away (tbl[idx].y), combine has nothing to fold and no frame appears - all eight struct-object-model variants measured vars=0.

- [s2] Session 1's H2 witnesses are resolved via build/bb2.map: 0x80047E5C = snd_GetFadeCurve (src/sound.c:993), 0x8007DEE4 = bb2_memset_8007DEE4. snd_GetFadeCurve is pure C with only s32 scalars and carries the phantom frame purely from g_snd_fade_curve[v1] + g_snd_fade_curve[v1+1] captured into named locals (vars=8; with adds instead of multiplies, vars=16). Aggregates were never required for the frame.

- [s2] KILLED on our function (measured vars=0 for every variant): block nesting / guard inversion / local count (8 variants), named pointer or offset staging locals (8), the struct-typed table object model incl. slot pointers and aggregate copies (8, the two that changed the frame spilled for real), and return-value / control-flow restructurings incl. shared return local, goto end, ternary+comma and else-return (9).

- [s2] The single-base +1 spelling also closed BOTH of session 1's scheduling placements for free: the extra address pseudo changes sched1's dependence graph, so the flag lbu issues after the first sh and `move $v0,$zero` lands right after the lbu, exactly as target has them. No barrier, no volatile, no pin.

- [s2] func_80086130 (src/main.c:1033, still queued, still pin-laden) writes the same two fields pre-scaled by 129 and has the identical 8-byte phantom frame. The single-base `+ 1` spelling is the first thing to try there - it was NOT touched this session (out of scope).

## What the function does (semantic model, recovered session 1)

A 24-entry table (`0x18` is the guard bound) with a **16-byte stride**. Each
slot holds two `s16` fields at the addresses splat named `D_80102A78` and
`D_80102A7A` — hence the `[idx * 8]` s16-element spelling of a 16-byte stride.
A parallel byte array `D_800F65E0` holds one flag byte per entry; this function
ORs in bits 0-1 to mark the slot dirty. Returns 0 on success, -1 if the index
is out of range.

Family (all three touch the same two fields + the same flag byte):
- `func_80086014` (this one, **queued**) — writes the pair **raw**.
- `func_80086130` (`src/main.c:1033`, **queued**, also pin-laden) — writes the
  same pair pre-scaled by 129 (`(v<<7)+v`). **Has the identical 8-byte phantom
  frame.**
- `func_80086080` (`src/main.c:1014`, **matched, pure C**) — the getter; calls
  `func_8008BD88(idx, &raw1, &raw2)` and divides both by 129.

So the setter/getter pair round-trips through a ×129 fixed-point scale, and
this function is the unscaled variant.

## The inherited body was a net negative, not just inert

The body at session start carried five `register asm("$N")` pins, a
`__asm__ volatile("" ::: "memory")` scheduling barrier, and a `volatile`
local. The cheat-invisible sandbox strips the pins and the barrier, but the
stripped `volatile` local survives as an ordinary volatile, forcing the flag
value through the stack: the stripped build emitted six `sw`/`lw` spill
instructions at `0(sp)` and scored **16**. Deleting all of it scored **10**.
The cheats were not merely score-inert — they were actively costing 6 points.

## Register allocation is ALREADY EXACT — the residual is not an RA problem

The naive pure-C spelling reproduces target's register assignment
instruction-for-instruction, including the non-obvious `move a3,a1` param-save
in the `bnez` delay slot. That copy exists because GCC computes the byte offset
`idx*16` into `$a1`, clobbering the incoming param, so it must save it first;
target does the same thing for the same reason. `$v1` = sign-extended index,
`$a1` = byte offset, `$a0` = flag byte, `$v0` = return value — all match.
No register-steering work is needed on this function.

## The entire residual, itemized (25 built vs 27 target)

1. **The phantom frame — 2 instructions, both of the 2 missing.** Target opens
   `addiu $sp,$sp,-8` and closes `addiu $sp,$sp,8` and never issues a single
   `$sp`-relative load or store. We emit no frame at all (`vars= 0`).
2. **Two scheduling placements.** We hoist `move v0,zero` to the top of the
   taken block (target: mid-block, right after the flag `lbu`), and we hoist
   the flag `lbu` above the first `sh` (target: after it). Both are cc1 sched1
   choices — the store to `D_80102A7A` provably cannot alias the load from
   `D_800F65E0` (distinct symbols), so GCC reorders freely. This placement was
   robust across all 20 variants probed this session, including the pin-laden
   inherited form.

## The frame axis, measured (the session's main result)

cc1 prints `get_frame_size()` itself, so the frame has a **direct gradient**
independent of the sandbox score: compile with the project's flags and read the
`.frame` comment (`vars=` IS `get_frame_size()`). Harness:
`tmp/grind/func_80086014/s1/{frame_probe.c,run_probe.sh}` — compiles a grid of
variants in one TU and reports insn count, `$sp`-traffic count, and `vars=` per
variant. Project cc1 flags (from `Makefile`; `main` is in neither `GP_FILES`
nor `NO_SR_FILES`): `-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1
-mno-abicalls -fno-builtin -w`.

**KILLED — local scalar spellings are completely inert here.** Ten spellings
(plain `s32`/`u8`/`s16`/`u16` locals, staged params, byte-offset pointer
spelling, shared return local, wider `s32` params with `(s16)` casts, and
notably the *documented* [[phantom-frame-slots-gcc272]] "minimal trigger" of
two HImode locals feeding an HImode bitwise expression) all reported
`vars= 0` and all emitted an identical 16-instruction body. Two of them were
additionally confirmed byte-identical through the full pipeline. **The
documented minimal trigger does not transplant into this function's context.**
Banked: `rejected/local-scalar-spellings-frame-inert.c`.

**CONFIRMED — a >=4-byte local AGGREGATE reproduces target's frame exactly.**
`union { s16 h[2]; s32 w; }` with field-wise stores gives
`vars= 8, regs= 0/0, args= 0` with **zero `$sp` traffic** — target's phantom
frame, from pure C, no dead declarations, no cheats. Two sub-findings:
- The aggregate must be **>= 4 bytes**: a 2-byte union gives `vars= 0`. 4 bytes
  rounds up to target's 8 via `MIPS_STACK_ALIGN`.
- **Zero traffic requires the aggregate collapse to ONE pseudo.** The union's
  `s32 w` view does that (SImode pseudo, fully forwarded). A plain 2-field
  struct accessed field-wise does not — GCC keeps two HImode subregs and spills
  them (`sp_traffic=3`).

**Rejected as a NET win (mechanism kept).** The same SImode collapse that buys
zero traffic forces a mode-punning pack/unpack round-trip costing 4-5
instructions (`andi`/`sll`/`or` … `sra`). We need to add exactly **2**. Full
gradient table and the emitted body:
`rejected/union-word-view-pack-unpack-tax.c`. Everything else about that form is
correct — it keeps the two separate `sh` stores at the right addresses, and cc1
emits the frame `subu` into the `bnez` delay slot where `prologue_fix` hoists it
to the top exactly as target has it.

## Corpus census — 21 pure-C witnesses for the phantom frame

`tmp/grind/func_80086014/s1/frame_census.py` scans all of `asm/funcs/*.s` for
functions that are leaf (no `jal`/`jalr`), adjust `$sp`, and never touch a
`$sp`-relative slot, cross-referenced against `engine/queue.json`. Result:
**38 such functions, 29 with frame==8, of which 21 are already matched
(not queued)** — so the phantom frame is an ordinary, frequently-reproduced
pure-C artifact in this tree, not an anomaly. This is strong independent
support for [[difficult-is-not-impossible]] on this function: 21 in-tree
pure-C functions already do the thing we are missing.

Two matched witnesses were readable and both share a shape: their guard/loop
bound is a **memory-loaded narrow (HImode) value** —
`md_option_reset_8004001C` / `md_option_reset_80040068` (`i < *(s16 *)a0`) and
`md_option_reset_800400B0` (`i < v1[0]`) in `src/config.c:516-538`. The
Judge-ruled comment immediately below them (`src/config.c:539`) names the
mechanism for a sibling: *"target's 0x28 frame is the combine-leftover of the
folded guard (phantom slot sp+20)"*. Probing that shape here (guard against a
memory-loaded `s16` bound) gave `vars= 0` — but our guard compares against a
**literal constant** on a register param, so a folded-guard leftover has
nothing to leave behind. Consistent with, not contrary to, the witness
evidence.

Caveat for whoever follows: the census's smallest witnesses
(`func_8007DEE4`, `func_8008339C`, `func_80083644`, `func_8004954C`,
`func_8007D308`, `func_80037AA4`, `func_80047E5C`) could not be located in
`src/` by name — `asm/funcs/` is a reference dump, not a build input, and these
compile under different (named) symbols. `engine diagnose` reported "not found
in any build/src/*.o (run `engine build` first)". **`func_80047E5C` is the
standout: matched, frame 8, and exactly 27 instructions — the same size as our
target.** Resolving it to its C body is cheap, un-done, and high-value.

## Surfaces deliberately not touched

No `regfix.txt` / `asmfix.txt` / `prologue_config.json` /
`inline_asm_canonical.txt` edits; no rules added; nothing committed. The only
tracked-file change is the `func_80086014` body in `src/main.c`.
`frame_fix_funcs.txt` does not exist at the repo root (only inside the
sandbox's generated `cfg/`), so there is no frame surface to be tempted by —
and adding one would be a build-time rewrite, i.e. a cheat by spelling.

- [s1] Floor moved 16 -> 10 this session. Built 25 insns vs target 27. Current src/main.c:990 body is 100% pure C: zero rules, zero pins, zero inline asm, zero volatile, zero dead code.

- [s1] The inherited pins/barrier/volatile were not merely score-inert, they were costing 6 points: the sandbox strips pins and __asm__ barriers but a 'volatile' local survives stripping and forced six sw/lw spills at 0(sp).

- [s1] Register allocation is ALREADY EXACT against target, including the non-obvious `move a3,a1` param-save in the bnez delay slot ($a1 is clobbered by the idx*16 byte-offset computation). $v1=index, $a1=offset, $a0=flags, $v0=return all match. No register-steering work is needed on this function.

- [s1] The entire residual is itemized: (1) the 8-byte phantom frame = both missing instructions; (2) two sched1 placements - we hoist `move v0,zero` to the top of the taken block (target: right after the flag lbu) and hoist the flag lbu above the first sh (target: after it). The lbu hoist was robust across all 20 variants probed, including the pin-laden inherited form.

- [s1] cc1 prints get_frame_size() itself, so the frame has a DIRECT gradient independent of the sandbox score: read the `.frame` comment (`vars=` IS get_frame_size). Project cc1 flags for main.c (neither GP_FILES nor NO_SR_FILES): -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w.

- [s1] Semantic model recovered: a 24-entry table (0x18 = the bound) with 16-byte stride; two s16 fields per slot at the addresses splat named D_80102A78/D_80102A7A (hence the [idx*8] s16-element spelling), plus a per-entry flag byte in D_800F65E0 whose bits 0-1 are ORed in to mark the slot dirty. Returns 0, or -1 out of range.

- [s1] Family: func_80086130 (src/main.c:1033) is the SAME setter with both values pre-scaled by 129 ((v<<7)+v); it is also queued, also pin-laden, and has the IDENTICAL 8-byte phantom frame - so the frame shape is a family property and solving it here should solve both. func_80086080 (src/main.c:1014) is the already-matched pure-C getter, which reads the pair back via func_8008BD88 and divides by 129.

- [s1] Corpus census (tmp/grind/func_80086014/s1/frame_census.py, over all asm/funcs/*.s cross-referenced with engine/queue.json): 38 leaf functions adjust $sp yet never touch a $sp-relative slot; 29 have frame==8; 21 of the 38 are ALREADY MATCHED. The phantom frame is an ordinary, frequently-reproduced pure-C artifact in this tree, not an anomaly - 21 in-tree pure-C functions already do the thing we are missing.

- [s1] func_80047E5C is the standout witness: matched, frame 8, and exactly 27 instructions - the same size as our target. It and the other small witnesses are not greppable in src/ by name because asm/funcs/ is a reference dump (not a build input) and they compile under named symbols; `engine diagnose` reports 'not found in any build/src/*.o (run `engine build` first)'. Resolving it is cheap and un-done.

- [s1] The dead-local-array frame-coercion carve-out is FORESEEABLY UNAVAILABLE here and should not be attempted: its 2026-07-01 gate requires the target bytes to contain corresponding dead stores, and this target has ZERO $sp traffic. Any frame fix must use a live, load-bearing aggregate.

- [s1] No forbidden surfaces touched: no regfix.txt / asmfix.txt / prologue_config.json / inline_asm_canonical.txt / .claude/rules / engine / tools / Makefile / *.ld edits, no rules added, no commits, no queue done / retire. frame_fix_funcs.txt does not exist at the repo root (only inside the sandbox's generated cfg/). The only tracked-file change is the func_80086014 body in src/main.c.
