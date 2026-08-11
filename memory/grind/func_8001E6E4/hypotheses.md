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

## [s6] H-F1: an allocation entry point OTHER than a declaration (a parameter shape, the static chain, a nonlocal-goto slot) can place 8 bytes in the vars area BELOW the work buffer
- mechanism: assign_stack_local is called from several places besides expand_decl; anything running inside expand_function_start (assign_parms, the static chain) allocates before the outer block's declarations and would therefore land at vars offset 0 - the one window s5's black-box sweep could not see
- probe: source census of every assign_stack_local/assign_stack_temp call site in tools/gcc-2.7.2, plus a 10-function instrumented-cc1 probe TU (tmp/grind/func_8001E6E4/s6/prealloc_probe.c, run via s6/run_probe.sh with BB2_FRAME_DEBUG=1) reporting the FRAMEDBG slot census, the .frame line and the insn count for each parameter/call shape
- result: EMPTY WINDOW. assign_parms can never call assign_stack_local on this target because REG_PARM_STACK_SPACE(fndecl)=16>0 keeps stack_parm non-null (function.c:3489-3500, mips.h:1822); measured, no parameter shape moves vars at all (p_parm_int_addr / p_parm_struct8_unused / p_parm_struct8_used / p_parm_double_unused all vars=72 with the single size=72 slot). An addressable sub-word parm does get a slot but via put_reg_into_stack during expansion, landing ABOVE the buffer (HImode size=2 at frame_offset 72->74, buffer unmoved). The static-chain slot (function.c:5036) is the only genuinely pre-body allocator and needs GNU nested functions, is 4 bytes, and emits a store; nonlocal-goto (stmt.c:669) and __builtin_apply (expr.c:8208) likewise. The 8 bytes can ONLY come from a declaration preceding the buffer - s5's law is now a white-box proof
- verdict: KILLED

## [s6] H-F2: the target frame is args=24 / vars=72 (the buffer sitting at the outgoing-args boundary), so there is no phantom slot to reproduce at all
- mechanism: the local base is STARTING_FRAME_OFFSET = current_function_outgoing_args_size (mips.h:1651), so a 17-24 byte outgoing arg block puts the work buffer at sp+0x18 with an ordinary 72-byte vars area; s2 had dismissed this partition by elimination only
- probe: instrumented-cc1 probes p_call5 (5-word outgoing call) and p_structret (struct-returning callee whose hidden pointer shifts the arg list), reading .frame + emitted asm
- result: the partition is REAL and reproduces the target's geometry (p_call5: vars=72, args=24, buffer at sp+24; p_structret: vars=80, args=24) - the only mechanism ever measured that moves the buffer up without allocating anything below it - but the stack argument word is always materialized: sw $5,16($sp) (here even in the jal delay slot). Every route to args_size>16 writes into sp+0x10..0x17, and the target provably touches nothing there. Killed by measurement rather than by argument
- verdict: KILLED

## [s6] H-F3: some function in the binary WRITES an 8-byte lead before a camera-style work buffer, naming the leading fields and satisfying the pad_lead rejection's reopen condition (s5 frontier probe (a), by idiom rather than by callee identity)
- mechanism: the pad_lead form is mechanically byte-perfect and was FAILed only for want of evidence about what the leading 8 bytes are; an in-binary instance of the same record with its lead written would supply exactly that evidence
- probe: tmp/grind/func_8001E6E4/s6/lead_census.py over all 1434 asm/funcs/*.s - per-function sp-store offsets, addiu-$sp record bases and jal targets; filters for (A) the 0x10/0x14 + 0x18/0x1c/0x20 shape, (D) address-taken records with >=5 consecutive stored words, (E) written lead at K + pointer handed at K+8, (F) both &rec and &rec+8 taken. Output banked at s6/lead_census.txt
- result: 52 address-taken multi-word stack records; 14 with the exact written-lead + pointer-at-base+8 shape; NONE of the 14 passes that pointer to func_8001A538 or func_80046BF4. The closest analogues (func_80061C00 / func_80061D74) drive RotMatrix/RotTrans/SetRotMatrix/SetTransMatrix - the ordinary PsyQ SVECTOR-plus-record adjacent-locals idiom, two independent locals, not one wider type with named leading fields. FAILED gate, matching s5's negative by-callee census
- verdict: KILLED

## Frontier after s6
The mechanical space is closed at the compiler-entry-point level: the target's
layout requires an 8-byte object DECLARED BEFORE the work buffer that emits
zero instructions (only three such constructs exist in GCC 2.7.2 - unused local
array and unwritten volatile scalar, both forbidden families, and the wider
declared buffer type = the evidence-gated s1 pad_lead form), and the only
alternative partition (args=24) is measured to require a store the target does
not have. Both external evidence searches for the wider type are now negative:
by callee identity (s5) and by binary-wide idiom (s6). One evidence probe from
s5's frontier remains UNRUN: the persisted (.data/.bss) instance search - trace
the D_800F5328 / D_800F6608 row type and D_800A36B4's pointee for a static
record whose first 8 bytes are written by some function. If that also comes
back negative, every sanctioned axis on this function is measured dead and the
function is an owner-escalation candidate: endgame-lock gate (1) is already a
documented FAIL (scan_hand_coded tier=LOW, 0/8, S1/S2/S6 absent, measured s6),
and gate (2) has no citable SOTN-master precedent for an invented struct lead,
so the owner's standing ruling (2026-07-27) would resolve it as REFUSED /
OWNER-ACCEPTED INCOMPLETE.

## [s6] An allocation entry point OTHER than a declaration (a parameter shape, the static chain, a nonlocal-goto slot) can place 8 bytes in the vars area BELOW the work buffer.
- mechanism: assign_stack_local is called from several places besides expand_decl; anything running inside expand_function_start (assign_parms, the static-chain slot) allocates before the outer block's declarations and would land at vars offset 0 - the one window s5's black-box sweep could not see.
- probe: Source census of every assign_stack_local/assign_stack_temp call site in tools/gcc-2.7.2, plus a 10-function instrumented-cc1 probe TU (tmp/grind/func_8001E6E4/s6/prealloc_probe.c via s6/run_probe.sh, BB2_FRAME_DEBUG=1) reporting the FRAMEDBG slot census + .frame line + insn count for each parameter/call shape.
- result: EMPTY WINDOW. assign_parms can never call assign_stack_local on this target: function.c:3489-3500 keeps stack_parm non-null whenever REG_PARM_STACK_SPACE(fndecl)>0, and mips.h:1822 makes that 16 unconditionally. Measured - p_parm_int_addr, p_parm_struct8_unused, p_parm_struct8_used, p_parm_double_unused all report vars=72 with the single ctx=stack_temp size=72 slot. An addressable sub-word parm does get a slot, but via put_reg_into_stack DURING expansion: ctx=put_reg_into_stack HImode size=2 at frame_offset 72->74 (and 74->76), i.e. ABOVE the buffer, which stays at the base. The only genuinely pre-body allocator is the static-chain slot (function.c:5036, 'Do this first, so it gets the first stack slot offset') - GNU nested functions only, Pmode=4 bytes, followed unconditionally by emit_move_insn. stmt.c:669 (nonlocal goto) and expr.c:8208 (__builtin_apply) likewise. So the 8 bytes can only come from a declaration preceding the buffer; s5's black-box law is now a white-box proof, and the entire 'maybe the original signature/prototype was different' family (extra params, aggregate params, doubles, varargs, struct-returning calls, extra call args) dies in one pass.
- verdict: KILLED

## [s6] The target frame is args=24 / vars=72 - the work buffer sitting at the outgoing-args boundary - so there is no phantom vars slot to reproduce at all.
- mechanism: The local base is STARTING_FRAME_OFFSET = current_function_outgoing_args_size (mips.h:1651), so a 17-24 byte outgoing argument block places the buffer at sp+0x18 with an ordinary 72-byte vars area. Session 2 dismissed this partition by elimination only, never by measurement.
- probe: Instrumented-cc1 probes p_call5 (a 5-word outgoing call) and p_structret (a struct-returning callee whose hidden pointer shifts the arg list), reading the .frame line and the emitted asm.
- result: The partition is REAL and reproduces the target geometry - p_call5: vars=72, args=24, buffer at sp+24; p_structret: vars=80, args=24 - the only mechanism ever measured on this function that moves the buffer up without allocating anything below it. But the stack argument word is always materialized: sw $5,16($sp) (here even filling the jal delay slot). Every route to args_size>16 (5th scalar arg, 8-byte-aligned arg forced past $a3, struct-return hidden pointer) writes into sp+0x10..0x17, and the target provably touches nothing there. Elimination argument upgraded to a measurement.
- verdict: KILLED

## [s6] Some function in the binary WRITES an 8-byte lead before a camera-style work buffer, naming the leading fields and satisfying the pad_lead rejection's explicit reopen condition (s5 frontier probe (a): search by idiom, not by callee identity).
- mechanism: The pad_lead form is mechanically byte-perfect and was FAILed only for want of evidence about what the leading 8 bytes are; an in-binary instance of the same record with its lead written would supply exactly that evidence.
- probe: tmp/grind/func_8001E6E4/s6/lead_census.py over all 1434 asm/funcs/*.s - per-function sp-relative store offsets, addiu-$sp address-taken record bases, and jal targets; filters (A) stores at 0x10/0x14 plus a triple at 0x18/0x1c/0x20, (D) address-taken records with >=5 consecutive stored words, (E) written lead at K plus pointer handed at K+8, (F) both &rec and &rec+8 taken. Full output banked at s6/lead_census.txt.
- result: 52 address-taken multi-word stack records; 14 with the exact written-lead + pointer-handed-at-base+8 shape; NONE of the 14 passes that pointer to func_8001A538 or func_80046BF4 (the camera consumers). The closest analogues, func_80061C00 and func_80061D74, drive RotMatrix/RotTrans/SetRotMatrix/SetTransMatrix - the ordinary PsyQ SVECTOR(8B)-plus-record adjacent-locals idiom, i.e. two INDEPENDENT locals, not one wider type with named leading fields. A FAILED gate, matching s5's negative by-callee census.
- verdict: KILLED

## [s7] H-F4: a PERSISTED (.data/.bss) instance of the same camera record exists whose first 8 bytes are written somewhere, naming the leading fields and satisfying the pad_lead rejection's reopen condition (s5 frontier probe (b), the last unrun evidence axis)
- mechanism: same evidence gate as the (negative) stack-idiom census, sourced from static data instead of another stack frame - the two table rows D_800F5328 / D_800F6608 ARE persisted instances of the record this function copies into `local`, so if the true record starts 8 bytes earlier, some function must write those static words
- probe: tmp/grind/func_8001E6E4/s7/bss_lead_census.py - emulates the %hi/%lo address-materialization idiom across all 1434 asm/funcs/*.s (symbol grep is insufficient: base-register stores are invisible to %lo(SYM) grep), resolving every lui/addiu + lw/sw/lh/sh/lb/sb pair to an effective address and reporting every access in a window around both row bases. Output banked at s7/bss_lead_census.txt
- result: NEGATIVE and CONTRADICTORY. 0x800F5320/0x800F5324 and 0x800F6600/0x800F6604 - the 8 bytes immediately below each row base - are never touched by any instruction in the binary; both records begin exactly at the base this function uses. The persisted rows' own geometry mirrors the local with vx at offset 0 (base+0/+4/+8 = vx,vy,vz; +0x10/+0x12/+0x14 = rx,ry,rz; +0x18 = dist), the same layout func_8001A538 exhibits from the callee side. The nearest lower symbol D_800F65F8/65F9 is a byte-flag pair 0x10 below with 0x65FA..0x6607 wholly untouched, i.e. a distinct object. The pad_lead reopen condition has now failed on all three independent evidence paths: callee identity (s3/s5), binary-wide stack idiom (s6), persisted .bss instance (s7)
- verdict: KILLED

## [s7] H-F5: some OTHER function in the binary produces an untouched leading frame hole from honest C, and its committed source shows the construct
- mechanism: the defect is a frame-layout shape, not a camera-specific one; if any of the 1434 functions has an allocated-but-never-touched leading vars region AND is reconstructed in src/, its C is a copyable, already-accepted precedent for whatever construct produces it
- probe: tmp/grind/func_8001E6E4/s7/hole_census.py over all asm/funcs/*.s - prologue frame size, every sp-relative memory offset, every addiu $r,$sp,K record base, the callee-save block; reports a >=8-byte gap between the 0x10 outgoing-args boundary and the lowest touched vars offset for every non-leaf. Output s7/hole_census.txt, cross-referenced against src/
- result: exactly FOUR functions in the whole binary have the shape - sprintf (PsyQ library, varargs, 512-byte internal buffer, not in src), func_8003CF84 (16-byte hole, frame 0x48), and the two camera siblings func_8001E6E4 / func_8001E404 (8-byte hole, frame 0x70). func_8003CF84 (src/code6cac_c2.c:862, queue distance 28, 0 rules) solves it in-tree with exactly the forbidden construct - `volatile s32 pad[4];` declared first, plus a second `volatile s32 pad2[2];` for its 0x2C region - so it is a THIRD member of this family, not a precedent. NO in-tree function produces an untouched leading frame hole from honest C; there is nothing to copy anywhere in the binary
- verdict: KILLED

## [s7] H-F6: a declared-type or alignment property of the work buffer itself (an 8-byte-aligned member, a wider aggregate alignment, the big-endian slot correction) makes GCC pad before the first frame slot and land the buffer at vars offset 8
- mechanism: assign_stack_local rounds frame_offset to the slot's alignment before allocating, so a slot demanding more alignment than the current offset provides forces padding BELOW it - the one remaining "make the buffer land higher without allocating anything below it" idea, never tested in s1-s6
- probe: source read of assign_stack_local (tools/gcc-2.7.2/function.c:669-742) plus the MIPS frame macros (mips.h:1082 BIGGEST_ALIGNMENT, :1645 FRAME_GROWS_DOWNWARD, :1651 STARTING_FRAME_OFFSET), and the last unaudited frame-equation term (mips.c:4531 pretend_args_size)
- result: IMPOSSIBLE BY CONSTRUCTION. FRAME_GROWS_DOWNWARD is undefined for MIPS, so the allocator does `frame_offset = CEIL_ROUND (frame_offset, alignment); ... frame_offset += size;` with frame_offset starting at 0 - and CEIL_ROUND(0, A) == 0 for every power-of-two A, including the BLKmode case (alignment = BIGGEST_ALIGNMENT/8 = 8). The first-allocated slot ALWAYS lands at vars offset 0 whatever its type, size or alignment. The bigend_correction branch needs BYTES_BIG_ENDIAN (this build is -mel) and in any case shifts only WITHIN an allocated slot, never its base. Separately, the one frame-equation term s6's partition omitted - MIPS_STACK_ALIGN(current_function_pretend_args_size), mips.c:4531 - is guarded by `ABI_64BIT && mips_isa >= 3` and is identically zero on o32 (same guard in INITIAL_ELIMINATION_OFFSET, mips.h:1758), so varargs/pretend-args is not a route either
- verdict: KILLED

## Frontier after s7
No mechanical axis and no evidence axis remains on this function, and the
closure is now white-box at every level: the frame equation is partitioned with
every term audited (args / vars / gp / fp / extra / pretend), the args=24
partition is measured to require a store the target does not have (s6), the
pre-declaration allocation window is provably empty (s6), alignment can never
pad before the first slot (s7), every expansion-time object grows the frame at
the TOP (s5), the structural spelling space is 19 measured forms (s1-s3), the
mutation space is four permuter seeds and ~100k iterations (s4-s5), and the
evidence gate for the only mechanically-perfect form (pad_lead / a wider
declared buffer type) has failed on all three independent paths - callee
identity (s3/s5), binary-wide stack idiom (s6) and persisted .bss instance (s7).
s7 additionally measured that NO function in the 1434-function binary produces
an untouched leading frame hole from honest C, so no in-tree precedent exists to
copy. Endgame-lock gate (1) is a documented FAIL (scan_hand_coded tier=LOW 0/8,
S1/S2/S6 absent, measured s6) and gate (2) has no citable SOTN-master precedent
for an invented struct lead. Recommendation to the driver (the exhaustion call
is the driver's, not a session's): route this function to `escalation` modality;
under the owner's standing ruling (2026-07-27) a both-gates-fail escalation
resolves as REFUSED / OWNER-ACCEPTED INCOMPLETE. Two transfer notes for whoever
gets there: the disposition should cover THREE functions, not two - func_8003CF84
(src/code6cac_c2.c:862, queue distance 28) is the same defect with a 16-byte
lead and the same committed `volatile s32 pad[4]` construct - and the s4 `wp`
staging chassis plus the s6/s7 instruments (fdbg.sh, run_probe.sh, lead_census.py,
bss_lead_census.py, hole_census.py) apply unchanged to all three.

## [s7] A persisted (.data/.bss) instance of the same camera record exists whose first 8 bytes are written somewhere, giving the leading fields externally-verifiable semantics and satisfying the pad_lead rejection's explicit reopen condition (s5 frontier probe (b), the ledger's last unrun evidence axis).
- mechanism: The two table rows D_800F5328 / D_800F6608 ARE persisted instances of the record this function copies into its work buffer, so if the true record starts 8 bytes earlier some function must write those static words. Same evidence gate as the (negative) stack-idiom census, sourced from static data rather than another stack frame.
- probe: tmp/grind/func_8001E6E4/s7/bss_lead_census.py: emulates the %hi/%lo address-materialization idiom across all 1434 asm/funcs/*.s (symbol grep alone is insufficient - base-register stores are invisible to %lo(SYM) grep), resolving every lui/addiu-%hi/%lo pair plus every lw/sw/lh/sh/lb/sb N($r) to an effective address, and reporting every access landing in a window around both row bases. Output banked at s7/bss_lead_census.txt.
- result: NEGATIVE and CONTRADICTORY. 0x800F5320 / 0x800F5324 and 0x800F6600 / 0x800F6604 - the 8 bytes immediately below each row base - are never touched by any instruction in the binary; both records begin exactly at the base this function uses. The persisted rows' own field geometry mirrors the local with vx at offset 0 (base+0/+4/+8 = vx,vy,vz; +0x10/+0x12/+0x14 = rx,ry,rz; +0x18 = dist), the same layout func_8001A538 exhibits from the callee side (s3). The nearest lower symbol, D_800F65F8/65F9, is a byte-flag pair 0x10 below with 0x65FA..0x6607 wholly untouched, i.e. a distinct object rather than a lead. The pad_lead reopen condition has now failed on all three independent evidence paths: callee identity (s3/s5), binary-wide stack idiom (s6), persisted .bss instance (s7).
- verdict: KILLED

## [s7] Some OTHER function in the binary produces an allocated-but-never-touched leading frame hole from honest C, and its committed source shows the construct that does it (a copyable, already-accepted in-tree precedent).
- mechanism: The defect is a frame-layout shape, not a camera-specific one. If any of the 1434 functions exhibits it and is reconstructed in src/, its C is precedent for whatever honest construct reserves leading frame bytes at zero instruction cost.
- probe: tmp/grind/func_8001E6E4/s7/hole_census.py over all asm/funcs/*.s: prologue frame size, every sp-relative memory offset, every addiu $r,$sp,K record base, and the callee-save block; reports a >=8-byte gap between the 0x10 outgoing-args boundary and the lowest touched vars offset for every non-leaf, cross-referenced against src/. Output s7/hole_census.txt.
- result: Exactly FOUR functions in the entire binary have the shape: sprintf (PsyQ library, varargs, 512-byte internal buffer, not reconstructed in src), func_8003CF84 (16-byte hole at sp+0x10..0x1F, frame 0x48), and the two camera siblings func_8001E6E4 / func_8001E404 (8-byte hole, frame 0x70). func_8003CF84 (src/code6cac_c2.c:862, queue distance 28, 0 rules) solves it in-tree with exactly the forbidden construct - `volatile s32 pad[4];` declared first, plus a second `volatile s32 pad2[2];` for its 0x2C region - so it is a THIRD member of this family, not a precedent. No in-tree function produces an untouched leading frame hole from honest C: there is nothing to copy anywhere in the binary.
- verdict: KILLED

## [s7] A declared-type or alignment property of the work buffer itself (an 8-byte-aligned member, a wider aggregate alignment, the big-endian slot correction) makes GCC pad before the first frame slot, landing the buffer at vars offset 8 without allocating any object below it.
- mechanism: assign_stack_local rounds frame_offset to the slot's alignment before allocating it, so a slot demanding more alignment than the current offset provides would force padding BELOW it - the last 'make the buffer land higher without allocating anything beneath it' idea, never tested across s1-s6.
- probe: Source read of assign_stack_local (tools/gcc-2.7.2/function.c:669-742) plus the MIPS frame macros (mips.h:1082 BIGGEST_ALIGNMENT=64, :1645 FRAME_GROWS_DOWNWARD commented out, :1651 STARTING_FRAME_OFFSET) and the one frame-equation term s6's partition omitted (mips.c:4531, pretend_args_size).
- result: IMPOSSIBLE BY CONSTRUCTION. With FRAME_GROWS_DOWNWARD undefined the allocator does `frame_offset = CEIL_ROUND (frame_offset, alignment); ... frame_offset += size;`, frame_offset starts at 0, and CEIL_ROUND(0, A) == 0 for every power-of-two A - including the BLKmode case where alignment = BIGGEST_ALIGNMENT/8 = 8. The first-allocated slot therefore ALWAYS lands at vars offset 0 whatever its type, size or alignment. The bigend_correction branch requires BYTES_BIG_ENDIAN (this build is -mel) and in any case shifts only WITHIN an allocated slot, never its base. Separately, mips.c:4531's MIPS_STACK_ALIGN(current_function_pretend_args_size) is guarded by `ABI_64BIT && mips_isa >= 3` and is identically zero on o32 (same guard in INITIAL_ELIMINATION_OFFSET, mips.h:1758), so varargs / pretend-args cannot shift the buffer either.
- verdict: KILLED

## [s8 — escalation/disposition] Endgame-lock gate (1): func_8001E6E4 carries STRONG hand-coded-asm signals, making authorized canonical inline asm the correct disposition.
- mechanism: `.claude/rules/endgame-lock-disposition.md` gate (1) admits canonical asm only on a STRONG `scan_hand_coded` tier, which requires S1 (multu pacing), S2 (empty-body branch) or S6 (BIOS jumptable) — the signals that only hand-written asm produces. s6 noted the tier in passing; the escalation session must re-run it as first-hand evidence.
- probe: `python3 tools/scan_hand_coded.py --single func_8001E6E4`, banked verbatim at tmp/grind/func_8001E6E4/s8/scan_hand_coded.txt.
- result: FAILED GATE. `tier=LOW score=0/8`, reason "no strong hand-coded indicators". All eight signals are absent: S1 0 multu/mflo pairs, S2 no empty-body branches, S3 71 insns with 8 spills over 8 distinct regs (ordinary allocator output), S4 max load burst 3 in any 8-insn window, S5 no high-similarity siblings (jaccard < 0.5), S6 no BIOS jumptable call pattern, S7 every callee-save use has its $sp save, S8 no redundant mask-before-shift. This is ordinary compiled C; asm is refused.
- verdict: KILLED

## [s8 — escalation/disposition] Endgame-lock gate (2): the closing construct (an UNWRITTEN leading local array / struct lead reserving 8 frame bytes) falls under the sanctioned written-never-read local-array carve-out, so an in-hand SOTN-master precedent exists for it.
- mechanism: The frozen SOTN list does sanction a dead local array; if the carve-out's scope covers an unwritten one, the mechanically byte-perfect pad_lead form becomes a sanctioned family rather than a cheat, and the function closes.
- probe: Read the carve-out's scope sentence verbatim from `.claude/rules/no-new-park-categories.md:255-262` and test its precondition against the measured target bytes for sp+0x10..0x17.
- result: FAILED GATE, and failed on the carve-out's OWN stated precondition rather than by interpretation. Verbatim scope: "sanctioned ONLY when the target bytes contain the corresponding dead stores (oracle-enforced), written (not merely declared), exhaustion-documented, FAKE-annotated, dual-reviewed. SOTN evidence: `u8 sp70[4]` written 4x/read 0x in two matched dra-core functions (62DEC.c), `s16 z[5]` x2, annotated `volatile u32 pad[4]; // FAKE`. The unwritten-array and `(void)&local` forms remain forbidden." The target contains NO stores in sp+0x10..0x17 — that untouched-ness IS the defect — so the precondition is measured false and the cited SOTN evidence is precedent for the OPPOSITE shape. The written variant is not an escape: s5 measured it (rejected/s5-first-declared-staging-object.c) and it writes the region the target never touches. No SOTN-master citation exists for an unwritten leading pad.
- verdict: KILLED

## [s8 — escalation/disposition] The honest floor is still 19 and the residual is still purely the +8 sp shift (re-measurement, not inheritance).
- mechanism: A disposition entry must rest on a measurement taken THIS session, not on an inherited number; and `build_insns == target_insns` is what distinguishes "frame-geometry residual" from "codegen residual".
- probe: Applied candidate.c's `wp` staging chassis to src/code6cac.c, ran `tools/wteng.ps1 main sandbox func_8001E6E4 --disable all`, then reverted src/code6cac.c to the HEAD form (so the driver's terminal-park commit sees a clean tree — a src-dirty `park:` commit trips park_src_guard and deadlocks the driver). Banked at tmp/grind/func_8001E6E4/s8/floor_remeasure.txt.
- result: score 19, target_insns 71, build_insns 71, rules_dropped 0, cheat_asm_stripped 139. Floor FLAT at 19 for the 8th consecutive session, and instruction-count parity confirms all 19 differing pairs are the uniform +8 $sp-offset shift.
- verdict: CONFIRMED

## Frontier after s8 (terminal)
DISPOSITION REACHED. Both endgame-lock AND-gates FAIL — gate (1) scan_hand_coded tier=LOW
0/8 (measured s8), gate (2) the unwritten leading pad is explicitly excluded by the
written-never-read carve-out's own scope sentence and has no SOTN-master citation (s8),
after the evidence gate already failed on four independent paths (s3/s5 callee identity,
s6 binary-wide stack-record idiom, s7 persisted .bss instance, s7 binary-wide frame-hole
census). Per the owner's standing auto-ruling (2026-07-27) the disposition is REFUSED /
OWNER-ACCEPTED INCOMPLETE, filed at docs/grind/decisions.md (entry
"## 2026-08-11 — func_8001E6E4 — OWNER-ESCALATION — RESOLVED BY STANDING RULING
(2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE"). Nothing is pending on the owner; the
driver parks the function terminally and the queue advances. The byte-match on main is held
by `s32 pre_pad[2];` in src/code6cac.c (0 regfix/asmfix rules) — retained, NOT sanctioned.
The same disposition is recommended for the two family members `func_8001E404` (identical
8-byte hole, same file) and `func_8003CF84` (src/code6cac_c2.c:862, 16-byte hole,
`volatile s32 pad[4]` + `volatile s32 pad2[2]`), confirmable in one measurement each via
s7/hole_census.py + s6/fdbg.sh rather than another six-modality grind.

## [s8] Endgame-lock gate (1): func_8001E6E4 carries STRONG hand-coded-asm signals, making authorized canonical inline asm the correct disposition.
- mechanism: endgame-lock-disposition.md gate (1) admits canonical asm only on a STRONG scan_hand_coded tier, which requires S1 (multu pacing), S2 (empty-body branch) or S6 (BIOS jumptable) - the signals only hand-written asm produces.
- probe: python3 tools/scan_hand_coded.py --single func_8001E6E4, banked verbatim at tmp/grind/func_8001E6E4/s8/scan_hand_coded.txt
- result: FAILED GATE. tier=LOW score=0/8, 'no strong hand-coded indicators'. All eight signals absent: S1 0 multu/mflo pairs; S2 no empty-body branches; S3 71 insns / 8 spills / 8 distinct regs (ordinary allocator output); S4 max load burst 3 per 8-insn window; S5 no high-similarity siblings (jaccard < 0.5); S6 no BIOS jumptable pattern; S7 every callee-save use has its $sp save; S8 no redundant mask-before-shift. Ordinary compiled C; asm refused.
- verdict: KILLED

## [s8] Endgame-lock gate (2): the only mechanically-viable closing construct - an UNWRITTEN leading local array / struct lead reserving 8 frame bytes - falls under the sanctioned written-never-read local-array carve-out, so an in-hand SOTN-master precedent exists for it.
- mechanism: The frozen SOTN list does sanction a dead local array; if its scope covers an unwritten one, the mechanically byte-perfect pad_lead form becomes a sanctioned family rather than a cheat and the function closes.
- probe: Read the carve-out's scope sentence verbatim from .claude/rules/no-new-park-categories.md:255-262 and test its precondition against the measured target bytes for sp+0x10..0x17.
- result: FAILED GATE, on the carve-out's OWN stated precondition rather than by interpretation. Verbatim: 'sanctioned ONLY when the target bytes contain the corresponding dead stores (oracle-enforced), written (not merely declared), exhaustion-documented, FAKE-annotated, dual-reviewed. SOTN evidence: u8 sp70[4] written 4x/read 0x in two matched dra-core functions (62DEC.c) ... The unwritten-array and (void)&local forms remain forbidden.' The target contains NO stores in sp+0x10..0x17 - that untouched-ness IS the defect - so the precondition is measured false and the cited SOTN evidence is precedent for the opposite shape. The written variant is not an escape: s5 measured it (rejected/s5-first-declared-staging-object.c) and it writes the region the target never touches. No SOTN-master citation exists for an unwritten leading pad.
- verdict: KILLED

## [s8] The honest floor is still 19 and the residual is still purely the uniform +8 sp shift (re-measured this session, not inherited).
- mechanism: A disposition entry must rest on a measurement taken THIS session; and build_insns == target_insns is what distinguishes a frame-geometry residual from a codegen residual.
- probe: Applied candidate.c's wp staging chassis to src/code6cac.c, ran tools/wteng.ps1 main sandbox func_8001E6E4 --disable all, then reverted src to the HEAD form. Banked at tmp/grind/func_8001E6E4/s8/floor_remeasure.txt.
- result: score 19, target_insns 71, build_insns 71, rules_dropped 0, cheat_asm_stripped 139. Floor FLAT at 19 for the 8th consecutive session; instruction-count parity confirms all 19 differing pairs are the uniform +8 $sp-offset shift.
- verdict: CONFIRMED
