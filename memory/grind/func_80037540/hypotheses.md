# Hypothesis ledger — func_80037540

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
