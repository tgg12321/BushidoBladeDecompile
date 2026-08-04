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

## [s3] alter_reg's slot-REUSE branch (reload1.c:2363-2367) can be triggered from pure C, giving an SImode reload spill an offset congruent to 0 (mod 8) — the one opening session 2's closed-form proof left.
- mechanism: The reuse branch is the only path in alter_reg that neither calls assign_stack_local nor applies any adjust: it takes `x = spill_stack_slot[from_reg]` with adjust == 0, and reload1.c:2410-2415 then re-MEMs it in the pseudo's own mode at offset 0 because the slot's mode differs. If the pre-existing slot was allocated for an 8-byte mode its bigend_correction was 8 - 8 = 0, so its base is ≡ 0 (mod 8) and the reusing SImode pseudo inherits that base. Reachability was established this session: from_reg != -1 comes from exactly one site (reload1.c:3499 in spill_hard_reg), whose non-elimination caller is reload1.c:2253 — reload commandeering a hard register as a spill register, announced by the "Spilling reg N." line at reload1.c:2232 that is already present in this function's own .greg dump.
- probe: Three generators, 23 compiled variants, all through tmp/grind/func_80060E38/s1/run.sh at the real build flags with cc1 -da, plus a classifier that separates a WORD slot (single-word sw/lw whose +4 neighbour is unused — target's shape) from an 8-byte slot, and extracts every "Spilling reg N." from each .greg dump. s3/gen3.py: v_di9 (9 long longs live across the store block), v_dihalf (long longs consumed as 32-bit halves), v_reload (address/reload pressure with no 8-byte modes), v_di_reload, v_union (union { long long; s32[2]; } address-taken locals), v_dbl9 (9 doubles), v_maxpress. s3/reuse_probe.py: v_scoped_2_12, v_scoped_3_16 (long longs die, then a nested scope opens SImode temps meant to inherit their hard registers). s3/reuse_probe2.py: 15 LEAF shapes (call-free like the target) v_leafA_{1,2,3}_{0,4,8} and v_leafB_{1,2,4}_{4,10}, varying how many 8-byte-mode values exist, whether they die before or live across the 32-store block, and how many SImode temps compete for the same caller-saves.
- result: ZERO single-word spill slots at 0 mod 8 in all 23 variants; every SImode reload spill is at 4 mod 8. This is not for want of triggering spill_hard_reg — the .greg dumps show reload commandeering registers 8/9, 24/25, 16/17, 12/13 and 2 across the family. v_dihalf is the sharpest: its DImode slots ARE at 0 and 8 (target's congruence) but each is accessed as two words (0/4, 8/12) while the nine SImode spills alongside sit at 20,28,36,.... The reason the branch never fires is structural: it needs an SImode pseudo and an 8-byte-mode pseudo CO-ALLOCATED to one commandeered hard register, and they never co-allocate, because global alloc places the 8-byte value in a register pair while the nine leftover constant pseudos are by construction the ones it could not place at all (reg_renumber < 0 => from_reg == -1). Statement order, scoping, mode mixing, extra pressure and leaf-vs-call shape were all inert on this.
- verdict: KILLED

## [s3] Even if the reuse branch did fire, it could produce func_80060E38's nine 0-mod-8 slots.
- mechanism: spill_stack_slot is an array indexed by HARD REGISTER, and it is written only in the "allocate a bigger slot" branch (reload1.c:2399). So each commandeered hard register can donate at most one reusable slot, and only if an 8-byte-mode pseudo was spilled from it first.
- probe: Counted what the target actually needs against what the mechanism can supply, and cross-checked against the measured probes: target asm/funcs/func_80060E38.s has nine single-word spill slots at 0,8,16,24,32,40,48,56,64 and a 139-instruction stream of lui/ori plus gp-relative sw with no 8-byte-mode instruction anywhere; every s3 probe shows reload commandeering at most one register pair (one 8-byte object) regardless of how much pressure is applied.
- result: Nine 0-mod-8 slots would require nine commandeered hard registers, each carrying its own prior long long / double pseudo. Each such pseudo mandates its own materialisation plus a two-word spill store, so any C supplying them necessarily destroys the 139 == 139 instruction alignment the current body already achieves. The route is arithmetically incompatible with the target independently of whether it can be triggered.
- verdict: KILLED

## [s3] The "allocate a bigger slot" branch (reload1.c:2369-2401) reaches 0 mod 8 when the slot mode is widened to DImode.
- mechanism: When spill_stack_slot[from_reg] already exists in a wider mode, that branch widens `mode` to the slot's mode before calling assign_stack_local, and assign_stack_local(DImode, 8, -1) has bigend_correction = 8 - GET_MODE_SIZE(DImode) = 0, i.e. it returns a base at 0 mod 8. Session 2 asserted the branch cancels but did not walk the mode-widening sub-case.
- probe: Re-derived the branch line by line from tools/gcc-2.7.2/reload1.c:2369-2415 and function.c:665-728.
- result: The widened call does return base+0 and `adjust = GET_MODE_SIZE(mode) - total_size` is 0, but reload1.c:2405 then fires because inherent_size (4) < total_size (8) and adds total_size - inherent_size = 4, so the SImode MEM is rebuilt at base+4. The branch restores the +4 by a different route. Session 2's closed form holds for this sub-case too.
- verdict: KILLED

## LIVE FRONTIER for session 4 (revised — the structural axis is now closed end to end)

The three branches of alter_reg are now individually accounted for: two allocating branches
by closed form (s2, re-derived and extended in s3) and the one non-allocating branch by 23
measured C variants plus an independent counting argument (s3). There is no remaining
C-level route by which a 4-byte reload spill reaches an offset ≡ 0 (mod 8) in this fork.
What is left is class-level, not a search over C forms for this function.

### H1 — the class disposition (unchanged from s2's G1, now with the last opening closed)
29 functions tree-wide have a stride-8 reload-spill block; all are at 0 mod 8, all still
carry regfix/asmfix rules, none has ever matched. The mechanism is one measured fork
property (BYTES_BIG_ENDIAN = 1 in our decompals cc1, 0 in the cc1psx that built the game).
Every surface that could change it is off the grind edit surface, and
[[no-compiler-divergence]] forbids the compiler-side fix. Whichever session is eventually
assigned `escalation` modality should file the CLASS in docs/grind/decisions.md citing
s2/spillscan2.txt, the closed-form proof, the cc1psx counter-exhibit, and now the s3
reuse-branch kill — and must NOT cite Ruling-2, whose scope limit explicitly excludes
non-crashing fork divergences. The driver has not declared exhaustion, so this must not be
escalated before that modality is assigned.

### H2 — forensics (still untried, and now cheaper to scope)
The instrumented cc1 at tools/gcc-2.7.2/cc1 (NOT build/cc1 — [[instrumented-cc1-location]])
could log every alter_reg call across the real src/*.c translation units with
(pseudo, from_reg, mode, total_size, chosen path, final offset). s3 has narrowed what that
would be worth: the interesting count is how often the 2363 REUSE path is taken tree-wide
and with what modes. If it never fires anywhere in BB2, the congruence has no C lever at
all and the class is closed by measurement as well as by proof. Note s3 already answered
the narrower question for this function's shape (23 variants, never taken), so forensics
should be scoped to the tree, not to func_80060E38, and should instrument small TUs first —
never recompile src/text1b.c whole.

### H3 — rederive (the one sub-case never measured)
assign_stack_temp-created slots (compiler temporaries for aggregate copies) take neither the
align == -1 spill path nor the plain declared-local path. A rederive session could express
the 32 constants as an aggregate initialisation or a memcpy-shaped construct and read the
emitted slot stride. Expect the instruction count to move away from 139; measure with
sandbox --disable all before believing anything. This is the last untried slot-creation
route in function.c.

### H4 — do NOT re-open
* Register allocation / scheduling (s1 K1) — our stream already matches target exactly.
* Post-cc1 pipeline stages (s1 K2) — the +4 is pre-maspsx.
* Declaration order / statement order / scoping / spill count / mode mixing (s2, 11
  variants; s3, 23 more) — measured dead twice, from two different angles.
* The func_8006BD28 "existence proof" (s2) — it is authorized canonical asm.
* long long / double / union mode mixing and added register pressure to trigger the reuse
  branch (s3) — measured dead and arithmetically insufficient.
* Frame-padding locals, dead arrays, register pins to manufacture the offset — the
  forbidden frame-coercion family, and score-inert under the cheat-invisible sandbox.

## [s3] alter_reg's slot-REUSE branch (reload1.c:2363-2367) can be triggered from pure C, giving an SImode reload spill an offset congruent to 0 (mod 8) — the single opening session 2's closed-form proof left open.
- mechanism: The reuse branch is the only path in alter_reg that neither calls assign_stack_local nor applies any adjust: it takes x = spill_stack_slot[from_reg] with adjust == 0, and reload1.c:2410-2415 re-MEMs it in the pseudo's own mode at offset 0 because the slot's mode differs. If the pre-existing slot was allocated for an 8-byte mode, its bigend_correction was 8 - 8 = 0, so its base is congruent to 0 mod 8 and the reusing SImode pseudo inherits that base. Reachability was established this session: from_reg != -1 comes from exactly one site (reload1.c:3499 inside spill_hard_reg), whose non-elimination caller is reload1.c:2253 — reload commandeering a hard register as a spill register, announced by the 'Spilling reg N.' line printed at reload1.c:2232, which is already present in this function's own .greg dump.
- probe: Three generators, 23 compiled variants, all through tmp/grind/func_80060E38/s1/run.sh at the real build flags with cc1 -da, plus a classifier separating a WORD slot (single-word sw/lw whose +4 neighbour is unused — target's shape) from an 8-byte slot, and extracting every 'Spilling reg N.' from each .greg dump. s3/gen3.py: v_di9 (9 long longs live across the store block), v_dihalf (long longs consumed as 32-bit halves), v_reload (address/reload pressure, no 8-byte modes), v_di_reload, v_union (union { long long; s32[2]; } address-taken locals), v_dbl9 (9 doubles), v_maxpress. s3/reuse_probe.py: v_scoped_2_12, v_scoped_3_16 (long longs die, then a nested scope opens SImode temps meant to inherit their hard registers). s3/reuse_probe2.py: 15 LEAF shapes (call-free like the target) v_leafA_{1,2,3}_{0,4,8} and v_leafB_{1,2,4}_{4,10}.
- result: ZERO single-word spill slots at 0 mod 8 in all 23 variants; every SImode reload spill is at 4 mod 8. Not for want of triggering spill_hard_reg — the .greg dumps show reload commandeering registers 8/9, 24/25, 16/17, 12/13 and 2 across the family. v_dihalf is the sharpest: its DImode slots ARE at 0 and 8 (target's congruence) but each is accessed as two words (0/4, 8/12) while the nine SImode spills alongside sit at 20,28,36,.... The branch never fires because it needs an SImode pseudo and an 8-byte-mode pseudo CO-ALLOCATED to one commandeered hard register, and they never co-allocate: global alloc places the 8-byte value in a register pair while the nine leftover constant pseudos are by construction the ones it could not place at all (reg_renumber < 0, hence from_reg == -1). Statement order, scoping, mode mixing, added pressure and leaf-vs-call shape were all inert.
- verdict: KILLED

## [s3] Even if the reuse branch did fire, it could supply func_80060E38's nine 0-mod-8 spill slots.
- mechanism: spill_stack_slot is an array indexed by HARD REGISTER, written only in the 'allocate a bigger slot' branch (reload1.c:2399). Each commandeered hard register can therefore donate at most one reusable slot, and only if an 8-byte-mode pseudo was spilled from it first.
- probe: Counted what the target needs against what the mechanism can supply, cross-checked against the measured probes: target asm/funcs/func_80060E38.s has nine single-word spill slots at 0,8,16,24,32,40,48,56,64 and a 139-instruction stream of lui/ori plus gp-relative sw with no 8-byte-mode instruction anywhere; every s3 probe shows reload commandeering at most one register pair (one 8-byte object) regardless of applied pressure.
- result: Nine 0-mod-8 slots would require nine commandeered hard registers, each carrying its own prior long long/double pseudo. Each such pseudo mandates its own materialisation plus a two-word spill store, so any C supplying them necessarily destroys the 139 == 139 instruction alignment the current body already achieves. The route is arithmetically incompatible with the target independently of triggerability.
- verdict: KILLED

## [s3] The 'allocate a bigger slot' branch (reload1.c:2369-2401) reaches 0 mod 8 when the slot mode is widened to DImode — a sub-case session 2 asserted but did not walk.
- mechanism: When spill_stack_slot[from_reg] already exists in a wider mode, that branch widens `mode` to the slot's mode before calling assign_stack_local, and assign_stack_local(DImode, 8, -1) has bigend_correction = 8 - GET_MODE_SIZE(DImode) = 0, returning a base at 0 mod 8.
- probe: Re-derived the branch line by line from tools/gcc-2.7.2/reload1.c:2369-2415 and function.c:665-728.
- result: The widened call does return base+0 and adjust = GET_MODE_SIZE(mode) - total_size is 0, but reload1.c:2405 then fires because inherent_size (4) < total_size (8) and adds total_size - inherent_size = 4, rebuilding the SImode MEM at base+4. The branch restores the +4 by a different route; session 2's closed form holds for this sub-case too.
- verdict: KILLED

## [s4] A randomized machine search over C forms (decomp-permuter) can find a spelling of func_80060E38 whose reload spill slots land at 0 mod 8, beating the floor of 18.
- mechanism: The permuter mutates the C (statement order, temporaries, types, expression re-association, inlining, variable reuse) at a scale no hand-designed variant set can reach. Sessions 2 and 3 killed the structural axis with 34 targeted variants plus a closed-form reading of reload1.c/function.c, but a proof read from compiler source is worth confirming against a large sample of forms nobody thought to write down. If ANY C form escaped the align == -1 spill path while keeping the 139-instruction stream, a randomized search over ~10^5 mutations is the instrument most likely to hit it.
- probe: Built and VALIDATED a fast permuter chassis (tmp/grind/func_80060E38/s4/mkws.sh -> s4/ws): compile.sh runs the real pipeline (cpp | cc1 | prologue_fix | maspsx with the Makefile's exact sdata/expand-lb/multu/label-nop flags | fix_lwl | multu_pad), extracts the .ent/.end region and assembles it; target.o is asm/funcs/func_80060E38.s with the r3000 prelude. Validation showed base.o vs target.o at 139 vs 139 instructions with exactly the 18 spill-offset lines differing — i.e. the chassis reproduces the engine's honest floor of 18. Two campaigns were then run through tools/permuter_campaign.py (which passes --stack-diffs by default; both launched at base_score 72, so sp offsets were scored rather than normalized away): chassis 1 `probe-chassis` (-j 8, the s1 absolute-constant body) and chassis 2 `base-arith-chassis` (-j 6, the same semantics written as `spbase = 0x1F800000` plus 32 `spbase + 0xNN` addends, a different mutation neighbourhood). Fresh-seed discipline was applied per basin: ~20 min and ~19 min windows respectively, each waited in-turn via `permuter_campaign.py wait`.
- result: 54,551 + 40,003 = 94,554 iterations, ZERO finds on either chassis — no output directory at any score below 72, not even an equal-score alternative. Both campaigns harvested with --stop and confirmed inactive in the registry. Chassis 2 additionally measured that GCC folds `spbase + 0xNN` back into absolute constants, giving the SAME 139 instructions and the SAME 4,12,...,68 offsets, so the arithmetic re-spelling is inert. The result is exactly what s2's closed form predicts (offset === -GET_MODE_SIZE(M) (mod 8), a function of the spilled pseudo's MODE and nothing the C controls) and it extends that proof with a machine-scale empirical sample.
- verdict: KILLED

## [s4] The permuter's default scorer is safe to use on func_80060E38.
- mechanism: Routine assumption — a permuter workspace scoring 0 means a byte match.
- probe: Read tools/permuter_campaign.py's documented rationale for passing --stack-diffs by default (owner directive 2026-07-13) and checked the base_score both campaigns reported on a workspace whose ONLY divergence from target is the sp-offset shift.
- result: The default scorer NORMALIZES sp-relative offsets away, so on this function it would score the current source 0 — a false match — while the true honest distance is 18. With --stack-diffs (the campaign wrapper's default) the same workspace correctly reports base_score 72. Raw `permuter.py <dir>` must never be used on func_80060E38 or on any of the 29 stride-8 spill-block functions; only tools/permuter_campaign.py.
- verdict: CONFIRMED

## LIVE FRONTIER for session 5 (revised — the permuter axis is now closed too)

Four axes are now measured dead: structural (s2, 11 variants), the alter_reg branch analysis
(s2 closed form + s3's reuse-branch kill, 23 variants), and randomized machine search (s4,
~94.5k mutations on two chassis). The mechanism is arithmetic in the spilled pseudo's mode and
is not addressable from C. What remains is one narrow measurement and one class disposition.

### I1 — forensics: does the reload1.c:2363 REUSE path EVER fire in Bushido Blade 2?
Unchanged from s3's H2 and still the best remaining probe. Use the instrumented cc1 at
tools/gcc-2.7.2/cc1 (NOT build/cc1 — [[instrumented-cc1-location]]) to log every alter_reg call
with (pseudo, from_reg, mode, total_size, chosen path, final offset) while compiling real
src/*.c translation units; count how often 2363 is taken and in what modes. Scope it to the
TREE (s3 already answered it for this function's shape) and instrument SMALL TUs first — never
recompile src/text1b.c whole. A tree-wide zero would close the congruence class by measurement
as well as by proof.

### I2 — rederive: assign_stack_temp-created slots
The last untried slot-creation route in function.c. Compiler temporaries for aggregate/structure
copies take neither the align == -1 spill path nor the plain declared-local path, so they may
carry no big-endian correction while still being 8-byte-strided. Express the 32 constants as an
aggregate initialisation or a memcpy-shaped construct and read the slot stride from the
s1/run.sh cc1 -da output. Expect the instruction count to move off 139; measure with
sandbox --disable all before believing anything.

### I3 — the class disposition (unchanged; NOT yet escalatable)
29 functions tree-wide have a stride-8 reload-spill block, all at 0 mod 8, all rule-carrying,
none ever matched. Whichever session is assigned `escalation` modality files the CLASS in
docs/grind/decisions.md citing s2/spillscan2.txt, the closed-form proof, the cc1psx
counter-exhibit, the s3 reuse-branch kill and now the s4 94.5k-mutation negative — and must NOT
cite Ruling-2, whose scope limit explicitly excludes non-crashing fork divergences. The driver
has not declared exhaustion, so this must not be escalated yet.

### I4 — do NOT re-open (added this session)
* Permuter campaigns on this function — 94,554 mutations across two chassis, zero finds.
* Raw `permuter.py` on this function — the default scorer false-matches at 0 (see the
  CONFIRMED entry above). Only tools/permuter_campaign.py, which passes --stack-diffs.
* Base-plus-offset arithmetic spellings of the 32 constants — GCC folds them back; identical
  139 instructions, identical 4,12,...,68 offsets.
* Everything in s3's H4 list (RA/scheduling, post-cc1 stages, order/scoping/spill count, the
  func_8006BD28 "existence proof", mode mixing for the reuse branch, frame-coercion cheats).

## [s4] A randomized machine search over C forms (decomp-permuter) can find a spelling of func_80060E38 whose reload spill slots land at 0 mod 8, beating the floor of 18.
- mechanism: The permuter mutates the C (statement order, temporaries, types, expression re-association, variable reuse) at a scale no hand-designed variant set can reach. Sessions 2-3 killed the structural axis with 34 targeted variants plus a closed-form reading of reload1.c/function.c, but a proof read from compiler source is worth confirming against a large sample of forms nobody thought to write down. If ANY C form escaped the align == -1 spill path while keeping the 139-instruction stream, a randomized search over ~10^5 mutations is the instrument most likely to hit it.
- probe: Built and VALIDATED a fast permuter chassis (tmp/grind/func_80060E38/s4/mkws.sh -> s4/ws): compile.sh runs the real build pipeline (cpp | cc1 | prologue_fix | maspsx with the Makefile's exact sdata/expand-lb/multu/label-nop flags | fix_lwl | multu_pad), extracts the .ent/.end region and assembles it; target.o is asm/funcs/func_80060E38.s assembled with the r3000 prelude. Validation: base.o vs target.o disassembled and diffed is 139 vs 139 instructions with EXACTLY the 18 spill-offset lines differing (sw/lw v0 at 4,12,...,68 vs 0,8,...,64) and nothing else, so the chassis reproduces the engine's honest floor of 18 bit for bit. Two campaigns were then run through tools/permuter_campaign.py (which passes --stack-diffs by default; both launched at base_score 72, so sp offsets were scored rather than normalized away): chassis 1 'probe-chassis' (-j 8, the s1 absolute-constant body) and chassis 2 'base-arith-chassis' (-j 6, the same semantics written as spbase = 0x1F800000 plus 32 spbase + 0xNN addends, a deliberately different mutation neighbourhood). Fresh-seed discipline applied per basin: ~20 min and ~19 min windows, each waited in-turn via permuter_campaign.py wait.
- result: 54,551 + 40,003 = 94,554 iterations, ZERO finds on either chassis - no output directory at any score below the 72 baseline, not even an equal-score alternative. Both campaigns harvested with --stop (procs_killed 9 on chassis 1) and confirmed alive:false / registered_active:false in the registry. Chassis 2 additionally measured that GCC folds spbase + 0xNN straight back into absolute constants, giving the SAME 139 instructions and the SAME 4,12,...,68 offsets, so the arithmetic re-spelling is codegen-inert. The outcome is exactly what session 2's closed form predicts (offset === -GET_MODE_SIZE(M) (mod 8) - a function of the spilled pseudo's MODE and of nothing the C controls) and extends that proof with a machine-scale empirical sample.
- verdict: KILLED

## [s4] The permuter's default scorer is safe to use on func_80060E38.
- mechanism: Routine assumption that a permuter workspace scoring 0 means a byte match.
- probe: Read tools/permuter_campaign.py's documented rationale for passing --stack-diffs by default (owner directive 2026-07-13) and checked the base_score both campaigns reported on a workspace whose ONLY divergence from target is the sp-offset shift.
- result: The permuter's default scorer NORMALIZES sp-relative offsets away, so on this function it would score the current source 0 - a FALSE MATCH - while the honest distance is 18. With --stack-diffs (the campaign wrapper's default) the same workspace correctly reports base_score 72. Raw `permuter.py <dir>` must never be used on func_80060E38 nor on any of the 29 stride-8 spill-block functions; only tools/permuter_campaign.py.
- verdict: CONFIRMED

## [s5] The aggregate / assign_stack_temp route (BLKmode slots, which function.c:702 exempts from the big-endian correction) can give func_80060E38 nine single-word stack slots at stride 8 and congruence 0 — the neighbourhood ~94.5k permuter mutations could not reach, because the randomizer never introduces an aggregate.
- mechanism: function.c:879 allocates an assign_stack_temp slot as assign_stack_local(mode, size, mode == BLKmode ? -1 : 0), so an aggregate takes the SAME align == -1 path as a reload spill (alignment 8, size CEIL_ROUNDed to 8); but function.c:702 computes bigend_correction only `if (BYTES_BIG_ENDIAN && mode != BLKmode)`, so the +4 that every SImode spill carries never fires for it. That is the one place in function.c where target's 8-byte frame behaviour and target's 0-mod-8 congruence coexist, and sessions 2-4 never entered it (s2's v_locals took the align == 0 path; s3 explored 8-byte MODES, not aggregates; s4's two permuter chassis were pure-scalar bodies and the randomizer has no aggregate-introducing operator).
- probe: tmp/grind/func_80060E38/s5/gen5.py generated 8 semantically-identical aggregate variants and compiled each through s1/run.sh at the real build flags with cc1 -da, classifying WORD slots (single-word sw/lw whose +/-4 neighbours are unused — target's shape) and their mod-8 congruence: v_structinit (32-field struct, brace-initialised), v_arrinit (s32 a[32] initialiser), v_structcopy (struct copied from a const extern aggregate — the memcpy shape), v_structret (struct returned by value, the genuine assign_stack_temp site at function.c:3736), v_structfield / v_structfield_at (9-field struct assigned field-by-field, keeping target's 32 lui/ori materialisations; the second address-taken via a pointer), v_structsmall (9-field aggregate + 23 scalars), and v_structpad (struct { s32 v; s32 pad; } p[32] — diagnostic only, since manufacturing frame padding is the forbidden dead-vars family).
- result: The mechanism is REAL — every BLKmode aggregate slot based at 0 mod 8, confirming the source reading — but it cannot produce target's shape. Brace-initialised aggregates collapse to a memcpy from a rodata image (91 insns; target's 32 lui/ori stream is gone). Field-assigned aggregates keep the stream (161 insns) and put the aggregate at 0..32 (0 mod 8) while the nine SImode reload spills alongside it stay at 44,52,...,108 — every one ≡ 4 (mod 8): an uncorrected BLKmode slot in the same frame does not perturb the corrected ones. v_structret gives no 0-mod-8 word slot and stops the function being a leaf. Only v_structpad reaches target's 0,8,16,... stride-8 word slots, and only by manufacturing padding (a cheat, score-inert under the cheat-invisible sandbox) at 184 insns vs 139. This closes the slot-creation taxonomy of function.c: spills (align -1) stride 8 / ≡ 4; locals (align 0) stride 4 / ≡ 0; aggregates (BLKmode) base ≡ 0 / members stride 4. For a 4-byte value, stride 8 and congruence 0 are mutually exclusive in this fork absent manufactured padding or 8-byte modes (killed in s3).
- verdict: KILLED

## [s5] Some permuter configuration — a different mutation neighbourhood, or a different weight profile — can find a form of func_80060E38 below the 18-instruction floor.
- mechanism: s4's negative rested on sample size (94,554 mutations) from two structurally similar scalar chassis under the stock weight profile. Two configurations were untested: a chassis seeded IN the aggregate neighbourhood (which the randomizer cannot reach on its own), and a weight profile that emphasises the declaration/type/temporary operators — the only legitimate operators that could change which pseudos reach reload.
- probe: (a) Read the permuter's whole operator set in tools/decomp-permuter/src/randomizer.py and identified every operator that could touch the frame. (b) Campaign A: label aggregate-chassis, dir s5/ws3, base = v_structfield (the aggregate neighbourhood), -j 8, base_score 5133 — 2,029 iterations. (c) Campaign B: label decl-type-weighted, dir s5/ws4, the validated near-floor s4 chassis (base_score 72) re-seeded with perm_pad_var_decl = 0.0 and perm_reorder_decls / perm_temp_for_expr / perm_randomize_internal_type / perm_struct_ref / perm_ins_block / perm_reorder_stmts boosted, -j 8 — 62,904 iterations over two fresh-seed windows (1113 s). Both waited in-turn via permuter_campaign.py wait and harvested with --stop.
- result: Campaign A produced 14 outputs, all in the 4851-5133 band — a different basin that never approached the 72 baseline (as expected: its aggregate slot is 0 mod 8 but stride 4). Campaign B produced ZERO finds in 62,904 iterations. More decisively, the operator audit is a STRUCTURAL kill: of 34 perm_* operators, exactly one targets the frame — perm_pad_var_decl (randomizer.py:2247, "Inserts an unused variable to adjust stack offsets") — and it is (i) a generator of the forbidden dead-vars/frame-padding family, so its finds could never be accepted, and (ii) inert regardless, because assign_stack_local does frame_offset = CEIL_ROUND(frame_offset, 8) before every align == -1 slot, erasing any sub-8-byte padding shift while a multiple-of-8 shift preserves the congruence. Every remaining operator is expression/statement-level and cannot move an offset fixed by GET_MODE_SIZE of the spilled pseudo. Running total across s4+s5: ~159,500 mutations on four chassis, no output below 72.
- verdict: KILLED

## [s5] Directed permuter (PERM_* annotation via tools/permuter_annotate.py) offers an untried probe for this function.
- mechanism: The mandated modality's brief names directed annotation as a distinct instrument from random campaigns, so it had to be checked rather than assumed unavailable.
- probe: tools/permuter_annotate.py --help / --list-hints, read against this function's shape (straight-line, call-free, loop-free, single-return, 32 constant stores).
- result: The hint catalog is register-asm-pins, shared-end-label, loop-rotation-two-shift, loop-counter-fills-load-delay. None applies: there is no loop, no multi-return, nothing to rotate, and register pins are a cheat family that the sandbox strips anyway. There is no directed-permuter probe left to run on func_80060E38.
- verdict: KILLED

## LIVE FRONTIER for session 6 (revised — the permuter axis is closed by construction, and the slot taxonomy is complete)

Five axes are now measured dead: structural (s2, 11 variants), the alter_reg branch analysis
(s2 closed form + s3's reuse-branch kill, 23 variants), randomized machine search (s4+s5,
~159.5k mutations on four chassis), the aggregate/assign_stack_temp slot route (s5, 8
variants), and directed permuter (s5, no applicable hint). Every slot-creation route in
`function.c` has now been individually measured, and none gives a 4-byte value stride 8 with
congruence 0.

### J1 — forensics: does the reload1.c:2363 REUSE path EVER fire in Bushido Blade 2?
Carried forward UNCHANGED from s3/H2 and s4/I1, and now the only measurement left that could
still surprise. Use the instrumented cc1 at tools/gcc-2.7.2/cc1 (NOT build/cc1 —
[[instrumented-cc1-location]]) to log every alter_reg call with (pseudo, from_reg, mode,
total_size, chosen path, final offset) while compiling real src/*.c translation units; count
how often 2363 is taken and in what modes. Scope it to the TREE (s3 answered it for this
function's shape, s4 and s5 for randomized and aggregate shapes) and instrument SMALL TUs
first — never recompile src/text1b.c whole. A tree-wide zero would close the congruence class
by measurement as well as by proof; a non-zero would name the exact C shape that reaches it.

### J2 — synthesis: the one untried modality on the ladder
`rederive` is now effectively spent (s5 measured its last sub-case, assign_stack_temp), so
after forensics the remaining ladder rung is `synthesis`. The honest synthesis question is not
"which C form" but "does the ledger, taken together, constitute exhaustion of the sanctioned
C-level search space for a frame-layout artifact whose offset is `-GET_MODE_SIZE(M) (mod 8)`".

### J3 — the class disposition (unchanged; still NOT escalatable by a non-escalation session)
29 functions tree-wide have a stride-8 reload-spill block, all at 0 mod 8, all rule-carrying,
none ever matched. Whichever session is assigned `escalation` modality files the CLASS in
docs/grind/decisions.md citing s2/spillscan2.txt, the closed-form proof, the cc1psx
counter-exhibit, the s3 reuse-branch kill, the s4+s5 ~159.5k-mutation negative, s5's
aggregate/taxonomy closure and s5's structural kill of the permuter's frame operator — and
must NOT cite Ruling-2, whose scope limit explicitly excludes non-crashing fork divergences.
The driver has not declared exhaustion, so this must not be escalated yet.

### J4 — do NOT re-open (added this session)
* Any aggregate / struct / array / memcpy / struct-return spelling — 8 measured variants; the
  BLKmode slot is 0 mod 8 but stride 4, and stride 8 needs manufactured padding (a cheat).
* Any further permuter campaign on this function, in any weight profile or chassis — the only
  frame-touching operator is a cheat generator and is arithmetically inert; ~159.5k mutations
  across four chassis, zero finds.
* Directed permuter — no applicable hint exists in the catalog for a straight-line, call-free,
  loop-free function.
* Everything in s4's I4 and s3's H4 lists.

## [s5] The aggregate / assign_stack_temp route (BLKmode slots, which function.c:702 exempts from the big-endian correction) can give func_80060E38 nine single-word stack slots at stride 8 and congruence 0 -- the neighbourhood session 4's ~94.5k permuter mutations could not reach, because the randomizer never introduces an aggregate.
- mechanism: function.c:879 allocates an assign_stack_temp slot as assign_stack_local(mode, size, mode == BLKmode ? -1 : 0), so an aggregate takes the SAME align == -1 path as a reload spill (alignment 8, size CEIL_ROUNDed to 8); but function.c:702 computes bigend_correction only 'if (BYTES_BIG_ENDIAN && mode != BLKmode)', so the +4 that every SImode spill carries never fires for it. That is the one place in function.c where target's 8-byte frame behaviour and target's 0-mod-8 congruence coexist. Sessions 2-4 never entered it: s2's v_locals took the align == 0 path, s3 explored 8-byte MODES rather than aggregates, and s4's two permuter chassis were pure-scalar bodies with no aggregate-introducing operator available.
- probe: tmp/grind/func_80060E38/s5/gen5.py generated 8 semantically identical aggregate variants, each compiled through tmp/grind/func_80060E38/s1/run.sh at the real build flags with cc1 -da, with a classifier extracting WORD slots (single-word sw/lw whose +/-4 neighbours are unused -- target's shape) and their mod-8 congruence: v_structinit (32-field struct, brace-initialised), v_arrinit (s32 a[32] initialiser), v_structcopy (struct copied from a const extern aggregate -- the memcpy shape), v_structret (struct returned by value, the genuine assign_stack_temp site at function.c:3736), v_structfield and v_structfield_at (9-field struct assigned field-by-field so target's 32 lui/ori materialisations survive; the second address-taken through a pointer), v_structsmall (9-field aggregate + 23 scalars), v_structpad (struct { s32 v; s32 pad; } p[32], diagnostic only since manufacturing frame padding is the forbidden dead-vars family).
- result: The mechanism is REAL -- every BLKmode aggregate slot based at 0 mod 8, confirming the source reading -- but it cannot produce target's shape. Brace-initialised aggregates collapse into a memcpy from a rodata image (91 insns; target's 32 lui/ori stream is destroyed). Field-assigned aggregates keep the stream (161 insns) and place the aggregate at 0..32 (0 mod 8) while the nine SImode reload spills alongside it remain at 44,52,...,108 -- every one congruent to 4 mod 8, so an uncorrected BLKmode slot in the same frame does not perturb the corrected ones at all. v_structsmall gives word slots 44,52,...,100 (all 4 mod 8). v_structret gives no 0-mod-8 word slot and stops the function being a leaf. Only v_structpad reaches target's 0,8,16,... stride-8 word slots, and only by manufacturing padding (a cheat, score-inert under the cheat-invisible sandbox) at 184 insns against target's 139. This closes the slot-creation taxonomy of function.c: reload spill (align -1) = stride 8 / 4 mod 8; declared local (align 0, s2 v_locals.c) = stride 4 / 0 mod 8; aggregate temp (BLKmode, s5) = base 0 mod 8 / members stride 4. For a 4-byte value, stride 8 and congruence 0 are mutually exclusive in this fork absent manufactured padding or 8-byte modes (killed in s3).
- verdict: KILLED

## [s5] Some permuter configuration -- a different mutation neighbourhood, or a different weight profile -- can find a form of func_80060E38 below the 18-instruction floor.
- mechanism: Session 4's negative rested on sample size (94,554 mutations) from two structurally similar scalar chassis under the stock weight profile. Two configurations were untested: a chassis seeded IN the aggregate neighbourhood (which the randomizer cannot reach on its own), and a weight profile emphasising the declaration/type/temporary operators -- the only legitimate operators that could change which pseudos reach reload.
- probe: (a) Read the whole operator set in tools/decomp-permuter/src/randomizer.py and identified every operator that could touch the frame. (b) Campaign A: label aggregate-chassis, dir tmp/grind/func_80060E38/s5/ws3, base = v_structfield, -j 8, base_score 5133, 2,029 iterations. (c) Campaign B: label decl-type-weighted, dir tmp/grind/func_80060E38/s5/ws4, the validated near-floor s4 chassis (base_score 72) re-seeded with perm_pad_var_decl = 0.0 and perm_reorder_decls / perm_temp_for_expr / perm_randomize_internal_type / perm_struct_ref / perm_ins_block / perm_reorder_stmts boosted, -j 8, 62,904 iterations over two fresh-seed windows (1113 s). Both campaigns waited in-turn via tools/permuter_campaign.py wait and harvested with --stop.
- result: Campaign A produced 14 outputs, all in the 4851-5133 band -- a different basin that never approached the 72 baseline, exactly as its measured frame shape predicts (aggregate slot at 0 mod 8 but stride 4). Campaign B produced ZERO finds in 62,904 iterations (finds_total 0, best_new_score null). More decisively, the operator audit is a STRUCTURAL kill: of 34 perm_* operators, exactly one targets the frame -- perm_pad_var_decl (randomizer.py:2247, docstring 'Inserts an unused variable to adjust stack offsets') -- and it is (i) a generator of the forbidden dead-vars/frame-padding family, so any find it produced could never be accepted, and (ii) inert regardless, because assign_stack_local does frame_offset = CEIL_ROUND(frame_offset, 8) before every align == -1 slot, erasing any sub-8-byte padding shift while a multiple-of-8 shift preserves the congruence. Every other operator is expression/statement-level and cannot move an offset fixed by GET_MODE_SIZE of the spilled pseudo. Running total across s4+s5: ~159,500 mutations on four chassis, no output below 72. Both s5 campaigns are stopped (procs_killed 9 on B) and show alive:false / registered_active:false in the registry.
- verdict: KILLED

## [s5] Directed permuter (PERM_* annotation via tools/permuter_annotate.py) offers an untried probe for this function.
- mechanism: The permuter modality brief names directed annotation as a distinct instrument from random campaigns, so it had to be checked rather than assumed unavailable.
- probe: tools/permuter_annotate.py --help / --list-hints, read against this function's shape (straight-line, call-free, loop-free, single-return, 32 constant stores plus 2 indirect stores).
- result: The hint catalog is register-asm-pins, shared-end-label, loop-rotation-two-shift, loop-counter-fills-load-delay. None applies: there is no loop to rotate, no multi-return to merge, no load-delay loop counter, and register pins are a cheat family the sandbox strips anyway. No directed-permuter probe remains for func_80060E38.
- verdict: KILLED

## [s6] The reload1.c:2363 slot-REUSE path — the only route in alter_reg with no big-endian correction, and the single opening sessions 2/3 left in the closed-form proof — fires SOMEWHERE in Bushido Blade 2, which would mean a 0-mod-8 single-word spill slot is reachable from compiled C in this fork.
- mechanism: alter_reg has three slot paths. path=1 (from_reg == -1, reload1.c:2352) and path=3 ("allocate a bigger slot", 2369) both call assign_stack_local(..., -1) and are proved by closed form to land a 4-byte value at 4 mod 8. path=2 (2363) instead takes x = spill_stack_slot[from_reg] with adjust == 0 and re-MEMs it at offset 0, inheriting the donor slot's base — so if a wider (8-byte) donor slot ever existed, a reusing SImode pseudo would inherit a 0-mod-8 base. Sessions 3-5 measured this dead for func_80060E38's SHAPE (23 targeted variants + ~159.5k permuter mutations), but the TREE-WIDE question — does the path fire anywhere in real BB2 code, and if so under what construct — was explicitly left open as the last measurement that could surprise.
- probe: Built an instrumented cc1 in a private copy of the GCC source (tmp/grind/func_80060E38/s6/gcc; tools/ untouched) with two env-gated traces in alter_reg — BB2ALTERREG on every slot allocation (function, pseudo, from_reg, path, mode size, inherent, total, adjust, final offset) and BB2ARCALL on every call with from_reg != -1 (the spill_hard_reg call site at reload1.c:3499) with (renumber, refs, equiv_constant, equiv_memory_loc). Validated codegen-inert: the rebuilt cc1's output on the s1 probe is byte-identical to stock build/cc1. Then swept ALL 31 src/*.c translation units at the Makefile's exact cpp defines and CC_FLAGS, piping stdin->stdout as the Makefile does (with `-o file`, GCC's exit-33 error path deletes the output, which silently truncated a first attempt). Aggregated with s6/agg.py and s6/agg2.py.
- result: 131 reload spill-slot allocations tree-wide, in 69 distinct functions across 15 TUs. path=1: 131. path=2: ZERO. path=3: ZERO. Every allocation is modesize 4 with inherent == total == 4, adjust == 0, and a final offset === 4 (mod 8); there is not one 4-byte-mode spill slot at any other congruence in the entire game. The complementary trace shows this is not because spill_hard_reg is dormant: it makes 98 alter_reg calls with from_reg != -1 across 51 functions in 10 TUs, but 97 of them have reg_renumber >= 0 (reload found the pseudo a hard register) and the 98th has reg_equiv_memory_loc set, so ZERO reach the slot-allocation block. spill_stack_slot[] is written only in the path=3 branch (reload1.c:2399), which therefore never executes, so path=2 — which reads that array — is unreachable BY CONSTRUCTION in this codebase, not merely unobserved. The congruence class is now closed by direct measurement of the compiler's own decisions as well as by the s2 closed form.
- verdict: KILLED

## [s6] The isolated s1/s2/s3/s5 probe harness might not be faithful to how func_80060E38 actually compiles inside src/text1b.c, leaving the whole five-session measurement chain resting on an unvalidated proxy.
- mechanism: Every variant campaign from s1 onward ran on a 45-line extracted reproducer compiled standalone, because src/text1b.c is ~490 KB preprocessed. If the real TU's declaration context changed pseudo numbering, spill count or slot placement, the ledger's conclusions would be about the harness rather than about the function.
- probe: Compiled the REAL src/text1b.c whole with the instrumented cc1 at build-identical flags and read off every BB2ALTERREG line for fn=func_80060E38.
- result: Exactly nine slot allocations, pseudos 75-83, all from_reg=-1 path=1 modesize=4 inherent=4 total=4 adjust=0, at offsets 4,12,20,28,36,44,52,60,68 — identical in count, path, mode and offset to the isolated probe's nine. The harness is faithful; sessions 1-5 measured the real thing.
- verdict: CONFIRMED (the harness-infidelity concern is killed)

## LIVE FRONTIER for session 7 (revised — the forensics axis is now closed too)

Five axes are measured dead: structural (s2, 11 variants), the alter_reg branch analysis
(s2 closed form + s3's reuse-branch kill, 23 variants), the slot-creation taxonomy in
function.c (s5's aggregate/BLKmode variants), randomized machine search (s4+s5, ~159.5k
mutations on four chassis), and now compiler forensics (s6: 131 traced slot allocations
tree-wide, 100% on the corrected path, and the uncorrected path proved unreachable because
spill_stack_slot[] is never populated in this codebase). No C-level lever remains and no
measurement remains that could surprise.

### J1 — synthesis (the one remaining ladder rung before escalation)
Assemble s1-s6 into a single statement of what an accepted form would have to produce — nine
single-word spill slots at stride 8 and congruence 0 from a 139-instruction leaf — and check
it against the sanctioned-technique catalog for any family not yet tried. Do not re-run any
measured axis; the value is in the cross-check, not in new probes.

### J2 — the class disposition (unchanged, still owner surface, still not yet due)
29 functions tree-wide have a stride-8 reload-spill block; all are at 0 mod 8, all still carry
rules, none has ever matched. s6 adds the decisive supporting number: across the ENTIRE
decompiled tree our fork produced 131 spill slots and every single one is at 4 mod 8, so no
BB2 function whose target spills can match while this fork compiles it. Whichever session is
assigned `escalation` modality should file the CLASS in docs/grind/decisions.md citing
s2/spillscan2.txt, the closed form, the cc1psx counter-exhibit, the s3 reuse kill, the s4+s5
~159.5k-mutation negative, s5's taxonomy closure, and s6's tree-wide trace — and must NOT
cite Ruling-2, whose scope limit excludes non-crashing fork divergences.

### J3 — do NOT re-open
Everything in s5's list, plus: the reload1.c:2363 REUSE path (s6, measured unreachable
tree-wide), the "allocate a bigger slot" path (s6, zero occurrences tree-wide), and any doubt
about the isolated harness (s6, validated against the real TU).

## [s6] The reload1.c:2363 slot-REUSE path — the only route in alter_reg with no big-endian correction, and the single opening sessions 2/3 left in the closed-form proof — fires somewhere in Bushido Blade 2, which would mean a 0-mod-8 single-word spill slot is reachable from compiled C in this fork.
- mechanism: alter_reg has three slot paths. path=1 (from_reg == -1, reload1.c:2352) and path=3 ('allocate a bigger slot', 2369) both call assign_stack_local(..., -1) and are proved by closed form to land a 4-byte value at 4 mod 8. path=2 (2363) instead takes x = spill_stack_slot[from_reg] with adjust == 0 and re-MEMs it at offset 0, inheriting the donor slot's base, so a wider (8-byte) donor slot would hand a reusing SImode pseudo a 0-mod-8 base. Sessions 3-5 measured this dead for this function's SHAPE (23 targeted variants plus ~159.5k permuter mutations); the TREE-WIDE question was explicitly left open as the last measurement that could surprise.
- probe: Built an instrumented cc1 in a PRIVATE copy of the GCC 2.7.2 source (tmp/grind/func_80060E38/s6/gcc — tools/ untouched) with two env-gated (BB2_ALTERREG_DEBUG) traces in alter_reg: BB2ALTERREG on every spill-slot allocation (function, pseudo, from_reg, path, mode size, inherent, total, adjust, final offset) and BB2ARCALL on every call with from_reg != -1 (the spill_hard_reg call site at reload1.c:3499) with (reg_renumber, refs, equiv_constant, equiv_memory_loc). Validated codegen-inert: the rebuilt cc1's output on the s1 probe is byte-identical (diff -q clean) to stock tools/gcc-2.7.2/build/cc1. Then swept ALL 31 src/*.c translation units at the Makefile's exact cpp defines and CC_FLAGS (GP_FILES and NO_SR_FILES are both empty, so all TUs share flags), piping stdin->stdout the way the Makefile does. Aggregated with s6/agg.py and s6/agg2.py.
- result: 131 reload spill-slot allocations tree-wide, in 69 distinct functions across 15 TUs. path=1: 131. path=2: ZERO. path=3: ZERO. Every allocation is modesize 4 with inherent == total == 4, adjust == 0, and a final offset === 4 (mod 8) — not one 4-byte-mode spill slot at any other congruence in the entire game. This is not because spill_hard_reg is dormant: it makes 98 alter_reg calls with from_reg != -1 across 51 functions in 10 TUs, but 97 have reg_renumber >= 0 (reload found the pseudo a hard register) and the 98th has reg_equiv_memory_loc set, so ZERO reach the slot-allocation block. spill_stack_slot[] is written only in the path=3 branch (reload1.c:2399), which therefore never executes, so path=2 — which reads that array — is unreachable BY CONSTRUCTION in this codebase, not merely unobserved.
- verdict: KILLED

## [s6] The isolated probe harness used by sessions 1-5 might not be faithful to how func_80060E38 actually compiles inside src/text1b.c, leaving the whole five-session measurement chain resting on an unvalidated proxy.
- mechanism: Every variant campaign from s1 onward ran on a 45-line extracted reproducer compiled standalone, because src/text1b.c is ~490 KB preprocessed. If the real TU's declaration context changed pseudo numbering, spill count or slot placement, the ledger's conclusions would describe the harness rather than the function.
- probe: Compiled the REAL src/text1b.c whole with the instrumented cc1 at build-identical flags and read off every BB2ALTERREG line for fn=func_80060E38.
- result: Exactly nine slot allocations, pseudos 75-83, all from_reg=-1 path=1 modesize=4 inherent=4 total=4 adjust=0, at offsets 4,12,20,28,36,44,52,60,68 — identical in count, path, mode and offset to the isolated probe's nine. The harness is faithful; sessions 1-5 measured the real thing.
- verdict: CONFIRMED

## [s7] Declared locals can leave GCC's frame cursor at an odd multiple of 4 when reload begins allocating spill slots, so the nine spill slots inherit that parity and land at 0 mod 8 — target's congruence — under ordinary C control.
- mechanism: Session 2's closed form is offset = frame_offset + (CEIL_ROUND(size,8) - GET_MODE_SIZE(mode)) + STARTING_FRAME_OFFSET. It is congruent to -GET_MODE_SIZE(mode) (mod 8) ONLY IF frame_offset is 8-aligned on entry to the allocation, and no session had measured that it is. assign_stack_local's align == 0 path (ordinary declared locals) uses alignment = GET_MODE_ALIGNMENT(mode)/8 and does NOT round the slot size, so locals genuinely advance the cursor at stride 4 (s32) or 1 (char) — session 2's v_locals measured exactly that. This was therefore the last C-controlled input to the formula, and uniquely it did not require changing the instruction stream: 4 bytes of locals takes the frame from 108 to 112 == 0x70, target's exact frame size, because the callee-save block already carries 4 bytes of MIPS_STACK_ALIGN slack.
- probe: Read tools/gcc-2.7.2/function.c:665-728 at the definition site, then instrumented assign_stack_local ITSELF (session 6 instrumented its caller alter_reg) in a private GCC copy at tmp/grind/func_80060E38/s7/gcc, env-gated on BB2_SLOCAL_DEBUG, logging align/mode/GET_MODE_SIZE/size_in/size/frame_offset-on-entry/frame_offset-after-rounding/bigend_correction/STARTING_FRAME_OFFSET/current_function_outgoing_args_size/final offset. Codegen-inertness control first: the instrumented cc1's asm on the s1 probe is byte-IDENTICAL to stock tools/gcc-2.7.2/build/cc1 with the trace both off and on (s7/control.sh). Then seven LEAF variants of the s1 probe (s7/gen7.py -> probes/p_loc{1,2,3x1,4,5,12,20}.c) carrying 1, 2, 3, 4, 5, 12 and 20 bytes of address-taken locals, each forced to memory by storing its address through an absolute pointer so the function stays call-free and outgoing_args_size stays 0 as in the real target.
- result: KILLED, and the mechanism is visible in the trace. function.c:692-698 rounds the cursor BEFORE using it — `frame_offset = CEIL_ROUND (frame_offset, alignment)` — and the spill path (align == -1) has alignment = BIGGEST_ALIGNMENT/8 = 8, so any parity the locals established is erased on the first spill allocation. Traces show fo_in=1 -> fo_rounded=8 (p_loc1) and fo_in=3 -> fo_rounded=8 (p_loc3x1). EVERY spill in EVERY variant is still at 4 mod 8: p_loc1/p_loc2/p_loc3x1 give 12,20,...,76; p_loc12 gives 20,...,84; p_loc20 gives 28,...,92. The whole-tree sweep agrees: of 317 align == -1 allocations in Bushido Blade 2, only 2 entered with a non-8-aligned frame_offset and both were rounded to 0 mod 8. (Policy note: even had it worked, a local declared purely to shift frame layout is the forbidden dead-vars-local-array / frame-coercion family and is stripped by the cheat-invisible sandbox; it was measured because the forensic fact — whether the congruence has ANY C-controlled input — is what the ledger and any future escalation need.)
- verdict: KILLED

## [s7] STARTING_FRAME_OFFSET can be congruent to 4 (mod 8), because mips.h:1651-1653 uses the RAW current_function_outgoing_args_size while the MIPS_STACK_ALIGN padding lives separately in compute_frame_size — session 1's frontier item F1 sub-question (a), asserted by sessions 2-6 but never measured.
- mechanism: STARTING_FRAME_OFFSET is the second free input of the closed form and is added to every slot's final offset. mips.h:1651-1653 is `(current_function_outgoing_args_size + (TARGET_ABICALLS ? MIPS_STACK_ALIGN (UNITS_PER_WORD) : 0))` — no rounding of the args size at the point of use; the MIPS_STACK_ALIGN of the args area happens elsewhere, at mips.c:4466 inside compute_frame_size. If any call in the program could leave outgoing_args_size at 4 mod 8, every spill slot in that function would land at 0 mod 8 — target's congruence — and the class of 29 stride-8-spill functions would have a C-reachable route through call-argument shape.
- probe: Read all four assignment sites of current_function_outgoing_args_size (calls.c:1400, 2400, 2750 and the expand_call path at 1241-1255) plus the MIPS parameter macros, then measured sfo on every assign_stack_local call in the entire game: s7/sweep7.sh compiles all 31 src/*.c translation units with the instrumented cc1 at the Makefile's exact flags, s7/agg7.py aggregates.
- result: KILLED. All three sites round args_size.constant up to STACK_BYTES = STACK_BOUNDARY/8 = 64/8 = 8 before assigning, then MAX it with REG_PARM_STACK_SPACE = MAX_ARGS_IN_REGISTERS*UNITS_PER_WORD - FIRST_PARM_OFFSET = 16; OUTGOING_REG_PARM_STACK_SPACE is defined (mips.h:1830) so the `-= reg_parm_stack_space` branch is compiled out, STACK_POINTER_OFFSET is 0 (mips.h:1295), and we build -mno-abicalls so the ABICALLS term is 0. Measurement: 1694 assign_stack_local calls across 31 TUs, sfo mod 8 == 0 in 1694/1694, with distinct sfo values {0, 16, 24, 32, 40}. This also retro-explains session 1's probe_call result (a 5-argument call = 20 raw bytes produced STARTING_FRAME_OFFSET 24, not 20).
- verdict: KILLED

## [s7] With both free inputs pinned, `offset === -GET_MODE_SIZE(mode) (mod 8)` is an UNCONDITIONAL theorem for this fork over all C inputs, not merely a per-path result.
- mechanism: The final offset of any stack slot is frame_offset(after CEIL_ROUND to alignment) + bigend_correction + STARTING_FRAME_OFFSET, with bigend_correction = CEIL_ROUND(size, alignment) - GET_MODE_SIZE(mode) under BYTES_BIG_ENDIAN. Sessions 2 and 3 proved the alter_reg adjust arithmetic cancels on every branch, but the result was conditional on two unmeasured assumptions: that the cursor is 8-aligned on entry and that STARTING_FRAME_OFFSET is a multiple of 8. s7 closes both, so nothing outside GET_MODE_SIZE(mode) can move the congruence, and mode is chosen by register allocation from the value's C type, not by slot layout.
- probe: The same tree sweep, split by mode size and align class.
- result: CONFIRMED. 4-byte-mode slots on the spill path (align == -1): 131/131 at 4 mod 8, zero at 0 — matching s6's independent alter_reg-side count exactly. 4-byte-mode slots on the declared-local path (align == 0): 43 at 0 mod 8 and 15 at 4 — locals CAN sit at 0 mod 8, but they are allocated at stride 4, which is the shape session 2's v_locals already killed (target needs nine slots at stride 8 AND congruence 0, and for a 4-byte value those two properties are mutually exclusive under this cc1: stride 8 implies align == -1 implies the +4). BLKmode/aggregate slots (msize == 0): 186 align == -1 and 1314 align == 0, all reported at 0 mod 8 because GET_MODE_SIZE(BLKmode) is 0 — consistent with the theorem, and already dead as a route per s5's taxonomy (an aggregate slot is one wide object, not nine single-word slots).
- verdict: CONFIRMED

## LIVE FRONTIER for session 8

The forensic account is now complete at BOTH levels: session 6 measured which path alter_reg
takes (131/131 corrected, the uncorrected path unreachable), and session 7 measured the two
inputs that path feeds to assign_stack_local (1694/1694 with STARTING_FRAME_OFFSET a multiple
of 8; every non-8-aligned cursor rounded away before use). The congruence of a 4-byte reload
spill slot in this fork is a function of the spilled value's MODE and nothing else, for every
C input, and mode is not a layout lever.

### J1 — nothing about this function is still a search; the remaining rung is synthesis
Six axes are measured dead: structural (s2, 11 variants), alter_reg branch analysis (s2
closed form + s3's 23-variant reuse kill), the function.c slot-creation taxonomy (s5),
randomized machine search (s4+s5, ~159.5k mutations on four chassis), the alter_reg path
census (s6, 131 allocations tree-wide) and now the closed form's two free inputs (s7, 1694
allocations tree-wide plus 7 parity probes). A synthesis session should state the acceptance
condition in one place — nine single-word spill slots at stride 8 AND congruence 0 from a
139-instruction leaf whose stream already matches target exactly — and cross-check it against
the sanctioned-technique catalog. It should NOT re-run any measured axis.

### J2 — the class disposition is unchanged and still not yet escalatable
func_80060E38 is one of 29 functions whose target contains a stride-8 reload-spill block
(s2/spillscan2.txt); all 29 are at 0 mod 8, all still carry rules, none has ever matched.
Whichever session is assigned `escalation` modality should file the CLASS in
docs/grind/decisions.md citing: s2/spillscan2.txt, the closed form, the cc1psx counter-exhibit,
s3's reuse-branch kill, the s4+s5 ~159.5k-mutation negative, s5's taxonomy closure, s6's
tree-wide path census and s7's tree-wide input census — and must NOT cite Ruling-2, whose
scope limit excludes non-crashing fork divergences. The driver has not declared exhaustion, so
this must not be escalated before that modality is assigned.

### J3 — the instrument is now two passes deep and still reusable
tmp/grind/func_80060E38/s7/gcc is a private GCC 2.7.2 tree carrying BOTH env-gated traces
(BB2_ALTERREG_DEBUG from s6, BB2_SLOCAL_DEBUG from s7), validated byte-identical to stock
build/cc1 on real input with the traces off AND on. s7/sweep7.sh drives all 31 TUs at
build-identical flags and s7/agg7.py aggregates; adding a trace to a third pass is a one-file
edit plus `make cc1` (~1 min). Keep the byte-identity control (s7/control.sh) as step one.

### J4 — do NOT re-open
* Everything in s3's H4 and s5/s6's do-not-reopen lists.
* Frame-offset parity from declared locals (s7) — CEIL_ROUND erases it, measured on 7 probes
  and on 317 tree-wide spill allocations.
* STARTING_FRAME_OFFSET / call-argument shape as a congruence lever (s7) — 1694/1694 slots
  measured with sfo a multiple of 8, and proved from all three assignment sites.

## [s7] Declared locals can leave GCC's frame cursor at an odd multiple of 4 when reload begins allocating spill slots, so the nine spill slots inherit that parity and land at 0 mod 8 - target's congruence - under ordinary C control.
- mechanism: Session 2's closed form is offset = frame_offset + (CEIL_ROUND(size,8) - GET_MODE_SIZE(mode)) + STARTING_FRAME_OFFSET, which is congruent to -GET_MODE_SIZE(mode) (mod 8) ONLY IF frame_offset is 8-aligned on entry - never measured by s1-s6. assign_stack_local's align == 0 path (ordinary declared locals) does NOT round the slot size, so locals really do advance the cursor at stride 4 (s32) or 1 (char), as s2's v_locals measured. Uniquely among all levers tried, this one needs no instruction-stream change: 4 bytes of locals takes the frame 108 -> 112 == 0x70, target's exact frame size, because the callee-save block already carries 4 bytes of MIPS_STACK_ALIGN slack.
- probe: Read tools/gcc-2.7.2/function.c:665-728 at the definition site, then instrumented assign_stack_local ITSELF (s6 instrumented its caller alter_reg) in a private GCC copy at tmp/grind/func_80060E38/s7/gcc, env-gated on BB2_SLOCAL_DEBUG, logging align/mode/GET_MODE_SIZE/size before and after rounding/frame_offset on entry and after CEIL_ROUND/bigend_correction/STARTING_FRAME_OFFSET/outgoing_args_size/final offset. Codegen-inertness control first (s7/control.sh): the instrumented cc1's asm on the s1 probe is byte-IDENTICAL to stock tools/gcc-2.7.2/build/cc1 with the trace off AND on. Then seven LEAF variants of the s1 probe (s7/gen7.py -> probes/p_loc{1,2,3x1,4,5,12,20}.c) carrying 1, 2, 3, 4, 5, 12 and 20 bytes of address-taken locals, each forced to memory by storing its address through an absolute pointer so the probe stays call-free and outgoing_args_size stays 0 as in the real target.
- result: Dead, with the mechanism visible in the trace: function.c:692-698 does 'frame_offset = CEIL_ROUND (frame_offset, alignment)' BEFORE using the cursor, and the spill path's alignment is BIGGEST_ALIGNMENT/8 == 8, so any parity the locals established is erased at the first spill allocation. Traces show fo_in=1 -> fo_rounded=8 (p_loc1) and fo_in=3 -> fo_rounded=8 (p_loc3x1). Every spill in every variant is still at 4 mod 8: p_loc1/p_loc2/p_loc3x1 give 12,20,...,76; p_loc12 gives 20,...,84; p_loc20 gives 28,...,92. Tree-wide the same: of 317 align == -1 allocations in the whole game only 2 entered non-8-aligned, and both were rounded to 0 mod 8. Policy note: even had it worked, a local declared purely to shift frame layout is the forbidden dead-vars-local-array / frame-coercion family and is stripped by the cheat-invisible sandbox - it was measured because the forensic fact (does the congruence have ANY C-controlled input) is what the ledger and any future escalation need.
- verdict: KILLED

## [s7] STARTING_FRAME_OFFSET can be congruent to 4 (mod 8), because mips.h:1651-1653 uses the RAW current_function_outgoing_args_size while the MIPS_STACK_ALIGN padding of the args area lives separately in compute_frame_size - session 1's frontier item F1 sub-question (a), asserted by sessions 2-6 but never measured.
- mechanism: STARTING_FRAME_OFFSET is the second free input of the closed form and is added to every slot's final offset. If any call shape could leave outgoing_args_size at 4 mod 8, every spill slot in that function would land at 0 mod 8 - target's congruence - and the 29-function stride-8-spill class would have a C-reachable route through call-argument shape.
- probe: Read all four assignment sites of current_function_outgoing_args_size (calls.c:1400, 2400, 2750 and the expand_call path at 1241-1255) plus the MIPS parameter macros, then measured sfo on every assign_stack_local call in the entire game: s7/sweep7.sh compiles all 31 src/*.c TUs with the instrumented cc1 at the Makefile's exact flags; s7/agg7.py aggregates.
- result: Dead by proof and by measurement. All three assignment sites (calls.c:1241, 2388, 2738) round args_size.constant up to STACK_BYTES = STACK_BOUNDARY/8 = 64/8 = 8 before assigning, then MAX it with REG_PARM_STACK_SPACE = MAX_ARGS_IN_REGISTERS*UNITS_PER_WORD - FIRST_PARM_OFFSET = 16; OUTGOING_REG_PARM_STACK_SPACE is defined (mips.h:1830) so the '-= reg_parm_stack_space' branch is compiled out; STACK_POINTER_OFFSET is 0 (mips.h:1295); -mno-abicalls zeroes the ABICALLS term. Measured: 1694 assign_stack_local calls across 31 TUs, sfo mod 8 == 0 in 1694/1694, distinct sfo values {0, 16, 24, 32, 40}. This also retro-explains session 1's probe_call, where a 5-argument call (20 raw bytes) produced STARTING_FRAME_OFFSET 24, not 20.
- verdict: KILLED

## [s7] With both free inputs pinned, 'offset === -GET_MODE_SIZE(mode) (mod 8)' is an UNCONDITIONAL theorem for this fork over all C inputs, not merely a per-path result.
- mechanism: Final slot offset = frame_offset(after CEIL_ROUND to alignment) + bigend_correction + STARTING_FRAME_OFFSET, with bigend_correction = CEIL_ROUND(size, alignment) - GET_MODE_SIZE(mode) under BYTES_BIG_ENDIAN. Sessions 2-3 proved the alter_reg adjust arithmetic cancels on every branch, but conditionally on the two assumptions s7 has now measured. The only surviving variable is the spilled value's machine mode, which register allocation derives from the value's C type, not from slot layout.
- probe: The same 31-TU sweep, split by GET_MODE_SIZE and align class (s7/agg7_report.txt).
- result: Confirmed. 4-byte-mode slots on the spill path (align == -1): 131/131 at 4 mod 8, zero at 0 - an exact independent match to s6's alter_reg-side count of 131. 4-byte-mode slots on the declared-local path (align == 0): 43 at 0 mod 8 and 15 at 4, i.e. locals CAN sit at 0 mod 8 but at stride 4, the shape s2's v_locals already killed (target needs nine slots at stride 8 AND congruence 0, mutually exclusive for a 4-byte value here since stride 8 implies align == -1 implies the +4). BLKmode/aggregate slots (GET_MODE_SIZE 0): 186 align == -1 and 1314 align == 0, all at 0 mod 8, consistent with the theorem and already dead per s5's taxonomy (one wide object, not nine single-word slots).
- verdict: CONFIRMED
