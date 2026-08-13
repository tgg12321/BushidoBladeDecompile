# HYPOTHESES — func_8001E404

## Session 1 (recon, 2026-08-12)

### H1 — "the 23-point residual is a multi-cause codegen gap that recon should map"
**Mechanism proposed:** ordinary register-allocation / scheduling divergence spread
across the body.
**Probe:** normalized instruction diff of the sandbox object AND the real build object
against `asm/funcs/func_8001E404.s` (`tmp/grind/func_8001E404/s1/diff.py`).
**Result:** real build = 184/184 instructions, ZERO real differences (function
byte-matches today); sandbox build differs only by a uniform −8 shift of every
`$sp`-relative offset plus the one knock-on address-materialization choice it forces.
**Verdict: KILLED.** There is no codegen gap. The entire residual is one 8-byte
allocated-but-never-touched leading frame region, stripped along with the committed
`pre_pad[2]` construct by the cheat-invisible sandbox.

### H2 — "`pre_pad[2]` is inert / DCE'd, so the honest floor is unrelated to it"
**Mechanism proposed:** GCC 2.7.2 deletes unused local arrays, so the construct cannot
be what produces the frame geometry.
**Probe:** compare the real build object (pre_pad present) vs the sandbox object
(cheat-stripped) vs target.
**Result:** pre_pad present ⇒ frame 0x70, vars base `sp+0x18`, byte-identical to target;
stripped ⇒ frame 0x68, vars base `sp+0x10`, 23 differing slots. The construct is fully
load-bearing; the "GCC DCEs it" reading is a sandbox-strip artifact (same disproof the
sibling recorded in its own s1).
**Verdict: KILLED.**

### H3 — "this is the same defect as the parked sibling func_8001E6E4"
**Mechanism proposed:** identical single-phantom-region frame defect; the sibling's
white-box frame partition (declaration-order theorem, empty pre-declaration window,
args-partition store requirement, top-growing expansion temps, zero pretend_args_size)
transfers unchanged because it is the same compiler, same file, same frame shape.
**Probe:** binary-wide hole census (`tmp/grind/func_8001E6E4/s7/hole_census.py`) +
frame gradient on the honest vs committed form — the exact two measurements
`docs/grind/decisions.md:4646-4648` prescribes for this case.
**Result:** census row `8 func_8001E404 112 0x18 0x60 code6cac.c` — identical to
`func_8001E6E4`; only four functions in 1434 have the shape and three are this family.
Frame gradient: honest 0x68/base 0x10 vs committed 0x70/base 0x18 == target.
**Verdict: CONFIRMED.**

### H4 — endgame gates for this function
**Probe:** `scan_hand_coded --single func_8001E404`; scope-sentence test of the
written-never-read local-array carve-out against the measured target bytes.
**Result:** Gate 1 `tier=LOW score=1/8` (S1/S2/S6 all absent) — FAIL. Gate 2: target
contains NO stores in `sp+0x10..0x17`, which is the carve-out's own stated
precondition — FAIL. No in-hand SOTN-master citation exists for an unwritten leading pad.
**Verdict: CONFIRMED (both gates fail).**

### H5 — "the 8 bytes live in the LOCALS partition (a pad below the first local)"
**Mechanism proposed:** `assign_stack_local` places something before the work buffer.
**Probe:** read the frame equation end-to-end — `STARTING_FRAME_OFFSET`
(mips.h:1651) + `compute_frame_size` (mips.c:4444-4535) — and enumerate every term.
**Result:** the first slot's address is *by definition* `$sp +
current_function_outgoing_args_size` (FRAME_GROWS_DOWNWARD undefined, TARGET_ABICALLS
off, no rounding at that point). Both forms have `get_frame_size() == 72`; the term
that differs is `current_function_outgoing_args_size` (24 original vs 16 ours).
`extra_size` ≡ 0 (non-ABICALLS) and the `pretend_args_size` term is gated on
`ABI_64BIT && mips_isa >= 3` ≡ 0 on o32 — no other term exists.
**Verdict: KILLED.** The residual is an ARGS-partition fact. Every locals-side
spelling (pad, lead array, struct lead, declaration reorder) is attacking the wrong
term of the equation — which is also why all 19 sibling spellings failed.

### H6 — "one of the callees originally took a 5th (stack) word argument, which is
what pushed outgoing_args_size to 24"
**Mechanism proposed:** on o32 a call with >4 words of arguments raises
`current_function_outgoing_args_size` (ACCUMULATE_OUTGOING_ARGS, mips.h:1787).
**Probe:** `tmp/grind/func_8001E404/s1/callee_stackarg_scan.py` — for each in-EXE
callee, scan its asm for any `$sp`-relative access at or above its own frame size
(the only place an incoming stack argument can be read).
**Result:** func_80046BF4 (frame 0x70), func_8001A538 (0x40), func_80061064 (0x20),
func_80041688 (0x38), func_8003F3D4 (leaf, 0x0) and the sibling func_8001E6E4 (0x70)
— **zero above-frame accesses in all six**. No callee takes a stack argument.
**Verdict: KILLED.** The direct-call route to args=24 is dead from both ends: no
callee reads such an arg, and the target stores nothing in `sp+0x10..0x17`.

## Frontier
Rebuilt this run around the args-partition reframing (H5). All three items are
probes of `current_function_outgoing_args_size`, the single term that must read 24.

1. **Expand-time-only args bump.** `expand_call` raises
   `current_function_outgoing_args_size` while expanding a call; `compute_frame_size`
   reads it at final. If a call site's insns are later removed (cse/jump
   const-folding an unreachable block, `delete_insn` in jump.c), the frame keeps the
   raised args size with no surviving stores — exactly the target's shape.
   *Next probe:* a throwaway TU with a 5-arg call in a block RTL (not the front end)
   proves unreachable; compile with the project cc1 flags and read the `# vars= …
   args= …` comment cc1 emits above `.frame` plus the emitted stores. This
   establishes the FACT first; whether any C shape that triggers it clears the cheat
   bar is a separate question to be answered only after the fact is known.
2. **Library-call route.** `emit_library_call` also feeds
   `current_function_outgoing_args_size`. Enumerate which GCC-2.7.2 libcalls
   reachable from plausible C in this body (block move for the `local = *(CamBuf *)s2`
   struct copy, 64-bit ops, float ops) report an args size above 16 with
   OUTGOING_REG_PARM_STACK_SPACE in effect. *Next probe:* compile one variant per
   libcall family, read the `# args=` comment; a hit gives an honest, human-plausible
   producer with no stack-arg store of our own.
3. **Instrument the args term directly.** Build the `# vars=/regs=/args=/extra=`
   comment gradient into the grind loop for this function: compile the current
   honest form and each candidate through cc1 to `.s` (not `.o`) and read the frame
   comment, so every future spelling is scored on the term it actually moves
   (args vs vars) instead of on the byte diff alone. Cheap, reusable for
   func_8001E6E4 and func_8003CF84 (the other two members of the census family). Standing context, NOT a disposition: `docs/grind/decisions.md:4640-4649` names this
function as a member of the three-function phantom-frame-hole family
(func_8001E6E4 / func_8001E404 / func_8003CF84), and both endgame gates measured
FAIL for it (H4). That matters only once the DRIVER declares exhaustion and assigns
`escalation` modality — it is not a reason to stop grinding in any other modality,
and an earlier owner-gated attempt from `recon` was correctly discarded on exactly
that ground.

Do NOT re-grind the 19 locals-side spellings in the sibling's ledger — H5 explains
why the whole class is mis-aimed (wrong term of the frame equation). The live search
is the args term only.

## [s1] The 23-point residual is a multi-cause codegen gap (RA/scheduling) spread across the body.
- mechanism: ordinary register-allocation / instruction-scheduling divergence
- probe: normalized instruction diff of the sandbox object and the real build object against asm/funcs/func_8001E404.s; plus a full $sp-offset map of the target
- result: real build = 184/184 with zero real differences (the function byte-matches today); the cheat-stripped sandbox build differs only by a uniform -8 shift of every $sp-relative offset (frame 0x70->0x68, saves 0x6C/68/64/60 -> 0x64/60/5C/58, buffer base 0x18->0x10) plus the one knock-on address-materialization choice that shift forces (target does `addiu $s0,$sp,0x18` once + `move $a0,$s0` twice = 184 insns; the short-frame build re-derives `addiu $a0,$sp,0x10` twice = 183).
- verdict: KILLED

## [s1] The 8 missing bytes live in the LOCALS partition, i.e. something must be allocated below the first declared local.
- mechanism: assign_stack_local placing an object ahead of the work buffer (the framing that produced every pad/lead-array spelling attempted on this family)
- probe: read the frame equation end-to-end in the frozen toolchain: STARTING_FRAME_OFFSET (tools/gcc-2.7.2/config/mips/mips.h:1651) and compute_frame_size (tools/gcc-2.7.2/config/mips/mips.c:4444-4535), enumerating every term and its o32 value
- result: FRAME_GROWS_DOWNWARD is undefined (mips.h:1645) and TARGET_ABICALLS is off, so STARTING_FRAME_OFFSET reduces to `current_function_outgoing_args_size` verbatim, unrounded: the first stack slot is at exactly $sp+outgoing_args_size. compute_frame_size has no other term that can carry the 8 bytes — extra_size = MIPS_STACK_ALIGN(TARGET_ABICALLS ? 4 : 0) = 0, fp_reg_size = 0, and the pretend_args_size term is gated on `ABI_64BIT && mips_isa >= 3` = 0 on o32. Target: first local at 0x18, saves at 0x60, frame 0x70 => get_frame_size() = 72 in BOTH forms; the single differing term is current_function_outgoing_args_size, 24 in the original vs 16 in our honest build.
- verdict: KILLED

## [s1] One of this function's callees originally took a 5th (stack) word argument, which is what raised outgoing_args_size to 24.
- mechanism: ACCUMULATE_OUTGOING_ARGS (mips.h:1787) + OUTGOING_REG_PARM_STACK_SPACE: a call with more than 4 words of arguments raises current_function_outgoing_args_size past the 16-byte REG_PARM_STACK_SPACE floor
- probe: tmp/grind/func_8001E404/s1/callee_stackarg_scan.py — for every in-EXE callee, scan its asm for any $sp-relative access at or above its own frame size (the only place an incoming stack argument can be read)
- result: zero above-frame accesses in all six scanned functions: func_80046BF4 (frame 0x70), func_8001A538 (0x40), func_80061064 (0x20), func_80041688 (0x38), func_8003F3D4 (leaf, frame 0), and the sibling func_8001E6E4 (0x70). No callee in the call set takes a stack argument. Combined with the target's own $sp map (lowest touched offset 0x18, no access anywhere in sp+0x10..0x17), the ordinary store-the-stack-arg route to args=24 is refuted from both ends.
- verdict: KILLED

## [s1] This function is the same single-phantom-frame-region defect as the parked sibling func_8001E6E4 named in docs/grind/decisions.md:4640-4649.
- mechanism: same compiler, same file, same 0x70 frame, same 8-byte untouched region; the sibling's measured frame analysis transfers
- probe: binary-wide hole census (four functions in 1434 have the shape) + the frame gradient on the honest vs committed form
- result: census row `8 func_8001E404 112 0x18 0x60 code6cac.c` is identical to func_8001E6E4's; honest form = frame 0x68 / first local 0x10, committed form = 0x70 / 0x18 = target. CONFIRMED as the same family — but this session's H5 shows the family's shared framing (a locals-side pad) was wrong, so the sibling's 19 measured locals-side spellings were all aimed at the wrong term and should not be re-run.
- verdict: CONFIRMED
