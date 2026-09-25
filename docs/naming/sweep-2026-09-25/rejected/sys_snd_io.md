# sys_snd_io vein - ideas examined and dropped (2026-09-25)

Scope: all 116 INFERRED targets in `targets.csv`. Mechanical pass: `dossier.py` (annotated asm, callee
tiers, callers, syms), `reach.py` (callee BFS with tiers), `xref.py` (data-symbol users), `ndpeek.py`
(NDATA.INF/NDATA.DAT read straight from `Bushido Blade 2 (USA).bin`).

## Dropped RENAME / UPGRADE ideas (verdict fell back to RESET or KEEP)

| addr | idea | why dropped |
|---|---|---|
| 0x80037AA4 | RENAME memcard free-block count | body sums DIRENTRY.size over the CONFIRMed memcard list and returns 15 - sum/8192. No call, reads globals, so it is neither api-restatement nor computation-restatement. Filed as RESET |
| 0x80037B00 | RENAME memcard file-exists test | same class problem (pure data walk, no call). RESET |
| 0x80038170 | RENAME memcard save-header builder | the layout match is to Sony's `_CARD` sample header (psyz samples/etc_card/cardio.h). A struct-layout match is not an admitted class; only strcpy is VERIFIED. RESET |
| 0x80036F28 | RENAME cdrom_GetFileSize | a bare read of the CONFIRMed g_cd_file_table size column. No admitted function class covers a table read. RESET |
| 0x8005BF3C | RENAME snd_ReverbOff | three VERIFIED SsUt* reverb calls, but the body also calls func_800858D0 (owner-held SsUtAllKeyOff PROBABLE). A restatement would leave out or guess that call. RESET |
| 0x8005BA6C | RENAME to a VAB-id-9 fake-open name | apiscan README left out hard-coded-VAB-id one-liners (a name would assert which VAB). RESET |
| 0x800168F0 / 0x80017F90 / 0x80017F98 | UPGRADE sys_StubEmpty* | body is `jr ra; nop`, but the `sys_` prefix is unproven. KEEP (bare `stub` empties were UPGRADEd instead) |
| 0x80016C3C | RENAME from the "OVER FLOW\n" string / UPGRADE sys_Panic | "Panic" (print + endless break-trap loop) is accurate, and no name the string gives is better. KEEP |
| 0x80016CF8 | UPGRADE file_LoadSoundData | the behaviour fits (snd_Init / snd_LoadCommonVab / snd_VabFakeOpen), but `file_` and `SoundData` are not a restatement. KEEP |
| 0x8007352C | UPGRADE satan_sprite_batch_render | the SetSprt/AddPrim loop fits "sprite batch render", but the `satan_` prefix is unproven (40 callers across many subsystems). KEEP |
| 0x8003E2C8 / 0x800457DC / 0x800457FC | UPGRADE literal `load_D_*` names | these read globals, so they are not computation-restatement. The literal name can't be wrong, so there is nothing to gain. KEEP |
| 0x8004C388 | RENAME to a vertex-midpoint computation name | plausible (it averages x/y/z and u/v of two 8-byte vertices), but the ruling needs emulation of the EXE words, and this was not run. KEEP. Possible future computation-restatement row |
| 0x80016C80 | RESET instead of RENAME | kept RENAME eff_Init (in-binary-string "eff_init:%08x size:%08x"). RESET is noted as the fallback in the row |

## Dropped RESET ideas (not contradicted, only unsupported)

| addr | name | why dropped |
|---|---|---|
| 0x80046AA0 | snd_StopAll | most of the body is heap-slot frees, but the reach includes SsVabClose [VERIFIED] via player_Destroy -> func_80045A50 -> func_8005B644 and func_80046020 -> func_8005B6AC. Not contradicted |
| 0x80047EC8 | snd_GetMaxFade | `return 0xD00`. Nothing proves or disproves "fade". (0xD00 also caps the common-VAB size in file_LoadSoundData; coincidence not evidence) |
| 0x80049E1C | snd_helper | fills D_80099C4E..D_80099CC2 with -1 and D_800A324C=-1. The data roles are unknown (D_800A324C is compared in efc_rob_type_dispatch). No contradiction shown |
| 0x80062020 | se_helper | a record copier called from the VAB-reload routine. "se" could be right |
| 0x800167AC/BC/D4, 0x8001DB58 | file_GetFlag* | the role of D_80106A73 is unknown; the `file_` prefix is unsupported but not contradicted |
| 0x800450BC / 0x80045188 / 0x80045194 / 0x80020CDC | seq_* | seq_Start reads NDATA id a0+0x25 (DATA7 group per the heuristic filemap) and sets a flag. The MIDI-style interpreter (saTan0Main) makes "sequence" possible, and nothing contradicts it |
| 0x8003F568 / 0x8003F5A8 | stage_ClearLighting / stage_SetLightPosDir | only the invented `g_stage_light_*` data names are unsupported. The bodies (clear / store) fit their verbs. The effect-free consumer stage_ApplyLighting was RESET instead |
| 0x800218C8 etc. | mario_test_* | game-semantic words (inventory, guard power, charm bonus, dialog memory, fade trigger) are unproven, but nothing in the bodies contradicts them. Not contradicting them from game knowledge alone (BB2 "has no inventory") follows the brief |
| 0x80038988 | file_io_state_dispatch | dispatches on the memcard state machine's result (func_80038734 -> func_80038658). Consistent |
| 0x800790A4 | stub | re-examined: rejected/lib_followups.md already dropped a RESET (a 4-word _send_pad patch payload, not a function). No new evidence |
| 0x800467A8 | stage_GetVariant | returns D_8009947A = g_stage_variant, which the stage loader sets to 0/1 (src/text1a_c2.c:113/132). Consistent |

## Out-of-vein observations (for other veins / follow-ups; NOT proposed here)

The snd_* sound.c cluster is really the **Marionation heap** (arena [0x800A9D10, 0x800EED10), slot table
D_800EED10). Several INFERRED names outside this vein describe the same routines wrongly:

- `func_80044FA0` **marionation_GetFrameOffset** (INFERRED): reads NDATA file id a0 into buffer a1 (func_80044E74 ->
  cdrom_StartReadAt). It prints `"Marionation over flow. No.%d (-%dbyte)\n"` @0x8001528C and halts when the heap's
  free bytes (D_800A33A4) are less than the file length. It does not "get a frame offset". Strong RESET candidate
  (possible in-binary-string name).
- `func_800455AC` **efc_particle_queue_entry** (INFERRED): allocates a heap slot record (id, base=D_800A33A0). Not
  a particle queue. RESET candidate.
- `func_800453E0` **channel_helper** (INFERRED): frees heap slot a0 (compaction + record removal). RESET candidate.
- `func_80044010` / `func_80044100` **prim_buffer_open_slot / prim_buffer_relocate_slot** (INFERRED): register and
  relocate an offset table in D_80103608[a1] (the pointer table the GTE model renderers read). The "slot" wording
  roughly fits; "prim_buffer" does not.
- `func_80045808` const_return_0x45000 = the heap size. `func_80045814` get_ptr_D_800A9D10 = the heap base
  (literal names, fine).
- `func_80046914` (AUTO, src alias **snd_StopBgm**): frees heap slot 8. Recorded as a MEDIUM RESET row in
  candidates.csv.
- **Data**: `g_snd_volume`, `g_snd_config_tbl`, `g_snd_fade_curve(_table)`, `g_snd_fade_pos/amt` and
  `g_snd_wave_phase_table` have companion rows in candidates.csv. `src/sound.c` also holds the D_800EF59C
  wave-mesh code (func_80047A90/func_80047BE0). The file name is part of why these got `snd_` names.
- NDATA file-id -> name mapping (`docs/formats/ndata_filemap.csv`) is a pool-order heuristic. It was only used as
  context, never as evidence.
