# Verifier report — near_manifest.csv (34 rows), default-refute, 2026-09-07

Independent re-derivation: `tmp/near_audit/verify.py` (placement scans, XREF/jal decode, desync
checks, collision grep, alternative-module sweep) and `verify2.py` (per-XDEF sub-function scans,
UT_KEYV callee sequence per build, delay-slot tails, entry-point check, EXE caller census).
Raw outputs: `verify_out.md`, `verify2_out.md`. Nothing in the tree was modified. The audit.py
leads were not trusted; every number below was recomputed from the .LIB modules, the EXE, and
build/bb2.map.

## Verdict table

| addr | proposed | class | verdict | reason (what I computed) |
|---|---|---|---|---|
| 0x80089A48 | _SpuSetAnyVoice | libscan-xref | CONFIRM | LIBSPU/S_SNV@0x80089A24 and S_SRV@0x8008A904 both 0/9 mismatch, unique, manifest CONTRADICTED/FILL; each has REL26 XREF `_SpuSetAnyVoice` at +0xC; EXE words 0x80089A30 and 0x8008A910 = `0C022692` = jal 0x80089A48 = func_80089A48 start. Body reads `_spu_env&1` then picks `_spu_RQ`/`_spu_RXX` and or-builds a 32-bit voice mask from a2/a3 (mode 0/1/8), consistent with the 4.1 S_SAV body (145/176 from +0xC). An underscore libspu internal that the game could not define. |
| 0x800841E0 | _SsSndCrescendo | libscan-xref | CONFIRM | LIBSND/SSCALL 0/157 at 0x80083F6C (matches.json single verbatim placement; second-best 135), XREF `_SsSndCrescendo` at +0xF8; EXE 0x80084064 = `0C021078` = jal 0x800841E0 = function start. Body clears seq flag with `addiu a0,-0x11` (~0x10) = psyz cres.c, while func_80084500 uses ~0x20 = decres.c, so the pair is not swappable. |
| 0x80084500 | _SsSndDecrescendo | libscan-xref | CONFIRM | Same SSCALL module, XREF `_SsSndDecrescendo` at +0x120; EXE 0x8008408C = `0C021140` = jal 0x80084500 = function start; body uses ~0x20 flag (`addiu a0,-0x21`) = decres.c. |
| 0x800863CC | _SsVmDamperOff | libscan-xref | CONFIRM | LIBSND/SSSTOP (4.0U) 0/118 at 0x80085270, unique (second 102); XREF `_SsVmDamperOff` at +0xC8; EXE 0x80085338 = `0C0218F3` = jal 0x800863CC = spu_ResetCounter start. C body is `D_800F66F8 = 0`, and D_800F66F8 is `_svm_damper` in the UT_KEYV/VM_ALOC2 HI/LO reloc web, so the body IS "damper off". |
| 0x800863DC | _SsVmFlush | libscan-xref | CONFIRM | SSCALL XREF `_SsVmFlush` at +0x44; EXE 0x80083FB0 = `0C0218F7` = jal 0x800863DC = func_800863DC start. Body: `_svm_envx_ptr=(p+1)&0xF; _svm_envx_hist[p]=0; loop i<_SsVmMaxVoice calling SpuGetVoiceEnvelope` = psyz vm_f.c `_SsVmFlush`. Also called from func_80086818 (_SsVmInit), as in Sony's tree. |
| 0x80086818 | _SsVmInit | libscan-xref | CONFIRM | LIBSND/SSINIT 0/58 at 0x80083A48 (manifest FILL, second 49); XREF `_SsVmInit` at +0x80; EXE 0x80083AC8 = `0C021A06` = jal 0x80086818 = func_80086818 start. Body: `_spu_setInTransfer(0); _svm_damper=0; SpuInitMalloc(0x20, rec+8); clear 0xC0 halfwords of _svm_sreg_buf; clear 24 dirty bytes` = psyz vm_init.c line for line. |
| 0x80087DA0 | _SsVmSeqKeyOff | libscan-xref | CONFIRM | PAUSE 0/39 at 0x800848AC (XREF at +0x58, EXE 0x80084904) and SSSTOP (XREF at +0xC0, EXE 0x80085330) both = `0C021F68` = jal 0x80087DA0 = spu_NotifyChannel start. Extra: the 4.0 VM_SEQ sub-function `_SsVmSeqKeyOff` places 4/38 at exactly 0x80087DA0. C body loops voices, matches `_svm_voice[i]` seq id, sets `_svm_cur.voice` and calls func_800871D4(0) (= _SsVmKeyOffNow). |
| 0x80085A40 | SsUtKeyOnV | libscan-near | CONFIRM | UT_KEYV 4.0J/4.0U 4/259 at 0x80085A40, second-best 235 at 0x8002F8EC; XDEF SsUtKeyOnV+0 = function start. The 4 diffs (0x80085CA4..CB4) recompute a voice index: lib `(3v)<<2+v)<<2` = 52·v, EXE `((8v-v)<<2 - v)<<1` = 54·v. **Attribution correction:** this is the `_svm_voice` element-stride change between 4.0 and 4.1 (the identical 4-word edit appears in VM_NOWOF 4.0-vs-EXE at 0x8008720C, and VM_NOWOF 4.1 already carries the EXE form), not the "Ut volume inconsistency" fix of libchg_e_41.txt line 447. Identity is unaffected and if anything stronger. All 6 REL26 XREFs resolve: _SsVmVSetUp 0x80087E3C, note2pitch2 0x80086BFC (both VERIFIED), plus the four rows below. |
| 0x800871D4 | _SsVmKeyOffNow | libscan-near | CONFIRM | VM_NOWOF 4.1/4.2 2/52 at 0x800871D4 (second 34); the 2 diffs are `jr ra`/`sh v0,0(at)` order (lib fills the delay slot with the store, EXE stores then `jr ra; nop`), same semantics. 4.0 build differs only by the 4-word stride edit above. XDEF +0 = function start. Independently pinned by UT_KEYV XREF at +0x3E8 (EXE jal 0x800871D4) and body = psyz vm_nowof.c (okof1/okof2 or-in, okon1/okon2 mask). |
| 0x800861BC | _SsVmDoAllocate | libscan-near | CONFIRM | Caller-pinned: UT_KEYV REL26 at +0x310 names `_SsVmDoAllocate` in ALL six builds (the callee name sequence is identical 4.0J..4.4), and the EXE jal there targets 0x800861BC = function start; the game has exactly one jal to it (from func_80085A40). Body: 4.0 VM_ALOC2 33/132 at -4 (register renumbering, lui-at addressing, the same stride edit); data refs _svm_voice/_svm_cur/_svm_sreg_buf/_svm_damper resolve to the web addresses. No other Sony module fits (best alternative SSNEXT 21/23). A 255/259 caller whose relocation slot is unchanged is sufficient here: the 4 edited words are an index computation 0x60 bytes away from any jal. |
| 0x80086CF8 | vmNoiseOn | libscan-near | CONFIRM | UT_KEYV +0x32C XREF `vmNoiseOn` (all 6 builds) -> EXE jal 0x80086CF8 = function start; sole caller in the image is func_80085A40. Body calls SpuSetNoiseClock + SpuSetNoiseVoice and touches _svm_cur (18 refs), _svm_voice, _svm_sreg_buf, _svm_sreg_dirty — a noise key-on, not an allocator. No body fit in any build (BB2's build of VM_NOISE is not among the six). Retire AllocBukiRmd_helper_80086CF8. |
| 0x800872A4 | _SsVmKeyOnNow | libscan-near | CONFIRM | UT_KEYV +0x348 XREF `_SsVmKeyOnNow` (all 6 builds) -> EXE jal 0x800872A4 = function start (sole caller func_80085A40; VM_NOWOF ends at 0x800872A0 with `nop`, so it is the next linked module = VM_NOWON, matching Sony link order). Body: leaf, 26 _svm_cur refs, writes _svm_sreg_buf/_svm_sreg_dirty. Retire AllocBukiRmd_helper_800872A4. |
| 0x80085FD8 | SsUtGetDetVVol | libscan-near | CONFIRM | Stronger than claimed: the SsUtGetDetVVol sub-function of UT_VVOL in 4.1, 4.2, 4.3, 4.4 places 0/14 at 0x80085FD8 (second-best 8); 4.3 UT_VVOL3 whole module 1/16 with the only diff the trailing pad vs func_80086014's prologue. XREF SpuGetVoiceVolume -> jal 0x8008BD88 = SpuGetVoiceVolume. C body passes voice (a1/a2 pointers fall through) to SpuGetVoiceVolume, else -1 = LIBSND.H `SsUtGetDetVVol(short, short*, short*)`. |
| 0x80086080 | SsUtGetVVol | libscan-near | CONFIRM | 4.3 UT_VVOL3 2/44 at 0x80086080 and 4.1 UT_VVOL:SsUtGetVVol sub-function 3/43 (second 37); diffs are the epilogue `jr ra`/`addiu sp,40` delay-slot order only. XDEF +0 = function start; XREF SpuGetVoiceVolume OK. C body divides both raw volumes by 129 = psyz ut_vvol.c SsUtGetVVol. |
| 0x80086014 | SsUtSetDetVVol | libscan-near | CONFIRM | Refuted the "weakest row" framing in the row's favour: the 4.0J/4.0U UT_VVOL `SsUtSetDetVVol` sub-function (26 words, XDEF +0x58) places **0/26 at exactly 0x80086014** (second-best 17 at 0x80086150), with the stack frame BB2 has; its HI/LO relocs resolve to _svm_sreg_buf 0x80102A78/+2 and _svm_sreg_dirty 0x800F65E0. The 4.3 7/28-at-+4 figure was just the frame-less later build. No other Sony function fits (next best SsUtGetDetVVol 4.3 11/16). C body = psyz SsUtSetDetVVol (right=+1, left=+0, dirty|=3). |
| 0x80087D10 | _SsVmGetSeqLVol | libscan-near | CONFIRM | 4.0 VM_SEQ sub-function `_SsVmGetSeqLVol` (17w) 0/17 at 0x80087D10; `_SsVmGetSeqRVol` 0/17 at 0x80087D58; the L/R pair differ in exactly one word and the EXE has `lh v0,88(v1)` (0x58 = SeqStruct.vol_l per psyz) at 0x80087D4C. 4.3 VM_SEQ_2 30/36 diffs are all lui/addu/lw vs lui-at/lw-off for _ss_score 0x80106F28. |
| 0x80087D58 | _SsVmGetSeqRVol | libscan-near | CONFIRM | Same placements; EXE `lh v0,90(v1)` (0x5A = vol_r) at 0x80087D94; XDEF +0x48 = function start; C body reads +0x5A. |
| 0x8007A5C4 | LoadClut | libscan-desync | CONFIRM | map object symbol gpu_LoadClut256; C def src/gpu.c:266; asm/funcs/LoadClut.s glabel LoadClut @8007A5C4; census VERIFIED (LIBGPU/EXT +0xEC); alias LoadClut=0x8007A5C4 only. |
| 0x8007A62C | LoadClut2 | libscan-desync | CONFIRM | map gpu_LoadClut16; C def gpu.c:275; LoadClut2.s glabel @8007A62C; census VERIFIED; no other use. |
| 0x8007A87C | NextPrim | libscan-desync | CONFIRM | map ot_GetTag; C def gpu.c:340; NextPrim.s @8007A87C; VERIFIED (LIBGPU/PRIM +0xF4). |
| 0x8007A898 | IsEndPrim | libscan-desync | CONFIRM | map ot_IsEnd; C def gpu.c:344; IsEndPrim.s @8007A898; VERIFIED. |
| 0x8007EF1C | SetColorMatrix | libscan-desync | CONFIRM (note) | map gte_SetColorMatrix; the "C definition" is a whole-body `__asm__("glabel gte_SetColorMatrix …")` string at src/display.c:2284 with externs in text1b.c:1162 and text1b_b.c:134 — a rename must edit the asm string and any inline_asm_canonical.txt key, not a C signature. SetColorMatrix.s @8007EF1C; VERIFIED (LIBGTE/MTX_11). |
| 0x80083688 | GetVideoMode | libscan-desync | CONFIRM | map sys_GetVideoMode; C def src/ings2.c:573; GetVideoMode.s @80083688; VERIFIED; web.md shows GetVideoMode callers agree. |
| 0x800848AC | _SsSndPause | libscan-desync | CONFIRM | map spu_SetMotionState; C def main.c:316 (proto :237 carries `/* _SsSndPause */`); _SsSndPause.s @800848AC; PAUSE 0/39 verbatim here (my scan); no reverse alias exists in named_syms (the only non-comment use of the name is the .s glabel). |
| 0x80084948 | _SsSndPlay | libscan-desync | CONFIRM | map spu_SetMotionCallback; C def main.c:329; _SsSndPlay.s @80084948; VERIFIED RESOLVED_LAYOUT. |
| 0x80085064 | _SsReadDeltaValue | libscan-desync | CONFIRM | map spu_ReadMotionFrame; C def main.c:576; _SsReadDeltaValue.s @80085064; VERIFIED (MIDITIME). |
| 0x80085114 | _SsSndNextSep | libscan-desync | CONFIRM | map spu_ResetMotionEntry; C def main.c:608; _SsSndNextSep.s @80085114; VERIFIED (NEXT). |
| 0x80085210 | _SsSndReplay | libscan-desync | CONFIRM | map spu_SetMotionActive; C def main.c:627; _SsSndReplay.s @80085210; VERIFIED (REPLAY); no reverse alias in named_syms. |
| 0x800885CC | _SpuInit | libscan-desync | CONFIRM | map spu_InitEx; C def main.c:1547; _SpuInit.s @800885CC; VERIFIED (S_INI +0). |
| 0x800886C4 | SpuStart | libscan-desync | CONFIRM | map spu_InitIrq; C def main.c:1588; SpuStart.s @800886C4; VERIFIED (S_INI +0xF8). |
| 0x80088F9C | _spu_Fw | libscan-desync | CONFIRM | map spu_TransferData; C def main.c:1848; _spu_Fw.s @80088F9C; VERIFIED (SPU +0x85C). |
| 0x80089024 | _spu_Fr | libscan-desync | CONFIRM | map spu_TransferDirect; C def main.c:1858; _spu_Fr.s @80089024; VERIFIED (SPU +0x8E4). |
| 0x800892D4 | _SpuDataCallback | libscan-desync | CONFIRM | map spu_SetCallback; C def main.c:1919; _SpuDataCallback.s @800892D4; VERIFIED (S_DCB). |
| 0x800899A8 | SpuFree | libscan-desync | CONFIRM | map spu_DmaTransfer; C def main.c:2074 (comment :2072 already says SpuFree); SpuFree.s @800899A8; VERIFIED (S_M_F). |

Collision grep (whole word, src/ include/ *.txt engine/queue.json): no proposed name is used for a
different address anywhere. Every hit is either the same-address reverse alias, the asm glabel, or a
comment/prototype annotation already naming the Sony symbol (main.c:233-238, :1077, :1165).
`_SpuSetAnyVoice` appears only inside a comment in volatile_extern_allowlist.txt:50.

## Other things noticed

- **Evidence-text correction for the SsUtKeyOnV row** (and by extension the ruling note): the
  4-word edit is a `_svm_voice` stride change 52 -> 54 bytes, present identically in VM_NOWOF
  (4.0 vs EXE, 0x8008720C) and already in the 4.1 VM_NOWOF. It is a 4.0->4.1 struct change, not
  the changelog's "Ut functions volume" fix. Suggest rewording so the manifest does not cite the
  changelog for it.
- **Extra caller-free identification not in the manifest:** the 4.0 VM_SEQ `_SsVmGetSeqVol`
  sub-function (24 words) places **0/24 at 0x80087CAC** = func_80087CAC (alias
  dispstuff_helper_80087CAC), unique (second-best 16). Same evidence grade as the L/R rows; its
  callers per psyz are _SsSndCrescendo/_SsSndDecrescendo, and the alias comment already records
  those two as its callers. Candidate for a row.
- **SSCALL is inside our `SsStart`** (0x80083E9C): the SsSeqCalledTbyT XDEF at 0x80083F6C is
  mid-function in our map, as boundary_fixes.md says. The three SSCALL-pinned rows do not depend
  on that boundary.
- **SsUtKeyOffV second entry** confirmed: 4.0 UT_KEYV `SsUtKeyOffV` sub-function 0/29 at
  0x80085DD4, preceded by `jr ra; nop`. Boundary follow-up, not a rename (agrees with the ruling).
- **web.py accepted-set note:** SSSTOP's manifest rows are classified RELOC_CHAIN_ID /
  RESOLVED_LAYOUT (not in web.py's ACCEPT set) and were hard-coded in; I re-verified the 4.0U
  placement is 0/118, unique. Fine, but the script's own filter would have excluded it.
- **Data names for the later wave (from bodies I read, not the reloc web):** `MarioCam_str` =
  0x800F19D0 is the `_ss_spu_vm_rec + 8` argument of SpuInitMalloc inside _SsVmInit (so
  `_ss_spu_vm_rec` = 0x800F19C8); `g_memcard_slot` = 0x80101BCC is `_SsVmMaxVoice` (already in
  web_data.md, and it is the loop bound in _SsVmFlush and _SsVmSeqKeyOff). Both are misleading
  game-flavoured names on sound-driver state.
- The two desync rows without a reverse alias (0x800848AC, 0x80085210) will need the alias
  added or the C symbol renamed; today the Sony name exists only as the .s glabel.
