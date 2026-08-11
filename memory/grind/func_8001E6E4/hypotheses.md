# Hypothesis ledger — func_8001E6E4

## Frontier (after session 1, recon)

The single gap: honest build needs vars=80 (frame 112) instead of vars=72,
codegen-neutral (all 71 insns already match modulo the +8 sp shift). The
gradient instrument is `tmp/frame_probe.sh func_8001E6E4 code6cac` (cc1
`.frame` line) — use it before any sandbox run; it is the only signal that
matters until vars=80, then sandbox must stay 19→0 (i.e. codegen unchanged).

### H1 — expansion-temp census via -da dumps (forensics)
The target's 8 phantom bytes came from SOME pseudo/temp that cc1psx's
expansion gave a stack slot but allocation kept in registers, or an
unallocated pseudo reload's alter_reg paid off ([[phantom-slot-frame-lever]]).
Session-1 blind spelling probes (compare local, u16 temps, dist scalar) all
failed to create one. Next probe: compile the honest form with `-da`, read the
greg dump's pseudo table + `Register dispositions` — enumerate EVERY pseudo,
confirm none is unallocated, then reason from mips.c/function.c
(assign_stack_local call sites) about which C shapes force an expansion-time
stack temp of exactly 8 bytes on THIS toolchain (e.g. structure-return temps,
DImode intermediates, aggregate temps from struct assignment). The sibling
function's `local = *(CamBuf *)s2;` struct-copy arm is a known aggregate-temp
producer candidate worth dumping too.

### H2 — joint-sibling / true-struct evidence hunt
func_8001E404 (same file, honest distance 23, same committed pre_pad cheat)
has the IDENTICAL +8 phantom region. Whatever original C produced it, it is
almost certainly the same idiom in both. Two evidence paths: (a) read the
three callees (func_80046BF4, func_8001A538 = MATRIX-arg function per commit
7f37c94c, func_80061064) for the REAL aggregate types they take — if arg0 is
genuinely a known PsyQ struct (e.g. part of a camera work area) whose layout
puts vx 8 bytes into a larger struct, the struct-lead layout becomes
evidence-grounded (this is the NEW-evidence condition the pad_lead rejection
explicitly left open); (b) any honest producer found on either sibling
transfers to the other — probe both with the frame gradient.

### H3 — Kengo cross-reference for original decomposition
The file carries a `kengo:MED | nm_mario_test/mario_test_Exec` annotation just
above the sibling (src/code6cac.c:1374). The Kengo (PS2 successor, partial
source) camera-work function family may show the original struct/local
declarations for this exact idiom (camera work buffer + rot + dist). A hit
naming the real leading 8 bytes (e.g. a `VECTOR`-preceding member or a
2-word header) converts the mechanically-proven struct-lead form from cheat
to evidence-grounded. Probe: search the Kengo source tree for the
mario_test/camera exec pattern feeding three callees with (&pos, &rot, dist).

## Killed this session (do not re-run)
- Named range-compare local (`u32 range = arg0 - 0x555;`) — no orphan, vars=72.
- Named u16 component temps — no HImode expression exists, vars=72.
- Named `s32 dist` scalar — register-allocated, no stack home, vars=72.
- Table-type lead (D_800F5328/D_800F6608 declared struct) — bare s32 externs,
  nothing in-tree.
- Duplicates scan — no leads for this pair.

## [s1] The candidate.c p0 named-intermediate form reproduces floor 19 on main (HEAD form is 21)
- mechanism: named-intermediate declaration order (sanctioned SOTN family) already vetted by prior session's reviewer
- probe: applied candidate form to src/code6cac.c, sandbox --disable all
- result: score 19, build_insns == target_insns == 71; left in src as final state
- verdict: CONFIRMED

## [s1] Unreferenced local arrays are DCE'd by GCC 2.7.2 and mechanically inert (prior session's claim)
- mechanism: claimed GCC DCE; actually the sandbox cheat-stripper deletes the array before compiling
- probe: frame gradient (cc1 .frame line) with and without s32 pre_pad[2]
- result: WITH pre_pad: frame 112/vars=80 (== target); WITHOUT: 104/72. pre_pad is load-bearing in the real build; the committed form byte-matches only via this cheat. Prior evidence note corrected in evidence.md
- verdict: KILLED

## [s1] A named range-compare local orphans its compare pseudo and reserves a phantom slot (phantom-slot producer #1)
- mechanism: folded loop-guard compare leaving unallocated pseudo for reload alter_reg
- probe: u32 range = arg0 - 0x555; if (range >= 0x556U) — frame probe
- result: vars=72 unchanged; straight-line range check materializes sltiu which consumes the pseudo in $a0; producer requires a loop-guard fold this function does not have
- verdict: KILLED

## [s1] Named u16 component temps create HImode stack temps (tslLineG5Init witness analog)
- mechanism: HImode expansion temps counted by get_frame_size
- probe: u16 rx/ry/rz named loads feeding the three s16 field adds — frame probe
- result: vars=72 unchanged; lhu results widen to SImode immediately and all adds are SImode; the witness's trigger (HImode bitwise expression) has no semantic analog here
- verdict: KILLED

## [s1] Named s32 dist scalar (m2c temp_a2/sp30 shape) gets an expansion-time stack home
- mechanism: expansion-time assign_stack_local for a live named scalar
- probe: s32 dist = ...; local.dist = dist; pass dist — frame probe
- result: vars=72 unchanged; scalar fully register-allocated to $a2, no stack home; codegen-neutral but frame-neutral
- verdict: KILLED

## [s2] A named select-pointer / second-handle / output intermediate orphans a pseudo at the guard-join label (combine orphan-USE producer)
- mechanism: combine distribute_notes REG_DEAD-at-CODE_LABEL bare-USE orphan; alter_reg pays off the slot (phantom-slot-frame-lever producer #2 analog)
- probe: frame gradient over sel-intermediate, sel-late-bind, u8*-base handle, out-intermediate, register-s2, u8*-primary spellings (sweep.py/sweep2.py)
- result: ALL vars=72; every intermediate coalesces or register-allocates cleanly; no orphan
- verdict: KILLED

## [s2] Guard re-association (two-sided compare / ternary / named range / split-init) leaves a folded compare pseudo without a home (producer #1 analog)
- mechanism: fold of a two-arm range test at tree level or store-flag intermediate stranding at reload
- probe: frame gradient over two-sided-compare, ternary-select, named-range, split-init-dist spellings
- result: ALL vars=72; fold happens at tree level (no pseudo ever exists) or the flag is branch-consumed
- verdict: KILLED

## [s2] Declaration order / granularity (separate pos/rot/dist/tail locals, either order) pads the frame to 80
- mechanism: per-local assign_stack_local alignment padding
- probe: frame gradient on B12/B12b separate-locals forms (diagnostic-only: dead tail) + B3 reversed order
- result: ALL vars=72; GCC 2.7.2 packs these locals with zero inter-local padding (16+8+4+44=72 exactly)
- verdict: KILLED

## Frontier after s2
Unchanged from s1: H1 (-da forensics: enumerate expansion temps + unallocated
pseudos, census assign_stack_local-forcing C shapes), H2 (callee-type evidence
hunt: func_80046BF4 / func_8001A538 / func_80061064 bodies+callers, joint with
sibling func_8001E404), H3 (Kengo source cross-reference). The structural
spelling space (named intermediates, handles, declaration order/granularity,
guard re-association) is measured dead across 15 forms — next session should
NOT re-sweep it; the producer must be identified forensically or evidenced
externally.

## [s2] A named select-pointer / second-handle / output intermediate orphans a pseudo at the guard-join label (combine orphan-USE, phantom-slot producer #2 analog)
- mechanism: combine distribute_notes REG_DEAD-at-CODE_LABEL bare USE; reload alter_reg pays off a stack slot counted by get_frame_size
- probe: cc1 .frame gradient over V1 sel-intermediate, V2 sel-late-bind, V3 u8*-base handle, V4 out-intermediate, B14 register-s2, B15 u8*-primary (tmp/grind/func_8001E6E4/s2/sweep.py + sweep2.py)
- result: ALL vars=72 (frame 104); every intermediate coalesces or register-allocates cleanly, no orphan created
- verdict: KILLED

## [s2] Guard re-association (two-sided compare, ternary select, named range flag, split-init dist) strands a folded compare pseudo without a home (producer #1 analog)
- mechanism: tree-level range fold or store-flag intermediate left unallocated at reload
- probe: frame gradient over V5 two-sided compare, B9 ternary, V7 named range, B4 split-init-dist
- result: ALL vars=72; the fold happens at tree level (no pseudo exists) or the flag is branch-consumed; target's own addiu/sltiu/bnez chain survives in bytes so the guard is not the producer
- verdict: KILLED

## [s2] Declaration order / granularity (separate pos/rot/dist/tail locals, either order; pointer-before-aggregate) pads the frame to 80
- mechanism: per-local assign_stack_local alignment padding
- probe: frame gradient on B12/B12b separate-locals (diagnostic-only measurement - dead tail never a candidate) and B3 reversed declaration order
- result: ALL vars=72; GCC 2.7.2 packs these locals with zero inter-local padding (16+8+4+44 = 72 exactly)
- verdict: KILLED
