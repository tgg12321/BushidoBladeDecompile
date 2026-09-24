# lib_followups: examined and dropped (2026-09-24)

Vein: the open follow-ups from the Sony-library naming waves (libscan-verbatim, libscan-xref, libscan-near, and the data wave).
Scripts are in this directory (`svm_cur_refs.py`, `near_data_refs.py`, `manifest_residue.py`,
`span_residue.py`, `map_vs_libsyms.py`, `in_object_names.py`, `main_probe.sh`, `write_candidates.py`).
**Environment note:** the PsyQ `.LIB` sets (`tmp/libscan/psyq40`, `tmp/libsnd_hunt/*`) are gone from
this checkout. No reloc could be re-read from the raw OBJs. Every reloc-level claim below cites the
committed audit outputs (`near_audit_report.md`, `data_verifier_report_2026-09-07.md`, `libsyms.json`,
`matches.json`).

## Dropped rows (one line each)

| addr | name today | considered | why dropped |
|---|---|---|---|
| 0x800790A4 | `stub_800790A4` / func_800790A4 | RESET | Not a function. It is a 4-word patch payload (`and v0,v0,s5` + 3 nops) that `_send_pad` copies into the BIOS pad handler (src/text1b_b.c:1774-1816). LIBAPI/SENDPAD has no OBJ symbol at that address (IN_SPAN_NO_SYMBOL). "stub" is vague but not false. The real fix is a splat data/code reclassification, not a rename. |
| 0x8008339C | `sys_MemClear` | Sony static `memclr` | Sony uses the same static name `memclr` in 3 modules (INTR 0x800831A4, INTR_VB, INTR_DMA). Three global glabels can't share one name, and any suffix we added would be our own invention. The current name is accurate. |
| 0x80083644 | `sys_MemClear2` | Sony static `memclr` | Same as above. |
| 0x800831A4 | `func_800831A4` (C, src/ings2.c:346) | Sony static `memclr` | Same collision. The auto name makes no claim. |
| 0x80081F1C | `cdrom_IrqHandler` (C, src/system.c:1194) | Sony static `callback` (LIBCD/BIOS) | `callback` is already used as an identifier in src/main.c, src/system.c, include/gpu.h and the registries. The current name is accurate: it's the CD IRQ callback. |
| 0x8008C978..0x8008D048 | inside `_comb_control` (INCLUDE_ASM) | `EvalpSio`, `r_sioinit/open/close/remove/strategy`, `__nulldev` (LIBCOMB statics) | These are mid-function addresses, not function starts, so there's nothing to rename. Splat boundary work (`boundary_fixes.md` class). `HandleSio` already lives in C. |
| 0x8008325C / 0x80079000 | inside `setjmp` / `FlushCache` | `longjmp`, `_SendPAD` | Owner-gated boundary splits (ADDENDUM-APPLY-PLAN.md Part B). Both hosts hold canonical-asm authorizations that would have to be re-derived. Unchanged. |
| 0x80086B38 / 0x80088BA0 / 0x80088C60 / 0x8008BA94 | `note2pitch`, `_spu_FiDMA`, `_spu_Fr_`, `_spu_2pitch` | none | The names already equal Sony XDEFs at exact offsets (libsyms.json). The census still tiers them INFERRED/unattributed because build_census.py doesn't credit the 2026-08-10 split XDEFs. That's a census attribution fix, not a rename. |
| 0x8005BE84 | func_8005BE84 | `snd_SetReverbType` (apiscan PLAUSIBLE) | Still dropped. The argument indexes a 4-byte-record table at D_8009AD1C, and the table's first s16 is what reaches SsUtSetReverbType. The body also calls unverified func_800858D0(0), ClearReverbWorkArea, SetReverbDepth(2*idx+1) and ReverbOn. Any short name would misstate the argument or summarise control flow. The apiscan verifier's objection stands. (Its manifest summary also says "d=(2*a0+1)" where the asm is `sll s0,1; addiu a0,s0,1`, which agrees.) |
| 0x80086CF8 | `vmNoiseOn` | `_SsVmKeyOn` (hunt PROBABLE) | Refuted at this address. The UT_KEYV XREF pins 0x80086CF8 as `vmNoiseOn` (near-tier wave). No other candidate address for `_SsVmKeyOn` exists, and it may not be linked at all. |
| 0x800A26E4 | `g_alarm_active_sentinel_plus_6` | Sony static `pitch_table` | Only the RESET is proposed (HIGH row). The name `pitch_table` comes from psyz source, not from an OBJ local-symbol record we can re-read (LIB set absent). |
| 0x8008E5A8 | `g_char_class_table` (named_syms.txt:1350) | none (game side) | Out of scope for this vein (a game table). Registry bug worth fixing, though: the same name is defined again at named_syms.txt:1855 = 0x8009BD8D, and the linker keeps the LAST definition (bb2.map 6339 and 6592 both show 0x8009bd8d). Line 1350's claim is therefore dead text. |
| 0x800F1B18 | `g_per_char_motion_buffer_table_1392` | none | Not examined beyond adjacency. Its 4-byte "prefix" 0x800F1B14 is `_svm_orev1` (HIGH row), so the "prefix" wording in its comment is wrong. The table itself needs a separate look. |
| 0x80083804 | `motion_Close` | `__do_global_dtors` | Kept as a MEDIUM record only; the HIGH row is RESET. The identity rests on Croc SLED-00038's shipped symbol table, which has the same crt0 layout (+0xA8/+0x70/+0x68). That's a cross-title evidence class the owner hasn't admitted, and no byte comparison is possible. |
| 0x800858D0, 0x8008B488, 0x80086130, 0x80084A7C, 0x80087770 | auto | SsUtAllKeyOff, SpuSetVoiceAttr, SsUtSetVVol, _SsSeqGetEof, _SsVmSetSeqVol | Kept as MEDIUM only. New body-level evidence is recorded in candidates.csv, but the owner's near-tier ruling requires a caller pin or a closer build for PROBABLE rows, and neither exists. |
| _svm_voice / _svm_sreg_buf D_-only offsets | `D_800F4E20` etc. | `_svm_voice_plus_0xN` | Kept as MEDIUM. Correct but pure churn (no false claim to retire). |
| 0x80102BF8 / 0x801027E8 / 0x80103604 | D_ only | `_autovol` / `_autopan` / `_svm_envx_ptr` | Kept as MEDIUM. Single-body evidence (the order of calls/statements in _SsVmFlush vs Sony vm_f.c), and these are D_-only fills. |
| save_vc_ctrl (0x800414FC) | Kengo name | none | Not library code. The override excludes it for bb2.ld/Makefile wiring reasons, and library evidence has nothing to add. |
| 0x80016888 | `gpu_InitDisplay_80016888` | none | Out of scope. Its comment calls ResetGraph(1) "gpu_SetMode(1)", an old misnomer; the body needs a separate API restatement look. |

## Status of the brief's items 1-7

1. **`_svm_cur` NEAR-only offsets: CLOSED (proposed).** 16 rows (+3/4/5/A/B/D/E/F/12/13/14/16/18/1A/1C/1E), all HIGH.
   The offsets come from the UT_KEYV/VM_NOWOF/VM_ALOC2 reloc lines in near_audit_report.md, and the extent from the 0x20-byte Sony object.
   This retires g_char_p3_byte_*, g_pair_state_b and g_weapon_frame_*. Follow-on finds come from the same NEAR relocs plus Sony body statements:
   `_svm_voice` (0x800F4E18, record 0 fields), `_svm_okon1/2`, `_svm_okof1/2`, `_svm_orev1/2`, `_svm_damper`,
   `_svm_sreg_dirty`, `_svm_sreg_buf`, `_svm_envx_hist`, and `_snd_seq_tick_env` +0x8/+0x14. Together they retire the
   "satan1 slot state", "char p21 field", "alarm", "weapon frame", "per-char motion buffer prefix" and "text1b gte rgb prefix" claims.
2. **`_ctype_` / `_spu_rev_param`: CLOSED (proposed).** `_ctype_`'s base byte 0x8009BD8C is the tail of dlabel D_8009BD88.
   The referenced `_ctype_+1` is D_8009BD8D, which also carries alias g_char_class_table (duplicated in named_syms.txt, see above).
   The proposal is `_ctype__plus_0x1`; a dlabel split would allow `_ctype_` proper. `_spu_rev_param` = dlabel D_800A2D94 (src/main.c:2851), proposed as a fill.
3. **PROBABLE rows: still OPEN.** No caller pin is possible:
   - SsUtAllKeyOff, SpuSetVoiceAttr and SsUtSetVVol are called only by game code or by non-accepted modules (VM_INIT, VM_F).
   - _SsSeqGetEof's only caller is in non-verbatim MIDIREAD.
   - The LIB sets needed to re-scan are no longer on disk.
   - _SsVmKeyOn's hunt address is refuted (it is vmNoiseOn).

   New body-level evidence is recorded as MEDIUM rows. The false alias `satan_voice_init` on 0x80084A7C is proposed for RESET (HIGH).
4. **`snd_SetReverbType` @0x8005BE84: stays unapplied** (reason in the table above).
5. **motion_Close: RESET (HIGH)**. It's still census action RESET; the 2651e2e5 wave missed it because the .s was split later.
   - `__do_global_dtors`: MEDIUM record only.
   - Bonus: `func_80083794` -> `__main` (HIGH). cc1psx injects `jal __main` as main()'s first call, and BB2's main's first call is func_80083794.
   - **gpu_EnableDisplay / gpu_DisableDisplay DO need action. The override assumed they were the corrections, and they are not.**
     - 0x80016868 is `ResetGraph(1)`, which has nothing to do with enabling the display.
     - 0x800168D0 is `SetDispMask(1)`, which turns the display ON. Its current name is inverted.
     - Proposed names are `gpu_ResetGraphMode1` and `gpu_SetDispMaskOn` (api-restatement, HIGH).
     - The stale "MISNAMED: actually gpu_*" comments at named_syms.txt:206-207 are why the census still says SUSPECT.
   - save_vc_ctrl: nothing from this vein.
6. **AUTO/unnamed Sony code in accepted spans: 8 HIGH rows.** These are C functions still carrying D_-style names at verified module-static addresses:
   trapIntrVSync, setIntrVSync, trapIntrDMA, setIntrDMA, _SsTrapIntrVSync, _SsSeqCalledTbyT_1per2, cb_read, cb_data.
   None has a census row, so naming_wave.py cannot apply them (same blocker as startIntr in naming-verification-2026-08-18.md).
   Also RESET dispstuff_helper_80087770. Its "dispstuff" tag was inherited from callers that are now _SsSndCrescendo/_SsSndDecrescendo.
   Every remaining manifest residue row (memclr x3, callback, stub, LIBCOMB statics) is dropped above.
7. **Other pending items:**
   - `longjmp`/`_SendPAD` splits: owner-gated, unchanged.
   - The census tier lag for the 4 split XDEFs (a build_census.py fix).
   - src/main.c:2487 still has the self-referential `#define _spu_memList ((SpuMemRec *)_spu_memList)` that the data-wave verifier warned about. It's legal C but misleading, and should become a cast at the use sites.
   - Stale "alarm"/"weapon frame" comments remain on names the data wave already corrected (named_syms.txt:598, 638, 652, 653, 1926-1928, 1932-1933).
   - The duplicate g_char_class_table definition.
   - apiscan's deliberately-left-out rows (comb sub-command wrappers, SsVabClose one-liners, VAB routines) remain open by design.
