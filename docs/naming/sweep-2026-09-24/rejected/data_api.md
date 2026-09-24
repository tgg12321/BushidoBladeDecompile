# data_api vein: examined and dropped

Method: `scan.py` walks every `jal <VERIFIED API>` in asm/funcs/*.s (1650 calls), resolves a0-a3 plus stack args
(including the delay slot) back to `%hi/%lo`, `%gp_rel`, `lw sym` or `sym+off`, and follows `$v0` to global stores.
Output is in `by_sym.txt` / `by_sym_f.txt` (232 symbols). `refs.py ADDR` lists every registry name at an address and
every asm reference. `cdtable.py` checks the 0x8008EC34 table against the ISO9660 directory of the BIN.
The rows below were looked at and NOT proposed, or were only kept as MEDIUM.

## Sony library-internal data (out of vein: this belongs to libscan / a LOCAL-symbol pass)
Only the cdread.c block and CdReadCallback's slot were proposed, because their legacy g_ names are contradicted.
- D_800A11C8, CD_status, CD_comstr, CD_intstr, D_800F19C0: printf args inside LIBCD CD_cw/CD_sync/CD_ready/getintr debug prints. Sony statics, and printf args pin nothing.
- D_800A12FC, D_800A13FC: CD_cw per-command tables. Already mapped in memory/closer/sony-naming-map.md; not API-pinned.
- D_800A1498, D_800A1504: libcd statics. D_800A1504 is the "saved result ptr" in the sony map. Not proposed.
- D_800A151C: VSync v_wait arg (libetc). D_800A1578+0x38 / D_800A15B4-4: setjmp/HookEntryInt jmp_buf in libetc intr.c. D_800A157C: ChangeClearPAD. D_800A2600/2604/2608/260C/263C: intr.c statics. Sony internals.
- D_8009BE74/BE76/BE84/BEE0: libgpu internal state (ResetGraph memset, GetDrawEnv/GetDispEnv/PutDispEnv memcpy). Sony static struct.
- D_8009BF48-BF88: libgpu _addque2/_exeque/_reset SetIntrMask save slots and get_alarm printf args. Sony statics.
- D_800A2CEC, D_800A2D1C, _spu_* rows, D_800F4E1E/4E2A/4E30 (_SsVmFlush/_SsVmVSetUp), D_800A269C (_SsInit), D_800A26D4, _snd_seq_tick_env_*: LIBSPU/LIBSND internals, already data-waved or in the sony map.
- D_800A3044, D_800A304C, D_800A307C, D_800F1AD8: libcomb (_comb_control/AddCOMB) internals. D_800F1838: RemovePatchPad/SetPatchPad INT_RP. D_800F1858/187C/189C, _que: libgpu _clr/_reset internals.
- D_80015F50/5C/68/74, D_800163E8/F8/640C, D_800164A8, D_80015A7C, D_80015C90: rodata strings used inside Sony library code (checkRECT names, printf formats, DelDrv arg).
- def_cbsync/def_cbready/def_cbread, D_80081F1C, D_80082050, D_80082320, D_800832F8, D_80083418, func_80036064, _exeque, func_8003A42C: FUNCTION addresses passed as callbacks. They belong to the func vein. D_80082050=cb_read and D_80082320=cb_data are already in the sony map.
- D_800A14E8/14EC/14F0 (cdread block +0x18/+0x1C/+0x20): existing names are roughly right (vsync pre/post, pos), so these are MEDIUM consolidation only.

## Game-side, not API-pinned enough (dropped)
- D_800A36DC, D_800A36E0 (g_text1b_ot_prim_cursor), D_800A37D4 (g_gnd_fog_active), D_800A38B4: primitive pointers passed to SetTile/SetPolyFT4/SetPolyG4/SetDrawMode/AddPrim. "prim pointer" is only a type; how they are allocated or advanced was not traced. D_800A37D4 is also read as data elsewhere (fog), so it is multi-use.
- D_800A3770 (g_video_fb_a): indexed [parity] base from which D_800A38B4 prim pointers are derived. Not API-pinned. The existing "FMV overlay framebuffer" name was NOT checked. Worth its own look.
- D_800A378C: pointer to an OT entry that the 256-entry sub-OT is linked into (func_80048BA4 80048F08). Plausibly "OT slot ptr", but there is one site and it wasn't traced. Dropped.
- D_800A3474 (g_scratchpad_ptr_30), D_800A34B8 (g_scratchpad_ptr_74), D_800A34CC (g_scratchpad_ptr_8c), D_800A34E0, D_800A34EC, D_800A3470: pointer variables to scratchpad passed as MATRIX*/long* to SetRotMatrix/CompMatrix/RotTransPers/ReadGeomScreen/RotMatrix. The existing scratchpad names are fine; API type adds nothing.
- D_80101BD0, D_800FF610 (g_gte_vector_template), D_80101E08, D_800FF56C, D_800EEDF0+0x18, D_80101E3C-0x14, D_800F10A0, D_8009BB74, D_8009BCD4, D_8009BB84, D_800EF0D8: MATRIX/VECTOR/SVECTOR args of MulMatrix*/ApplyMatrix/ScaleMatrixL/RotMatrixZYX/RotTransPers3/4 in single game functions. Type-only; a name would have to say what the matrix represents, which is game semantics.
- D_8009BCD0: SetDrawOffset "args 2/3". SetDrawOffset takes (DR_OFFSET*, u_short*), so these are stale register values. Scanner artifact.
- D_800A30D4 (SetDispMask a1), D_80099D90/D_8009B2E0/D_8009B388/D_8009B390/D_800F33D8/D_800A3478/D_800A34C8/D_800A387C/D_80106A50/D_801077B0/D_80101EC8/D_80106A78/D_800A37E8 (rand/ratan2/rcos/rsin/SquareRoot0 args), D_800A32C0/D_800A32EC/D_800A34FC (DrawSync a1/a2), D_800A12FC (VSync a2), D_800A36A0 (rsin/SetTile), D_800A35A8 (LoadImage deref), D_800EF848 (SetDrawMove arg6), D_800EFB78/D_800EFB3C (SsStart/SsInit take no args), D_8009AD1C (SsUtSetReverbType arg = table value): stale-register artifacts or arguments that are plain numbers. Nothing to name. (ratan2 returns an angle, but storing one pins nothing about the variable.)
- D_800A336C, D_800A33CA, D_800EEE02, D_800F6340+0xA, D_800A37C4: stores of ratan2/rand return values. An angle or random value says nothing about the variable's role.
- D_8009AD18: SsVabClose(lbu X). A VAB id constant byte, but which VAB is game semantics (same reason apiscan left SsVabClose one-liners out).
- D_800A3378: SetDrawMove DR_MOVE* pointer var (func_800401CC). Single site, crosses a label.
- D_8009B8C8: ReadGeomScreen "arg1" is a scanner artifact (it takes no args).
- D_8008F1C0: strcpy source = Shift-JIS string at 0x8008F1C0 (fullwidth "BB2 Narukaga..."). Belongs to the in-binary-string vein, not API role.
- D_80010000..D_800158CC etc.: printf/sprintf/puts format strings. The string vein owns these.
- D_800109B0/BC/C8: memcard sprintf formats. Already aliased by the apiscan wave (g_str_memcard_fmt etc.).
- D_80101E62/D_80101E8C/D_80101E90/D_80101E98: CD streaming state-machine words around g_cd_loc. CdControlF params resolved through register reuse (D_80101E8C-0x20 = g_cd_loc). D_80101E90 is a real CdControlF(0xE=CdlSetmode, &X) param, so it is plausibly the mode byte. Only one site, so dropped. D_80101E62 is a jump-table state index, not API.
- D_800A36AC (g_frame_parity): MEDIUM only. Not every reader masks with &1.
- D_800A3220 / D_80090178 (LoadImage rect + pixels) and D_800A36B8 / D_80101E70: MEDIUM only (single site or untraced writer).
- MotDataBaseAddress / D_800FF6A8 (memcpy dst), _ss_score (_SsSndNextSep): memcpy/libsnd args pin nothing new.

## Flags for the lead (outside this vein)
- `puts` (0x80082000, VERIFIED LIBC2/PUTS) spans 263 insns. Its tail holds the libcd cdread.c statics cb_read/cb_data (sony map notes this). Function boundary, not a naming problem.
- Many correct g_ aliases from the 2026-09-07 apiscan wave (memcard/comb events, fds, bufs) are registered in named_syms.txt, but src/asm still use D_. The HIGH "promote" rows here make them canonical and retire the stale pre-wave aliases at the same address (g_kernel_event_*, g_memcard_event_*).
- Sony module-LOCAL data names (cdread.c state block, CD_ReadCallbackFunc slot, _ss_spu_vm_rec) could come from a libscan pass over the OBJ local-symbol tables. The data wave only used XDEF relocations.
