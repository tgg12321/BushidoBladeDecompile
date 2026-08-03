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

## Session 2 (structural, 2026-08-03) — floor 18 (unchanged; re-measured)

### F1 RESOLVED — the `func_8006BD28` "existence proof" was an artifact of an incomplete census
`func_8006BD28` is **authorized whole-body canonical asm**: `src/text1b.c:15814` is
`__asm__("glabel func_8006BD28\n" …)`, and `inline_asm_canonical.txt:338` authorizes it
("structural asmfix … wholesale-replaced the inner loop body with literal target
instructions … Not C-form-reachable in this fork. User-authorized 2026-06-07"). Its
`0x18/0x20` slots are *target bytes pasted into the source*, not cc1 output. Session 1's
`spillscan.py` cross-referenced `regfix.txt`/`asmfix.txt`/`engine/queue.json` but **not**
`inline_asm_canonical.txt`, which is exactly how a canonical-asm function was mistaken for a
clean compiled counter-example. **There is no in-tree existence proof of a `≡ 0 (mod 8)`
compiled spill block.** (Lesson for any future census in this repo: a function with zero
rules and not queued is NOT necessarily compiled C — check `inline_asm_canonical.txt`.)

### The stricter tree-wide census (`s2/spillscan2.py` → `spillscan2.txt`)
Signature: an offset counts as a spill slot only if the target has BOTH `sw $r,O($sp)` and
`lw $r2,O($sp)` with both registers caller-save (an outgoing-arg home is stored but never
reloaded; a callee-save is `s0-s8/ra`). Slots are then grouped into maximal stride-8 runs of
length >= 3. Result over all `asm/funcs/*.s`:

| congruence | status | blocks |
|---|---|---|
| `mod8 == 0` | RULES (regfix/asmfix-carrying) | **29** |
| `mod8 == 0` | CLEAN | **0** |
| `mod8 == 4` | anything | **0** |

29 blocks across 29 distinct functions, `func_80060E38` among them
(`mod8=0 n=9 [0,8,16,24,32,40,48,56,64]`). **Every function in this game whose target
contains a genuine stride-8 reload-spill block is still cheat-carrying; not one has ever
been matched.** This is not a func_80060E38 quirk, it is a class.

### PROVED FROM SOURCE — an SImode reload spill can never be `≡ 0 (mod 8)` in this fork
Reading `reload1.c:2337-2410` in full (session 1 traced only the `2352` fresh-slot line)
closes every branch:
* `from_reg == -1` (our case — `reg_renumber[i] < 0`, the pseudo never got a hard reg):
  `x = assign_stack_local(SImode, total_size, -1)`, `adjust = inherent - total`; then
  `if (BYTES_BIG_ENDIAN && inherent < total) adjust += total - inherent`. The two cancel
  **identically**, for every `total_size`. Net address = `assign_stack_local`'s own return.
* "allocate a bigger slot" (`from_reg != -1`, no reusable slot):
  `adjust = GET_MODE_SIZE(mode) - total_size`, then the same `+= total - inherent`. Same
  cancellation.
* `assign_stack_local` (`function.c:666-727`): `align == -1` gives `alignment = 8`,
  `size = CEIL_ROUND(size,8)`, `frame_offset = CEIL_ROUND(frame_offset, 8)`, and
  `bigend_correction = size - GET_MODE_SIZE(mode)`.

So for a pseudo of mode M:
`offset = STARTING_FRAME_OFFSET + 8k + (CEIL_ROUND(total,8) - GET_MODE_SIZE(M))`.
`STARTING_FRAME_OFFSET` is `current_function_outgoing_args_size` (0 for our leaf; a
multiple of 8 otherwise — `probe_call.c` measured 24 for a 5-word arg list). Therefore
**`offset === -GET_MODE_SIZE(M) (mod 8)`**: 4-byte modes (SImode/SFmode) always land at
`=== 4`, and only an 8-byte mode (DImode/DFmode) lands at `=== 0`. Target's 9 slots are
single-word `sw $v0`/`lw $v0` of SImode constants, so the 8-byte-mode escape is unusable —
a DImode slot would make the spill two words and change the instruction count.
The one theoretical `=== 0` route for an SImode pseudo is the **slot-reuse** branch at
`reload1.c:2384` (`x = spill_stack_slot[from_reg]` when that slot was allocated in a wider
mode), which skips the correction entirely — but it requires (a) `from_reg != -1`, i.e. the
pseudo was hard-allocated and then hard-spilled, and (b) a *prior* DImode spill from the
same hard register. Both need `long long`/`double` in a function whose target has neither.

### The cc1psx counter-exhibit — the ORIGINAL compiler puts them at 0,8,…,64
`tmp/grind/func_80060E38/s2/psx.sh` feeds **the exact same preprocessed C**
(`s1/dumps/probe/in.i`) to the original PsyQ `cc1psx` (GCC 2.7.2.SN.1, via
`tools/cc1psx_wrapper.sh`, same flags), rc=0:

    cc1psx : subu $sp,$sp,112 ; slots 0 8 16 24 32 40 48 56 64  (+ 72..104 callee-saves)
    our cc1: subu $sp,$sp,112 ; slots 4 12 20 28 36 44 52 60 68 (+ 72..104 callee-saves)

Same 136 cc1-level instructions, same 0x70 frame, same callee-save block — the spill-slot
congruence is the *only* frame difference, and cc1psx's is target's. That is a direct
measurement of the divergence: `BYTES_BIG_ENDIAN` is 0 in the compiler that built the game
and 1 in ours, so `bigend_correction` is 0 there and 4 here.
**Honest qualifier:** cc1psx does not otherwise reproduce target from this C — a positional
compare (`s2/norm_diff.py`) shows the streams agree for the first 31 instructions and then
diverge in *order* (cc1psx starts interleaving the `sw ...(D_800A...)` global stores
earlier; 67 positional mismatches, 34 difflib lines). Our fork's schedule and register
allocation are the ones that match target exactly (session 1). So cc1psx is evidence about
the *slot congruence* only — not a drop-in that would match this function.

### KILLED — structural levers are inert on the spill base (10 measured variants)
`s2/gen.py` generates variants of the reproducer, each run through `s1/run.sh`
(dumps in `s1/dumps/<tag>/`). Offsets observed:

| variant | transform | spill offsets |
|---|---|---|
| `v_rev` | store order fully reversed | 4 12 20 28 36 44 52 60 68 |
| `v_scope` | stores split into two `{ }` block scopes | 4 12 … 68 |
| `v_argsfirst` | the two `*(s32*)0x1F8000xx` stores hoisted to the top | 4 12 … 68 |
| `v_interleave` | those two stores moved into the middle | 4 12 … 68 |
| `v_more` | 40 stores (more register pressure) | 4 12 … 132 (17 slots) |
| `v_ll` | a `long long` temp live across the block | 4 12 … 108 |
| `v_llarr` | three `long long` temps | 4 12 … 92 |
| `v_dbl` | a `double` temp | 4 12 … 68 |
| `v_half` | only 16 stores | *no spills at all* (0 4 8 12 = callee-saves) |
| `v_dipress` | 12 `long long` products, all live | *no spills* (GCC found an order) |

Every single spill in every variant is `=== 4 (mod 8)`. Statement order, block scoping,
declaration order, spill count, and mixing in 8-byte-mode values do not move the congruence
— exactly as the source proof predicts. **The structural axis is measured dead for this
function.**

### `v_half` — the hoist boundary (F3 diagnostic, closed)
16 constant stores fit entirely in registers: zero spills. The 9 spills in the real body are
a hard consequence of GCC hoisting all 32 `lui/ori` pairs above all 32 stores. Any variant
that changes the spill count changes the instruction count and moves away from the 139/139
alignment, so F3 is confirmed as a diagnostic only, never a closing lever. Closed.

### Ruling-2 (`fork-divergence-inline-asm`) does NOT cover this case
The 2026-07-13 owner ruling authorizes region-scoped inline asm only when our fork
**SIGSEGVs** on the faithful source (Gate 1). Its explicit scope limit: "Does NOT sanction
fork-divergences without a crash. If our fork compiles the faithful source but emits
different bytes (register allocation, scheduling, reload-count, prologue-order differences),
those are NOT this class. They're ordinary Grinder search items. The crash is the
load-bearing evidence." Our cc1 compiles this source cleanly. So the existing carve-out is
unavailable and must not be cited by a future escalation without a fresh owner ruling.

### Also checked
`Makefile` contains **no** `CC1_PSX_FILES` / `cc1psx` gate — the per-file cc1psx opt-in the
wrapper's header describes is not wired into the build at all, and the Makefile is off the
grind edit surface anyway ([[no-compiler-divergence]]).

### Artifacts (session 2)
* `tmp/grind/func_80060E38/s2/gen.py` + `v_*.c` — the 10 structural variants
* `tmp/grind/func_80060E38/s2/spillscan2.py` / `spillscan2.txt` — canonical-asm-aware census
* `tmp/grind/func_80060E38/s2/psx.sh`, `probe_cc1psx.s` — the cc1psx counter-exhibit
* `tmp/grind/func_80060E38/s2/cmp.sh`, `norm_diff.py`, `ours.norm`, `psx.norm`,
  `ours_vs_psx.diff` — the ours-vs-cc1psx stream comparison

- [s2] Floor re-measured with sandbox --disable all: score 18, target_insns 139 == build_insns 139, 18 rules dropped. No src edits made this session; floor unchanged.

- [s2] F1 is DEAD and was never a real contradiction: func_8006BD28 is a whole-body __asm__("glabel ...") authorized at inline_asm_canonical.txt:338, so its 0x18/0x20 slots are pasted target bytes, not cc1 output. Session 1's census omitted inline_asm_canonical.txt. No in-tree compiled function has a +0-mod-8 spill block.

- [s2] Stricter tree-wide census (s2/spillscan2.py, signature = offset with BOTH a caller-save sw and a caller-save lw, grouped into stride-8 runs of >=3): 29 spill blocks in the whole game, ALL at mod8==0, ALL in functions still carrying regfix/asmfix rules, ZERO clean, ZERO at mod8==4. Every function whose target spills is unmatched. This is a class-wide blocker, not a func_80060E38 quirk.

- [s2] PROOF from reload1.c:2337-2410 + function.c:666-727: for a spilled pseudo of mode M, offset = STARTING_FRAME_OFFSET + 8k + (CEIL_ROUND(total_size,8) - GET_MODE_SIZE(M)), and the big-endian 'cancel' adjustments in both alter_reg branches cancel identically for every total_size. Since STARTING_FRAME_OFFSET is a multiple of 8, offset === -GET_MODE_SIZE(M) (mod 8). 4-byte modes ALWAYS land at 4 mod 8; only DImode/DFmode land at 0. Target's slots are single-word SImode sw/lw, so the 8-byte-mode escape is unusable.

- [s2] The only theoretical SImode +0 route is the slot-REUSE branch at reload1.c:2384 (reusing a wider pre-existing spill_stack_slot skips the correction), which needs from_reg != -1 AND a prior DImode spill from the same hard register. Our 9 pseudos take the from_reg == -1 path (reg_renumber < 0), and the target function has no 8-byte-mode values. Unreachable.

- [s2] cc1psx COUNTER-EXHIBIT: the original PsyQ compiler, fed the exact same preprocessed C, emits subu $sp,$sp,112 with spill slots at 0,8,16,24,32,40,48,56,64 — precisely target's. Ours emits 4,12,...,68 from the same input. Same 136 cc1 instructions, same frame, same callee-save block. The divergence is measured, not inferred: BYTES_BIG_ENDIAN is 0 in the game's compiler and 1 in our decompals fork.

- [s2] Honest qualifier on the counter-exhibit: cc1psx does NOT otherwise reproduce target from this C — the streams agree for 31 instructions then diverge in ORDER (cc1psx interleaves the global stores earlier; 67 positional mismatches). Our fork is the one whose schedule and RA match target exactly. cc1psx is evidence about slot congruence only.

- [s2] Ten structural variants (order reversal, block scoping, hoisting the indirect stores, interleaving, +8 stores, long long / double temps, 16-store reduction) all measured via cc1 -da: EVERY spill in EVERY variant is 4 mod 8. Statement order, scoping, declaration order, spill count and 8-byte-mode mixing are all inert on the congruence. The structural axis is dead.

- [s2] v_half (16 stores) spills nothing, closing F3: the 9 spills are a hard consequence of GCC hoisting all 32 lui/ori pairs above all 32 stores, and any spill-count change moves away from the 139/139 instruction alignment. F3 is a diagnostic, never a closing lever.

- [s2] Ruling-2 (.claude/rules/fork-divergence-inline-asm.md) explicitly does NOT cover this: it requires a cc1 SIGSEGV on the faithful source (Gate 1) and its scope limit names 'our fork compiles the faithful source but emits different bytes' as an ordinary Grinder item. Our cc1 compiles cleanly. A future escalation must not cite Ruling-2 without a fresh owner ruling.

- [s2] Makefile has no CC1_PSX_FILES / cc1psx gate — the per-file cc1psx opt-in described in tools/cc1psx_wrapper.sh's header is not wired into the build, and the Makefile is off the grind edit surface regardless.

- [s2] Floor re-measured this session: sandbox func_80060E38 --disable all = score 18, target_insns 139 == build_insns 139, 18 rules dropped. No src/ edits were made (src/ is clean in git status), so the floor is unchanged from session 1.

- [s2] func_8006BD28 — session 1's decisive 'existence proof' of a +0-mod-8 spill block — is authorized whole-body canonical asm (src/text1b.c:15814 __asm__("glabel func_8006BD28"), authorization at inline_asm_canonical.txt:338). Its stack slots are pasted target bytes, not cc1 output. Session 1's spillscan.py cross-referenced regfix.txt/asmfix.txt/engine/queue.json but not inline_asm_canonical.txt. General lesson for this repo: zero rules + not queued does NOT imply compiled C.

- [s2] Canonical-asm-aware tree-wide census (tmp/grind/func_80060E38/s2/spillscan2.py, signature = an offset carrying BOTH a caller-save sw and a caller-save lw, grouped into stride-8 runs of >= 3): 29 spill blocks in 29 distinct functions, ALL at mod8 == 0, ALL in rule-carrying functions, ZERO clean, ZERO at mod8 == 4. No function in Bushido Blade 2 whose target spills has ever been matched.

- [s2] Closed-form proof from reload1.c:2337-2410 + function.c:666-727: a spilled pseudo of mode M lands at STARTING_FRAME_OFFSET + 8k + (CEIL_ROUND(total_size,8) - GET_MODE_SIZE(M)); the big-endian pre-adjust in both allocating branches of alter_reg is cancelled exactly by the later 'adjust += total_size - inherent_size'. Since STARTING_FRAME_OFFSET is always a multiple of 8, offset === -GET_MODE_SIZE(M) (mod 8): 4-byte modes always 4, only 8-byte modes 0.

- [s2] The single remaining theoretical 0-mod-8 route for an SImode pseudo is the slot-reuse branch at reload1.c:2384, which returns a wider pre-existing slot's address with no correction. It requires from_reg != -1 (pseudo hard-allocated then hard-spilled) plus a prior DImode spill from the same hard register. Our 9 pseudos have reg_renumber < 0 and take the from_reg == -1 path, and the target function contains no 8-byte-mode values.

- [s2] Eleven measured C variants (v_rev, v_scope, v_argsfirst, v_interleave, v_more, v_half, v_ll, v_llarr, v_dbl, v_dipress, v_locals) confirm the closed form empirically: every reload spill in every variant is at 4 mod 8, and declared locals give stride 4 rather than target's stride 8.

- [s2] cc1psx counter-exhibit: the original PsyQ compiler, fed the byte-identical preprocessed input, emits subu $sp,$sp,112 with spill slots at 0,8,...,64 — precisely target's — while our fork emits 4,12,...,68 from the same input with the same 136 instructions and the same callee-save block. Qualifier: cc1psx's instruction ORDER diverges from target after the first 31 instructions (67 positional mismatches); our fork is the one that matches target's schedule and RA exactly.

- [s2] Ruling-2 (.claude/rules/fork-divergence-inline-asm.md, owner 2026-07-13) explicitly does NOT cover this case: it requires a reproducible cc1 SIGSEGV (Gate 1), and its scope limit names 'our fork compiles the faithful source but emits different bytes' as an ordinary Grinder item. Our cc1 compiles this source cleanly, so a future escalation must not cite Ruling-2 without a fresh owner ruling.

- [s2] The Makefile contains no CC1_PSX_FILES / cc1psx gate — the per-file cc1psx opt-in described in tools/cc1psx_wrapper.sh's header is not wired into the build, and the Makefile is off the grind edit surface regardless ([[no-compiler-divergence]]).

- [s2] Ledger banked: memory/grind/func_80060E38/evidence.md and hypotheses.md carry the full session-2 blocks; candidate.c header updated (body unchanged, still the best form); two disproven forms saved to memory/grind/func_80060E38/rejected/.

## Session 3 (structural, 2026-08-03) — floor 18 (unchanged; re-measured)

Session 3's mandate was the structural axis, which session 2 had measured dead across 11
variants. Rather than re-run that space, this session attacked the single opening session
2's closed-form proof explicitly left: alter_reg's **slot-REUSE branch**. That is the only
place in the compiler where a 4-byte reload spill can escape the `+4` big-endian
correction, so it is the only structural lever that could ever move this function's floor.

### The reuse branch, read at source (independent re-derivation, not inherited)
`reload1.c:2331` guards the whole slot-assignment block with `reg_renumber[i] < 0`, and
`alter_reg(i, from_reg)` has three paths:
* **2349 `from_reg == -1`** — `x = assign_stack_local (mode, total_size, -1)`;
  `adjust = inherent_size - total_size`, cancelled exactly by
  `2405: if (BYTES_BIG_ENDIAN && inherent_size < total_size) adjust += total_size - inherent_size`.
  Net adjust 0 ⇒ the MEM is `assign_stack_local`'s own return, i.e. `base + bigend_correction`.
* **2363 reuse** — `x = spill_stack_slot[from_reg]` with `adjust` left at **0** and no call
  to `assign_stack_local` at all. `2410` then re-MEMs it in the pseudo's own mode at
  offset 0 because `GET_MODE (x) != GET_MODE (regno_reg_rtx[i])`. **This is the only path
  with no bigend_correction.** If the pre-existing slot was allocated for an 8-byte mode
  (`bigend_correction = 8 - GET_MODE_SIZE(DImode) = 0`, so its base is ≡ 0 mod 8), an
  SImode pseudo reusing it lands at ≡ 0 mod 8 — target's congruence.
* **2369 "allocate a bigger slot"** — re-derived and found NOT to help even when the mode
  is widened to DImode: `assign_stack_local (DImode, 8, -1)` returns `base + 0`, but then
  `adjust = GET_MODE_SIZE(mode) - total_size = 0` and `2405` adds
  `total_size - inherent_size = 8 - 4 = 4`, putting the SImode MEM back at `base + 4`.
`assign_stack_local` re-read at `function.c:665-728`: `align == -1` ⇒ `alignment = 8` and
`size = CEIL_ROUND (size, 8)`; `702-703` `bigend_correction = size - GET_MODE_SIZE (mode)`;
no `FRAME_GROWS_DOWNWARD` on this target. Session 2's closed form is confirmed verbatim.

### Where `from_reg != -1` comes from (new, and it is REACHABLE)
`alter_reg` is called with a real `from_reg` from exactly one site: `reload1.c:3499`, inside
`spill_hard_reg (regno, …)`. Its callers are the eliminable-register sites (`715`, `1523`,
`719`, `1534`) and, decisively, **`reload1.c:2253` — reload commandeering a hard register as
a spill register**, announced in the `.greg` dump one line earlier at `2232`
(`"Spilling reg %d.\n"`). So the `Spilling reg 2.` line session 1 saw in `probe`'s `.greg`
IS `spill_hard_reg(2, …)`: the `from_reg != -1` machinery is live in this very function.
That makes the reuse branch a genuine, testable structural target rather than a theoretical
one — which is why this session spent itself on it.

### 23 measured variants — the reuse branch never fires from C
Two generators, all compiled with the real build flags through `s1/run.sh` (cpp + cc1 -da),
with a classifier that distinguishes a **WORD slot** (an offset used by a single-word
`sw`/`lw` whose `+4` neighbour is unused — the shape target's nine slots have) from an
8-byte slot (offsets `O` and `O+4` both used):

| generator | variants | result |
|---|---|---|
| `s3/gen3.py` | `v_di9` (9 long longs live across the block), `v_dihalf` (long longs consumed as 32-bit halves), `v_reload` (volatile pointer/address reload pressure, no 8-byte modes), `v_di_reload`, `v_union` (`union { long long; s32[2]; }` locals, address-taken), `v_dbl9` (9 doubles), `v_maxpress` (long longs + reload pressure + 36 stores) | every SImode spill ≡ 4 (mod 8) |
| `s3/reuse_probe.py` | `v_scoped_2_12`, `v_scoped_3_16` — long longs die, then a nested scope opens SImode temps meant to inherit their hard registers | no word slot at 0 mod 8 |
| `s3/reuse_probe2.py` | 15 **leaf** shapes `v_leafA_{1,2,3}_{0,4,8}` and `v_leafB_{1,2,4}_{4,10}` — call-free like the target, 8-byte-mode values either dying before or living across the 32-store block, with 0–10 competing SImode temps | no word slot at 0 mod 8 |

`Spilling reg` lines confirm reload really did commandeer the 8-byte-mode values' registers:
`8,9` (leafA/leafB), `24,25` (v_scoped), `16,17` + `64,65` (v_dihalf), `12,13`
(v_di_reload, v_maxpress), `2` (the original probe). In `v_dihalf` the DImode slots are
plainly visible at offsets `0` and `8` — **≡ 0 (mod 8), exactly the congruence we want** —
but they are read and written as TWO words (`0`/`4`, `8`/`12`) and the nine SImode spills
still sit at `20, 28, 36, …`. No SImode pseudo ever shared a commandeered hard register with
an 8-byte-mode pseudo, because the two never end up co-allocated: global alloc places the
8-byte value in a register pair and the leftover constant pseudos are precisely the ones it
could NOT place (`reg_renumber < 0`, i.e. `from_reg == -1`).

### The reuse branch is ALSO arithmetically insufficient — this is the stronger kill
`spill_stack_slot` is indexed **by hard register**, so the branch can yield at most **one**
0-mod-8 slot per commandeered hard register, and only if that register already carries a
spilled 8-byte-mode pseudo. `func_80060E38`'s target needs **nine** distinct slots at
`0,8,…,64`. That would require nine commandeered hard registers, each with its own prior
`long long`/`double` pseudo — while target's 139-instruction stream is lui/ori plus
gp-relative `sw` with **not one** 8-byte-mode instruction in it, and every measured probe
shows reload commandeering at most one register pair (i.e. one 8-byte object). So even a
firing reuse branch could not reproduce target, and any C that supplies the nine 8-byte
pseudos leaves the 139/139 instruction alignment the current body already has. **The reuse
route is dead as a closing lever, not merely unobserved** — which closes the last opening in
session 2's proof and, with it, the structural axis in full.

### Artifacts (session 3)
* `tmp/grind/func_80060E38/s3/gen3.py`, `runall.sh`, `v_*.c`, `v_*.log` — the 8 mode /
  pressure variants and their offset reports
* `tmp/grind/func_80060E38/s3/reuse_probe.py` — call-containing reuse shapes + the
  word-slot classifier and `.greg` `Spilling reg` extraction
* `tmp/grind/func_80060E38/s3/reuse_probe2.py` — the 15 leaf-shaped reuse variants
* RTL dump sets for every variant under `tmp/grind/func_80060E38/s1/dumps/<tag>/`

- [s3] Floor re-measured: sandbox func_80060E38 --disable all = score 18, target_insns 139 == build_insns 139, 18 rules dropped. No src/ edits were made this session (git status shows only engine-generated metrics/events.jsonl), so the floor is unchanged from sessions 1 and 2.

- [s3] Session 2's closed-form spill-offset proof was independently re-derived from tools/gcc-2.7.2/reload1.c:2331-2419 and function.c:665-728 and is CONFIRMED verbatim, including the previously unexamined detail that the "allocate a bigger slot" branch does not help even when the slot mode is widened to DImode: assign_stack_local(DImode,8,-1) returns base+0 and adjust = 0, but reload1.c:2405 then adds total_size - inherent_size = 4, restoring base+4 for the SImode MEM.

- [s3] NEW MECHANISM FACT: alter_reg is called with from_reg != -1 from exactly one site, reload1.c:3499 inside spill_hard_reg, whose non-elimination caller is reload1.c:2253 — reload commandeering a hard register as a spill register, announced by the "Spilling reg N." line printed at reload1.c:2232. The "Spilling reg 2." that session 1 saw in the probe's .greg dump therefore proves the from_reg != -1 machinery is live in this function, making the slot-reuse branch a real testable target rather than a theoretical one.

- [s3] 23 new compiled C variants (s3/gen3.py: v_di9 v_dihalf v_reload v_di_reload v_union v_dbl9 v_maxpress; s3/reuse_probe.py: v_scoped_2_12 v_scoped_3_16; s3/reuse_probe2.py: 15 leaf shapes v_leafA_{1,2,3}_{0,4,8} and v_leafB_{1,2,4}_{4,10}) produced ZERO single-word spill slots at 0 mod 8. Every SImode reload spill in every shape is at 4 mod 8, including shapes whose .greg dumps prove reload commandeered the 8-byte-mode value's register pair (Spilling reg 8/9, 24/25, 16/17, 12/13).

- [s3] v_dihalf is the sharpest datapoint: its DImode spill slots ARE at 0 and 8 — exactly target's congruence — but each is accessed as two words (0/4 and 8/12), while the nine SImode spills alongside them still sit at 20,28,36,... Target's nine slots are single-word sw/lw. The 0-mod-8 congruence and the single-word access are, measurably, mutually exclusive for a reload spill in this fork.

- [s3] Root cause of the non-firing: the reuse branch needs an SImode pseudo and an 8-byte-mode pseudo CO-ALLOCATED to one hard register that reload then commandeers. They never co-allocate, because global alloc places the 8-byte value in a register pair and the nine leftover constant pseudos are by construction the ones it could not place at all (reg_renumber < 0 => from_reg == -1). No amount of statement order, scoping, mode mixing or added pressure changed that in 23 shapes.

- [s3] STRONGER KILL (arithmetic, independent of triggerability): spill_stack_slot is indexed by hard register, so the reuse branch yields at most ONE 0-mod-8 slot per commandeered hard register, each requiring its own prior 8-byte-mode spill from that register. func_80060E38 needs NINE slots at 0,8,...,64, i.e. nine commandeered registers each with a long long/double pseudo — while target's 139-instruction stream contains no 8-byte-mode instruction at all and every probe shows reload commandeering at most one register pair. The reuse route cannot reproduce target even if it could be triggered.

- [s3] Consequence for the ledger: with the reuse branch closed both empirically and arithmetically, the structural axis for func_80060E38 is exhausted end to end — the closed form covers the two allocating branches, and the one non-allocating branch is now measured and disproved. Remaining ladder modalities (forensics, rederive, synthesis) inherit a class-level question, not a search over C forms for this function.

- [s3] Disproven form banked at memory/grind/func_80060E38/rejected/dimode-slot-reuse-cannot-yield-nine-word-slots.c with the full rationale; candidate.c header updated with the session-3 summary (body unchanged, still the best form).

- [s3] Floor re-measured this session: sandbox func_80060E38 --disable all = score 18, target_insns 139 == build_insns 139, 18 rules dropped. No src/ edits were made (git status shows only engine-generated metrics/events.jsonl), so the floor is unchanged from sessions 1 and 2.

- [s3] Session 2's closed-form spill-offset proof was independently re-derived from tools/gcc-2.7.2/reload1.c:2331-2419 and function.c:665-728 and is CONFIRMED verbatim, including the previously unexamined mode-widening sub-case of the 'allocate a bigger slot' branch, which also lands back at base+4 via reload1.c:2405.

- [s3] NEW MECHANISM FACT: alter_reg is called with from_reg != -1 from exactly one site, reload1.c:3499 inside spill_hard_reg, whose non-elimination caller is reload1.c:2253 — reload commandeering a hard register as a spill register, announced by the 'Spilling reg N.' line printed at reload1.c:2232. The 'Spilling reg 2.' session 1 saw in the probe's .greg dump therefore proves the from_reg != -1 machinery is live in this function, making the slot-reuse branch a real testable target rather than a theoretical one.

- [s3] 23 new compiled C variants (s3/gen3.py: v_di9 v_dihalf v_reload v_di_reload v_union v_dbl9 v_maxpress; s3/reuse_probe.py: v_scoped_2_12 v_scoped_3_16; s3/reuse_probe2.py: 15 leaf shapes v_leafA_{1,2,3}_{0,4,8} and v_leafB_{1,2,4}_{4,10}) produced ZERO single-word spill slots at 0 mod 8. Every SImode reload spill in every shape is at 4 mod 8, including shapes whose .greg dumps prove reload commandeered the 8-byte-mode value's register pair (Spilling reg 8/9, 24/25, 16/17, 12/13).

- [s3] v_dihalf is the sharpest datapoint: its DImode spill slots ARE at 0 and 8 — exactly target's congruence — but each is accessed as two words (0/4 and 8/12), while the nine SImode spills alongside them still sit at 20,28,36,.... Target's nine slots are single-word sw/lw. The 0-mod-8 congruence and single-word access are, measurably, mutually exclusive for a reload spill in this fork.

- [s3] Root cause of the non-firing: the reuse branch needs an SImode pseudo and an 8-byte-mode pseudo co-allocated to one hard register that reload then commandeers. They never co-allocate, because global alloc places the 8-byte value in a register pair while the nine leftover constant pseudos are by construction the ones it could not place at all (reg_renumber < 0 => from_reg == -1). No amount of statement order, scoping, mode mixing or added pressure changed that across 23 shapes.

- [s3] STRONGER KILL (arithmetic, independent of triggerability): spill_stack_slot is indexed by hard register, so the reuse branch yields at most ONE 0-mod-8 slot per commandeered hard register, each requiring its own prior 8-byte-mode spill from that register. func_80060E38 needs NINE slots at 0,8,...,64 — nine commandeered registers each with a long long/double pseudo — while target's 139-instruction stream contains no 8-byte-mode instruction at all and every probe shows reload commandeering at most one register pair.

- [s3] Consequence: all three branches of alter_reg are now individually accounted for — two allocating branches by closed form (s2, re-derived and extended in s3) and the one non-allocating branch by 23 measured variants plus a counting argument (s3). The structural axis for func_80060E38 is exhausted end to end; what remains is the class-level question about the 29 tree-wide stride-8 spill blocks, not a search over C forms for this function.

- [s3] Disproven form banked at memory/grind/func_80060E38/rejected/dimode-slot-reuse-cannot-yield-nine-word-slots.c with the full rationale; candidate.c header updated with the session-3 summary (body unchanged, still the best form); evidence.md and hypotheses.md carry the full session-3 blocks.

## Session 4 (permuter, 2026-08-03) — floor 18 (unchanged; re-measured)

Session 4's mandate was the permuter axis. Sessions 2 and 3 measured 34 HAND-DESIGNED C
variants and proved the offset closed-form from the compiler source; this session put that
proof to a MACHINE search — ~94,500 randomized C mutations across two structurally distinct
chassis — so that the "no C form reaches 0 mod 8" claim rests on a large empirical sample as
well as on the reading of `reload1.c` / `function.c`.

### A validated fast permuter chassis now exists (reusable, and the main artifact)
`tmp/grind/func_80060E38/s4/mkws.sh` builds `tmp/grind/func_80060E38/s4/ws`, a decomp-permuter
workspace whose `compile.sh` runs the REAL build pipeline (`cpp | cc1 | prologue_fix | maspsx
(with the Makefile's exact sdata/expand-lb/multu/label-nop flags) | fix_lwl | multu_pad`),
extracts the `.ent func_80060E38 … .end func_80060E38` region and assembles it, and whose
`target.o` is `asm/funcs/func_80060E38.s` assembled with the r3000 prelude. Its base is the
s1 isolated probe rather than `src/text1b.c`, so an iteration costs ~0.02 s of wall clock at
`-j 8` instead of a full-TU compile.

**Validation (the load-bearing part):** `base.o` vs `target.o`, disassembled and diffed, is
**139 vs 139 instructions with EXACTLY 18 differing lines — the nine `sw v0,N(sp)` and nine
`lw v0,N(sp)` spill offsets (4,12,…,68 vs 0,8,…,64) and nothing else.** The chassis therefore
reproduces the engine's honest floor of 18 bit-for-bit, so a permuter find on it would be a
real find. Any later session can reuse it directly (`bash tmp/grind/func_80060E38/s4/mkws.sh`).

### `--stack-diffs` is MANDATORY for this function (and the campaign wrapper supplies it)
`tools/permuter_campaign.py`'s help documents that launches pass `--stack-diffs` by default
(2026-07-13) precisely because the permuter's default scorer NORMALIZES sp-relative offsets
away. func_80060E38 is the textbook case: its entire remaining gap IS an sp-offset shift, so
under the default scorer this workspace would report **score 0 — a false match**. Both
campaigns launched at `base_score = 72` (the weighted score of 18 differing instructions),
confirming stack-diff scoring was live. **A future session must never run raw `permuter.py`
on this function; it will report a match that does not exist.**

### The campaigns (both harvested and stopped in-session)

| chassis | label | shape | jobs | elapsed | iterations | finds |
|---|---|---|---|---|---|---|
| `s4/ws`  | `probe-chassis`      | the s1 probe: 32 absolute `0x1F8000xx` constants stored to the 32 globals | 8 | 1199 s (~20 min) | **54,551** | **0** |
| `s4/ws2` | `base-arith-chassis` | same semantics via a `spbase = 0x1F800000` local plus 32 `spbase + 0xNN` addends — a different source shape and therefore a different mutation neighbourhood | 6 | ~19 min | **40,003** | **0** |

**~94,554 randomized C mutations, ZERO outputs at any score below the 72 baseline.** Not one
find at all — the permuter never even produced an equal-score alternative worth writing out.
Both campaigns were harvested with `--stop` (`procs_killed: 9` on chassis 1) and the registry
shows both `alive: false, registered_active: false`; nothing outlives this session.

Chassis 2 is itself a small measured datapoint: GCC folds `spbase + 0xNN` back into absolute
constants, so the arithmetic form compiles to the SAME 139 instructions with the SAME
`4,12,…,68` spill offsets. Rewriting the constants as base-plus-offset arithmetic is inert.

### What this adds to the ledger (and what it does not)
It does NOT add a new mechanism — s2's closed form
(`offset === -GET_MODE_SIZE(M) (mod 8)`) already predicted this outcome exactly, and s3 closed
the reuse-branch escape. What it adds is the **scale of the negative**: the congruence is now
known to be immovable not only under 34 targeted human variants and a source-level proof, but
under ~94.5k machine-generated ones on two chassis. The permuter axis for func_80060E38 is
measured dead, and — because the mechanism is arithmetic in the spilled pseudo's MODE, not in
the C — the same result is predicted for the other 28 members of the stride-8 spill class.
A future session should NOT spend another campaign here.

### Artifacts (session 4)
* `tmp/grind/func_80060E38/s4/mkws.sh` — builds + validates the chassis (139/139, 18-line diff)
* `tmp/grind/func_80060E38/s4/mkws2.sh`, `base2.c` — the base-arithmetic second chassis
* `tmp/grind/func_80060E38/s4/ws/{base.c,compile.sh,settings.toml,target.s,base.txt,tgt.txt,campaign.log,campaign_meta.json}`
* `tmp/grind/func_80060E38/s4/ws2/{base.c,compile.sh,base.txt,campaign.log,campaign_meta.json}`
* `tmp/grind/func_80060E38/s4/h1.sh`, `h2.sh` — the harvest/stop drivers

- [s4] Floor re-measured at session start: sandbox func_80060E38 --disable all = score 18, target_insns 139 == build_insns 139, 18 rules dropped. No src/ edits were made this session (git status shows only engine-generated metrics/events.jsonl), so the floor is unchanged from sessions 1-3.

- [s4] A validated fast permuter chassis now exists at tmp/grind/func_80060E38/s4/ws, built by s4/mkws.sh. Its compile.sh runs the real build pipeline (cpp | cc1 | prologue_fix | maspsx with the Makefile's exact flags | fix_lwl | multu_pad), extracts the .ent/.end region and assembles it; target.o is asm/funcs/func_80060E38.s with the r3000 prelude. Validation: base.o vs target.o is 139 vs 139 instructions differing in EXACTLY the 18 spill offsets (sw/lw v0 at 4,12,...,68 vs 0,8,...,64) and nothing else, so the chassis reproduces the engine's honest floor of 18 exactly. It costs ~0.02 s per iteration at -j 8 instead of a full src/text1b.c compile, and is reusable as-is.

- [s4] CRITICAL TOOLING FACT for this function: the permuter's default scorer normalizes sp-relative offsets away, so a raw `permuter.py` run on func_80060E38 reports score 0 — a FALSE MATCH — because the entire remaining gap IS an sp-offset shift. tools/permuter_campaign.py passes --stack-diffs by default (owner directive 2026-07-13), and both s4 campaigns launched at base_score 72 (the weighted score of 18 differing instructions), confirming stack-diff scoring was live. Never run raw permuter.py on this function or on any of the 29 stride-8 spill-block functions.

- [s4] Campaign 1 (label probe-chassis, dir tmp/grind/func_80060E38/s4/ws, -j 8, base_score 72): 54,551 iterations over 1199 s. finds_total 0, finds_new 0, best_new_score null. Harvested with --stop (procs_killed 9); registry shows alive false, registered_active false.

- [s4] Campaign 2 (label base-arith-chassis, dir tmp/grind/func_80060E38/s4/ws2, -j 6, base_score 72): a structurally different chassis in which the 32 scratchpad constants are written as `spbase + 0xNN` off a `spbase = 0x1F800000` local, giving the permuter a different mutation neighbourhood. 40,003 iterations over ~19 min, 0 finds. Harvested with --stop; inactive in the registry.

- [s4] TOTAL: ~94,554 randomized C mutations across two chassis produced ZERO permuter outputs at any score below the 72 baseline — not even an equal-score alternative. This is the empirical counterpart to s2's closed-form proof and s3's reuse-branch kill: the spill-slot congruence is immovable under machine search as well as under 34 hand-designed variants.

- [s4] Chassis 2 is also a measured variant in its own right: GCC folds `spbase + 0xNN` back into absolute constants, so the base-plus-offset arithmetic form compiles to the SAME 139 instructions with the SAME 4,12,...,68 spill offsets. Expressing the constants as arithmetic off a base local is inert on the congruence, as the closed form predicts (the offset depends only on the spilled pseudo's mode, never on the C).

- [s4] Consequence for the ladder: the permuter axis is now measured dead for func_80060E38, and since the mechanism is arithmetic in GET_MODE_SIZE of the spilled pseudo rather than anything the C controls, the same negative is predicted for the other 28 functions in the stride-8 spill class. A future session should not spend another campaign here; the untried modalities are forensics (tree-wide alter_reg instrumentation) and rederive (assign_stack_temp-created slots).

- [s4] Both campaigns were stopped inside the session; no permuter process outlives it. Disproven form banked at memory/grind/func_80060E38/rejected/permuter-94k-mutations-cannot-move-spill-congruence.c; candidate.c header updated (body unchanged, still the best form).

- [s4] Floor re-measured at session start: sandbox func_80060E38 --disable all = score 18, target_insns 139 == build_insns 139, 18 rules dropped. No src/ edits were made this session (git status shows only engine-generated metrics/events.jsonl), so the floor is unchanged from sessions 1-3.

- [s4] A validated fast permuter chassis now exists at tmp/grind/func_80060E38/s4/ws, built by s4/mkws.sh. Its compile.sh runs the real build pipeline (cpp | cc1 | prologue_fix | maspsx with the Makefile's exact flags | fix_lwl | multu_pad), extracts the .ent/.end region and assembles it; target.o is asm/funcs/func_80060E38.s with the r3000 prelude. Validation: base.o vs target.o is 139 vs 139 instructions differing in EXACTLY the 18 spill offsets and nothing else, so it reproduces the engine's honest floor of 18 exactly. It costs ~0.02 s per iteration at -j 8 instead of a full src/text1b.c compile, and is reusable as-is by any later session.

- [s4] CRITICAL TOOLING FACT: the permuter's default scorer normalizes sp-relative offsets away, so a raw permuter.py run on func_80060E38 reports score 0 - a false match - because the entire remaining gap IS an sp-offset shift. tools/permuter_campaign.py passes --stack-diffs by default (owner directive 2026-07-13); both s4 campaigns launched at base_score 72 (the weighted score of 18 differing instructions), confirming stack-diff scoring was live.

- [s4] Campaign 1 (label probe-chassis, dir tmp/grind/func_80060E38/s4/ws, -j 8, base_score 72): 54,551 iterations over 1199 s, finds_total 0, finds_new 0, best_new_score null. Harvested with --stop (procs_killed 9); registry shows alive false, registered_active false.

- [s4] Campaign 2 (label base-arith-chassis, dir tmp/grind/func_80060E38/s4/ws2, -j 6, base_score 72): the 32 scratchpad constants written as spbase + 0xNN off a spbase = 0x1F800000 local, a different mutation neighbourhood. 40,003 iterations over ~19 min, 0 finds. Harvested with --stop; inactive in the registry.

- [s4] TOTAL: ~94,554 randomized C mutations across two chassis produced ZERO permuter outputs at any score below the 72 baseline - not even an equal-score alternative. This is the empirical counterpart to session 2's closed-form proof and session 3's reuse-branch kill.

- [s4] Chassis 2 is also a measured variant in its own right: GCC folds spbase + 0xNN back into absolute constants, so the base-plus-offset arithmetic form compiles to the SAME 139 instructions with the SAME 4,12,...,68 spill offsets. Expressing the constants as arithmetic off a base local is inert on the congruence, as the closed form predicts.

- [s4] Consequence for the ladder: the permuter axis is now measured dead for func_80060E38, and since the mechanism is arithmetic in GET_MODE_SIZE of the spilled pseudo rather than anything the C controls, the same negative is predicted for the other 28 functions in the stride-8 spill class. A future session should not spend another campaign here; the untried modalities are forensics (tree-wide alter_reg instrumentation) and rederive (assign_stack_temp-created slots).

- [s4] Both campaigns were stopped inside the session; no permuter process outlives it (registry status confirms both dirs alive:false, registered_active:false).

- [s4] Ledger banked: evidence.md and hypotheses.md carry the full session-4 blocks; candidate.c header updated (body unchanged, still the best form); disproven form saved to memory/grind/func_80060E38/rejected/permuter-94k-mutations-cannot-move-spill-congruence.c.
