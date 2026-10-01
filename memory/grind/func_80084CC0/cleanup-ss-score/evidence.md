# cheat-cleanup 2026-09-30 (laneF): `_ss_score` typed as SOTN's `struct SeqStruct *_ss_score[32]`

Source: docs/audits/RETRO-AUDIT-2026-09-29.md, "Follow-ups (not started)": adopt Sony's
`SeqStruct *_ss_score[32]` (SOTN libsnd_i.h:176) across the main.c consumers.

This ledger covers ALL 18 consumers (it lives under func_80084CC0 because that is the one
function whose close needed more than the retype). Consumers, all in src/main.c:
SsSeqCalledTbyT, _SsSndCrescendo, _SsSndDecrescendo, _SsSndPause, _SsSeqPlay, _SsSeqGetEof,
func_80084CC0, _SsReadDeltaValue, _SsSndNextSep, _SsSndReplay, _SsSndStop, _SsSndTempo, vmNoiseOn,
_SsVmKeyOnNow, func_80087770, _SsVmGetSeqVol, func_80087D10, func_80087D58.

## 1. SOTN's declaration fits, SOTN's record layout does not

- The TABLE matches. `_ss_score` = 0x80106F28, and the next symbol, `_SsMarkCallback`, is at
  0x80106FA8 = +0x80 = 32 words. Every consumer loads `_ss_score[sep]` as a pointer, then adds
  `seq * 0xB0`. So SOTN's `struct SeqStruct *_ss_score[32]` (sotn-decomp
  src/main/psxsdk/libsnd/libsnd_i.h:176 @aa53500) is the right declaration.
- The RECORD does not match. SOTN's `struct SeqStruct` (libsnd_i.h:120-171) is 0xAC bytes, but BB2's
  stride is 0xB0. The fields are also in a different order. SOTN's flag word is `unk90` at 0x90;
  BB2's is at 0x98. SOTN's `read_pos` is at +4; BB2's read pointer is at +0. The sozud psy-q-decomp
  3.5 `snd_defs.h` layout (tmp/psyq-decomp-ref/src/snd/snd_defs.h) is identical to SOTN's. BB2's
  LIBSND is the interim 4.0-lineage build (memory/closer/libsnd-hunt-report.md). This is the same
  kind of skew as psyz's 52-byte vs BB2's 54-byte SpuVoice (include/sound.h).
- So include/sound.h gets SOTN's declaration plus a BB2-layout `struct SeqStruct` of 0xB0 bytes.
  I derived each field by lining up the same Sony function in SOTN and in BB2, statement by
  statement. The main alignments:
  - _SsSndStop (SOTN stop.c vs BB2): unk2b->0x14, unk80->0x88, unk27->0x1C, unk13->0x18,
    unk14->0x19, unk29->0x1E, unk15->0x1A, unk16->0x1B, unk2a->0x1F, channel->0x17, unk48->0x21,
    unk28->0x1D, unk10->0x15, unk11->0x16, delta_value=unk7c -> 0x90=0x84, unk8c=unk84 ->
    0x94=0x8C, unk70=unk72 -> 0x54=0x56, read_pos/loop_pos=next_sep_pos -> 0x0/0x8=0x4.
    In the loop, programs->0x37, panpot->0x27 and vol->0x60; unk78/7A->0x5C/5E.
  - _SsSndCrescendo (cres.c): unk98->0xA0, unk42->0x4C, unk40->0x4A, unk3E->0x48, unk94->0x9C.
  - _SsSndTempo (tempo.c): unkA0->0xA8, unk44->0x4E, unk8c->0x94, unkA4->0xAC, unk4a->0x50.
  - _SsSeqPlay (seqread.c): delta_value->0x90, unk70->0x54, unk6E->0x52.
  - _SsSeqGetEof (seqread.c _SsGetMetaEvent case 0x2F): unk48->0x21, unk46->0x20, unk3C->0x22,
    unk0->0x23. BB2 alone has +0x0C, which replaces next_sep_pos under flag 0x400.
  - SpuVmGetSeqVol (vmanager.c): unk74/unk76 -> 0x58/0x5A.
  - func_80084CC0: +0x10 is BB2-only; it is compared with read_pos under flags 0x401.
  Nothing in BB2 touches 0x24-0x26, 0x80 or 0xA4, so those are unkNN pads. 0x47 is natural
  alignment padding. Types come from BB2's load and store widths (lbu, lh/lhu, lw, and
  slt vs sltu).

## 2. Whole-TU measurement

tu.py (this dir) builds a modified main.c copy with ALL rules disabled and compares every function's
exact bytes to build/src/main.o. The baseline (unmodified main.c) gives 116/116 identical.

| step | result |
|---|---|
| retype + SOTN-shape bodies (all 18) | 115/116 byte-identical; func_80084CC0 score 9 (238/233), _SsSndReplay 1, _SsSndTempo 3 |
| _SsSndTempo: keep the landed `new_val` / `goto tempo_store` shape (only the accesses retyped) | 0 |
| _SsSndReplay: `score` pointer + `score->unk14 = 1` (SOTN replay.c indexes twice: 1, operand order) | 0 |
| func_80084CC0 case 0xE0 `state->read_pos++` (scalar handlers) | 9 |
| func_80084CC0 case 0xE0 `cmd_ptr = state->read_pos; state->read_pos = cmd_ptr + 1` | 25 |
| func_80084CC0 + D_800F334x handler table as ONE `_SsFCALL` object | 0 (233/233) |
| + `if (state->read_pos == state->unk10 + 1)` (was `ptr + 1 == ... + 1`) | 0 |
| `if (ptr == state->unk10)` | 9 (232/233) |

Final: 116/116 score 0. func_80084CC0 differs only in how its handler relocations are written
(`D_800F3340+0x10` vs `D_800F3350`), and those link to the same bytes; the full-build SHA1 proves it.

## 3. Why func_80084CC0 needs the `_SsFCALL` aggregate (sched1 dumps, this dir)

The dumps are sched1 (`-dS`) excerpts of case 0xE0 (`unk16 = 0xE0; read_pos++; pitchbend(a0, a1)`):
- sched1_base_0xE0.txt: the landed cast body. `*(u8 **)state` is a NON-struct `mem:SI`, so the
  load of scalar D_800F3348 (insn 300) depends on BOTH stores (286 QI, 293 SI). It therefore stays
  after the read_pos store, the tail matches the second switch's 0xE0 case, and jump2 cross-jumps it
  (target `j 13e8`).
- sched1_scalar_handlers_0xE0.txt: `state->read_pos` is `mem/s:SI`. GCC 2.7.2 true_dependence
  drops the conflict between an in-struct store with a varying address and a fixed scalar load
  (the MEM_IN_STRUCT_P rule). So the D_800F3348 load (insn 298) depends only on the QI store 284.
  It hoists above the read_pos increment, the tails differ, and the cross-jump is lost (score 9).
- sched1_final_0xE0.txt: the handler is `D_800F3340.pitchbend`, an in-struct load (`mem/s:SI`,
  const sym+8). That load (insn 307) again depends on 289 and 298, so the target shape returns.

So the target's schedule needs either a non-struct read_pos or an in-struct handler load. A
non-struct read_pos is exactly the cast/pun this cleanup removes. An in-struct handler is Sony's
own declaration: `_SsFCALL` in PsyQ 4.0 LIBSND.H (noteon, programchange, pitchbend, metaevent,
control[13], ccentry[20] = 37 words = 148 bytes). The 4.0 SSINIT object defines a 148-byte common
`SsFCALL`, and the 4.0 MIDIREAD _SsGetSeqData reaches it through SsFCALL+0/+4/+8/+0xC/+0x10
(docs/naming/sweep-2026-09-29/held.csv rows 0x80084CC0 and 0x800F3340). The typedef is pasted
verbatim, K&R `()` pointers included. The unprototyped calls produce the same argument code
(233/233). The 0xB0 handler is `control[0]` (LIBSND.H `#define CC_NUMBER 0`;
`SsFCALL.control[CC_NUMBER] = _SsSetControlChange`). The object keeps its splat name D_800F3340
because the sweep holds the name `SsFCALL` at MEDIUM.

## 4. Other constructs retired along the way

- _SsSndPause / _SsSeqGetEof / _SsSndNextSep / _SsSndReplay / _SsSndStop / _SsSndTempo: the
  `shifted = a0 << 16; addr = (s32 *)&_ss_score; base_ptr = addr + (shifted >> 14)` preamble
  (a pointer alias to the global plus a hand-folded s16 index) is gone. `&_ss_score[a0][a1]` with
  s16 parameters gives the same sll 16 / sra 14.
- _SsSndReplay / _SsSndNextSep: parameter 0 becomes s16 as in SOTN (was s32 plus the manual
  shift). The SsSeqCalledTbyT call drops its `(s16)i` cast.
- _SsReadDeltaValue: parameter 0 becomes s16 as in SOTN seqread.c (`s32 _SsReadDeltaValue(s16,
  s16)`). The `(s32)(arg0 << 16) >> 14` fold is gone. Also gone: the dead `result = val << 2;`
  before the VLQ loop (overwritten unread) and the hand-split `result = val << 2; result =
  (result + val) << 1;`. They become SOTN's `val * 10`. Whole-TU compare: 0 differ (w2 variant,
  measured on the spliced src).
- Measured and kept: func_80084CC0's block-local `u8 *cp` in the second switch's 0x90 case (the
  landed body's documented choice). Spelling it through the shared `cmd_ptr` scores 17 (w3 variant).
  `cp` holds the read pointer and is read twice (`cp + 1`, `cp[0]`).
- _SsVmGetSeqVol: the FAKE pointer alias `ptr = &_svm_cur.seq_sep_no` (annotated "all pointer-free
  spellings measured 12") is gone. SOTN's SpuVmGetSeqVol shape (vmanager.c) is byte-identical once
  the table is typed.
- func_80087D10 / func_80087D58: SOTN's SpuVmGetSeqLVol/RVol shape.
- _SsSeqGetEof: the `val`/`threshold` locals are gone (SOTN `score->unk48++` shape).
- _SsSndStop: the `ip = p + i` pointer loop becomes SOTN's `for` over programs/panpot/vol.
- SsSeqCalledTbyT: the SS_SCORE_FLAG cast macro becomes SOTN's `_ss_score[i][j].unk98`.
