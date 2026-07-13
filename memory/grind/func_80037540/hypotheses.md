# Hypothesis ledger — func_80037540

## s4 (structural, 2026-07-13) — the minimum is proven; the Judge's census is run

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
