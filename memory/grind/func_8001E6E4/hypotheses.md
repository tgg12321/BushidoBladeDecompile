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

## [s3] H2: the three callees' real argument types reveal a genuine camera struct whose vx sits 8 bytes in
- mechanism: evidence-grounding the mechanically-proven struct-lead layout (the pad_lead rejection's explicit reopen condition)
- probe: read all three callee bodies + the table writer + caller census (src/sound.c:274, src/code6cac.c:687, src/text1b.c:3638, src/code6cac.c:796)
- result: func_80046BF4 takes discrete (s32 *pos, u16 *rot, s32 dist); func_8001A538 reads its arg0 with vx at OFFSET 0 (0x0/4/8, 0x10/12/14, 0x18) — affirmatively contradicts leading fields; func_80061064 is void(void) and ignores its args; func_8001B3C0 writes the table head as bare word globals; func_8001E6E4 has no in-tree C callers
- verdict: KILLED

## [s3] H3: Kengo source shows the original local declarations for this camera-exec idiom
- mechanism: Marionation engine reuse; the file's kengo:MED annotation adjacent to the pair
- probe: Kengo/ tree census + disassembly of mario_test_Exec (0x135a90), marionation_camera_Exec (0x135560), mottest_camera_control (0x11a500) — artifacts in tmp/grind/func_8001E6E4/s3/
- result: no Kengo C source or C-locals debug info exists in-repo (kengo_debug_full.txt is .dsm/.vsm line info only); the annotated mario_test_Exec is a Pad_Rpt/fnt_print debug menu (false lead); the PS2 camera layer was rewritten (float/VU0/global-based, no-arg helpers) — the PS1 local work-buffer idiom does not survive
- verdict: KILLED

## [s3] A genuinely-used pointer-array local (s32 *ps[2] feeding the three calls) reserves the 8 phantom bytes at zero codegen cost
- mechanism: GCC 2.7.2 allocates local arrays in the frame; array uses might CSE into registers leaving only the slot
- probe: tmp/grind/func_8001E6E4/s3/probe_ps_array.py frame gradient + emitted-asm inspection (variant_ps_array.s)
- result: vars=80 / frame 112 reached (first honest spelling to do so) BUT the array stays memory-resident: sw $4,16($sp) / sw $2,20($sp) / lw $4,16($sp) materialize and regs drop 4/0 -> 3/0 (callee-save s2 freed, saves land at wrong offsets). Not codegen-neutral
- verdict: KILLED

## Frontier after s3
H1 forensics ONLY: -da greg/combine dump census (enumerate every pseudo +
confirm none unallocated; census assign_stack_local / assign_stack_temp call
sites; then aggregate-temp-forcing C shapes screened with the frame gradient).
Structural spelling space is closed (19 forms across s1/s2/s3: every honest
addressable-8-byte local costs instructions, every register-allocatable
spelling stays vars=72). Both external evidence axes (callee types, Kengo) are
measured dead. The sibling func_8001E404's struct-copy arm
(`local = *(CamBuf *)s2;`) remains the one named aggregate-temp producer
candidate to dump alongside the -da census.

## [s3] The three callees' real argument types reveal a genuine camera struct whose vx sits 8 bytes in (H2, the pad_lead rejection's reopen condition)
- mechanism: evidence-grounding the mechanically-proven struct-lead layout via externally-verifiable field semantics
- probe: read all three callee bodies + table writer + caller census: src/sound.c:274, src/code6cac.c:687, src/text1b.c:3638, src/code6cac.c:796
- result: func_80046BF4 takes discrete (s32 *pos, u16 *rot, s32 dist); func_8001A538 reads arg0 with vx at OFFSET 0 (0x0/4/8, 0x10/12/14, 0x18) - affirmatively contradicts leading fields; func_80061064 is void(void) ignoring its args; func_8001B3C0 writes the table head as bare word globals; no in-tree C callers of func_8001E6E4
- verdict: KILLED

## [s3] Kengo source shows the original local declarations for this camera-exec idiom (H3)
- mechanism: Marionation engine reuse; kengo:MED nm_mario_test/mario_test_Exec annotation at src/code6cac.c:1374
- probe: Kengo/ tree census + disassembly of mario_test_Exec, marionation_camera_Exec, mottest_camera_control (artifacts in tmp/grind/func_8001E6E4/s3/)
- result: no Kengo C source or C-locals debug info exists in-repo (kengo_debug_full.txt is .dsm/.vsm asm line info only); the annotated mario_test_Exec is a Pad_Rpt/fnt_print debug menu (false lead); the PS2 camera layer was rewritten float/VU0/global-based with no-arg helpers - the PS1 local work-buffer idiom does not survive
- verdict: KILLED

## [s3] A genuinely-used pointer-array local (s32 *ps[2] feeding the three calls) reserves the 8 phantom bytes at zero codegen cost - the one honest addressable-8-byte spelling s1/s2 never swept
- mechanism: GCC 2.7.2 allocates local arrays in the frame; array element uses might CSE into registers leaving only the slot
- probe: tmp/grind/func_8001E6E4/s3/probe_ps_array.py cc1 .frame gradient + emitted-asm inspection
- result: vars=80 / frame 112 reached (first honest spelling to hit target frame size) BUT the array stays memory-resident: sw $4,16($sp) / sw $2,20($sp) / lw $4,16($sp) materialize and regs drop 4/0 -> 3/0 (save block at wrong offsets). Not codegen-neutral
- verdict: KILLED

## [s4] Random/directed permuter search finds an honest vars=80 frame producer (permuter modality over the closed structural space)
- mechanism: permuter mutation classes (temp_for_expr, reorder, ins_block, type-randomize, struct_ref, ...) explore spellings beyond the 19 hand forms; --stack-diffs scorer makes the +8 shift visible; any score-0 without dead/volatile constructs would be an honest producer
- probe: 3 campaigns (tmp/grind/func_8001E6E4/s4/perm, telemetry in metrics/events.jsonl): default weights -> score-0 at iter 461; perm_pad_var_decl=0 reseed; wp-chassis reseed, 43k+ iters
- result: the ONLY score-0 basin is `volatile unsigned short pad;` (unwritten volatile dead scalar, forbidden family — rejected per policy, banked at rejected/permuter-volatile-dead-scalar.c). Written-volatile variant reaches vars=80 but materializes its store (score 100). Non-volatile dead scalars/pointers do NOT move the frame (vars=72). No honest vars=80 form exists in the mutation-reachable space
- verdict: KILLED

## [s4] A staged work-pointer (`wp = &local;` mid-block, first call via wp) closes the residual non-shift codegen divergence at zero cost (permuter output-140, DISCOVERED not searched-for)
- mechanism: the named pointer intermediate makes GCC stage &local into callee-save $s0 (addiu s0,sp,N in the former nop slot) and use `move a0,s0` for the first call — the shape target has, and the shape the real 112-frame build already produced naturally; at 104 the honest build previously emitted nop + 2x addiu a0,sp,16 instead
- probe: asm diff find-vs-base-vs-target (s4/asmdiff.sh); byte-equality probe with pre_pad at frame 112 (s4/test_wp_prepad.sh); engine sandbox on src with wp applied
- result: honest build now 71/71 with EXACTLY the 19-insn +8 sp shift and zero structural divergence; with pre_pad: byte-identical (diff 0) — oracle-safe; sandbox --disable all = 19 (metric unchanged, gap now purely the phantom slot). Banked into src + candidate.c
- verdict: CONFIRMED

## Frontier after s4
H1 forensics ONLY, unchanged but sharpened: the honest gap is now a pure
phantom-slot problem on an otherwise byte-perfect body (wp chassis). The -da
greg/expansion census should also explain the now-measured frame-coupled
scheduling flip (at vars=80 GCC naturally stages &local into $s0; at vars=72
it emits per-call addiu + leaves a nop) — the same mechanism that sizes the
frame likely gates that staging, so the producer census has a second
observable to match. Local-declaration space is now TRIPLY dead (19 hand
forms + 43k permuter iterations + the volatile-only-zero measurement).
Sibling func_8001E404 note: the wp staging discovery plausibly transfers to
its honest form (same idiom) — worth one frame-gradient probe in its next
session, but NOT from this function's budget.

## [s4] Random/directed permuter search over the honest form finds an honest vars=80 frame producer beyond the 19 hand-swept structural spellings
- mechanism: permuter mutation classes (temp_for_expr, reorders, ins_block, type randomization, struct_ref, ...) with the --stack-diffs scorer making the +8 sp shift visible; stop-on-zero
- probe: 3 campaigns in tmp/grind/func_8001E6E4/s4/perm (telemetry in metrics/events.jsonl): default weights; perm_pad_var_decl weight-zeroed; reseed from the wp chassis — 43k+ iterations on the final seed
- result: Only score-0 basin: `volatile unsigned short pad;` — unwritten volatile dead scalar, forbidden family (unused-local frame coercion via volatile-typed scalar), rejected without submission, banked at memory/grind/func_8001E6E4/rejected/permuter-volatile-dead-scalar.c. New mechanical facts: GCC 2.7.2 gives an UNWRITTEN volatile scalar a stack home at zero byte cost (the only inert vars=80 local spelling); non-volatile dead scalars/pointers leave vars=72; a WRITTEN volatile reaches vars=80 but materializes its store (score 100). No honest vars=80 form exists in the mutation-reachable local-declaration space
- verdict: KILLED

## [s4] A staged work-pointer (CamWork *wp; wp = &local; between the rx and ry stores; first call takes (s32 *)wp) closes the residual non-shift codegen divergence at zero cost
- mechanism: the genuinely-used named pointer intermediate (sanctioned named-intermediate family) makes GCC stage &local into callee-save $s0 in the former nop slot (addiu s0,sp,16) and emit move a0,s0 for the first call — the exact shape the target has and the real frame-112 build already produced naturally
- probe: permuter find output-140 vetted, asm-diffed vs base and target (s4/asmdiff.sh), byte-equality probe with pre_pad at frame 112 (s4/test_wp_prepad.sh), applied to src/code6cac.c, engine sandbox --disable all
- result: honest 104-frame build is now codegen-structurally IDENTICAL to target: 71/71 insns, remaining diff = exactly the 19-insn uniform +8 sp shift, nothing else. With pre_pad (frame 112): byte-identical, diff 0 — oracle-safe. Sandbox = 19 (metric unchanged; the honest gap is now purely the phantom slot). Banked into src + candidate.c
- verdict: CONFIRMED

## [s5] H1: some expansion-time temp / unallocated pseudo (aggregate temp, spill slot, inner-scope object) supplies the target's 8 phantom bytes
- mechanism: phantom-slot-frame-lever - get_frame_size counts slots no insn touches; assign_stack_temp / reload alter_reg produce them
- probe: 11-variant frame+diff screen (tmp/grind/func_8001E6E4/s5/screen.py) reporting BOTH the cc1 .frame line and the objdump diff-line count vs target, incl. the decisive controls arr_first_dead (dead array declared FIRST) vs arr_last_dead (declared LAST) and scope_pair (inner-block aggregate)
- result: STRUCTURALLY IMPOSSIBLE. GCC 2.7.2 hands out the vars area lowest-offset-first in allocation order, and the outer block's declared locals are allocated before any statement is expanded, so every expansion temp / inner-scope local / spill slot lands ABOVE `local` (measured: scope_pair's q at sp+0x58; arr_last_dead reaches vars=80 with `local` still at sp+0x10 and 9 differing insn pairs). The target needs the 8 bytes BELOW `local` (buffer at sp+0x18). No expansion-temp mechanism can produce that layout - the -da census cannot succeed and should not be run
- verdict: KILLED

## [s5] A genuinely-used 8-byte object DECLARED FIRST reproduces the full target frame layout at acceptable codegen cost
- mechanism: declaration order controls slot order (first-declared = lowest offset), so a first-declared 8-byte object reserves exactly the phantom region and pushes the work buffer to sp+0x18
- probe: pair_first (Pair2 staged into local.vx/vy), arr_first_used (s32 t[2] same role), rot4_first (Rot4 aggregate copy) screened, then two fresh-seed permuter campaigns (s5-pair-first ~25 min / s5-arr-first-used 40409 iters, perm_pad_var_decl weight-zeroed, --stack-diffs, --stop-on-zero)
- result: layout REPRODUCED exactly (frame 112, buffer at 0x18, saves 0x60-0x6C, 71 insns) and the raw diff drops to 14 insn pairs - the best honest form ever measured here (wp chassis is 19) - but the staging writes into sp+0x10/0x14 are themselves the divergence, and target never touches that region. Campaigns found no honest score-0: best 16 (semantically invalid - permuter repointed wp at q, passing the wrong buffer to func_80046BF4) and best 108 flat. Banked rejected/s5-first-declared-staging-object.c
- verdict: KILLED

## Frontier after s5
The mechanical question is now CLOSED and the residual is purely an EVIDENCE
question. Proven this session: the target's layout requires an 8-byte object
declared BEFORE the work buffer that emits zero instructions, and GCC 2.7.2
offers exactly three such constructs - unused local array (forbidden family),
unwritten volatile scalar (forbidden family, s4), and a WIDER DECLARED TYPE for
the work buffer whose leading 8 bytes this function simply never touches (the
s1 `pad_lead` form: mechanically byte-perfect, cheat-reviewer FAILed only for
want of evidence about what those bytes are). Every other axis is measured
dead: structural spelling (19 forms, s1-s3), local-declaration mutation space
(s4: 43k iters), expansion temps (s5: structurally impossible), callee-type
evidence (s3), Kengo (s3), random permuter from four distinct seeds (s4+s5).
Remaining live axis - EVIDENCE for the wider buffer type, from sources not yet
tried: (a) a binary-wide FRAME/IDIOM census for other functions that build a
camera-style work buffer and DO write its leading 8 bytes (s5 only checked the
4 in-tree callers of the two camera callees; the search should be by frame
shape and by the pass-(&pos,&rot,dist) idiom, not by callee identity); (b) the
.data/.bss camera work areas (the D_800F5328 / D_800F6608 row type and
D_800A36B4's pointee) for a PERSISTED instance of the same record whose first 8
bytes are written by some function - that would name the fields and satisfy the
pad_lead rejection's explicit reopen condition. If that census comes back
negative it is a FAILED gate, and the function becomes an owner-escalation
candidate once the driver declares exhaustion.

## [s5] H1: some expansion-time temp / unallocated pseudo (aggregate temp, reload spill slot, inner-scope object) supplies the target's 8 phantom bytes — the ledger's sole live axis since s1
- mechanism: phantom-slot-frame-lever: get_frame_size counts slots no insn touches; assign_stack_temp / reload alter_reg produce them
- probe: 11-variant screen (tmp/grind/func_8001E6E4/s5/screen.py) reporting BOTH observables — the cc1 .frame line AND the objdump diff-line count vs target — with the decisive controls arr_first_dead (dead s32 t[2] declared FIRST = pre_pad shape) vs arr_last_dead (same array declared LAST) and scope_pair (8-byte aggregate in a disjoint inner block)
- result: Structurally impossible. arr_first_dead: frame 112, 71 insns, objdump diff 0. arr_last_dead: frame 112 (vars=80!) but `local` stays at sp+0x10 -> 9 differing insn pairs. scope_pair: its 8 bytes land at sp+0x58, above `local`. agg_rot4/agg_vec3 aggregate temps: frame 112/120 with `local` unmoved and +5 insns. GCC 2.7.2 hands out the vars area lowest-offset-first in allocation order and expands the outer block's declared locals before any statement, so every expansion temp / inner-scope local / spill slot lands ABOVE `local` and grows the frame at the TOP. The target needs the 8 bytes BELOW `local` (buffer at sp+0x18). The -da expansion-temp census cannot succeed and should not be run.
- verdict: KILLED

## [s5] A genuinely-used 8-byte object DECLARED FIRST reproduces the full target frame layout at acceptable codegen cost (the corollary chassis the mechanism finding suggests)
- mechanism: declaration order controls slot order (first-declared = lowest sp offset), so a first-declared 8-byte object reserves exactly the phantom region and pushes the work buffer to sp+0x18
- probe: pair_first (Pair2 staged into local.vx/vy), arr_first_used (s32 t[2] in the same role) and rot4_first (Rot4 aggregate copy) screened; then two fresh-seed permuter campaigns with perm_pad_var_decl weight-zeroed, --stack-diffs, --stop-on-zero: s5-pair-first (~25 min, 462+ outputs) and s5-arr-first-used (40409 iterations, ~18 min across two 9-minute windows)
- result: Layout reproduced exactly — frame 112, buffer at sp+0x18, saves at 0x60-0x6C, 71 insns — and the raw diff drops to 14 differing insn pairs, the closest honest form ever measured on this function (the banked wp chassis is 19). But the staging writes into sp+0x10/0x14 ARE the divergence (target never touches that region; it has 2 scheduling nops and a lui/lw pair there instead), so the family cannot close by construction. Campaigns found no honest score-0: best 16 is semantically INVALID (the permuter repointed `wp` at `q`, so func_80046BF4 would receive the wrong buffer) and the second campaign sat flat at 108. Banked rejected/s5-first-declared-staging-object.c
- verdict: KILLED
