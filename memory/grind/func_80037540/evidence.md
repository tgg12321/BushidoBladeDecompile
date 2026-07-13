# Evidence bank — func_80037540

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
