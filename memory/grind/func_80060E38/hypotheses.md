# Hypothesis ledger — func_80060E38

Floor history: s1 recon = **18** (baseline; no prior sessions).

## KILLED in session 1

### K1 — "the 18-instruction gap is a register-allocation or scheduling divergence"
**KILLED.** Disassembled our sandbox object and diffed against `asm/funcs/func_80060E38.s`
instruction by instruction: 139 vs 139, identical opcodes, identical operand registers,
identical order, identical delay-slot fills, identical prologue/epilogue save block, identical
frame size `0x70`. The ONLY difference is the immediate on the 18 spill `sw`/`lw` instructions
(`sp+4,12,…,68` vs `sp+0,8,…,64`). Consequence: **every RA / scheduling lever in the technique
index is inapplicable here.** Do not open `register-alloc-pure-c`, `.greg` allocno tuning,
dead-store RA steering, or permuter register search for this function — there is no register
diff to close.

### K2 — "the +4 shift is injected by a post-cc1 pipeline stage"
**KILLED.** `func_80060E38` is absent from `frame_fix_funcs.txt`, `prologue_config.json`,
`delay_slot_ra_funcs.txt`, `multu_funcs.txt`, `expand_lb_funcs.txt`. Compiling the isolated
probe with raw `cpp | cc1` (no prologue_fix, no maspsx, no regfix) already yields
`4,12,20,…,68`. The shift originates in cc1.

### K3 — "the spill slots are declared locals whose frame placement the C controls"
**KILLED.** The pre-reload RTL dumps (`in.i.rtl`, `in.i.lreg`) contain no stack MEMs and no
`virtual-stack-vars` references at all; `(reg:SI 29 sp)` first appears in `in.i.greg`. The
slots are created by `reload1.c:2352` (`alter_reg`) after register allocation. There is no
declaration-order or local-layout lever, because there are no locals.

## CONFIRMED in session 1

### C1 — the `+4` is `assign_stack_local`'s big-endian correction on a rounded spill slot
`reload1.c:2352` calls `assign_stack_local(SImode, 4, -1)`. The `align == -1` branch
(`function.c:681-684`) sets `alignment = BIGGEST_ALIGNMENT/8 = 8` **and rounds `size` up to 8**;
`function.c:702-703` then computes `bigend_correction = size - GET_MODE_SIZE(mode) = 8 - 4 = 4`.
`reload1.c:2353-2358` / `2405-2406` only cancel the correction when `total_size > inherent_size`
(here both are 4), so the correction survives. Slot base advances 0,8,16,… (the stride 8 both
builds show); the MEM sits at base+4.

### C2 — our cc1 is compiled BIG-ENDIAN, which is what arms C1
`BYTES_BIG_ENDIAN` is `(TARGET_BIG_ENDIAN != 0)`; cc1 rejects `-EL`/`-EB` so it is compiled in.
`int lowword(long long x){return (int)x;}` → `move $2,$5` (low word in the SECOND arg register)
and an `a:8/b:24` bitfield extracts the LOW 24 bits — both say big-endian, and the second
matches the already-documented `[[bitfield-direction-divergence]]` behaviour. Not new, not
fixable here (toolchain is frozen and off my edit surface).

### C3 — measured frame model
`spill offset = current_function_outgoing_args_size + frame_offset + 4`, frame_offset stepping
0,8,16,…  Measured by `probe_call.c`: adding a 5-arg call moves the whole block from
`{4,12,…,68}` to `{28,36,…,92}` — base moved by exactly the outgoing-args area, the `+4` did not
move. For our leaf, `outgoing_args_size = 0`, forcing `{4,12,…,68}`. Target needs `{0,8,…,64}`.

## LIVE FRONTIER for session 2 (ordered)

### F1 — resolve the `func_8006BD28` contradiction (DO THIS FIRST; it is decisive)
C1+C3 read naively say every SImode reload spill under our cc1 is at an offset `≡ 4 (mod 8)`,
which would make this function unreachable. **The tree falsifies that**: `func_8006BD28` is
CLEAN (zero rules, not queued ⇒ byte-matching under the oracle today) and its caller-save stack
slots are at `0x18` and `0x20` — stride 8, both `≡ 0 (mod 8)`. Our cc1 demonstrably emits those
bytes. So a C construct that reaches `≡ 0 (mod 8)` exists in this tree.
**Mechanism to test:** is it (a) `STARTING_FRAME_OFFSET` being the *unpadded*
`current_function_outgoing_args_size` (`mips.h:1651-1653` uses it raw; the `MIPS_STACK_ALIGN`
padding lives separately in `compute_frame_size`), so an arg area of `0x14` yields `0x14+4=0x18`
and the mod-8 generalization is simply false — or (b) those slots being `assign_stack_temp` /
declared-local slots that take the `align == 0` path (alignment 4, no size rounding,
`bigend_correction = 4-4 = 0` ⇒ `+0`)?
**Probe:** find `func_8006BD28`'s C body (grep `src/*.c`), extract it into the same isolated
harness, run `run.sh`, confirm it reproduces `0x18/0x20`, then read its `in.i.greg` for whether
those MEMs came from `alter_reg` (spills) or were present pre-reload (`in.i.lreg` stack MEMs).
Whichever answer comes back names the exact construct to transplant here. Cost: ~3 turns.

### F2 — force `bigend_correction` to 0 by making `total_size == GET_MODE_SIZE(mode)`
`bigend_correction = size - GET_MODE_SIZE(mode)` where `size` is the 8-rounded slot size. It is
zero iff the spilled pseudo's mode is 8 bytes wide (DImode) — but then the spill would be two
words, not the single `sw` target uses. The remaining, more promising variant: `reload1.c:2384`
takes a *different* path when a spill slot is **shared/reused** across pseudos
(`spill_stack_slot[from_reg]`, `spill_stack_slot_width`), with its own `adjust` arithmetic at
`2386-2400` that DOES compensate. **Probe:** vary the C so fewer distinct pseudos spill and the
same slot is reused (e.g. sink some constants through one reused local, or shorten the live
range of the hoisted `lui/ori` pairs by moving a few global stores earlier), then check whether
any slot lands at `≡ 0 (mod 8)`. Read `reload1.c:2360-2410` in full first — this session only
traced the `2352` path, not the `2384` shared-slot path.

### F3 — reduce the spill count so the block starts lower / disappears
The 9 spills exist because GCC hoists all 32 `lui/ori` constants above all 32 stores, exceeding
the register file. Splitting the body so constants are materialized nearer their stores would
cut spills — but note the target ITSELF spills 9 with the same 139-instruction stream, so any
form with a different spill count changes the instruction count and moves AWAY from the floor.
Treat F3 as a diagnostic for understanding the hoist, not as a closing lever, and measure with
`sandbox` before believing it.

## Guardrails for whoever picks this up
* This is NOT a register/scheduling problem (K1). Do not spend a session on RA levers.
* Never Read or recompile `src/text1b.c` whole (~125k tokens). Use
  `tmp/grind/func_80060E38/s1/probe.c` + `run.sh` — seconds per variant, full RTL dumps.
* The `+4` is a toolchain property, not a cheat surface. Do NOT respell it as a frame-padding
  local, a dead array, a `register asm()` pin, or anything else that manufactures the offset —
  all of those are the forbidden `dead-vars-local-array` / frame-coercion family, and the
  sandbox strips them anyway so they cannot move the floor.
* If F1 and F2 both come back dead AND the driver has assigned `escalation` modality, the honest
  framing is: a frozen-toolchain frame-layout artifact with no C lever, evidenced by C1/C2/C3.
  Until then this function is grindable and the outcome is `progress`.

## [s1] The 18-instruction gap is a register-allocation or instruction-scheduling divergence.
- mechanism: Standard plateau shape for this project: RA tiebreakers / sched1 hoists producing renamed registers or moved instructions.
- probe: Disassembled the sandbox object (tmp/sandbox/func_80060E38/text1b.o) and compared instruction-by-instruction against asm/funcs/func_80060E38.s.
- result: 139 build insns vs 139 target insns. Identical opcodes, identical operand registers ($fp/$t9/$t8/$s7..$s0/$t7..$t0/$a3/$a2/$v1), identical order, identical delay-slot fills, identical prologue save block (s0-s7,fp at 0x48-0x68), identical frame size 0x70. The ONLY differences are the 18 sp-relative immediates on the spill sw/lw pairs.
- verdict: KILLED

## [s1] The +4 offset shift is injected by a post-cc1 pipeline stage (prologue_fix / frame_fix / maspsx / regfix).
- mechanism: The engine pipeline has per-function gate lists that rewrite frame references.
- probe: Grepped func_80060E38 against frame_fix_funcs.txt, prologue_config.json, delay_slot_ra_funcs.txt, multu_funcs.txt, expand_lb_funcs.txt; then compiled an isolated 45-line reproducer with raw cpp|cc1 only (no prologue_fix, no maspsx, no regfix).
- result: Absent from every gate list. Raw cc1 output already emits 4,12,20,28,36,44,52,60,68($sp). The shift originates in cc1.
- verdict: KILLED

## [s1] The 9 stack slots are declared locals, so C-level declaration order / local layout can move them.
- mechanism: assign_stack_local for ordinary locals runs at expand time in declaration order, giving the C source a layout lever.
- probe: cc1 -da on the isolated reproducer; inspected in.i.rtl, in.i.lreg (pre-reload) and in.i.greg (post-reload) for stack MEMs and virtual-stack-vars references.
- result: Zero stack MEMs and zero virtual-stack-vars references pre-reload; (reg:SI 29 sp) first appears in in.i.greg. .greg reports 'Spilling reg 2.' and pseudos 84-105 in hard regs. The slots are created by reload1.c:2352 alter_reg after RA. There are no locals, hence no declaration-order lever.
- verdict: KILLED

## [s1] The +4 is assign_stack_local's big-endian correction applied to a size-rounded reload spill slot.
- mechanism: reload1.c:2352 calls assign_stack_local(SImode, 4, -1). The align==-1 branch (function.c:681-684) sets alignment = BIGGEST_ALIGNMENT/8 = 8 AND rounds size up to 8. function.c:702-703 then sets bigend_correction = size - GET_MODE_SIZE(mode) = 8-4 = 4. reload1.c:2353-2358 and 2405-2406 only cancel the correction when total_size > inherent_size (here both are 4), so it survives. Slot bases advance 0,8,16,... (the stride 8 both builds show) and each MEM sits at base+4.
- probe: Read function.c:666-745 and reload1.c:2337-2410 in tools/gcc-2.7.2/; confirmed BYTES_BIG_ENDIAN via two independent codegen probes (int lowword(long long x){return (int)x;} compiles to 'move $2,$5', taking the SECOND arg register as the low word; an a:8/b:24 bitfield extracts the LOW 24 bits, matching the documented bitfield-direction-divergence HIGH-first behaviour). cc1 rejects -EL/-EB outright, so endianness is compiled in.
- result: Mechanism confirmed end to end and consistent with every observed offset.
- verdict: CONFIRMED

## [s1] spill offset = current_function_outgoing_args_size + frame_offset + 4, with the +4 invariant and frame_offset stepping 0,8,16,...
- mechanism: STARTING_FRAME_OFFSET (mips.h:1651-1653) = current_function_outgoing_args_size for a non-ABICALLS target; frame_offset is assign_stack_local's running 8-aligned cursor; +4 is the bigend_correction from the previous hypothesis.
- probe: Built probe_call.c — identical body plus a trailing 5-argument call sink5(1,2,3,4,5), so the function is no longer a leaf — and re-ran the same cpp|cc1 harness.
- result: Outgoing-arg store landed at 16($sp) and the spill block moved from {4,12,...,68} to {28,36,44,52,60,68,76,84,92}: base moved by exactly the outgoing-args area (24), stride stayed 8, the +4 did not move. For our leaf, outgoing_args_size = 0, forcing {4,12,...,68}; target needs {0,8,...,64}.
- verdict: CONFIRMED

## [s2] func_8006BD28 proves a +0-mod-8 spill block is reachable from compiled C in this tree (session 1's F1).
- mechanism: Session 1 read func_8006BD28 as CLEAN (zero regfix/asmfix rules, not in engine/queue.json) with caller-save stack slots at 0x18/0x20, stride 8, congruent to 0 mod 8 — an in-tree existence proof that our big-endian cc1 can emit +0 spill slots.
- probe: Grepped src/*.c for func_8006BD28's body and cross-referenced inline_asm_canonical.txt, which session 1's spillscan.py did not consult.
- result: src/text1b.c:15814 is __asm__("glabel func_8006BD28\n" ...) — a whole-body canonical-asm authorization recorded at inline_asm_canonical.txt:338 ("wholesale-replaced the inner loop body with literal target instructions ... Not C-form-reachable in this fork. User-authorized 2026-06-07"). Its 0x18/0x20 slots are pasted target bytes, never compiler output. The census was blind to the canonical-asm bucket, so "zero rules + not queued" wrongly read as "compiled and matched".
- verdict: KILLED

## [s2] Some compiled, byte-matched function in this tree has a stride-8 reload-spill block at 0 mod 8.
- mechanism: If any clean function had one, the +4 would be a per-function accident rather than a toolchain invariant, and a transplantable C construct would exist.
- probe: Wrote tmp/grind/func_80060E38/s2/spillscan2.py — a stricter, canonical-asm-aware census. An offset counts as a spill slot only if the target function contains BOTH a caller-save `sw $r,O($sp)` and a caller-save `lw $r2,O($sp)` (arg homes are stored but never reloaded; callee-saves are s0-s8/ra); slots are then grouped into maximal stride-8 runs of length >= 3, and each function is classified CLEAN / RULES / QUEUED / CANONICAL-ASM.
- result: 29 stride-8 spill blocks across the whole game, in 29 distinct functions. ALL of them are at mod8 == 0 and ALL of them are in functions still carrying regfix/asmfix rules. Zero CLEAN. Zero at mod8 == 4. func_80060E38 is one of the 29 (n=9, [0,8,16,24,32,40,48,56,64]). Every function in Bushido Blade 2 whose target contains a genuine reload-spill block is still unmatched.
- verdict: KILLED

## [s2] A C-level structural transform (statement order, block scoping, declaration order, spill count, or mode mixing) can move the spill-slot congruence off 4 mod 8.
- mechanism: The mandated structural axis — the codegen-technique-index levers (block-local splits, declaration order, type narrowing/widening, statement re-association) applied to the 32-store body.
- probe: tmp/grind/func_80060E38/s2/gen.py generated 10 variants of the s1 reproducer (v_rev full order reversal, v_scope two block scopes, v_argsfirst / v_interleave moving the two indirect stores, v_more 40 stores, v_half 16 stores, v_ll one long long temp, v_llarr three long long temps, v_dbl a double temp, v_dipress 12 simultaneously-live long long products). Each was compiled through tmp/grind/func_80060E38/s1/run.sh (real build flags, cc1 -da) and its N($sp) offsets read off.
- result: EVERY spill in EVERY variant is congruent to 4 mod 8 (4,12,20,...). v_half and v_dipress produce no spills at all. Statement order, scoping, declaration order, spill count and the presence of 8-byte-mode values are all inert on the congruence.
- verdict: KILLED

## [s2] An SImode reload spill slot can be placed at an offset congruent to 0 mod 8 by any C input to our cc1.
- mechanism: reload1.c:2337-2410 (alter_reg) has three slot paths, of which session 1 traced only the fresh-slot line at 2352; the other two (the "allocate a bigger slot" branch and the shared-slot reuse branch at 2384) have their own big-endian adjust arithmetic that session 1 hypothesised might compensate.
- probe: Read reload1.c:2337-2410 and function.c:666-727 in full and derived the closed-form offset for every branch, then checked the derivation against all 12 compiled probes (s1 probe + probe_call + the 10 s2 variants).
- result: In both allocating branches the pre-adjust (inherent - total, or GET_MODE_SIZE(mode) - total) is cancelled exactly by the later `if (BYTES_BIG_ENDIAN && inherent_size < total_size) adjust += total_size - inherent_size`, for every total_size. So offset = STARTING_FRAME_OFFSET + 8k + (CEIL_ROUND(total,8) - GET_MODE_SIZE(M)), and since STARTING_FRAME_OFFSET is always a multiple of 8, offset === -GET_MODE_SIZE(M) (mod 8). A 4-byte mode is therefore ALWAYS at 4 mod 8. Only an 8-byte mode (DImode/DFmode) reaches 0 mod 8, and that would make the spill two words instead of the single sw/lw the target uses. The one residual route — the 2384 reuse branch, which returns the wider slot's address with no correction — requires from_reg != -1 (the pseudo hard-allocated then hard-spilled) AND a prior DImode spill from the same hard register; our 9 pseudos have reg_renumber < 0 and take the from_reg == -1 path, and the function has no 8-byte-mode values. Every measured probe agrees.
- verdict: KILLED

## [s2] The original PsyQ compiler places this function's spill slots at 0,8,...,64 from the same C source we are compiling.
- mechanism: If the divergence is the big-endian correction, then cc1psx (BYTES_BIG_ENDIAN == 0) fed identical input must produce the target's congruence while our fork produces +4 — a direct measurement of the fork divergence rather than an inference from reading GCC source.
- probe: tmp/grind/func_80060E38/s2/psx.sh pipes the byte-identical preprocessed input s1/dumps/probe/in.i into tools/cc1psx_wrapper.sh with the equivalent flags (-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -w), then compares offsets, frame size and instruction count; s2/norm_diff.py does an offset-normalized positional comparison of the two instruction streams.
- result: cc1psx exits 0 and emits `subu $sp,$sp,112` with slots at 0,8,16,24,32,40,48,56,64 — exactly target's — against our 4,12,...,68 from the same input, with the same 136 cc1-level instructions, the same 0x70 frame and the same callee-save block at 72..104. CONFIRMED. Qualifier: cc1psx does not otherwise reproduce target from this C — the streams agree for 31 instructions and then diverge in ORDER (cc1psx interleaves the global stores earlier; 67 positional mismatches), and it is OUR fork whose schedule and register allocation match target exactly. The counter-exhibit is evidence about slot congruence only.
- verdict: CONFIRMED

## [s2] The nine stack slots could be DECLARED LOCALS (align == 0 path, bigend_correction 0) instead of reload spills, giving a C-controlled layout at 0 mod 8.
- mechanism: assign_stack_local's align == 0 branch takes alignment = GET_MODE_ALIGNMENT(SImode)/8 = 4 and does NOT round size, so bigend_correction = 4 - 4 = 0 and the slot lands at +0 — the correction that produces the +4 never fires for ordinary locals. If a C form could hold the nine constants in address-taken locals, the block would start at +0.
- probe: tmp/grind/func_80060E38/s2/v_locals.c — nine s32 locals initialised to the scratchpad constants, each address-taken (sink(&x)) to force them to memory, then stored to the globals; compiled through s1/run.sh.
- result: Offsets 16,20,24,28,32,36,40,44,48 — base = outgoing_args_size (16, the function now calls sink), and **stride 4**, not 8. The align == 0 path does place locals without the +4, but it also does not round the slot size, so a block of SImode locals is contiguous at stride 4 with alternating congruence. Target needs nine slots at **stride 8, every one congruent to 0** — the signature of the align == -1 spill path, which is exactly the path that carries the +4. The two properties (stride 8 and congruence 0) are mutually exclusive for 4-byte values under this cc1: stride 8 implies align == -1 implies +4. Reaching stride 8 with locals would require 8-byte-aligned local objects (long long / double), i.e. the forbidden dead-vars-local-array frame-coercion family, and it would also change the stores from single sw to two-word accesses.
- verdict: KILLED

## LIVE FRONTIER for session 3 (revised — F1/F2/F3 are all closed)

The structural, order, scoping, mode and spill-count axes are measured dead, and the
mechanism is now proved from the compiler source and confirmed against the original
compiler. What remains is not a search over C forms for THIS function; it is a class-level
question about the 29 functions whose targets spill.

### G1 — is the +4 removable at a legitimate, non-cheat, in-tree surface?
The offset is `CEIL_ROUND(total,8) - GET_MODE_SIZE(M)` from `function.c:702-703`, armed by
`BYTES_BIG_ENDIAN`. Everything that could change it (the cc1 binary, its configuration, the
Makefile, prologue_config.json, regfix/asmfix) is outside the grind edit surface, and
[[no-compiler-divergence]] forbids the compiler-side fix. So this is an operator/owner
surface, not a grind surface. The honest next step is an OWNER-ESCALATION describing the
class (29 functions, all spilling, none matched, one measured mechanism) — but note the
existing carve-out does NOT apply: `.claude/rules/fork-divergence-inline-asm.md` (Ruling-2,
2026-07-13) requires a cc1 SIGSEGV and explicitly excludes "our fork compiles the faithful
source but emits different bytes". A fresh ruling would be needed.

### G2 — the remaining untried modalities on the ladder
`forensics` (instrumented cc1 at tools/gcc-2.7.2/cc1 — see [[instrumented-cc1-location]];
dump `assign_stack_local` call sites live to see whether any real BB2 translation unit ever
takes the slot-reuse path at reload1.c:2384 with a wider pre-existing slot — the single
remaining `=== 0` route the source proof leaves open), `rederive` and `synthesis`. Note the
obvious rederive question is already answered: `v_locals.c` measured that declared locals
give stride **4**, not target's stride 8, so "the slots are locals, not spills" is dead
(stride 8 for a 4-byte value implies the align == -1 path implies the +4).

### G3 — do NOT re-open
* Register allocation / scheduling (K1, s1) — our stream already matches target exactly.
* Post-cc1 pipeline stages (K2, s1) — the +4 is pre-maspsx.
* Declaration order / statement order / scoping / spill count (s2) — 10 measured variants.
* The func_8006BD28 "existence proof" (s2) — it is authorized canonical asm.
* Frame-padding locals, dead arrays, register pins to manufacture the offset — the forbidden
  frame-coercion family, and score-inert under the cheat-invisible sandbox anyway.

## [s2] func_8006BD28 proves a +0-mod-8 spill block is reachable from compiled C in this tree (session 1's decisive frontier item F1).
- mechanism: Session 1 read func_8006BD28 as CLEAN (zero regfix/asmfix rules, absent from engine/queue.json) with caller-save stack slots at 0x18/0x20, stride 8, both congruent to 0 mod 8 — an in-tree existence proof that our big-endian cc1 can emit +0 spill slots, which would falsify the 'every spill is 4 mod 8' generalization.
- probe: Grepped src/*.c for func_8006BD28's body and cross-referenced inline_asm_canonical.txt, which session 1's spillscan.py never consulted.
- result: src/text1b.c:15814 is __asm__("glabel func_8006BD28\n" ...) — a whole-body canonical-asm authorization recorded at inline_asm_canonical.txt:338 ('wholesale-replaced the inner loop body with literal target instructions ... Not C-form-reachable in this fork. User-authorized 2026-06-07'). Its 0x18/0x20 slots are pasted target bytes, never compiler output. The census was blind to the canonical-asm bucket, so 'zero rules + not queued' wrongly read as 'compiled and matched'.
- verdict: KILLED

## [s2] Some compiled, byte-matched function in this tree has a stride-8 reload-spill block at 0 mod 8.
- mechanism: If any clean function had one, the +4 would be a per-function accident rather than a toolchain invariant, and a transplantable C construct would exist.
- probe: tmp/grind/func_80060E38/s2/spillscan2.py — a stricter, canonical-asm-aware census over all asm/funcs/*.s. An offset counts as a spill slot only if the target contains BOTH a caller-save 'sw $r,O($sp)' and a caller-save 'lw $r2,O($sp)' (an outgoing-arg home is stored but never reloaded; callee-saves are s0-s8/ra); slots are grouped into maximal stride-8 runs of length >= 3; each function is classified CLEAN / RULES / QUEUED / CANONICAL-ASM.
- result: 29 stride-8 spill blocks across the whole game, in 29 distinct functions. ALL are at mod8 == 0 and ALL sit in functions still carrying regfix/asmfix rules. Zero CLEAN, zero at mod8 == 4. func_80060E38 is one of them (n=9, [0,8,16,24,32,40,48,56,64]). Every BB2 function whose target contains a genuine reload-spill block is still unmatched — this is a class-wide blocker, not a func_80060E38 quirk.
- verdict: KILLED

## [s2] A C-level structural transform (statement order, block scoping, declaration order, spill count, or mode mixing) can move the spill-slot congruence off 4 mod 8.
- mechanism: The mandated structural axis — the codegen-technique-index levers (block-local splits, declaration order, type narrowing/widening, statement re-association) applied to the 32-store body.
- probe: tmp/grind/func_80060E38/s2/gen.py generated 10 variants of the s1 reproducer (v_rev full order reversal, v_scope two block scopes, v_argsfirst and v_interleave relocating the two indirect stores, v_more 40 stores, v_half 16 stores, v_ll one long long temp, v_llarr three long long temps, v_dbl a double temp, v_dipress 12 simultaneously-live long long products). Each compiled through tmp/grind/func_80060E38/s1/run.sh at real build flags with cc1 -da; N($sp) offsets read off each output.
- result: EVERY spill in EVERY variant is congruent to 4 mod 8 (4,12,20,...,68 for the 9-slot forms; 4,12,...,132 for v_more's 17 slots). v_half (16 stores) and v_dipress produce no spills at all. Statement order, scoping, declaration order, spill count and the presence of 8-byte-mode values are all inert on the congruence.
- verdict: KILLED

## [s2] An SImode reload spill slot can be placed at an offset congruent to 0 mod 8 by some C input to our cc1 (session 1's frontier item F2 — the shared-slot path at reload1.c:2384 compensates the big-endian correction).
- mechanism: reload1.c:2337-2410 (alter_reg) has three slot paths, of which session 1 traced only the fresh-slot call at 2352; the 'allocate a bigger slot' branch and the shared-slot reuse branch at 2384 have their own adjust arithmetic that session 1 hypothesised might compensate.
- probe: Read reload1.c:2337-2410 and function.c:666-727 in full, derived the closed-form offset for every branch, and checked the derivation against all 12 compiled probes (s1 probe + probe_call + the 10 s2 variants).
- result: In both allocating branches the pre-adjust (inherent_size - total_size, or GET_MODE_SIZE(mode) - total_size) is cancelled exactly by the later 'if (BYTES_BIG_ENDIAN && inherent_size < total_size) adjust += total_size - inherent_size', for every total_size. So offset = STARTING_FRAME_OFFSET + 8k + (CEIL_ROUND(total,8) - GET_MODE_SIZE(M)); STARTING_FRAME_OFFSET is always a multiple of 8, hence offset === -GET_MODE_SIZE(M) (mod 8). A 4-byte mode is therefore ALWAYS at 4 mod 8, and only DImode/DFmode reaches 0 — which would make the spill two words instead of the single sw/lw target uses. The one residual route, the 2384 reuse branch (it returns the wider slot's address with no correction), needs from_reg != -1 AND a prior DImode spill from the same hard register; our 9 pseudos have reg_renumber < 0 and take the from_reg == -1 path, and the function has no 8-byte-mode values. Every measured probe agrees with the closed form.
- verdict: KILLED

## [s2] The nine stack slots could be DECLARED LOCALS (align == 0 path, bigend_correction 0) rather than reload spills, giving a C-controlled layout at 0 mod 8.
- mechanism: assign_stack_local's align == 0 branch uses alignment = GET_MODE_ALIGNMENT(SImode)/8 = 4 and does NOT round the size, so bigend_correction = 4 - 4 = 0 and the slot lands at +0 — the correction that produces the +4 never fires for ordinary locals.
- probe: tmp/grind/func_80060E38/s2/v_locals.c — nine s32 locals initialised to the scratchpad constants, each address-taken via sink(&x) to force them to memory, then stored to the globals; compiled through s1/run.sh.
- result: Offsets 16,20,24,28,32,36,40,44,48: base = outgoing_args_size (16, since the probe now calls sink) and stride 4, NOT stride 8. The same align == 0 path that drops the +4 also drops the size rounding. Target needs nine slots at stride 8, every one congruent to 0 — the signature of the align == -1 spill path, which is exactly the path carrying the +4. For a 4-byte value the two properties are mutually exclusive under this cc1; reaching stride 8 with locals would require 8-byte-aligned local objects (long long / double), i.e. the forbidden dead-vars-local-array frame-coercion family, and would change the stores from single sw to two-word accesses.
- verdict: KILLED

## [s2] The original PsyQ compiler places this function's spill slots at 0,8,...,64 from the same C source we are compiling — i.e. the divergence is our fork's endianness configuration, measured rather than inferred.
- mechanism: If the +4 is the big-endian correction, then cc1psx (BYTES_BIG_ENDIAN == 0) fed byte-identical input must produce the target's congruence while our fork produces +4.
- probe: tmp/grind/func_80060E38/s2/psx.sh pipes the byte-identical preprocessed input s1/dumps/probe/in.i into tools/cc1psx_wrapper.sh with equivalent flags (-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -w) and compares offsets, frame size and instruction count; s2/norm_diff.py does an offset-normalized positional comparison of the two instruction streams.
- result: cc1psx exits 0 and emits 'subu $sp,$sp,112' with slots at 0,8,16,24,32,40,48,56,64 — exactly target's — against our 4,12,...,68 from the same input, with the same 136 cc1-level instructions, the same 0x70 frame and the same callee-save block at 72..104. HONEST QUALIFIER: cc1psx does not otherwise reproduce target from this C — the streams agree for 31 instructions then diverge in ORDER (cc1psx interleaves the global stores earlier; 67 positional mismatches, 34 difflib lines), and it is OUR fork whose schedule and register allocation match target exactly. The counter-exhibit is evidence about slot congruence only, not a drop-in that would match this function.
- verdict: CONFIRMED
