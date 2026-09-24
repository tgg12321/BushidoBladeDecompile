# vmNoiseOn — evidence (manual session 2026-09-24)

Sony LIBSND `vmNoiseOn` (vm_no1.c), 0x80086CF8, 305 insns, src/main.c.
Reference C: sotn-decomp src/main/psxsdk/libsnd/vmanager.c vmNoiseOn (US
main build, `[0x12A0C, c, psxsdk/libsnd/vmanager]` in config/splat.us.main.yaml,
no INCLUDE_ASM in the file). psyz vm_no1.c leaves it INCLUDE_ASM. BB2 deltas:
SpuSetNoiseClock/SpuSetNoiseVoice calls instead of SOTN's direct SPU register
pokes; BB2's 0x36-byte voice record.

## Floor history (sandbox --disable all, measured this session)
- 305: INCLUDE_ASM stub.
- 49: SOTN shape transcribed with the per-word `*(T *)((u8 *)&_svm_voice_plus_N + vc * 54)` spelling.
  Target has 4 callee-saves (s0-s3) where ours had 3: the zero-extended voice index sits in
  call-saved $s0 in the target, ours in $a0.
- 15: `idx = vc;` bound BEFORE the SpuSetNoiseClock call (tmp/vmn/a.c). Mechanism (instrumented
  cc1, BB2_FINDREG_DEBUG on the index pseudo): spelled with vc at each use, the zero_extend is born
  after the call, crosses no call, and global.c pass 0 gives it $a0. Bound before the call it is
  live across the jal -> call-saved -> $s0; sched1 still emits the andi after the jal because no
  dependence ties a pure register insn to the call. Prologue/frame now match.
- 9: record-field style store for the voice-state writes (tmp/vmn/b3.c) moved the _SsVmMaxVoice
  loop-guard load above the `unk04 = 0xA` store (in-struct MEM, sched.c:834-839 exemption).
- 8 -> 0: the voice-reset loop `_svm_voice[voice].unk1b &= 1`. Every per-word spelling
  (`*(s8 *)((u8 *)&sym + voice * 54)`, `(&sym)[voice * 54]`, `*(&sym + voice * 54)`, split
  load/store) expands `&sym` first, memory_address forces the SYMBOL_REF into a pseudo, cse shares
  the sum and loop.c hoists `la t0,sym` out of the loop (3 extra insns). A declared record array
  (ARRAY_REF/COMPONENT_REF) keeps `(plus reg (const sym+0x1D))` in each MEM, as the target has.
  With a stand-in struct array the residual was only relocation-symbol naming (8); with the real
  `_svm_voice` symbol it is 0.

## `idx` re-measured on the FINAL body (2026-09-24, after layer-2 round 1)
Whole-TU compare (tmp/vmn/tucheck.py, merge + Ruling 8 cascade + prototype in place):
- final candidate (idx bound before SpuSetNoiseClock, used for the sreg writes and the
  bit masks; vc at the two `_svm_voice[]` uses): 129/129 identical.
- idx removed, vc at every use: vmNoiseOn differs (385 vs 387 lines): frame 0x28 not 0x30,
  vc lives in $s2 and the index in a caller-saved reg, one fewer call-saved register saved,
  every s-register in the volume math shifts down by one.
- idx also at the two `_svm_voice[]` uses: vmNoiseOn differs (382 vs 387 lines). The target
  zero-extends vc again there (`andi $v1,$s3,0xFF` at 0x80087010 and 0x800870A4) instead of
  reusing $s0.
- adding `extern s32 SpuSetNoiseClock(s32);` (was implicitly declared): byte-neutral.

## The _svm_voice aggregate merge (lands with this function)
`struct SpuVoice` (include/sound.h): psyz libsnd_private.h `struct SpuVoice` (0x34 bytes) plus
one BB2-only halfword at +0x0C, so psyz fields from `note` on sit 2 bytes later. Evidence the
shift is at +0x0C: SsUtKeyOnV stores note at +0xE, 0x21 at +0x10 (psyz unke), program at +0x12
(psyz unk10), prog +0x14, tone +0x16, vabId +0x18 (psyz 0x16); _SsVmInit writes 0x40 to the
byte at +0xA (psyz unka); unk1b (key state) at +0x1D (psyz 0x1B). Stride 54 = 0x36 throughout
(_SsVmInit, _SsVmKeyOffNow, SsUtKeyOnV, _SsVmSeqKeyOff, asm-only _SsVmFlush/func_80087770).

Byte-neutrality (tmp/vmn/tucheck.py: whole-TU compile through the real pipeline, every
function's disassembly compared with relocations resolved to addresses): all 129 main.c
functions identical, including every converted sibling:
- SsUtKeyOnV, _SsVmDoAllocate, _SsVmInit, _SsVmKeyOnNow, _SsVmSeqKeyOff: direct conversion.
- _SsVmKeyOffNow: psyz vm_nowof.c body verbatim now matches; the FAKE okof1/okof2 staging
  locals are gone.
- func_800858D0: plain C now matches; the empty-if (F6) closer, the `u` carrier, the
  `offset` {stride, 1} reuse and the `ff` constant-holder are all gone. (Straight conversion
  keeping them differed: li 24 before the stride calc; the minimal form matched.)

## The `temp` pan cascade (owner Ruling 8, 2026-09-24, vmNoiseOn only)
The SOTN spelling reuses one `u32 temp` for the three pan stages (tone_pan, mpan, pan).
Receipts (Ruling 8 prong D). Both measured on the whole TU with the rest of candidate.c
unchanged (tmp/vmn/tucheck.py: every main.c function's disassembly compared with relocations
resolved; the candidate is 129/129 identical, vmNoiseOn 387 lines):
- Direct reads: the Ruling 8 (B) cascade with every `temp` replaced by the field it holds
  (`if (_svm_cur.tone_pan < 0x40) { voll = voll_t; volr = (volr_t * _svm_cur.tone_pan) / 0x3F; } ...`
  and likewise `_svm_cur.mpan`, `_svm_cur.pan`); `u32 temp;` removed. Result: vmNoiseOn differs,
  388 lines. No other function differs.
- One local per stage: the same cascade with `u32 tone_pan; u32 mpan; u32 pan;` declared at
  function scope, `tone_pan = _svm_cur.tone_pan;` / `mpan = _svm_cur.mpan;` / `pan = _svm_cur.pan;`
  as the stage heads, each read only in its stage. Result: vmNoiseOn differs, 388 lines (the same
  diff as direct reads). No other function differs.
In both, the tone_pan load schedules after the tone_vol multiply chain, one extra load-delay nop
is emitted, and v0/v1 (and a0/v1) swap through the three stages. The layer-2 rule-text reviewer
re-measured both independently (same result).
Mechanism (author's reading, not confirmed by an instrumented sched dump of vmNoiseOn): a
single-set pan pseudo gets sched1's birthing boost (sched.c adjust_priority -> birthing_insn_p,
`reg_n_sets == 1`), and the three-write temp does not. A local written three times with
different fields is outside Ruling 5/6, so the owner was asked; the answer was "Allow,
vmNoiseOn only" (Ruling 8). The question told the owner the alternatives were "off by a few
register choices", which understated the scheduling difference; the rule records the correction.
