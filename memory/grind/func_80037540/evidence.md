# Evidence bank — func_80037540

## s5 (structural, 2026-07-13) — **THE SEARCH SPACE IS OPEN. The "proof" that closed it is false.**

- [s5] **PHANTOM FRAME SLOTS EXIST, AND ONE IS ALREADY SHIPPED, MATCHED AND CHEAT-FREE
  IN THIS TREE.** `tslLineG5Init` (src/code6cac_c2.c:1267) is **COMPLETED-C**:
  `sandbox tslLineG5Init --disable all` -> **score 0, rules_dropped 0**, absent from
  engine/queue.json, no cheat-asm (its one regfix.txt hit, line 203, is a `#` COMMENT,
  not a rule). cc1's own frame comment for it:
      .frame $sp,48,$31   # vars= 8, regs= 4/0, args= 24, extra= 0
  => **get_frame_size() == 8.** The COMPLETE list of sp-relative accesses it emits:
  the four register saves (32,36,40,44) and `sw $2,16($sp)` twice (the 5th outgoing
  arg — inside the ARGS region [0,24), not the locals region). **Nothing touches
  [24,32).** Eight bytes reserved, never written, never read. Its C declares five
  plain scalars — no array, no dead declaration, no annotation, no cheat. The original
  binary has the same frame (asm/funcs/tslLineG5Init.s: `addiu $sp,$sp,-0x30`).
  => **GCC 2.7.2 reserves locals frame bytes for ORDINARY LIVE LOCALS that no
  instruction ever touches.**

- [s5] **THEREFORE s4's TRICHOTOMY — the sole basis for "the original declared a dead
  tail" and for the owner ruling-request — IS FALSE.** s4 argued: to hold frame bytes
  an object must defeat scalar-promotion => >=2 elements => >=2 extra stores; so any
  0x48 form either emits stores the target lacks OR declares an unwritten tail; "no
  fourth branch exists". It missed the branch above: **get_frame_size > bytes-written,
  arising from live code with zero stores.** The frame equation (H7) and the six-store
  count are still correct; the INFERENCE from them is not. The a-fortiori SOTN
  argument, the capacity-buffer ruling question, and the "HELD, blocked on a single
  owner yes/no" disposition all rest on a refuted premise.
  **func_80037540 is NOT blocked. It has real, un-searched space.**

- [s5] MECHANISM (one-factor bisect + minimal reproducer,
  tmp/grind/func_80037540/s2/minrepro.py): the trigger is **two HImode (`s16`) locals
  feeding an HImode BITWISE expression** (`(a & ~b) & 1`) — cc1 allocates a stack temp
  for the HImode computation and then never uses it (vars=8, untouched; no call
  required). Controls: remove the pair -> vars=0; use ONE s16 -> vars=0; widen the
  pair to s32 -> vars=0.

- [s5] **NEW INSTRUMENT — measure get_frame_size DIRECTLY.** cc1 prints it:
      bash tmp/grind/func_80037540/s2/build.sh <candidate.c>
      -> .frame $sp,N,$31   # vars= V, regs= R, args= A, extra= E
  V IS get_frame_size(). Target requires **V in [25,32], args=16, regs=6, six stores
  only**. Harness validated against the banked ladder: `s32 sp[8]` -> vars=32/frame
  0x48; `s32 sp[6]` -> vars=24/frame 0x40. Use this as the gradient — the sandbox
  score cannot separate "wrong frame" from "wrong codegen".

- [s5] KILLED — the phantom slot does NOT port via index locals. Measured (all
  vars=24, frame 0x40): s16 pair, u16 pair, u8 pair, three s16, both indices live
  across the 2nd call, a `long long` pair — including with an **s16-returning**
  func_80036EA8 so the sign-extension is free. func_80037540 has no HImode bitwise
  semantics (indices are only shifted, `idx * 8`), so no HImode temp is created.
  rejected/s16-index-locals-do-not-port-phantom-slot.c

- [s5] KILLED — Kengo cannot attest the bound. `Kengo/disc/SLUS_200.21` has a 515 KB
  `.mdebug` section, but `objdump --debugging` only reads its tiny `.stab`. Wrote a
  real MIPS ECOFF parser (tmp/.../mdebug.py; SYMR bitfields validated: stFile's index
  == the FDR's csym). Result: 240 files, **14,845 symbols, ZERO stLocal/stParam** —
  Kengo shipped procedure-granularity debug info only (no locals, no types, no
  structs). With s3's MOVOVL kill (consumer arity = 6), the external-evidence avenue
  is EXHAUSTED. (Parser is reusable; the data isn't there.)

- [s5] IN-BINARY CENSUS (args-aware; tmp/.../bb2_frame_slack2.py — resolves every
  callee's arity from its prototype, so the outgoing-args region is exact rather than
  assumed 16). **28 BB2 functions reserve locals frame bytes they never touch.** In
  light of the phantom-slot finding these are mostly NOT dead declarations — they are
  the ordinary GCC 2.7.2 artifact. (v1 of this census wrongly assumed args=16 for
  every function; tslLineG5Init exposed it — it passes a 5th arg on the stack, so its
  args region is 24, not 16. Fixed.)

- [s5] FLAGGED, NOT EXPLOITED: the sibling `func_8003DBE4` (src/code6cac_c2.c:1320)
  ships `s32 buf[2]; /* dead local: reserves the target's 8 extra frame bytes */` on
  main — verbatim the forbidden unwritten-array construct. It is **ACTIVE in the queue
  at distance 21**, so it is NOT precedent for acceptance; it is an open cheat sitting
  in an INCOMPLETE function. The phantom-slot mechanism may dissolve it.

- Audit diagnosis (regressions.md): s32 sp[8] oversized by 2 elements: sp[6] and sp[7] are never written or read; callee is invoked with count=6, so only sp[0]-sp[5] have semantic purpose. The 2 unused elements inflate the stack frame from 0x40 (sp[6] would give 16+24+24=64) to 0x48 (sp[8] gives 16+32+24=72), matching the target prologue `addiu $sp,$sp,-0x48`. This is the 'oversized local array frame-coercion' pattern (inline-asm-policy expanded catalog: 'Unused fixed-size local arrays — s32 buf[N]; declared with no use, to force GCC to reserve frame bytes'). The correct fix is to find a C form where a 0x48 frame arises naturally (e.g., a genuine 8-element array if the original spec used one, a second local variable, or a different struct layout) — or to determine whether sp[6]/sp[7] ARE legitimately read by marionation_camera_Init_80037468 despite the count=6 argument. Until resolved, this function is cheat-carried and must remain INCOMPLETE.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct at src/code6cac_b2_post.c:497: `s32 sp[8]`; only sp[0]-sp[5] written (lines 501-507); callee invoked with count=6 (line 509).

- [s1] [fable-blitz 2026-07-07] LOAD-BEARING per target bytes: asm/funcs/func_80037540.s:2 prologue is `addiu $sp,$sp,-0x48`; array lives at 0x10 (asm line 33: `addiu $a1,$sp,0x10`), used elements end 0x28, but callee saves start at s0=0x30 (asm lines 3-15: s0 0x30, s1 0x34, s2 0x38, s3 0x3C, s4 0x40, ra 0x44). The 8-byte gap 0x28-0x30 is exactly the 2 extra elements; with sp[6] the frame would be 0x40 with saves at 0x28.

- [s1] [fable-blitz 2026-07-07] The diagnosis's open question (do sp[6]/sp[7] get read by the callee?) resolves to UNKNOWABLE-STATICALLY-BUT-PLAUSIBLE: marionation_camera_Init_80037468 (src/code6cac_b2_post.c:471-493) never indexes a1 itself - it forwards it verbatim to `bios_Exec(sp, a0, a1)` (line 488), the PS1 Exec(header, argc, argv) idiom with argc=6, argv=the flagged array. The consumer is the exec'd program, outside the binary. A capacity-8 argv buffer filled with 6 entries is plausible human C for this idiom.

- [s1] [fable-blitz 2026-07-07] CLEAN-RESHAPE LEAD (unmeasured): the 6 written slots factor as two 4-word records - sp[0..3] = {SpecialCam+idx*8, a3, a0, a1}, sp[4..7] = {SpecialCam+idx2*8, a4, -, -} (second record half-filled). `struct {s32 cam,x,y,z;} entries[2]` = 0x20 bytes at 0x10 ends exactly at 0x30 -> frame 0x48 arises NATURALLY with full semantic purpose (2 camera entries), no oversizing. Store offsets/order identical to the flat array.

- [s1] [fable-blitz 2026-07-07] Family check: dead-vars-local-array carve-out (2026-07-01) does NOT literally cover this - it requires WRITTEN-never-read elements with dead stores present in target bytes; here sp[6]/sp[7] are never written (no stores exist in target for 0x28-0x2F), so the gap is pure frame reservation - the still-forbidden shape unless the capacity-buffer reading is accepted as natural.

- [s1] [fable-blitz 2026-07-07] Callee's own `s32 sp[16]` with sp[8]=a2, sp[9]=0 (src/code6cac_b2_post.c:472,485-486) is the 0x40-byte Exec header struct - corroborates the Exec-idiom reading of the whole cluster and suggests these buffers were declared by capacity, not by exact use, in the original source.
  **[s2 CORRECTION - this inference is FALSE, see below]**

## s2 (structural, 2026-07-13) — the frame-size space is now FULLY MEASURED

- [s2] FRAME SWEEP (sandbox --disable all, all forms same 43 target insns):
    `s32 sp[6]` (24B locals, tail-free) -> **15**
    `s32 sp[7]` (28B locals)            -> **0**
    `s32 sp[8]` (32B locals, on main)   -> **0**
    `CamEntry sp[2]` struct-pair (32B)  -> **0**
  Target locals region = 0x10..0x30; target contains EXACTLY SIX stores
  (0x10,0x14,0x18,0x1C,0x20,0x24) = 24 bytes written. Bytes 0x28-0x2F never
  written and never read by the target.

- [s2] **sp[7] and sp[8] are BYTE-INDISTINGUISHABLE.** MIPS frames round to 8:
  16 + 28 + 24 = 0x44 -> aligned to **0x48**; 16 + 32 + 24 = **0x48**. So the
  original's declared BOUND IS NOT RECOVERABLE FROM THE ORACLE. This KILLS the
  "power-of-two capacity-8 argv buffer" story that s1's frontier proposed to seek
  a ruling on — 8 is not attested by the bytes; 7 works identically. Any specific
  bound we write is a guess fitted to a frame size.

- [s2] sp[6] is off by 15 with **identical instruction count (43 vs 43)** and the
  six stores in the correct order; all 15 diffs are sp-relative offset shifts from
  frame 0x40 vs 0x48. => The C is OTHERWISE CORRECT. The locals-region SIZE is the
  ENTIRE and ONLY gap. Nothing else remains to restructure.

- [s2] **KILLED — the s1 sibling precedent is factually WRONG.** s1 claimed the
  adjacent COMPLETED-C callee marionation_camera_Init_80037468 declares `s32 sp[16]`
  and writes only 2 of 16 words (i.e. an accepted oversized buffer). It does not.
  `special_camera_get_rot_dir(sp)` is a callee that POPULATES that buffer as an
  out-parameter: asm/funcs/special_camera_get_rot_dir.s:33-50 is a copy loop
  (16 B/iter) plus a 12-byte tail = **60 bytes written into the caller's buffer**,
  then read back (lw 0x8/0xC). It is the PsyQ `struct EXEC` (15 words = 60B);
  sp[8]/sp[9] are s_addr/s_size, which is exactly the pair the callee overrides
  before bios_Exec. So that buffer is essentially FULLY written and its size is set
  by a REAL externally-defined type. It is NOT precedent for an unwritten tail.
  (Side note: that sibling would be more honestly typed `struct EXEC` — byte-neutral
  cleanup, and its `s32 sp[16]` spelling is what made the false precedent plausible.)

- [s2] **KILLED — struct-pair reshape (s1's top frontier lead).** `CamEntry sp[2]`
  scores 0 but emits BYTE-IDENTICAL output to `s32 sp[8]` => it changes zero
  semantics, only relabels the same 8 dead bytes. Its data model is incoherent:
  the six words are {&SpecialCam+i*8, a3, a0, a1, &SpecialCam+j*8, a4} — entry[0]
  fills 4 fields, entry[1] fills 2; real parallel records fill the same fields.
  These six words are the FLAT argv payload of bios_Exec(hdr, argc=6, argv), a
  parameter list, not records. cheat-reviewer (layer-2, default-FAIL) FAILED it as
  strictly WORSE than the flat array (fabricated type + fabricated field names
  permanently injected into the tree). See rejected/struct-pair-relabels-unwritten-tail.c

- [s2] NO 32-BYTE TYPE IS ATTESTED ANYWHERE IN THE TREE. `bios_Exec` is declared
  `extern void bios_Exec(s32 *, s32, s32 *)` (include/code6cac.h:456) — argv is a
  FLAT s32*, argc=6. func_80037540's sole caller is src/code6cac_c2.c:405
  `func_80037540(v, (s32)0x80118000, 1, 0xCF8, 0xB01)`, so the argv payload is
  {&SpecialCam[i], 0xCF8, v, 0x80118000, &SpecialCam[j], 0xB01} — six flat words
  whose consumer is the exec'd program, OUTSIDE this binary. func_80037540 is the
  only caller of marionation_camera_Init_80037468, which is the only bios_Exec
  launcher of this shape. Nothing in-tree attests a 32-byte object.

## s3 (structural, 2026-07-13) — the Judge's external-evidence probe is RUN; space is PROVEN finite

- [s3] **THE EXEC'D PROGRAM IS `disc/STR/MOVOVL.EXE`, AND ITS ARITY IS 6.** This is
  the probe the Judge mandated, executed end-to-end.
  Chain: `special_camera_get_rot_dir(dest)` does NOT copy an EXEC header from a
  global (s1/s2 both assumed a global) — it **CD-reads 0x800 bytes from the disc**
  (`func_800372F4(0x800, sp_buf, 0x80)` after `func_80080258(2, SpecialCam[i], 0)`
  = SetLoc) and copies `sp_buf[0x10..0x4C]` = **60 bytes** into `dest`. That is the
  PS-EXE header's `struct EXEC` (15 words) read straight off the disc. So the
  exec'd program is a PS-EXE **on the disc**, located by a SpecialCam MSF record.
  MOVOVL.EXE is the only other EXE on the disc (`find disc -iname '*.EXE'`).
  Its header: pc0=0x801DA084, t_addr=0x801D8800.
  pc0 is crt0 (`lui/addiu gp; j 0x801D91CC`) — it does not touch a0/a1, so argc/argv
  reach main at **0x801D91CC** intact. main does `move s3,a1` (s3 = argv), and:
    * `s3` is NEVER reassigned before its last use (only the prologue callee-save
      spill `sw s3,308(sp)` + epilogue reload);
    * the argv POINTER never escapes main (never passed to a callee, never stored
      anywhere else) — so no callee can read slots main doesn't;
    * main reads **exactly** `0(s3) 4(s3) 8(s3) 12(s3) 16(s3) 20(s3)` = argv[0..5],
      then kills it with `move s3,zero` (0x801D9344, the delay slot of the last use).
  Uses line up perfectly with BB2's payload: `f(2, argv[4], 0)` = CdControl SetLoc on
  the &SpecialCam record; `f(0x4C, argv[3], 0x80, ...)` on argv[3]=0x80118000 (a load
  address); `f(argv[0],argv[1],argv[2],argv[3],argv[5])` = the movie-play call.
  => **argv[6] / argv[7] ARE NEVER READ BY THE CONSUMER.**
  **KILLS the s1 frontier item and the whole "capacity-8 argv buffer" ruling
  argument.** 8 is attested by NOTHING: not the oracle (s2: sp[7]==sp[8] byte-for-byte),
  not the exec'd program. The declared bound is unrecoverable, full stop.
  (Also checked: MOVOVL contains no Exec of its own — its only BIOS A0 calls are
  0x49/0x44/0x72, no 0x43 — so there is no reciprocal Exec launch site to mine for
  an array-bound convention. There are no other Exec launch sites anywhere.)

- [s3] **THE FRAME FORMULA IS AIRTIGHT — locals size is the ONLY free term.**
  Read from `tools/gcc-2.7.2/config/mips/mips.c:compute_frame_size` (o32, no
  ABICALLS, no FP):
      total = MIPS_STACK_ALIGN(get_frame_size())        <- var_size
            + MIPS_STACK_ALIGN(outgoing_args_size)
            + MIPS_STACK_ALIGN(gp_reg_size)
  `extra_size` = 0 (guarded by TARGET_ABICALLS); `pretend_args_size` is added only
  under `ABI_64BIT && mips_isa >= 3`; `calls_alloca` only matters when args_size==0.
  Both other terms are PINNED BY THE TARGET BYTES:
    * outgoing_args_size = 16 — the target takes `addiu $a1,$sp,0x10`, so the locals
      region starts at 0x10; any other value moves the array base.
    * gp_reg_size = 24 — the target has exactly six saves (s0@0x30 s1@0x34 s2@0x38
      s3@0x3C s4@0x40 ra@0x44). (A 7th save would round 28->32 and also yield 0x48 —
      but that needs a 7th `sw`, and the target has none.)
  => **total = 0x48 <=> get_frame_size() in [25,32].** Exhaustive.

- [s3] **THE PROOF, closed with a measurement.** A locals object of 25-32 bytes is
  REQUIRED; the target WRITES exactly 24 bytes (six stores, 0x10..0x24; nothing at
  0x28-0x2F). Probed the one form that makes all 32 bytes semantically real with
  ZERO dead declarations — `s32 argv[6]; s32 idx[2];`, giving the two
  func_80036EA8() results a genuine home (both elements written AND read):
      **sandbox --disable all = 2** (45 build insns vs 43 target)
  The frame is CORRECT (-0x48) and all 43 target instructions match in shape; the
  only excess is exactly the two `sw` into idx[0]@0x28 / idx[1]@0x2C. See
  rejected/idx2-fully-written-32b-locals-adds-2-stores.c.
  Full frame-size ladder, all measured:
      s32 sp[6]               (24B, tail-free)          -> 15
      s32 argv[6]; s32 idx[2] (32B, every byte written)  ->  2
      s32 sp[7]               (28B)                      ->  0
      s32 sp[8]               (32B, on main)             ->  0
  **Trichotomy (exhaustive, no fourth branch):** to reach 0x48 the C must declare
  25-32 bytes of locals. Either it WRITES the extra bytes (=> emits stores the target
  does not contain, +2 as measured) or it does NOT (=> an unwritten tail). Therefore
  **the ORIGINAL SOURCE ITSELF declared a locals object strictly larger than what it
  wrote.** This is no longer an inference from one failed variant — it follows from
  the compiler's own frame equation plus the target's store count.
  NB this also LOWERS the best zero-dead-bytes floor from 15 (s2's sp[6]) to **2**.

- [s2] **THE CORE FINDING (drives the ruling-request).** The target's prologue
  reserves 28-32 bytes of locals while the target writes exactly 24. No C form can
  write all of them — the target has only six stores. Therefore **the ORIGINAL
  SOURCE ITSELF contained a declaration whose only emitted effect was frame
  reservation.** The dead frame bytes are a property of the original, not a
  construct we manufacture. The oracle-enforced proof here is the PROLOGUE
  IMMEDIATE (`addiu $sp,$sp,-0x48`) rather than a dead `sw` — which is the exact
  precondition the 2026-07-01 dead-vars-local-array carve-out was written around
  (it requires dead STORES in target bytes). Unwritten-tail is currently still
  forbidden. This is a genuine policy gap; s2 does NOT self-approve it.

## s4 (structural, 2026-07-13) — the trichotomy's minimum is PROVEN, and the Judge's census is RUN

- [s4] **THE "+1 EXTRA STORE" FORM DOES NOT EXIST — s3's +2 is the PROVEN MINIMUM.**
  s3 measured ONE member of the fully-written branch (`s32 argv[6]; s32 idx[2];`,
  32B, score 2) and inferred the branch was lossy. The cheaper member it never
  probed is `s32 argv[6]; s32 idx[1];` (28B, one extra `sw`) — which SHOULD give
  score 1. MEASURED: **score 15, frame -0x40**, and objdump shows **NO STORE INTO
  idx[0] AT ALL**. GCC 2.7.2 **scalar-promotes a single-element local array**
  (constant index, address never taken) into a register — it survives in $v0/$s0
  across both `jal`s. get_frame_size = 24, NOT 28. The form degenerates exactly to
  `s32 sp[6]`. Artifact: tmp/grind/func_80037540/s2/idx1_promoted.objdump
  => To occupy frame bytes at all, the extra object must DEFEAT promotion: it needs
  >=2 elements (or an address escape — and this function has no callee that takes a
  pointer: func_80036EA8(int,int), func_800392B8(void), and argv is already the
  pointer arg). >=2 elements written => >=2 extra stores. o32 MIPS-I has no 8-byte
  store, so an 8-byte object cannot be fully written in one instruction either.
  **The fully-written branch's floor is exactly +2, and it is now a proof, not a
  sample.** The trichotomy (H7) is closed at both ends by measurement.
      s32 sp[6]                 (24B, tail-free)               -> 15
      s32 argv[6]; s32 idx[1]   (PROMOTED; degenerates to 24B) -> 15   [s4, new]
      s32 argv[6]; s32 idx[2]   (32B, every byte written)      ->  2   [PROVEN MIN]
      s32 sp[7] / s32 sp[8]     (28B/32B, unwritten tail)      ->  0
  rejected/idx1-scalar-promoted-zero-frame-bytes.c

- [s4] **THE JUDGE'S MANDATED CENSUS IS RUN — and it INVERTS the premise.**
  The Judge's binding constraint: "the ONLY sanctioned next step is to assemble a
  SOTN-master-branch (+oot/papermario/MGS/VS/ESA) evidence census on the
  capacity-declared-partially-filled-buffer family and put it to the owner for a
  carve-out ruling." Done, mechanically, over fresh clones of each tree.
  Scripts: tmp/grind/func_80037540/s2/census_unwritten_tail.py (family A),
           tmp/grind/func_80037540/s2/census_untouched_pad.py  (family B)

  **Family A — the EXACT shape (capacity buffer, constant-index, unwritten tail,
  base escapes to a callee): ZERO instances in SOTN master.** Detector calibrated
  against the known `u8 sp70[4]` case (dra/62DEC.c) and against a raw grep (it sees
  124 of ~151 local fixed-size arrays). Across the entire population, NOT ONE array
  has its top constant index below N-1. So there is **no direct precedent** for our
  literal shape. Reported honestly; it is not the end of the story.

  **Family B — the STRICTLY STRONGER shape (local array declared and NEVER TOUCHED
  AT ALL; its only possible effect is get_frame_size): 356 instances.**
      SOTN master        40   (dra/, ric/, maria/, boss/, st/, weapon/, psxsdk)
      oot                38
      papermario          3
      Vagrant Story      30   (ser-pounce/rood-reverse — PS1, Square, PsyQ-era)
      MGS (mgs_reversing) 245
      ESA                 --  (repo not identified; NOT censused — disclosed, not silently dropped)
  Representative, all in ordinary matched gameplay functions:
      SOTN  dra/6BF64.c:156          `byte stackpad[40];`      (in CheckFloor, core DRA)
      SOTN  boss/bo4/unk_45354.c:115 `s32 unused_stack[2];`    (5-statement function; 8 bytes)
      SOTN  st/sel/stream.c:80       `volatile u32 pad[4]; // FAKE`
      VS    BATTLE.PRG/3A1A0.c:7     `int pad04[5];`
      MGS   chara/torture/johnny.c:38 `char pad1[0x48];`
  **This is verbatim the construct BB2's inline-asm-policy expanded catalog forbids**
  ("Unused fixed-size local arrays — `s32 buf[N];` declared with no use, to force GCC
  to reserve frame bytes") and that the 2026-07-01 dead-vars-local-array ruling
  explicitly kept forbidden ("The unwritten-array and `(void)&local` forms remain
  forbidden") while sanctioning only the weaker WRITTEN-never-read form. That ruling's
  census evidently did not surface these 356 instances.

- [s4] **SOTN ANSWERS THE JUDGE'S CENTRAL OBJECTION VERBATIM.** The Judge's binding
  reason for refusing every 0x48 form was: *"the oracle attests no bound (sp[7] and
  sp[8] are byte-indistinguishable), so any such number is a guess."* SOTN ships
  exactly that situation and documents it as a RANGE:
      src/boss/bo4/unk_45354.c:463  `u8 _pad[40]; // n.b.! needs to be 33-40 bytes (inclusive)`
      src/boss/bo4/unk_45354.c:794  `u8 _pad[40]; // any size between 33-40 (inclusive);`
      src/boss/bo4/unk_46E7C.c:859  `u8 _pad[40]; // must be between 33 & 40`
  An unrecoverable bound inside a frame-determined range is NORMAL in the community
  bar BB2 holds itself to; the convention is to pick a value in the range and write
  the range in a comment. **BB2's range here is get_frame_size in [25,32]** (H7),
  i.e. `s32 sp[7]` or `s32 sp[8]` — the identical situation, arrived at by the
  identical reasoning.

- [s4] **BB2's CONSTRUCT IS STRICTLY MILDER THAN THE 356 PRECEDENTS.** func_80037540's
  array is NOT dead: 6 of its 8 words are written and its base is passed to a callee
  as the argv of `bios_Exec(hdr, argc=6, argv)` (s3: the consumer, MOVOVL.EXE's main,
  reads argv[0..5]). It is a capacity-declared, partially-filled parameter buffer —
  a shape a human writes. The 356 precedents are 100%-dead pads with names that
  announce the intent (`pad`, `stackpad`, `unused_stack`, `padding`, `dummy`). If a
  wholly-dead `byte stackpad[40]` clears the SOTN bar inside CheckFloor, a 75%-written
  argv buffer clears it a fortiori. => a-fortiori argument, NOT a direct-precedent one;
  s4 does NOT self-approve. -> ruling-request.

- [s2] tslLineG5Init (src/code6cac_c2.c:1267) is COMPLETED-C - sandbox --disable all = 0, rules_dropped = 0, not in engine/queue.json, no cheat-asm - and has get_frame_size() == 8 with NOTHING touching those 8 bytes. Its C declares five plain scalars: no array, no dead declaration, no annotation. This is an in-tree, byte-verified, cheat-free witness that reserved-but-untouched frame bytes arise from ORDINARY LIVE CODE.

- [s2] Independently re-read tools/gcc-2.7.2/config/mips/mips.c:compute_frame_size: total = MIPS_STACK_ALIGN(get_frame_size) + MIPS_STACK_ALIGN(outgoing_args) + extra(0, TARGET_ABICALLS-gated) + MIPS_STACK_ALIGN(gp_reg_size) + MIPS_STACK_ALIGN(fp). MIPS_STACK_ALIGN is ((x)+7)&~7 (mips.h:2063). So H7's frame 0x48 <=> get_frame_size in [25,32] is CONFIRMED - the equation was never the problem; the inference layered on top of it was.

- [s2] s4's trichotomy is refuted, not merely doubted: its load-bearing step ('to hold frame bytes an object must defeat scalar-promotion => >=2 elements => >=2 extra stores') is falsified by a live counterexample in this same binary. Consequently 'the ORIGINAL SOURCE ITSELF declared a locals object strictly larger than what it wrote' does NOT follow, and the whole owner ruling-request (capacity-declared-partially-filled-buffer carve-out, the a-fortiori SOTN argument, the 'HELD, blocked on a single owner yes/no' disposition) rests on a refuted premise. Nothing should be escalated to the owner on this ground.

- [s2] NEW INSTRUMENT: cc1 prints get_frame_size() itself in its .frame comment ('# vars= V, regs= R, args= A, extra= E'). tmp/grind/func_80037540/s2/build.sh compiles any candidate with the project's exact flags and prints it. Harness validated against the banked ladder: s32 sp[8] -> vars=32/frame 0x48; s32 sp[6] -> vars=24/frame 0x40. Target needs vars in [25,32], args=16, regs=6, six stores only. This is a direct gradient on the ONE free term - strictly better than the sandbox score, which cannot separate 'wrong frame' from 'wrong codegen'.

- [s2] Phantom-slot mechanism, minimal reproducer (minrepro.py): two HImode (s16) locals feeding an HImode BITWISE expression ((a & ~b) & 1) -> vars=8, untouched. Controls: pair removed -> vars=0; one s16 -> vars=0; pair widened to s32 -> vars=0. No call required.

- [s2] The phantom slot does NOT port to func_80037540 via index locals: s16/u16/u8 pairs, three s16, both indices live across the 2nd call, and a long long pair ALL measure vars=24/frame 0x40 - even with an s16-returning func_80036EA8 (which makes the sext free). func_80037540's indices are only shifted (idx*8), never used in HImode bitwise ops, so no HImode temp is created.

- [s2] Args-aware in-binary census (bb2_frame_slack2.py): 28 BB2 functions reserve locals frame bytes they never touch. Given the phantom-slot finding these are mostly the ordinary GCC 2.7.2 artifact, not dead declarations. (The first census version wrongly assumed outgoing_args=16 for every function; tslLineG5Init exposed the bug - it passes a 5th arg on the stack, so its args region is 24. The fixed version resolves every callee's arity from its prototype.)

- [s2] Kengo's .mdebug carries no local-variable debug info at all (14,845 symbols, 0 stLocal/stParam; procedure granularity only). The reusable ECOFF parser is at tmp/grind/func_80037540/s2/mdebug.py.

- [s2] FLAGGED (not exploited, not my function): the sibling func_8003DBE4 (src/code6cac_c2.c:1320) ships 's32 buf[2]; /* dead local: reserves the target's 8 extra frame bytes */' on main - verbatim the forbidden unwritten-array construct. It is ACTIVE in the queue at distance 21, so it is NOT precedent for acceptance; it is an open cheat in an INCOMPLETE function that the phantom-slot mechanism may well dissolve.

- [s2] No src/ edits were made this session: src/code6cac_b2_post.c still carries the held sp[8] form (sandbox --disable all = 0, target_insns 43 == build_insns 43).
