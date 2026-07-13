# Hypothesis ledger — func_80037540

## s5 (structural, 2026-07-13) — **THE "CLOSED SEARCH SPACE" IS FALSE. Item is NOT blocked.**

### H11 — KILLED. Kengo's shipped ELF can attest the original local declarations.
"The Marionation engine's PS2 successor ships `Kengo/disc/SLUS_200.21` with a
515 KB `.mdebug` (MIPS ECOFF symbolic) section. If it carries source-level local
declarations, it attests array bounds directly — the 'evidence external to the
frame' the Judge demanded, from the SAME team."
- Probe: `objdump --debugging` reads only the tiny `.stab` (1215 lines of .dsm/.vsm
  line records) and cannot parse `.mdebug` at all. So I wrote a real MIPS ECOFF
  parser (tmp/grind/func_80037540/s2/mdebug.py): HDRR -> FDR -> SYMR/AUX.
  Bitfield packing validated against ground truth (little-endian SYMR is
  st[0:6] sc[6:11] index[12:32]; check: stFile's index == that FDR's csym == 36).
- Result: 240 file descriptors, **14,845 local symbols, and ZERO stLocal / stParam.**
  The symbol stream is Proc/End/Label/Static/File only — Kengo was built at
  procedure granularity (glevel 0). No locals, no types, no struct definitions.
- Verdict: **KILLED.** Kengo cannot attest any declared bound. With s3's MOVOVL kill
  (consumer arity = 6), the external-evidence avenue is now EXHAUSTED. Do not
  re-propose Kengo debug info as an evidence source (the parser is reusable, but
  the data simply is not there).

### H12 — **CONFIRMED, and it DEMOLISHES s4's trichotomy.** GCC 2.7.2 reserves frame
bytes for ORDINARY LIVE LOCALS that no instruction touches — a PHANTOM SLOT.
"s4's load-bearing step: 'to hold frame bytes an object must defeat scalar-promotion
=> >=2 elements => >=2 EXTRA STORES.' If a live local can occupy frame bytes with
ZERO stores, the trichotomy has a third branch and the unwritten tail is not forced."
- Probe: BB2 in-binary census (args-aware; tmp/.../bb2_frame_slack2.py) for functions
  whose frame reserves locals they never touch => 28 candidates. Drilled the one that
  is already COMPLETED: **tslLineG5Init** (src/code6cac_c2.c:1267).
- Result: **tslLineG5Init is COMPLETED-C — `sandbox --disable all` = 0, rules_dropped
  = 0, not in the queue, no cheat-asm** (the single regfix.txt hit for it is a `#`
  COMMENT on line 203, not a rule). And cc1 reports for it:
      .frame $sp,48,$31   # vars= 8, regs= 4/0, args= 24, extra= 0
  **get_frame_size() == 8.** Every sp-relative access in the function: the 4 saves
  (32,36,40,44) and `sw $2,16($sp)` (5th outgoing arg, inside args). **NOTHING
  touches [24,32).** Eight reserved bytes, never written, never read — and its C
  declares only five plain scalars, no array, no dead decl, no cheat. The original
  binary agrees (asm/funcs/tslLineG5Init.s: `addiu $sp,$sp,-0x30`).
- Mechanism (one-factor bisect + minimal reproducer, minrepro.py): two HImode (`s16`)
  locals feeding an HImode BITWISE expr (`(a & ~b) & 1`) make cc1 allocate a stack
  temp for the HImode computation and then never use it. Removing the pair, using one
  s16, or widening to s32 all give vars=0. A call is not required.
- Verdict: **CONFIRMED. s4's H9/H8 trichotomy is FALSE** — it enumerated only
  (a) extra stores and (b) unwritten tail, and MISSED (c) phantom slots from live
  code. Therefore "the ORIGINAL SOURCE ITSELF declared a locals object strictly
  larger than what it wrote" does NOT follow, the a-fortiori ruling argument is moot,
  and **func_80037540 is not blocked on an owner ruling — it has real search left.**

### H13 — KILLED (this port of it). s16/u16/u8 index locals carry the phantom slot here.
"The two func_80036EA8() results are camera-table INDICES; declaring them s16 is
semantically natural and adds 4 bytes => get_frame_size 24->28 => ALIGN8 => 32 =>
frame 0x48, with no extra stores."
- Probe: sweep on cc1's `vars=` readout (sweep2.py), incl. an s16-RETURNING
  func_80036EA8 so the sign-extension is free (GCC trusts a callee's s16 return).
- Result: **every variant stays at vars=24 / frame 0x40** — s16 pair, u16 pair, u8
  pair, three s16, both indices live across the 2nd call, a long long pair. The
  phantom slot does NOT appear: func_80037540 has no HImode bitwise semantics (its
  indices are only shifted, `idx * 8`), so cc1 creates no HImode temp.
- Verdict: **KILLED.** rejected/s16-index-locals-do-not-port-phantom-slot.c
  The MECHANISM is real and the space is open; THIS port of it is dead.

## THE NEW INSTRUMENT (s5) — search on get_frame_size directly, not the sandbox
cc1 prints get_frame_size() in its own `.frame` comment:
    bash tmp/grind/func_80037540/s2/build.sh <candidate.c>
    -> .frame $sp,N,$31   # vars= V, regs= R, args= A, extra= E
V IS get_frame_size(). The target needs **V in [25,32], args=16, regs=6, six stores
only**. This is a direct gradient on the one free term; the sandbox score only
reports the aggregate distance and cannot distinguish "wrong frame" from "wrong
codegen". Every future probe on this function should be measured this way first.

## s4 (structural, 2026-07-13) — the minimum is proven; the Judge's census is run
**[s5 NOTE: H8/H9's "trichotomy" conclusion below is REFUTED by H12. The individual
MEASUREMENTS in s4 remain valid; the INFERENCE drawn from them does not.]**

### H9 — KILLED. The fully-written branch can cost only ONE extra store.
"s3 measured `argv[6] + idx[2]` (32B, +2 stores). The cheaper unprobed member is
`argv[6] + idx[1]` (28B): 25-32 bytes of locals, ONE extra `sw`. Expect score 1."
- Probe: `s32 argv[6]; s32 idx[1];` with idx[0] written once and read once.
  sandbox --disable all + objdump.
- Result: **score 15, frame -0x40, and NO STORE INTO idx[0] EXISTS.** GCC 2.7.2
  scalar-promotes a single-element local array (constant index, address never taken)
  into a register — the value lives in $v0/$s0 across both jal's. get_frame_size = 24,
  not 28. The form degenerates EXACTLY to `s32 sp[6]` (43 insns, score 15, all diffs
  sp-relative offset shifts).
- Verdict: **KILLED — and it upgrades s3's inference to a PROOF.** A "+1 extra store"
  form is not merely worse, it is UNREACHABLE: any object small enough to be written
  by a single `sw` gets promoted and contributes ZERO frame bytes. To hold frame bytes
  an object must defeat promotion => >=2 elements => >=2 stores (o32 MIPS-I has no
  8-byte store; and no callee here takes a pointer, so no address escape is available).
  **The fully-written branch's floor is exactly +2 — proven, not sampled.**
  rejected/idx1-scalar-promoted-zero-frame-bytes.c

### H10 — the Judge's mandated census. RUN. Result INVERTS the premise.
"Assemble a SOTN-master (+oot/papermario/MGS/VS/ESA) census on the
capacity-declared-partially-filled-buffer family and put it to the owner." (BINDING)
- Probe: fresh clones of each tree; two mechanical detectors (census_unwritten_tail.py,
  census_untouched_pad.py), the first calibrated against SOTN's known `u8 sp70[4]`.
- Result A — our EXACT shape (unwritten tail + escaping base): **0 in SOTN.** No direct
  precedent. Reported honestly.
- Result B — the STRICTLY STRONGER shape (local array declared and never touched at
  all; only possible effect is get_frame_size): **356 instances** — SOTN 40, oot 38,
  papermario 3, Vagrant Story 30, MGS 245. (ESA repo not identified; not censused.)
  In ordinary matched gameplay functions: SOTN `byte stackpad[40]` inside CheckFloor
  (dra/6BF64.c), `s32 unused_stack[2]`, `volatile u32 pad[4]; // FAKE`; VS `int pad04[5]`;
  MGS `char pad1[0x48]`. **This is verbatim the family BB2's inline-asm-policy forbids**
  and that the 2026-07-01 ruling explicitly kept forbidden — on a census that evidently
  missed all 356.
- Result C — the Judge's central objection ("the bound is attested by nothing, so any
  number is a guess") is one SOTN **answers explicitly**:
  `u8 _pad[40]; // n.b.! needs to be 33-40 bytes (inclusive)` (bo4/unk_45354.c:463, and
  two siblings). The community convention for an unrecoverable bound inside a
  frame-determined range is: pick a value in the range, document the range. BB2's
  range is get_frame_size in [25,32] (H7) => `s32 sp[7]` or `s32 sp[8]`.
- Verdict: **the premise that this family is universally forbidden at the SOTN bar is
  FALSIFIED.** And BB2's construct is strictly MILDER than all 356 (ours is a live,
  75%-written argv buffer passed to a callee, not a dead pad). The argument is
  a fortiori, not direct-precedent — so s4 does **NOT** self-approve. -> ruling-request.

## s3 (structural, 2026-07-13) — the Judge's mandated probe, run to the end

### H6 — KILLED. The exec'd program reads argv[6]/argv[7], proving the 8 slots real.
(The Judge's BINDING frontier item: "recover the bios_Exec argv block's real arity
from the exec'd program (MOVOVL.EXE / NDATA overlays) or other Exec launch sites.")
- Probe: `special_camera_get_rot_dir` CD-reads the exec'd program's PS-EXE header off
  the disc (not from a global, as s1/s2 assumed) => the target is a disc PS-EXE.
  disc/STR/MOVOVL.EXE is the only other EXE on the disc. Disassembled it
  (`--adjust-vma=0x801D8800`), followed crt0 pc0=0x801DA084 -> main=0x801D91CC, which
  takes argv in s3 (`move s3,a1`).
- Result: s3 is never reassigned before use, the argv pointer never escapes main, and
  main reads **exactly argv[0]..argv[5]** (offsets 0,4,8,12,16,20), then `move s3,zero`.
  Its uses match BB2's payload exactly (SetLoc on the &SpecialCam record at argv[4];
  0x80118000 load address at argv[3]; the 5-arg play call on argv[0..3],argv[5]).
  MOVOVL has no Exec of its own (BIOS A0 calls: 0x49/0x44/0x72, no 0x43), and there
  are no other Exec launch sites in the tree — no sibling convention to mine.
- Verdict: **KILLED — arity is 6.** The bound 7-or-8 is attested by NOTHING: not the
  oracle (s2: byte-identical), not the consumer. The "capacity-8 argv buffer is
  natural C" ruling argument is dead; do not revive it. This closes the Judge's
  frontier item, negatively.

### H7 — CONFIRMED. The frame equation makes the search space finite and provable.
- Probe: read `tools/gcc-2.7.2/config/mips/mips.c:compute_frame_size`.
- Result: o32/no-ABICALLS/no-FP reduces to
    total = MIPS_STACK_ALIGN(get_frame_size()) + MIPS_STACK_ALIGN(outgoing_args)
          + MIPS_STACK_ALIGN(gp_reg_size)
  `extra_size`=0, `pretend_args_size` is ABI_64BIT-only, alloca only bites at
  args_size==0. The target PINS outgoing_args=16 (array base 0x10, `addiu $a1,$sp,0x10`)
  and gp_reg_size=24 (exactly six saves; a 7th would need a 7th `sw`, absent).
- Verdict: **CONFIRMED. total=0x48 <=> get_frame_size() in [25,32].** The declared
  locals size is the ONLY free term. There is no other lever, structural or otherwise.

### H8 — KILLED (and it is the PROOF). A 32-byte locals object with every byte written.
"If all 32 bytes have real semantic purpose, the 0x48 frame arises with zero dead
declarations and the unwritten-tail objection evaporates."
- Probe: `s32 argv[6]; s32 idx[2];` — the two func_80036EA8() results get a genuine
  home; both idx elements are written AND read. get_frame_size = 32. sandbox --disable all.
- Result: **score 2** (45 build insns vs 43 target). Frame is CORRECT (-0x48), all 43
  target insns match in shape; the only excess is exactly the two `sw` into idx[0]@0x28
  and idx[1]@0x2C. **The target contains no such stores** — it has exactly six.
- Verdict: **KILLED as a match, CONFIRMED as the proof.** With H7 this is a closed
  trichotomy: reaching 0x48 requires 25-32 declared bytes; C that WRITES the extra bytes
  emits stores the target lacks (+2, measured); C that does NOT write them declares an
  unwritten tail. No fourth branch exists. **=> the ORIGINAL declared a locals object
  strictly larger than what it wrote.** Best zero-dead-bytes floor improves 15 -> 2.
  rejected/idx2-fully-written-32b-locals-adds-2-stores.c

## Search space after s3 — CLOSED (structural levers are exhausted, provably)
Every pure-C form reaching frame 0x48 is exactly one of:
  (A) an argv aggregate of 25-32 bytes (s32 sp[7] / sp[8] / any struct whose first 24
      bytes are the six argv words) — has an unwritten tail; both score 0; the bound is
      unrecoverable from oracle AND consumer;
  (B) argv[6] (24B) + a second aggregate or address-taken scalar of 1-8 bytes — that
      object is wholly dead (strictly worse than (A): same slack, plus a fabricated
      declaration);
  (C) argv[6] + a spilled scalar — impossible, the target has no spill stores.
Nothing else can move the frame (H7). Nothing writes the tail (H8). The remaining
question is not a search problem; it is a POLICY question. -> ruling-request.

## s2 (structural, 2026-07-13)

### H1 — KILLED. Struct-pair reshape gives a naturally-32-byte object.
"Reshape to `struct {s32 cam,a,b,c;} entries[2]`; entry0 fully written, entry1
half written; frame 0x48 arises naturally with zero unused declarations."
(s1's TOP frontier lead.)
- Probe: wrote CamEntry sp[2]; sandbox --disable all.
- Result: **score 0 — but BYTE-IDENTICAL to `s32 sp[8]`.** Zero bytes changed =>
  zero semantics changed. It relabels the same 8 dead bytes as fields w1/w2.
  Data model incoherent (entry[0] fills 4 fields, entry[1] fills 2; the six words
  are the flat argv payload of bios_Exec, not records). cheat-reviewer FAILED it
  as strictly worse than the flat array (fabricated type + names into the tree).
- Verdict: **KILLED.** rejected/struct-pair-relabels-unwritten-tail.c

### H2 — KILLED. The sibling `s32 sp[16]` is accepted precedent for an oversized buffer.
"The adjacent COMPLETED-C callee declares s32 sp[16] and writes only 2 of 16 words,
so a capacity-declared partially-filled buffer is already accepted project practice."
- Probe: read asm/funcs/special_camera_get_rot_dir.s (the callee it hands sp to).
- Result: that function is an OUT-PARAMETER POPULATOR — copy loop 16B/iter + 12B
  tail = **60 of 64 bytes written into the caller's buffer**, then read back. The
  buffer is the PsyQ `struct EXEC` (15 words); sp[8]/sp[9] are s_addr/s_size. It is
  essentially fully written and its SIZE is fixed by a real external type.
- Verdict: **KILLED.** Not a precedent. This was the load-bearing support for the
  planned "capacity buffer is natural C" ruling argument; it is gone.

### H3 — KILLED. The bound 8 is meaningful ("capacity-8 argv buffer, power of two").
- Probe: sandbox `s32 sp[7]` (28B locals).
- Result: **score 0.** MIPS frames round to 8: 16+28+24 = 0x44 -> 0x48, identical to
  16+32+24 = 0x48. sp[7] and sp[8] are BYTE-INDISTINGUISHABLE.
- Verdict: **KILLED.** The declared bound is NOT recoverable from the oracle. "8" is
  a guess fitted to a frame size, not an attested capacity. Do not defend it.

### H4 — CONFIRMED. The locals-region SIZE is the entire and only gap.
- Probe: sandbox `s32 sp[6]` (the only tail-free form).
- Result: score 15, but **43 build insns vs 43 target insns**, six stores in correct
  order; all 15 diffs are sp-relative offset shifts (frame 0x40 vs 0x48).
- Verdict: **CONFIRMED.** The C body is otherwise correct. There is no remaining
  structural lever — nothing to reorder, re-type, split, or re-associate.

### H5 — CONFIRMED. The dead frame bytes are in the ORIGINAL, not manufactured by us.
- Probe: count target stores into the locals region (exactly six, 24 bytes) vs the
  region the prologue reserves (28-32 bytes).
- Result: no C form can write all the reserved bytes, because the target contains
  only six stores. So the original source necessarily declared a locals object
  strictly larger than what it wrote.
- Verdict: **CONFIRMED.** => The ruling question (see outcome JSON). The oracle-
  enforced proof is the PROLOGUE IMMEDIATE, not a dead `sw`; the 2026-07-01
  dead-vars-local-array carve-out requires dead STORES, so the unwritten-tail case
  falls in a genuine policy gap. NOT self-approved.

## Search space now eliminated (do not re-derive)
- 32-byte-locals forms that a reviewer would accept: **none can be named.** Ruled out
  by measurement/inspection: a second genuinely-used 8-byte local (target has no
  stores/loads outside 0x10-0x24, so any such local is dead); wider element type or
  different stride (stores are `sw`, 4-byte stride); a real 32-byte struct (nothing
  in-tree attests one — see evidence.md); compiler-forced alignment padding (sp[6]
  yields a clean 0x40, no padding); a larger outgoing-arg region from a 5+-arg callee
  (buffer base is 0x10 in target AND in sp[6], so the outgoing region is exactly
  0x10); reload/spill slots (target has no spill stores).
- Every remaining candidate reduces to "declare N more bytes because the frame says
  N more bytes."

## [s2] H12: GCC 2.7.2 can reserve locals frame bytes for ORDINARY LIVE LOCALS that no instruction touches (a 'phantom slot'), so s4's trichotomy - 'extra frame bytes require either extra stores or an unwritten-tail array' - has a third branch and is false.
- mechanism: cc1 allocates a stack temp for an HImode computation (two s16 locals feeding a bitwise expr like (a & ~b) & 1) and then never uses it; get_frame_size counts it, no store is ever emitted. Controls: drop the pair -> vars=0; one s16 -> vars=0; widen to s32 -> vars=0. A call is not required.
- probe: args-aware in-binary census (bb2_frame_slack2.py) -> 28 BB2 functions reserve untouched locals; drilled tslLineG5Init (src/code6cac_c2.c:1267), the one already COMPLETED. Engine: sandbox tslLineG5Init --disable all -> score 0, rules_dropped 0, absent from queue.json, no cheat-asm (its single regfix.txt hit, line 203, is a '#' comment). cc1: '.frame $sp,48,$31 # vars= 8, regs= 4/0, args= 24, extra= 0'; the ONLY sp accesses are the 4 saves (32-44) and 'sw $2,16($sp)' (5th outgoing arg, inside the args region) - nothing touches [24,32). Original binary agrees: asm/funcs/tslLineG5Init.s is -0x30. Minimal reproducer + one-factor bisect in minrepro.py / bisect2.py.
- result: get_frame_size()==8 with zero stores into it, in a byte-matching cheat-free function whose C declares only five plain scalars. Independently re-derived mips.c:compute_frame_size to confirm frame = ALIGN8(vars) + ALIGN8(args) + ALIGN8(gp_regs) (so H7 and the [25,32] bound stay valid; only the inference from them collapses).
- verdict: CONFIRMED

## [s2] H13: the phantom slot ports to func_80037540 via HImode index locals - the two func_80036EA8() results are camera-table indices, so declaring them s16 is semantically natural and would take get_frame_size 24 -> 28 -> ALIGN8 -> 32 -> frame 0x48 with no extra stores.
- mechanism: HImode locals inducing an unused stack temp, as in tslLineG5Init; an s16-RETURNING func_80036EA8 makes the sign-extension free because GCC trusts a callee's s16 return to arrive sign-extended.
- probe: sweep2.py, measuring cc1's own vars= readout: s16 pair, u16 pair, u8 pair, three s16, both indices live across the 2nd call, long long pair - each with int- and s16-returning func_80036EA8.
- result: Every variant stays at vars=24 / frame 0x40. The phantom slot does not appear: func_80037540 has no HImode BITWISE semantics (its indices are only shifted, idx*8), so cc1 creates no HImode temp. The mechanism is real; this port of it is dead.
- verdict: KILLED

## [s2] H11: Kengo's shipped ELF attests the original local declarations - the Marionation PS2 successor has a 515KB .mdebug (MIPS ECOFF) section, which would give source-level local names/types/array bounds from the SAME team (the 'evidence external to the frame' the Judge demanded).
- mechanism: External evidence, not codegen. objdump --debugging cannot parse .mdebug (it read only the tiny .stab: 1215 lines of .dsm/.vsm line records), so I wrote a real HDRR->FDR->SYMR/AUX parser; the little-endian SYMR bitfield packing was validated against ground truth (st[0:6] sc[6:11] index[12:32]; check: stFile's index == that FDR's csym == 36).
- probe: mdebug.py + hist.py over Kengo/disc/SLUS_200.21: full st/sc histogram across all local symbols, plus the nm_special_cam.c symbol stream.
- result: 240 file descriptors, 14,845 symbols, ZERO stLocal/stParam. The stream is Proc/End/Label/Static/File only - Kengo was built at procedure granularity (glevel 0). No locals, no types, no struct definitions. Combined with s3's MOVOVL kill (consumer arity = 6), the external-evidence avenue is now EXHAUSTED.
- verdict: KILLED

## [s3] H14 — The phantom slot is a characterizable GCC 2.7.2 pass artifact with a nameable entry condition, not an opaque 'cc1 allocates a temp' (s5's account).
- mechanism: reload's alter_reg() (reload1.c:658) gives a stack slot to any pseudo with reg_renumber<0 && reg_n_refs>0 && no equiv. flow records reg_n_refs>0; combine (which runs AFTER flow) then deletes every insn referencing the pseudo WITHOUT decrementing reg_n_refs; regclass, now having no cost data for it, defaults its preferred class to ST_REGS (degenerate — holds no integer registers); so it can never be allocated, alter_reg calls assign_stack_local on the STALE ref count, and since no insn references it, no load or store is ever emitted. A stale-ref bug, not a feature.
- probe: gdb -batch -ex 'break assign_stack_local' -ex bt on cc1 compiling s2's minimal reproducer. Backtrace: assign_stack_local <- alter_reg (reload1.c:2352) <- reload (reload1.c:658) <- global_alloc. Confirmed against tslLineG5Init's own lreg dump, which literally reads 'Register 92 used 2 times across 1 insns in block 7; ST_REGS or none.' No stack MEM appears in ANY of the 13 RTL dumps (.rtl/.jump/.cse/.loop/.cse2/.flow/.combine/.sched/.lreg/.greg/.sched2/.jump2/.dbr) — the slot is allocated and never referenced.
- result: Mechanism identified end-to-end and independently confirmed on the in-tree witness. This replaces s5's 'cc1 allocates a stack temp for an HImode computation' with the actual pass, the actual call site, and the actual entry condition.
- verdict: CONFIRMED

## [s3] H15 — The phantom ports to func_80037540 via NARROW PARAMETERS (never probed: s2 only narrowed the index locals). The call site is func_80037540(v, 0x80118000, 1, 0xCF8, 0xB01), so a2=1 / a3=0xCF8 / a4=0xB01 are all 16-bit-shaped, and the target stores every param with a plain `sw` (no sign-extension) — consistent with narrow params, since GCC marks an incoming narrow param's pseudo SUBREG_PROMOTED_VAR_P.
- mechanism: A narrow param would make GCC create the HImode->SImode extension pseudo that combine can then eliminate, inducing the phantom with no extra stores.
- probe: 44 variants on cc1's own vars= readout (tmp/grind/func_80037540/s3/sweep_params.py + sweep_all.sh): each of a0..a4 as s16/u16/s8/u8; all 10 same-type pairs; all-s16; all-u16.
- result: ALL 44 give vars=24 / frame 0x40. GCC 2.7.2 hands an incoming narrow param to the body as a PROMOTED SUBREG, so no extension pseudo is ever created and there is nothing for combine to delete. (a4_u16 is the one that even changes the load — `lhu $20,82($sp)` — and it still gives vars=24.)
- verdict: KILLED

## [s3] H16 — The phantom can be induced by ANY computation combine folds away, so a semantically honest re-spelling of an existing computation will induce it at zero instruction cost. Best candidate: SpecialCam records are 8 bytes = TWO WORDS, so a word-index `v0 * 2` scaled by 4 by pointer arithmetic is honest C that expands to `sll 1; sll 2` and folds to the target's single `sll 3`.
- mechanism: combine merges the redundant insn into its consumer, the pseudo loses all refs, and (per H14) the stale count earns it a phantom slot — while the emitted code is unchanged.
- probe: 19 variants: 11 combine-fold reformulations (pointer/CamRec* reshapes, pre-scaled index, walking pointer, long long index, s16-returning callee, redundant u16/s16 round-trip) + 8 two-stage-shift spellings (word-index into an s32 array, explicit *4, shift spelling, byte-index, halfword-index).
- result: ALL 19 give vars=24. The word-index form DOES fold exactly as predicted — combine emits a single `sll $2,$2,3` and the identical 41 insns — but produces NO phantom. So combine's ordinary folds (shift-of-shift, address folding) DECREMENT reg_n_refs correctly. The stale-ref bug is specific to the HImode-extension elimination path, and to nothing else.
- verdict: KILLED

## [s3] H17 — THE DECIDER. A phantom slot is reachable in func_80037540 at all, i.e. there exists a pure-C form with a tail-free `s32 sp[6]` whose get_frame_size lands in [25,32] while emitting only the target's 43 instructions.
- mechanism: Per H14+H16 the ONLY combine path with the stale-ref bug is eliminating a redundant HImode->SImode sign-extension. Combine can only prove that extension redundant when the value is already sign-extended in its register — which happens only for a value loaded by `lh`/`lb` (narrow loads sign-extend on load). Every other entry route for a narrow value was measured and creates no deletable pseudo.
- probe: Exhaustive over the ways a narrow value can enter a function, measuring vars AND instruction count: s16 param (promoted subreg -> vars=24), s16 call return (promoted subreg -> vars=24), truncated arithmetic (extension not redundant; emits sll/sra -> vars=24, 53 insns), lhu/lbu zero-extending loads (-> vars=24), lh sign-extending load (-> vars=32, frame 0x48). Plus a POSITIVE CONTROL: the faithful tslLineG5Init trigger grafted into this function's exact shape at 4 different positions.
- result: The positive control WORKS — all four grafts give vars=32 / frame 0x48 on top of a TAIL-FREE `s32 sp[6]`, proving the target frame needs no unwritten tail and that the harness is sound. But every one costs instructions: 56 insns vs the baseline's 41, because the phantom rides on emitted lh/nor/and/andi/branches. The biconditional is exact across the sweep: vars=32 <=> the function emits a narrow `lh` load. **func_80037540's target contains SEVEN loads, ALL `lw`, and ZERO narrow loads** — and there is no room to add one, since the tail-free `s32 sp[6]` form already emits 43 insns against a 43-insn target. Branch (c) is closed.
- verdict: KILLED

## [s3] H18 — The four census functions that carry untouched reserved locals with NO narrow load in their target asm (func_800520B8, func_8006BD28, func_800644FC, func_800165F8) are counterexamples that falsify H17's necessity condition. Two of them (func_800165F8, func_800644FC) have exactly func_80037540's shape: args=16, slack 8.
- mechanism: If a cheat-free compiled-C function reserves untouched frame bytes without any narrow load, then some OTHER phantom inducer exists and the space stays open.
- probe: Checked each one's completion status (engine/queue.json, regfix.txt, inline_asm_canonical.txt) and read its actual C body.
- result: NOT ONE is a legitimate witness. func_800520B8 and func_8006BD28 are AUTHORIZED CANONICAL-ASM (inline_asm_canonical.txt lines 238/338) — hand-written asm, not compiled C. func_800644FC is INCOMPLETE, IN QUEUE, carrying 13 regfix rules, and its 'phantom' is a manufactured cheat: `s32 dummy_pad; __asm__ volatile("" : "=m"(dummy_pad));` — an `=m` constraint that forces a frame slot while emitting nothing. func_800165F8 = file_LoadSectors (src/ings.c:163) is INCOMPLETE, IN QUEUE, and ships `s32 _pad[2];` — a dead unwritten pad, i.e. the very family under dispute. => Among cheat-free compiled-C functions, 24/24 phantom slots in BB2 come with a narrow load. The necessity condition holds across the whole binary.
- verdict: KILLED
