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
