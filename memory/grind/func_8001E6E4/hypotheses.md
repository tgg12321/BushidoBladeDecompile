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
