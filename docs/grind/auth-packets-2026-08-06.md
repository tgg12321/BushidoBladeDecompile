# Canonical-asm authorization evidence packets — 2026-08-06

Evidence packets behind the 2026-08-06 owner grant (decisions.md commit f010b4b6) that
`inline_asm_canonical.txt` rows cite. Trimmed 2026-10-01 to the granted units only; the
Batch-A packets (the 26 ASM-STRUCTURAL items — all rated WEAK, recommended AGAINST, none
granted), the two CONTINGENT island packets (func_8002BEA0, func_8002EA24), the summary
tables and counts resolve at git tag `pre-slim-2026-10-01` (same path).

# Batch B — LIBGTE leaves

## func_80052B00 — DECISIVE — recommend FOR

- **File:** src/text1b.c · **asm:** asm/funcs/func_80052B00.s · **17 instructions** ·
  queue: parked, ASM-PARTIAL, honest distance 18 (= the whole body), 1 rule
  (`regfix.txt:3411 fill_delay @ 16 <- 15`) plus 8 `register asm("$N")` pins in HEAD.
- **Signals.** Eight `lw` from `*$a0` at 0x80052B00-0x80052B1C feeding
  `ctc2 $t0-$t7, $0-$7` at 0x80052B20-0x80052B3C, every one splat-tagged
  `/* handwritten instruction */`; `jr $ra` at 0x80052B3C with **`ctc2 $t7, $7` in the
  delay slot** at 0x80052B40. Zero general-purpose computation.
- **Why it is unreachable from C** (the decisive part, and not a "we are stuck" claim):
  the granted func_80052B44 ruling established from `tools/gcc-2.7.2/reorg.c` that
  `stop_search_p` halts the delay-slot search at ASM_INPUT and `resource_conflicts_p`
  returns 1 for volatile asm — GCC 2.7.2 can **never** put a `ctc2` in a `jr $ra` delay
  slot. `ctc2` can only enter compilation as inline asm. The target bytes are therefore
  unreachable from any C input to the frozen compiler.
- **Precedent fit.** `inline_asm_canonical.txt:340` (func_80052B44, Judge-authorized
  2026-07-27) is the same construct one register-file wider; func_8007ED6C is the older
  verbatim-LIBGTE precedent. This function is *narrower* than both.
- **On the LOW scanner tier.** S1/S2/S6 target general-purpose kernels and S3/S4 need
  >= 40 instructions; a 17-instruction straight-line cop2 leaf cannot fire them. The
  func_80052B44 ruling already recorded that LOW does not disqualify this family.
- **Proposed entry (approve verbatim):**

```
func_80052B00  # LIBGTE SetRotMatrix leaf: 8x lw <- *a0 -> 8x ctc2 $0-$7 (3x3 R matrix + CR7), the last sitting IN the jr-ra delay slot at 0x80052B40. All eight cop2 ops splat-tagged 'handwritten instruction'; zero general-purpose computation. GCC 2.7.2 cannot fill a delay slot with asm (reorg.c stop_search_p halts at ASM_INPUT), so the bytes are unreachable from any C. Same construct as authorized sibling func_80052B44.
```

## func_80052A88 — DECISIVE — recommend FOR

- **File:** src/text1b.c · **asm:** asm/funcs/func_80052A88.s · **30 instructions** ·
  queue: active, ASM-PARTIAL, distance 25, 1 rule.
- **Signals.** Nine splat `handwritten instruction` tags: `ctc2 $t0-$t3, $0-$3`
  (0x80052AA8-0x80052AB4), `ctc2 $t4-$t7, $4-$7` (0x80052AC8-0x80052AD4),
  `mtc2 $t0, $0` (0x80052AD8). Plus `lwc2 $1, 0x8($a1)` (0x80052ADC), two **unfilled GTE
  latency nops** (0x80052AE0/AE4), `mvmva 1,0,0,0,0` (0x80052AE8), a third latency nop,
  then `swc2 $9/$10/$11` — with **`swc2 $11, 0x8($a2)` in the `jr $ra` delay slot**
  (0x80052AFC). The `lh/lhu/sll/or` at 0x80052AB8-0x80052AC4 is the standard PsyQ
  16-bit-pair pack feeding `mtc2`.
- **Why unreachable from C.** Same reorg.c delay-slot impossibility as func_80052B00,
  here on `swc2`; `mvmva`, `lwc2` and the hand-placed GTE latency nops have no C form
  either. This is verbatim PsyQ LIBGTE (`RotTrans`-family matrix-vector multiply).
- **Precedent fit.** Directly inside the gte-wrapper carve-out
  (`pre-slim-2026-10-01:.claude/rules/gte-wrapper-misroute-park.md`) and the func_80052B44 family; the
  func_80052B44 ruling named this function as needing its own sign-off.
- **Proposed entry (approve verbatim):**

```
func_80052A88  # LIBGTE matrix-vector leaf: 8x ctc2 $0-$7 + mtc2 $0 (9 splat 'handwritten instruction' tags), lwc2 $1, two unfilled GTE latency nops, mvmva 1,0,0,0,0, then swc2 $9/$10/$11 with swc2 $11 IN the jr-ra delay slot (0x80052AFC). Verbatim PsyQ hand-written asm; the delay-slot swc2, the mvmva and the latency nops are all unreachable from C under GCC 2.7.2 (reorg.c stop_search_p). Sibling of authorized func_80052B44.
```

## func_80052B7C — DECISIVE — recommend FOR

- **File:** src/text1b.c · **asm:** asm/funcs/func_80052B7C.s · **26 instructions** ·
  queue: active, ASM-PARTIAL, distance 20, 1 rule.
- **Signals.** Eight splat-tagged `ctc2 $t0-$t7, $0-$7` (0x80052B9C-0x80052BB8) fed by
  five `lw` from `*$a0` and three `lh` from `*$a1`; `lwc2 $0/$1` (0x80052BBC/BC0), two
  unfilled GTE latency nops, `mvmva 1,0,0,0,0` (0x80052BCC), a third nop, then
  `swc2 $9/$10` and **`swc2 $11, 0x8($a3)` in the `jr $ra` delay slot** (0x80052BE0).
- **Why unreachable from C.** Identical argument to func_80052A88 — delay-slot cop2
  store plus mvmva plus hand-placed latency nops.
- **Proposed entry (approve verbatim):**

```
func_80052B7C  # LIBGTE matrix-vector leaf (4-arg form): 8x ctc2 $0-$7 (8 splat 'handwritten instruction' tags) from *a0 words + *a1 halfwords, lwc2 $0/$1, two unfilled GTE latency nops, mvmva 1,0,0,0,0, swc2 $9/$10 and swc2 $11 IN the jr-ra delay slot (0x80052BE0). Verbatim PsyQ hand-written asm, unreachable from C under GCC 2.7.2. Sibling of authorized func_80052B44.
```

# Batch 2 — census-eligible units

## Per-unit packets

### `ang_hosei` — DECISIVE — recommend FOR (whole-function, 9 instructions only)

- **File:** src/ings2.c:609 (`INCLUDE_ASM`) · **asm:** asm/funcs/ang_hosei.s lines 2-11 ·
  `0x800836C8-0x800836E8` · queue: active, ASM-PARTIAL, distance 51, 0 rules.
- **The function is 9 instructions, not 51.** The recorded distance of 51 counts the boot
  stub fused onto it (next section). The real body:

```
800836C8  addu  $a3, $a2, $zero      # arg shift: a2 -> a3
800836CC  addu  $a2, $a1, $zero      #            a1 -> a2
800836D0  addu  $a1, $a0, $zero      #            a0 -> a1
800836D4  break 0, 263               # Marionation engine call, code 0x107
800836D8  beqz  $v0, .L800836E4
800836DC   addu $v0, $v1, $zero      # (delay) success: return $v1
800836E0  addiu $v0, $zero, -0x1     # failure: return -1
800836E4: jr    $ra
800836E8   nop
```

- **Why it is unreachable from C.** Two independent reasons. (a) `break` with a custom
  code field cannot be emitted by GCC 2.7.2 at all, and maspsx cannot assemble `break`
  with an arbitrary code — the three authorized siblings all encode it as a raw `.word`
  for exactly this reason (`inline_asm_canonical.txt:131-133`). (b) The three `addu`
  shifts implement a **custom calling convention**: the callee reads its arguments from
  `$a1/$a2/$a3` and returns a pair in `$v0/$v1`, with `$a0` left free for the engine.
  No C function signature produces that shift, and no C construct reads a second return
  register.
- **Precedent fit.** Exact. `func_80083698` (code 0x103), `md_gview_init` (0x104) and
  `bios_FileReadRaw`/`func_8008393C` (0x105) are all authorized, all in the same
  address neighbourhood, all described in the existing entries as "break with custom code
  plus return-value handling." `ang_hosei` is code 0x107 and has the same shape plus the
  arg shift. The measured corpus census above shows these four are the *only* non-div-guard
  `break` users in the executable.
- **Naming note (not blocking).** `ang_hosei` is engine-call-0x107, not angle correction.
  Its call sites (`src/ings.c:141,143,170` — `ang_hosei(fd, 0, 2)`, `ang_hosei(fd,
  sector << 11, 0)`) read as a file seek/read/size primitive, which fits the
  `bios_FileReadRaw` family. `named_syms.txt` already flags two other `ang_hosei*` symbols
  as misnamed (lines 2497, 3457), so the prefix is known drift. Renaming is optional and
  independent of the authorization.
- **Proposed entry (approve verbatim, and only after the split below):**

```
ang_hosei  # Marionation engine call, break code 0x107: custom-ABI arg shift (a0/a1/a2 -> a1/a2/a3, leaving a0 for the engine), `break 0, 263`, then a $v0/$v1 two-register return select (beqz $v0 -> return $v1 else -1). `break` with a custom code field has no C form and maspsx cannot assemble it (the authorized siblings encode it as a raw .word); the arg shift and the $v1 second return value have no C signature. Fourth and last member of the authorized break-trampoline family (func_80083698 0x103, md_gview_init 0x104, func_8008393C 0x105).
```

### Boot/entry stub at `0x800836EC` — DECISIVE — recommend FOR, **after a symbol split**

This is the Wave-1 finding, assessed.

- **Currently:** lines 12-54 of `asm/funcs/ang_hosei.s`, with no symbol of its own, emitted
  inside `endlabel ang_hosei`. Nothing reaches it — `ang_hosei` returns at `0x800836E4`
  with a `nop` delay slot, so there is no fallthrough; the only entry is the PS-EXE
  header. `AGENTS.md:32`, `README.md:51` and `docs/ARCHITECTURE.md:33` all record
  **`0x800836EC` as the executable's entry point**, and grep confirms no symbol,
  `symbol_addrs.txt` entry, or `named_syms.txt` entry exists at that address.

- **Is it hand-written asm?** Yes, and this one is not a close call. It is a textbook crt0:

| Address | Instruction | Why no C form exists |
|---|---|---|
| `800836EC-8008370C` | `lui/addiu` bounds + `sw $zero` loop over `D_800A3308 -> D_801078E0` | BSS clear; runs *before* any C environment |
| `80083710-80083724` | `lw D_800A2690`; `addi $v0, $v0, -0x8` **(trapping, splat-tagged `handwritten instruction`)**; `or $sp, $v0, $t0` with `$t0 = 0x80000000` | **Assigns the stack pointer.** C has no construct that writes `$sp`. |
| `80083728-80083734` | `sll $a0,3 / srl $a0,3` | strips the KSEG0 bit off the heap base by shifting — a hand idiom, not a C mask |
| `80083760-80083764` | `sw $ra, D_800A3668` | **saves the return address into a global, not a stack slot** — because no frame exists yet |
| `80083768-8008376C` | `lui/addiu $gp, %hi/%lo(_gp)` | **assigns `$gp`.** GCC assumes `$gp` is already live; it never initialises it. |
| `80083770` | `addu $fp, $sp, $zero` | assigns the frame pointer directly |
| `80083774-80083778` | `jal bios_InitHeap` with `addi $a0, $a0, 4` **(trapping, splat-tagged)** in the delay slot | second trapping-arith instance |
| `8008377C-8008378C` | reload `$ra` from the global, `jal` into `main` | — |
| `80083790` | `break 0, 1` | program terminate; custom break code, no C form |

  A function with no prologue, no frame, no callee-saves, that *constructs* `$sp`, `$gp`
  and `$fp` and stores `$ra` in a global, is hand-written by definition — this is the code
  that establishes the environment C compilation presupposes. Two of its instructions
  carry splat's own `/* handwritten instruction */` tag, and it contributes 2 of the 3
  queued trapping-arith hits measured above.

  Secondary finding: the `jal` at `0x80083788` targets
  `cpu_set_move_command_and_dir_for_no_action_2`. A crt0's final call is `main`. That
  symbol name is almost certainly drift and worth re-checking independently of this packet.

- **Splat / symbol_addrs implications — read before acting.** The split is cheap but the
  obvious route is booby-trapped:

  1. **Do NOT add `0x800836EC` to `symbol_addrs.txt` and re-run splat.** `CLAUDE.md` and
     `splat.yaml` both record that **`bb2.ld` is hand-maintained and `make setup` must not
     be run** — it re-adds dead rodata lines and conflicts with the const declarations now
     living in `src/*.c`. Regenerating to pick up one symbol would cost a `bb2.ld`
     recovery.
  2. **The manual split is byte-neutral and is the route to take.** `INCLUDE_ASM` expands
     (`include/include_asm.h:7-15`) to `.section .text` + `.set noat/noreorder` +
     `.include "<folder>/<name>.s"`. So: cut lines 12-54 of `asm/funcs/ang_hosei.s` into a
     new `asm/funcs/<newsym>.s` with its own `glabel`/`endlabel`, close `ang_hosei.s` after
     line 11, and put two consecutive `INCLUDE_ASM` lines in `src/ings2.c` where line 609
     is now. The instruction stream, its order and its alignment are unchanged (both
     halves are 4-byte aligned and contiguous at `0x800836E8 -> 0x800836EC`), and
     `.section .text` at an already-aligned offset emits no padding. It adds a symbol and
     zero bytes.
  3. `symbol_addrs.txt` should still get the name so future splat runs and `named_syms.txt`
     agree, but it is documentation here, not the mechanism.
  4. **Oracle-verify the split on its own commit, before any authorization commit.** It is
     byte-neutral by construction but it touches the build pipeline, so it should stand or
     fall on `verify-oracle` alone rather than being bundled with a rules change.
  5. Suggested symbol name: `_start` or `main_entry` (whichever matches the project's
     convention for the PS-EXE entry; nothing currently claims either).

- **Consequence for the queue.** Once split, `ang_hosei`'s recorded distance of 51 is
  wrong by construction — the 9-instruction trampoline is the whole function, and the
  42-instruction stub becomes a separate item. `docs/HANDOFF-2026-08-06B.md:51` already
  records this ("recorded distance 51 overstates the 9-insn function"). A `queue regen`
  after the split will correct it.

- **Proposed entry (approve verbatim, for the new symbol):**

```
<newsym>  # PS-EXE entry point / crt0 at 0x800836EC (AGENTS.md records this as the executable's entry). No prologue, no frame, no callee-saves: zeroes BSS from D_800A3308 to D_801078E0, computes and ASSIGNS $sp from D_800A2690 via a trapping `addi` (splat-tagged 'handwritten instruction'), strips KSEG0 off the heap base by sll/srl, saves $ra into the GLOBAL D_800A3668 because no stack frame exists yet, loads $gp from _gp, sets $fp from $sp, calls bios_InitHeap with a second trapping `addi` in the delay slot, then jal's main and terminates with `break 0, 1`. Startup code that establishes the environment C compilation presupposes — $sp/$gp/$fp assignment and a custom-code break have no C form. Split from asm/funcs/ang_hosei.s, where splat had fused it onto the unrelated 9-instruction function ending at 0x800836E8.
```

### `func_8004C388` — DECISIVE — recommend FOR (whole-function)

- **File:** src/text1b.c (HEAD line 1325) · **asm:** asm/funcs/func_8004C388.s · **30 instructions** ·
  queue: active, ASM-PARTIAL, distance 29, 0 rules (but see the injection finding above).
- **Shape.** A frameless, call-free leaf: six `lh` loads of two 3-vectors from `*$a0`/`*$a1`
  (`8004C388-8004C39C`), three trapping `add` (`8004C3A0`, `8004C3A4`, `8004C3A8`), three
  `sra 1` — a midpoint average — then three `sh` to `*$a2`. The tail
  (`8004C3C4-8004C3FC`) does the same midpoint on a packed byte pair out of halfword 3
  (`andi 0xFF00` / `andi 0xFF`, two more trapping `add`, `srl 1`, re-pack via `or`), with
  the final `sh $t3, 0x6($a2)` in the `jr $ra` delay slot.
- **Decisive signal.** **Five trapping `add`** at `8004C3A0/A4/A8/DC/E0`, every one carrying
  splat's `/* handwritten instruction */` tag. Per the corpus measurement above, trapping
  arithmetic appears in 64 authorized-canonical functions and **0 COMPLETED-C functions** —
  GCC 2.7.2 emits `addu` for C `+` unconditionally. There is no C input to the frozen
  compiler that produces these five bytes.
- **Why whole-function rather than island.** The five adds are not one span; they are
  interleaved through the whole body at instructions 8, 9, 10, 22 and 23 of 30. An island
  decomposition means five separate `__asm__` blocks with operand bindings surrounding
  ~25 instructions of load/shift/store — and the register allocation of that C would have
  to be steered to match, which is what the current source is doing with pins and hardcoded
  templates. The function is a 30-instruction hand-written leaf inside a 60-of-73
  authorized band; whole-function is the honest form.
- **Cluster fit.** Immediate neighbours `func_8004C1F4` (before) and `func_8004C404`
  (after) are both already authorized canonical, and both also contain trapping arith
  (22 and 16 instances). This function is a hole in a signed-off object file.
- **Proposed entry (approve verbatim):**

```
func_8004C388  # Hand-written midpoint-average leaf: 6x lh of two s16 3-vectors -> 5x TRAPPING `add` (splat-tagged 'handwritten instruction') -> sra/srl 1 -> 3x sh, plus a packed-byte-pair midpoint on halfword 3 with the final sh in the jr-ra delay slot. Frameless, call-free, no spills. GCC 2.7.2 emits `addu` for C addition unconditionally (addsi3); trapping `add` appears in 64 authorized-canonical functions and 0 COMPLETED-C functions corpus-wide, so these bytes are unreachable from any C. Sits between authorized func_8004C1F4 and func_8004C404 in a band where 60 of 73 functions are already authorized.
```

### `func_80052788` — DECISIVE — recommend FOR (whole-function)

- **File:** src/text1b.c (HEAD line 1628) · **asm:** asm/funcs/func_80052788.s · **29 instructions** ·
  queue: active, ASM-PARTIAL, distance 23, 0 rules (see the injection finding above).
- **Shape.** GTE `gpf`/`gpl` interpolation leaf. 15 of 27 body instructions are cop2:
  `ori $t3, $zero, 0x1000` then a **trapping `sub`** to form `0x1000 - t` (`80052798`),
  `mtc2 $t3/$t0/$t1/$t2 -> $8/$9/$10/$11`, `gpf 1` (`800527B4`), a second `mtc2` quartet,
  two unfilled GTE latency `nop`s, `gpl 1` (`800527D8`), then `mfc2` reads and three `sh`
  with the last in the `jr $ra` delay slot.
- **Three independent decisive signals.**
  1. **Trapping `sub`** at `80052798`, splat-tagged — the 64-vs-0 argument above.
  2. **`mfc2 $t0, $9` emitted twice in a row** at `800527DC` and `800527E0`, both
     splat-tagged. The second read is mathematically redundant and its destination is
     immediately overwritten. GCC's CSE collapses an identical back-to-back read
     unconditionally; a hand coder writes it as a GTE result-latency pad. This is the
     census's X1 signal and it is the same *kind* of construct as the S8
     redundant-mask signal that carried `func_8007EDBC` to authorization
     (`.claude/rules/packed-multiply-cluster.md`).
  3. `gpf`/`gpl` and the hand-placed latency `nop`s have no C form.
- **Cluster fit.** Immediate neighbours `func_80052754` (before) and `func_800527FC`
  (after) are both authorized canonical.
- **Proposed entry (approve verbatim):**

```
func_80052788  # GTE gpf/gpl interpolation leaf, 15/27 cop2: TRAPPING `sub` forms 0x1000-t (splat-tagged; GCC 2.7.2 emits subu for C subtraction — trapping arith is present in 64 authorized-canonical and 0 COMPLETED-C functions corpus-wide), 4x mtc2 -> gpf 1 -> 4x mtc2 -> two unfilled GTE latency nops -> gpl 1 -> mfc2 reads with `mfc2 $t0,$9` emitted TWICE back-to-back as a result-latency pad (GCC's CSE collapses an identical adjacent read unconditionally), then 3x sh with the last in the jr-ra delay slot. Frameless leaf between authorized func_80052754 and func_800527FC.
```

### `game_2d_CheckLifeGaugeNoDisp` — DECISIVE — recommend FOR (whole-function)

- **File:** src/text1b.c (HEAD line 1746) · **asm:** asm/funcs/game_2d_CheckLifeGaugeNoDisp.s ·
  **26 instructions** · queue: active, ASM-PARTIAL, distance 20, 1 rule
  (`regfix.txt:3321 fill_delay @ 24 <- 23`).
- **Shape.** Verbatim PsyQ LIBGTE matrix-vector multiply. Eight `lw` from `*$a0`
  (`80052A20-80052A3C`) feeding **eight splat-tagged `ctc2 $t0-$t7, $0-$7`**
  (`80052A40-80052A5C`), `lwc2 $0, 0x0($a1)` / `lwc2 $1, 0x4($a1)`, two unfilled GTE
  latency `nop`s, `mvmva 1,0,0,0,0` (`80052A70`), a third latency `nop`, then
  `swc2 $9/$10` and **`swc2 $11, 0x8($a2)` in the `jr $ra` delay slot** (`80052A84`).
  Zero general-purpose computation — 14 of 24 body instructions are cop2 and the other
  ten are the `lw` feed.
- **Decisive signal.** The delay-slot `swc2`. Per the granted `func_80052B44` ruling
  (batch 1 above, from `tools/gcc-2.7.2/reorg.c`): `stop_search_p` halts the delay-slot
  search at `ASM_INPUT` and `resource_conflicts_p` returns 1 for volatile asm, so **GCC
  2.7.2 can never place a cop2 op in a `jr $ra` delay slot**. `cop2` can only enter
  compilation as inline asm. The bytes are unreachable from any C input.
- **Precedent fit.** This is the same construct as authorized `func_8007ED6C`
  (`inline_asm_canonical.txt:308`) — the census measures opcode-signature Jaccard 0.704
  against it — and the direct sibling of `func_80052A88` / `func_80052B7C`, both rated
  DECISIVE in batch 1. It is the plainest member of the family: no halfword packing at all.
- **Naming note (not blocking).** The symbol is misleading. `game_2d_CheckLifeGaugeNoDisp`
  reads as a 2D UI predicate returning a flag; the body is a GTE 3x3-matrix × vector
  multiply writing three words through `$a2` and returning nothing. Its two call sites
  (`src/config.c:263-264`) pass three pointers, consistent with the GTE reading and not
  with the name. Worth re-checking alongside the `ang_hosei` family drift.
- **Proposed entry (approve verbatim):**

```
game_2d_CheckLifeGaugeNoDisp  # LIBGTE matrix-vector leaf (MISNAMED — body is a GTE 3x3 x vector multiply, not a UI predicate): 8x lw <- *a0 -> 8x ctc2 $0-$7 (all splat-tagged 'handwritten instruction'), lwc2 $0/$1 <- *a1, two unfilled GTE latency nops, mvmva 1,0,0,0,0, a third latency nop, then swc2 $9/$10 and swc2 $11 IN the jr-ra delay slot (0x80052A84). Zero general-purpose computation. GCC 2.7.2 cannot fill a delay slot with asm (reorg.c stop_search_p halts at ASM_INPUT), so the bytes are unreachable from any C. Opcode-signature Jaccard 0.704 to authorized func_8007ED6C; direct sibling of authorized func_80052B44.
```

### `func_80052930` — STRONG — recommend FOR (whole-function; island fallback delimited)

- **File:** src/text1b.c (HEAD line 1674) · **asm:** asm/funcs/func_80052930.s · **60 instructions** ·
  queue: active, ASM-PARTIAL, distance 51, 0 rules. Currently carries 11 `register asm("$N")`
  pins in HEAD (`src/text1b.c:1675-1685`).
- **Shape.** The three-cycle sibling of the family: eight splat-tagged
  `ctc2 $t0-$t4, $0-$4` + `ctc2 $zero, $5-$7` (`80052948-80052964`) load a packed 3x3
  rotation matrix with zero translation, then **three `mvmva 1,0,0,0,0` cycles**
  (`8005299C`, `800529CC`, `80052A00`), each writing its vector via `mtc2 $0/$1` and
  reading the previous cycle's result via `mfc2 $t5/$t6/$t7, $9/$10/$11`.
- **Why STRONG and not DECISIVE.** There is no single GCC-impossible instruction here:
  the delay slot holds an ordinary `sh $t7, 0x10($a2)` (GCC fills those routinely), there
  is no trapping arithmetic, and volatile `__asm__` blocks are not reordered by GCC, so
  an island form is *technically* constructible. I am not going to call that decisive.
- **Why the evidence is still strong.**
  1. **The interleave is hand-scheduling.** Cycle N+1's `mtc2` setup and cycle N's `mfc2`
     result reads are placed *inside* the preceding `mvmva`'s latency window, and the
     general-purpose packing (`and`/`andi`/`or`/`sll`/`srl` at
     `8005297C-80052994`, `800529A0-800529A8`, `800529D0-800529DC`) is threaded into the
     same windows. `pre-slim-2026-10-01:.claude/rules/gte-3x3.md` names exactly this — "the
     mvmva→mfc2→mtc2→nop→mvmva pipeline interleaving (cycle N+1's setup during cycle N's
     GTE latency) is canonical hand-scheduling" — as the signature that carried
     `calc_fc_frame_8007EC5C` to ASM-WHOLE authorization on 2026-05-31.
  2. **The `0xFFFF0000` mask is materialised once** into `$t9` at `80052944`, before the
     first `ctc2`, and held live across all 60 instructions for three uses. `$t9` is a
     caller-saved temp; GCC's CSE/remat would not reserve one across a 55-instruction span
     for a constant that costs one `lui`.
  3. **Cluster.** It sits in the middle of the `0x80052788-0x80052B7C` run, immediately
     after authorized `func_800527FC` and immediately before
     `game_2d_CheckLifeGaugeNoDisp`; the band is 60-of-73 authorized.
  4. It is a frameless, call-free, spill-free leaf (census: `S3:no-spills`).
- **Island fallback, delimited.** If the owner prefers the region-granular disposition,
  the cop2 spans are **10 regions**, 26 of 60 instructions:

  | Region | Range | Ops |
  |---|---|---|
  | 1 | `80052948-80052964` | `ctc2 $t0-$t4, $0-$4`; `ctc2 $zero, $5-$7` (8) |
  | 2 | `80052988` | `mtc2 $t5, $0` |
  | 3 | `80052990` | `mtc2 $t6, $1` |
  | 4 | `8005299C` | `mvmva 1,0,0,0,0` |
  | 5 | `800529AC-800529B4` | `mfc2 $t5/$t6/$t7, $9/$10/$11` |
  | 6 | `800529B8-800529BC` | `mtc2 $v0, $0`; `mtc2 $v1, $1` |
  | 7 | `800529CC` | `mvmva 1,0,0,0,0` |
  | 8 | `800529E0-800529E8` | `mfc2 $t5/$t6/$t7, $9/$10/$11` |
  | 9 | `800529EC-800529F0` | `mtc2 $v0, $0`; `mtc2 $v1, $1` |
  | 10 | `80052A00-80052A0C` | `mvmva`; `mfc2 $t5/$t6/$t7` |

  Ten `__asm__` blocks separated by one-to-four instructions of packing, in a
  60-instruction leaf, is the argument *against* the island form rather than for it — but
  the decomposition is recorded so the owner can rule either way on facts rather than on
  my framing.
- **Proposed entry (approve verbatim, whole-function disposition):**

```
func_80052930  # LIBGTE 3-cycle matrix-vector leaf: 5x ctc2 $0-$4 (packed 3x3 R matrix) + 3x ctc2 $zero to $5-$7 (zero translation), all splat-tagged 'handwritten instruction', then three mvmva 1,0,0,0,0 cycles with each cycle's mtc2 $0/$1 setup and the previous cycle's mfc2 $9/$10/$11 result reads placed INSIDE the preceding mvmva's latency window, and the halfword packing threaded into the same windows. Canonical hand-scheduling per the calc_fc_frame_8007EC5C precedent (pre-slim-2026-10-01:.claude/rules/gte-3x3.md, authorized 2026-05-31). The 0xFFFF0000 mask is materialised once into caller-saved $t9 and held live across all 60 instructions for three uses. Frameless, call-free, spill-free leaf in the 0x80052788-0x80052B7C hand-written run.
```
