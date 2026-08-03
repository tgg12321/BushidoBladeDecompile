# Evidence bank — func_80060E38

## Session 1 (recon, 2026-08-03) — floor 18

### The function
`src/text1b.c:13562`, `void func_80060E38(s32 arg0, s32 arg1)`. A pure scratchpad-pointer
table initializer: 32 straight-line assignments of `0x1F8000xx` constants to the globals
`D_800A3468 … D_800A34EC`, followed by `*(s32 *)0x1F800004 = arg0;` and
`*(s32 *)0x1F800008 = arg1;`. It is a **leaf** (no `jal` anywhere in target).

Target: `asm/funcs/func_80060E38.s`, 139 instructions, frame `addiu $sp, $sp, -0x70`.
GCC hoists all 32 `lui/ori` constant materializations to the top of the function, keeps
23 of them live in registers, and **spills 9 of them to the stack**, then emits all 32
`sw …%gp_rel(D_…)($gp)` stores at the bottom.

### The gap is EXACTLY and ONLY the spill-slot base — nothing else
`canonical` = verdict **C**, distance 18. `sandbox --disable all` = **18**,
`target_insns 139 == build_insns 139`. The 18 regfix rules at `regfix.txt:2788-2806` are a
pure mechanical stack-offset shift and nothing else:

| | slot offsets used |
|---|---|
| target | `0x00 0x08 0x10 0x18 0x20 0x28 0x30 0x38 0x40` (9 slots, stride 8) |
| ours   | `0x04 0x0C 0x14 0x1C 0x24 0x2C 0x34 0x3C 0x44` (9 slots, stride 8) |

Everything else is byte-identical: instruction stream, instruction ORDER, register
allocation (incl. the exact `$fp/$t9/$t8/$s7…$s0/$t7…$t0/$a3/$a2/$v1` assignment),
scheduling, delay slots, the saved-register block (`s0-s7,fp` at `0x48…0x68`), and the
frame size `0x70`. Verified by disassembling the sandbox object
(`tmp/grind/func_80060E38/s1/ours_disasm.txt`, 139 lines) against the target `.s`.
**This is a frame-LAYOUT delta, not a codegen / RA / scheduling delta.** Do not spend
sessions on register-allocation or scheduling levers — there is nothing wrong there.

### KILLED: it is not a pipeline-stage artifact
`func_80060E38` appears in **none** of `frame_fix_funcs.txt`, `prologue_config.json`,
`delay_slot_ra_funcs.txt`, `multu_funcs.txt`, `expand_lb_funcs.txt`. The `+4` is emitted
by **cc1 itself**, before `prologue_fix` / `maspsx` / `regfix` ever run.

### Fast standalone reproducer (use this — do NOT recompile text1b.c)
`src/text1b.c` is ~125k tokens; never read or recompile it whole for this function.
`tmp/grind/func_80060E38/s1/probe.c` is a 45-line self-contained file (only
`typedef int s32;` + the 32 externs + the function body) that reproduces the exact
`4,12,20,…,68` spill offsets under the real build flags. Driver:

    wsl bash -c "cd <repo> && sh tmp/grind/func_80060E38/s1/run.sh [some_variant.c]"

It runs `mipsel-linux-gnu-cpp … | cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1
-mno-abicalls -fno-builtin -w -da`, prints the `N($sp)` offsets, and leaves the full RTL
dump set (`in.i.rtl .lreg .greg .combine .sched2 .dbr` …) in `dumps/<tag>/`. Iterating a
C variant is seconds, not a full-file build.

### Mechanism, traced to the GCC source

1. The 9 slots are **reload spill slots**, not declared locals. Proof: the pre-reload RTL
   dumps (`in.i.rtl`, `in.i.lreg`) contain **zero** stack MEMs / `virtual-stack-vars`
   references; `(reg:SI 29 sp)` first appears in `in.i.greg` (post-reload). The `.greg`
   "Register dispositions" lists pseudos 84-105 in hard regs, and reports `Spilling reg 2.`
2. `reload1.c:2352` allocates each spill slot with **`assign_stack_local (SImode,
   total_size, -1)`** — note `align == -1`.
3. `function.c:681-684`, the `align == -1` branch: `alignment = BIGGEST_ALIGNMENT /
   BITS_PER_UNIT` (= 64/8 = **8**, `mips.h:1082`) **and `size = CEIL_ROUND (size, alignment)`**
   — so a 4-byte SImode spill is rounded up to an **8-byte** slot. This is the source of the
   stride 8 seen in BOTH builds.
4. `function.c:702-703`: `if (BYTES_BIG_ENDIAN && mode != BLKmode) bigend_correction =
   size - GET_MODE_SIZE (mode);` → `8 - 4 = ` **`4`**.
5. `reload1.c:2353-2358` "cancel the big-endian correction" computes
   `adjust = inherent_size - total_size` = `4 - 4` = **0**, and `reload1.c:2405-2406`
   (`inherent_size < total_size`) is likewise false. **The cancellation does not fire**,
   because it only compensates for `total_size > inherent_size` — it is blind to the
   rounding `assign_stack_local` performed internally on its own copy of `size`.
   Net result: the MEM lands at `slot_base + 4`.

### CONFIRMED: our cc1 is compiled BIG-ENDIAN
`mips.h:980` `#define BYTES_BIG_ENDIAN (TARGET_BIG_ENDIAN != 0)`. The build passes no
endianness flag and cc1 rejects `-EL`/`-EB` outright (`cc1: Invalid option`), so it is
compiled in. Two independent probes (`tmp/grind/func_80060E38/s1/endian2.s`):
* `int lowword(long long x) { return (int)x; }` compiles to `move $2,$5` — it takes the
  **second** argument register as the low word ⇒ `WORDS_BIG_ENDIAN == 1`.
* `struct B { unsigned a:8; unsigned b:24; }; getb` extracts the **low** 24 bits
  (`and $2,$4,0x00ffffff`) ⇒ first-declared bitfield sits in the HIGH bits — exactly the
  behaviour the existing `[[bitfield-direction-divergence]]` rule already documents
  ("our fork allocates bitfields HIGH-first"). This is a long-standing, independently
  observed property of the tree, not a new discovery.

So `BYTES_BIG_ENDIAN == 1` for our cc1, step 4 fires, and every SImode reload spill under
this toolchain is placed at `slot_base + 4`.

### CONFIRMED model (measured, not inferred)
    spill MEM offset = current_function_outgoing_args_size + frame_offset + 4
with `frame_offset` advancing 0,8,16,… (8-byte rounded slots).

Measured with `probe_call.c` (identical body + a trailing 5-arg `sink5(1,2,3,4,5)` call, so
the function is no longer a leaf): the outgoing-arg store lands at `16($sp)` and the spill
block moves to `28,36,44,52,60,68,76,84,92($sp)` — i.e. base `24 + 4 = 28`, stride 8. The
`+4` is invariant; only the base moved, by exactly `outgoing_args_size`.

For func_80060E38 the target is a leaf ⇒ `outgoing_args_size = 0` ⇒ our cc1 must emit
`0 + {0,8,…,64} + 4` = `{4,12,…,68}`. That is precisely the observed diff. And since
`current_function_outgoing_args_size` is `MIPS_STACK_ALIGN`-padded to a multiple of 8
(`mips.h:2063`), the naive reading is that **every** SImode reload spill under this cc1 is
at an offset `≡ 4 (mod 8)` — which would make target's `≡ 0 (mod 8)` unreachable.

### THE OPEN CONTRADICTION — and it is a live in-tree existence proof
That naive reading is **falsified by the tree itself**, which is the single most important
thing this session found and the right place for session 2 to start.

`tmp/grind/func_80060E38/s1/spillscan.py` scans every `asm/funcs/*.s` for sp-relative
`sw`/`lw` of **caller-save** registers (`$v0/$v1/$aN/$tN`; prologue saves of `s0-s8/ra`
excluded) and cross-references `regfix.txt`/`asmfix.txt`/`engine/queue.json`. Output:
`tmp/grind/func_80060E38/s1/spillscan.txt` — 132 such functions, **38 of them CLEAN**
(zero rules, not queued ⇒ already byte-matching under the oracle).

**`func_8006BD28` is the sharp one: CLEAN/matched, and its caller-save stack slots are at
`0x18` and `0x20` — both `≡ 0 (mod 8)`, stride 8.** Our cc1 provably produces those bytes
today (the function carries no rules and the full-build SHA1 matches the oracle).

So there EXISTS a C construct, live in this tree, that yields stride-8 caller-save stack
slots at `≡ 0 (mod 8)` under our big-endian cc1. Either
(a) `func_8006BD28`'s `outgoing_args_size` is `20 (0x14)` — i.e. `≡ 4 (mod 8)`, meaning
    `STARTING_FRAME_OFFSET` is the **unpadded** arg size (`mips.h:1651-1653` reads
    `current_function_outgoing_args_size` directly, with no `MIPS_STACK_ALIGN` wrapper —
    the padding is applied separately inside `compute_frame_size`), which would make the
    `≡ 4 (mod 8)` generalization simply WRONG; or
(b) those slots are not `align == -1` reload spills at all but `assign_stack_temp` /
    declared-local slots, which take the `align == 0` path (alignment 4, **no** size
    rounding, `bigend_correction = 4 - 4 = 0`) and land at `+0`.

Note `probe_call.c` gave base `24`, not `20`, for a 5-word arg list — consistent with (b)
and with `outgoing_args_size` being padded there. Distinguishing (a) from (b) is a single
cheap experiment and it decides whether this function is closable.

### What is NOT the problem (do not re-derive)
* Register allocation — matches target exactly.
* Instruction scheduling / delay slots — match target exactly.
* Statement order in the C — already matches the target's store order, including the
  out-of-sequence `D_800A3480/3484/348C` trio near the end (`src/text1b.c:13591-13593`).
* Any regfix/asmfix/maspsx/prologue_fix stage — the `+4` is pre-maspsx, from cc1.
* Compiler flags / cc1psx — out of scope by standing policy; and this is a frame-layout
  artifact, not a codegen divergence.

### Artifacts
* `tmp/grind/func_80060E38/s1/probe.c` — standalone reproducer (floor shape)
* `tmp/grind/func_80060E38/s1/probe_call.c` — outgoing-args model probe
* `tmp/grind/func_80060E38/s1/run.sh` — cpp+cc1 `-da` driver, prints `N($sp)` offsets
* `tmp/grind/func_80060E38/s1/ours_disasm.txt` — our 139-insn disassembly
* `tmp/grind/func_80060E38/s1/spillscan.py` / `.txt` — tree-wide spill census
* `tmp/grind/func_80060E38/s1/endian2.s` — big-endian confirmation
* `tmp/grind/func_80060E38/s1/dumps/probe/in.i.{rtl,lreg,greg,…}` — full RTL dump set

- [s1] Baseline: canonical verdict C, distance 18; sandbox --disable all score 18, target_insns 139 == build_insns 139, 18 rules dropped. No prior ledger — this is the first measured floor.

- [s1] func_80060E38 (src/text1b.c:13562) is a leaf scratchpad-pointer table initializer: 32 assignments of 0x1F8000xx constants to D_800A3468..D_800A34EC, then *(s32*)0x1F800004 = arg0 and *(s32*)0x1F800008 = arg1. GCC hoists all 32 lui/ori pairs above all 32 stores, keeps 23 in registers and spills 9.

- [s1] The entire 18-instruction distance is the spill-slot base: target uses sp+{0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,0x40}, ours uses sp+{0x04,0x0C,0x14,0x1C,0x24,0x2C,0x34,0x3C,0x44}. A uniform +4. The 18 regfix rules at regfix.txt:2788-2806 are exactly that mechanical shift and nothing else.

- [s1] Everything except those 18 immediates is byte-identical to target: instruction stream, order, register allocation, delay slots, 0x70 frame size, and the s0-s7/fp save block at 0x48-0x68. This is a frame-LAYOUT delta, so no RA or scheduling lever in the technique index applies.

- [s1] Stride 8 in BOTH builds is explained: reload spill slots are allocated with align == -1, and function.c:681-684 rounds a 4-byte SImode slot up to 8 bytes with 8-byte alignment (BIGGEST_ALIGNMENT is 64 at mips.h:1082).

- [s1] Our cc1 is compiled BIG-ENDIAN (BYTES_BIG_ENDIAN = TARGET_BIG_ENDIAN != 0, mips.h:980; cc1 rejects -EL/-EB so it is compiled in). Independently corroborated by the pre-existing bitfield-direction-divergence rule ('our fork allocates bitfields HIGH-first'). This is a long-standing frozen-toolchain property, not a new regression, and it is what arms the bigend_correction.

- [s1] The GCC big-endian cancellation at reload1.c:2353-2358 and 2405-2406 keys off total_size > inherent_size, so it is blind to the size rounding assign_stack_local performs internally on its own copy of size — which is why the +4 survives here.

- [s1] A fast standalone reproducer now exists: tmp/grind/func_80060E38/s1/probe.c (45 lines, typedef + 32 externs + the body) reproduces 4,12,...,68 exactly under the real build flags, driven by tmp/grind/func_80060E38/s1/run.sh, which also leaves the full cc1 -da RTL dump set. Iterating a C variant takes seconds instead of recompiling the ~125k-token src/text1b.c.

- [s1] Tree-wide census (tmp/grind/func_80060E38/s1/spillscan.py -> spillscan.txt): 132 functions have >=3 caller-save sp-relative slots; 38 are CLEAN (zero regfix/asmfix rules, not queued, therefore byte-matching under the oracle).

- [s1] CONTRADICTION / existence proof: func_8006BD28 is CLEAN and matched, and its caller-save stack slots sit at 0x18 and 0x20 — stride 8, both congruent to 0 mod 8. Our cc1 provably emits those bytes today. So the naive reading of the confirmed model (that every SImode spill under this cc1 must be 4 mod 8) is FALSE, and a C construct reaching +0 exists live in this tree. Resolving how is the decisive next step.

- [s1] Note for the resolution: mips.h:1651-1653 reads current_function_outgoing_args_size RAW in STARTING_FRAME_OFFSET, with no MIPS_STACK_ALIGN wrapper (the padding is applied separately in compute_frame_size), so an unpadded arg area of 0x14 would give 0x14+4 = 0x18 and 0x14+12 = 0x20 — exactly func_8006BD28's slots.

- [s1] No source edits were made this session (recon modality); src/ is clean and the floor is unchanged at 18.
