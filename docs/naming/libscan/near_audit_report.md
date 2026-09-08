# Near-verbatim audit — SLUS_006.63

## LIBSND/UT_KEYV — SsUtKeyOnV claimed @0x80085A40 (func_80085A40)

| ver | words | scored | best @ | mism | 2nd-best @ | mism | sym offset -> addr | our fn at addr |
|---|---:|---:|---|---:|---|---:|---|---|
| 40J | 259 | 259 | 0x80085A40 | 4 | 0x8002F8EC | 235 | SsUtKeyOnV+0x0->0x80085A40; SsUtKeyOffV+0x394->0x80085DD4 | func_80085A40; func_80085A40 (+0x394 into it) |
| 40U | 259 | 259 | 0x80085A40 | 4 | 0x8002F8EC | 235 | SsUtKeyOnV+0x0->0x80085A40; SsUtKeyOffV+0x394->0x80085DD4 | func_80085A40; func_80085A40 (+0x394 into it) |
| 41 | 248 | 248 | 0x80085A70 | 207 | 0x8007FD54 | 227 | SsUtKeyOnV+0x0->0x80085A70; SsUtKeyOffV+0x360->0x80085DD0 | func_80085A40 (+0x30 into it); func_80085A40 (+0x390 into it) |
| 42 | 248 | 248 | 0x80085A70 | 207 | 0x8007FD54 | 227 | SsUtKeyOnV+0x0->0x80085A70; SsUtKeyOffV+0x360->0x80085DD0 | func_80085A40 (+0x30 into it); func_80085A40 (+0x390 into it) |
| 43 | 248 | 248 | 0x80085A64 | 211 | 0x8007FCC8 | 229 | SsUtKeyOnV+0x0->0x80085A64; SsUtKeyOffV+0x36C->0x80085DD0 | func_80085A40 (+0x24 into it); func_80085A40 (+0x390 into it) |
| 44 | 244 | 244 | 0x80085A40 | 209 | 0x8007FCC8 | 224 | SsUtKeyOnV+0x0->0x80085A40; SsUtKeyOffV+0x35C->0x80085D9C | func_80085A40; func_80085A40 (+0x35C into it) |

**Best build 40J @0x80085A40, mismatch 4/259 scored words. Differing words (lib | EXE):**

| addr | lib word | lib insn | EXE word | EXE insn |
|---|---|---|---|---|
| 0x80085CA4 | 00031040 | `sll	v0,v1,0x1` | 000310C0 | `sll	v0,v1,0x3` |
| 0x80085CA8 | 00431021 | `addu	v0,v0,v1` | 00431023 | `subu	v0,v0,v1` |
| 0x80085CB0 | 00431021 | `addu	v0,v0,v1` | 00431023 | `subu	v0,v0,v1` |
| 0x80085CB4 | 00021080 | `sll	v0,v0,0x2` | 00021040 | `sll	v0,v0,0x1` |

**REL26 callees at the 40J placement (module says -> EXE jal target -> our name):**

- 0x80085ACC: module `sym:_SsVmVSetUp` -> EXE jal 0x80087E3C = **_SsVmVSetUp** OK
- 0x80085D50: module `sym:_SsVmDoAllocate` -> EXE jal 0x800861BC = **func_800861BC** MISMATCH
- 0x80085D6C: module `sym:vmNoiseOn` -> EXE jal 0x80086CF8 = **func_80086CF8** MISMATCH
- 0x80085D7C: module `sym:note2pitch2` -> EXE jal 0x80086BFC = **note2pitch2** OK
- 0x80085D88: module `sym:_SsVmKeyOnNow` -> EXE jal 0x800872A4 = **func_800872A4** MISMATCH
- 0x80085E28: module `sym:_SsVmKeyOffNow` -> EXE jal 0x800871D4 = **func_800871D4** MISMATCH
- data ref `sym:_snd_ev_flag` -> EXE hi/lo 0x800FF630 = g_ings2_counter_FF630
- data ref `(0x16+sym:_svm_cur)` -> EXE hi/lo 0x80102806 = g_weapon_frame_col_plus_10
- data ref `(0x2+sym:_svm_cur)` -> EXE hi/lo 0x801027F2 = g_char_p3_byte_f2_801027F2
- data ref `(0x3+sym:_svm_cur)` -> EXE hi/lo 0x801027F3 = D_801027F3
- data ref `(0xc+sym:_svm_cur)` -> EXE hi/lo 0x801027FC = g_weapon_frame_col
- data ref `(0x5+sym:_svm_cur)` -> EXE hi/lo 0x801027F5 = g_char_p3_byte_f5_801027F5
- data ref `(0x4+sym:_svm_cur)` -> EXE hi/lo 0x801027F4 = g_char_p3_byte_f4_801027F4
- data ref `sym:_svm_pg` -> EXE hi/lo 0x800FF6A0 = g_voice_state_ptr
- data ref `(0xa+sym:_svm_cur)` -> EXE hi/lo 0x801027FA = g_char_p3_byte_fa_801027FA
- data ref `(0xb+sym:_svm_cur)` -> EXE hi/lo 0x801027FB = g_char_p3_byte_fb_801027FB
- data ref `(0x7+sym:_svm_cur)` -> EXE hi/lo 0x801027F7 = g_weapon_frame_row
- data ref `sym:_svm_cur` -> EXE hi/lo 0x801027F0 = D_801027F0
- data ref `sym:_svm_tn` -> EXE hi/lo 0x80101BC8 = g_weapon_frame_data_base
- data ref `(0xf+sym:_svm_cur)` -> EXE hi/lo 0x801027FF = D_801027FF
- data ref `(0x18+sym:_svm_cur)` -> EXE hi/lo 0x80102808 = g_weapon_frame_player_flag
- data ref `(0xd+sym:_svm_cur)` -> EXE hi/lo 0x801027FD = g_char_p3_byte_fd_801027FD
- data ref `(0xe+sym:_svm_cur)` -> EXE hi/lo 0x801027FE = g_char_p3_byte_fe_801027FE
- data ref `(0x10+sym:_svm_cur)` -> EXE hi/lo 0x80102800 = g_pair_state_a_80102800
- data ref `(0x11+sym:_svm_cur)` -> EXE hi/lo 0x80102801 = D_80102801
- data ref `(0x14+sym:_svm_cur)` -> EXE hi/lo 0x80102804 = g_pair_state_b_80102804
- data ref `(0x12+sym:_svm_cur)` -> EXE hi/lo 0x80102802 = D_80102802
- data ref `(0x13+sym:_svm_cur)` -> EXE hi/lo 0x80102803 = D_80102803
- data ref `(0x1a+sym:_svm_cur)` -> EXE hi/lo 0x8010280A = g_weapon_frame_idx
- data ref `(0xe+sym:_svm_voice)` -> EXE hi/lo 0x800F4E28 = g_satan1_slot_state_field_at_E
- data ref `(0x16+sym:_svm_voice)` -> EXE hi/lo 0x800F4E30 = D_800F4E30
- data ref `(0x12+sym:_svm_voice)` -> EXE hi/lo 0x800F4E2C = g_satan1_slot_state_field_at_12
- data ref `(0x10+sym:_svm_voice)` -> EXE hi/lo 0x800F4E2A = g_satan1_slot_state_field_at_10
- data ref `sym:_svm_voice` -> EXE hi/lo 0x800F4E18 = g_satan1_slot_state_anchor_FE18
- data ref `(0xc+sym:_svm_voice)` -> EXE hi/lo 0x800F4E26 = D_800F4E26
- data ref `(0x1b+sym:_svm_voice)` -> EXE hi/lo 0x800F4E35 = g_satan1_slot_state_field_at_14_plus_7
- data ref `(0x2+sym:_svm_voice)` -> EXE hi/lo 0x800F4E1A = g_satan1_slot_state_table
- data ref `(0x14+sym:_svm_voice)` -> EXE hi/lo 0x800F4E2E = g_satan1_slot_state_field_at_14

## LIBSND/UT_KEYV — SsUtKeyOffV claimed @0x80085DD4 ((interior of func_80085A40))

| ver | words | scored | best @ | mism | 2nd-best @ | mism | sym offset -> addr | our fn at addr |
|---|---:|---:|---|---:|---|---:|---|---|
| 40J | 259 | 259 | 0x80085A40 | 4 | 0x8002F8EC | 235 | SsUtKeyOnV+0x0->0x80085A40; SsUtKeyOffV+0x394->0x80085DD4 | func_80085A40; func_80085A40 (+0x394 into it) |
| 40U | 259 | 259 | 0x80085A40 | 4 | 0x8002F8EC | 235 | SsUtKeyOnV+0x0->0x80085A40; SsUtKeyOffV+0x394->0x80085DD4 | func_80085A40; func_80085A40 (+0x394 into it) |
| 41 | 248 | 248 | 0x80085A70 | 207 | 0x8007FD54 | 227 | SsUtKeyOnV+0x0->0x80085A70; SsUtKeyOffV+0x360->0x80085DD0 | func_80085A40 (+0x30 into it); func_80085A40 (+0x390 into it) |
| 42 | 248 | 248 | 0x80085A70 | 207 | 0x8007FD54 | 227 | SsUtKeyOnV+0x0->0x80085A70; SsUtKeyOffV+0x360->0x80085DD0 | func_80085A40 (+0x30 into it); func_80085A40 (+0x390 into it) |
| 43 | 248 | 248 | 0x80085A64 | 211 | 0x8007FCC8 | 229 | SsUtKeyOnV+0x0->0x80085A64; SsUtKeyOffV+0x36C->0x80085DD0 | func_80085A40 (+0x24 into it); func_80085A40 (+0x390 into it) |
| 44 | 244 | 244 | 0x80085A40 | 209 | 0x8007FCC8 | 224 | SsUtKeyOnV+0x0->0x80085A40; SsUtKeyOffV+0x35C->0x80085D9C | func_80085A40; func_80085A40 (+0x35C into it) |

**Best build 40J @0x80085A40, mismatch 4/259 scored words. Differing words (lib | EXE):**

| addr | lib word | lib insn | EXE word | EXE insn |
|---|---|---|---|---|
| 0x80085CA4 | 00031040 | `sll	v0,v1,0x1` | 000310C0 | `sll	v0,v1,0x3` |
| 0x80085CA8 | 00431021 | `addu	v0,v0,v1` | 00431023 | `subu	v0,v0,v1` |
| 0x80085CB0 | 00431021 | `addu	v0,v0,v1` | 00431023 | `subu	v0,v0,v1` |
| 0x80085CB4 | 00021080 | `sll	v0,v0,0x2` | 00021040 | `sll	v0,v0,0x1` |

**REL26 callees at the 40J placement (module says -> EXE jal target -> our name):**

- 0x80085ACC: module `sym:_SsVmVSetUp` -> EXE jal 0x80087E3C = **_SsVmVSetUp** OK
- 0x80085D50: module `sym:_SsVmDoAllocate` -> EXE jal 0x800861BC = **func_800861BC** MISMATCH
- 0x80085D6C: module `sym:vmNoiseOn` -> EXE jal 0x80086CF8 = **func_80086CF8** MISMATCH
- 0x80085D7C: module `sym:note2pitch2` -> EXE jal 0x80086BFC = **note2pitch2** OK
- 0x80085D88: module `sym:_SsVmKeyOnNow` -> EXE jal 0x800872A4 = **func_800872A4** MISMATCH
- 0x80085E28: module `sym:_SsVmKeyOffNow` -> EXE jal 0x800871D4 = **func_800871D4** MISMATCH
- data ref `sym:_snd_ev_flag` -> EXE hi/lo 0x800FF630 = g_ings2_counter_FF630
- data ref `(0x16+sym:_svm_cur)` -> EXE hi/lo 0x80102806 = g_weapon_frame_col_plus_10
- data ref `(0x2+sym:_svm_cur)` -> EXE hi/lo 0x801027F2 = g_char_p3_byte_f2_801027F2
- data ref `(0x3+sym:_svm_cur)` -> EXE hi/lo 0x801027F3 = D_801027F3
- data ref `(0xc+sym:_svm_cur)` -> EXE hi/lo 0x801027FC = g_weapon_frame_col
- data ref `(0x5+sym:_svm_cur)` -> EXE hi/lo 0x801027F5 = g_char_p3_byte_f5_801027F5
- data ref `(0x4+sym:_svm_cur)` -> EXE hi/lo 0x801027F4 = g_char_p3_byte_f4_801027F4
- data ref `sym:_svm_pg` -> EXE hi/lo 0x800FF6A0 = g_voice_state_ptr
- data ref `(0xa+sym:_svm_cur)` -> EXE hi/lo 0x801027FA = g_char_p3_byte_fa_801027FA
- data ref `(0xb+sym:_svm_cur)` -> EXE hi/lo 0x801027FB = g_char_p3_byte_fb_801027FB
- data ref `(0x7+sym:_svm_cur)` -> EXE hi/lo 0x801027F7 = g_weapon_frame_row
- data ref `sym:_svm_cur` -> EXE hi/lo 0x801027F0 = D_801027F0
- data ref `sym:_svm_tn` -> EXE hi/lo 0x80101BC8 = g_weapon_frame_data_base
- data ref `(0xf+sym:_svm_cur)` -> EXE hi/lo 0x801027FF = D_801027FF
- data ref `(0x18+sym:_svm_cur)` -> EXE hi/lo 0x80102808 = g_weapon_frame_player_flag
- data ref `(0xd+sym:_svm_cur)` -> EXE hi/lo 0x801027FD = g_char_p3_byte_fd_801027FD
- data ref `(0xe+sym:_svm_cur)` -> EXE hi/lo 0x801027FE = g_char_p3_byte_fe_801027FE
- data ref `(0x10+sym:_svm_cur)` -> EXE hi/lo 0x80102800 = g_pair_state_a_80102800
- data ref `(0x11+sym:_svm_cur)` -> EXE hi/lo 0x80102801 = D_80102801
- data ref `(0x14+sym:_svm_cur)` -> EXE hi/lo 0x80102804 = g_pair_state_b_80102804
- data ref `(0x12+sym:_svm_cur)` -> EXE hi/lo 0x80102802 = D_80102802
- data ref `(0x13+sym:_svm_cur)` -> EXE hi/lo 0x80102803 = D_80102803
- data ref `(0x1a+sym:_svm_cur)` -> EXE hi/lo 0x8010280A = g_weapon_frame_idx
- data ref `(0xe+sym:_svm_voice)` -> EXE hi/lo 0x800F4E28 = g_satan1_slot_state_field_at_E
- data ref `(0x16+sym:_svm_voice)` -> EXE hi/lo 0x800F4E30 = D_800F4E30
- data ref `(0x12+sym:_svm_voice)` -> EXE hi/lo 0x800F4E2C = g_satan1_slot_state_field_at_12
- data ref `(0x10+sym:_svm_voice)` -> EXE hi/lo 0x800F4E2A = g_satan1_slot_state_field_at_10
- data ref `sym:_svm_voice` -> EXE hi/lo 0x800F4E18 = g_satan1_slot_state_anchor_FE18
- data ref `(0xc+sym:_svm_voice)` -> EXE hi/lo 0x800F4E26 = D_800F4E26
- data ref `(0x1b+sym:_svm_voice)` -> EXE hi/lo 0x800F4E35 = g_satan1_slot_state_field_at_14_plus_7
- data ref `(0x2+sym:_svm_voice)` -> EXE hi/lo 0x800F4E1A = g_satan1_slot_state_table
- data ref `(0x14+sym:_svm_voice)` -> EXE hi/lo 0x800F4E2E = g_satan1_slot_state_field_at_14

## LIBSND/VM_NOWOF — _SsVmKeyOffNow claimed @0x800871D4 (func_800871D4)

| ver | words | scored | best @ | mism | 2nd-best @ | mism | sym offset -> addr | our fn at addr |
|---|---:|---:|---|---:|---|---:|---|---|
| 40J | 52 | 52 | 0x800871D4 | 4 | 0x8008769C | 33 | _SsVmKeyOffNow+0x0->0x800871D4 | func_800871D4 |
| 40U | 52 | 52 | 0x800871D4 | 4 | 0x8008769C | 33 | _SsVmKeyOffNow+0x0->0x800871D4 | func_800871D4 |
| 41 | 52 | 52 | 0x800871D4 | 2 | 0x8008769C | 34 | _SsVmKeyOffNow+0x0->0x800871D4 | func_800871D4 |
| 42 | 52 | 52 | 0x800871D4 | 2 | 0x8008769C | 34 | _SsVmKeyOffNow+0x0->0x800871D4 | func_800871D4 |
| 43 | 48 | 48 | 0x800871E0 | 18 | 0x800876A8 | 30 | _SsVmKeyOffNow+0x0->0x800871E0 | func_800871D4 (+0xC into it) |
| 44 | 48 | 48 | 0x800871E0 | 18 | 0x800876A8 | 30 | _SsVmKeyOffNow+0x0->0x800871E0 | func_800871D4 (+0xC into it) |

**Best build 41 @0x800871D4, mismatch 2/52 scored words. Differing words (lib | EXE):**

| addr | lib word | lib insn | EXE word | EXE insn |
|---|---|---|---|---|
| 0x80087298 | 03E00008 | `jr	ra` | A4221B12 | `sh	v0,6930(at)` |
| 0x8008729C | A4220000 | `sh	v0,0(at)` | 03E00008 | `jr	ra` |

**REL26 callees at the 41 placement (module says -> EXE jal target -> our name):**

- data ref `(0x1a+sym:_svm_cur)` -> EXE hi/lo 0x8010280A = g_weapon_frame_idx
- data ref `(0x1d+sym:_svm_voice)` -> EXE hi/lo 0x800F4E35 = g_satan1_slot_state_field_at_14_plus_7
- data ref `sym:_svm_okof1` -> EXE hi/lo 0x801078D8 = g_char_p21_field_420
- data ref `sym:_svm_okof2` -> EXE hi/lo 0x801078DA = g_char_p21_field_422
- data ref `(0x4+sym:_svm_voice)` -> EXE hi/lo 0x800F4E1C = g_satan1_slot_state_table_plus_2
- data ref `sym:_svm_voice` -> EXE hi/lo 0x800F4E18 = g_satan1_slot_state_anchor_FE18
- data ref `sym:_svm_okon1` -> EXE hi/lo 0x800F1B10 = g_spu_xfer_status_snapshot_plus_8_plus_16
- data ref `sym:_svm_okon2` -> EXE hi/lo 0x800F1B12 = g_main_mask_reg_800F1B12
- data ref `sym:_svm_okon2` -> EXE hi/lo 0x800F0008 = ?

## LIBSND/UT_VVOL — SsUtGetDetVVol claimed @0x80085FD8 (func_80085FD8 (C))

| ver | words | scored | best @ | mism | 2nd-best @ | mism | sym offset -> addr | our fn at addr |
|---|---:|---:|---|---:|---|---:|---|---|
| 40J | 141 | 141 | 0x80085FBC | 111 | 0x80087A30 | 130 | SsUtGetDetVVol+0x0->0x80085FBC; SsUtSetDetVVol+0x58->0x80086014; SsUtGetVVol+0xC4->0x80086080; SsUtSetVVol+0x150->0x8008610C | SsUtReverbOn (+0x4 into it); func_80086014; func_80086080; func_80086080 (+0x8C into it) |
| 40U | 141 | 141 | 0x80085FBC | 111 | 0x80087A30 | 130 | SsUtGetDetVVol+0x0->0x80085FBC; SsUtSetDetVVol+0x58->0x80086014; SsUtGetVVol+0xC4->0x80086080; SsUtSetVVol+0x150->0x8008610C | SsUtReverbOn (+0x4 into it); func_80086014; func_80086080; func_80086080 (+0x8C into it) |
| 41 | 116 | 116 | 0x80085FE0 | 73 | 0x8008AE40 | 107 | SsUtGetDetVVol+0x0->0x80085FE0; SsUtSetDetVVol+0x3C->0x8008601C; SsUtGetVVol+0xA0->0x80086080; SsUtSetVVol+0x14C->0x8008612C | func_80085FD8 (+0x8 into it); func_80086014 (+0x8 into it); func_80086080; func_80086080 (+0xAC into it) |
| 42 | 116 | 116 | 0x80085FE0 | 73 | 0x8008AE40 | 107 | SsUtGetDetVVol+0x0->0x80085FE0; SsUtSetDetVVol+0x3C->0x8008601C; SsUtGetVVol+0xA0->0x80086080; SsUtSetVVol+0x14C->0x8008612C | func_80085FD8 (+0x8 into it); func_80086014 (+0x8 into it); func_80086080; func_80086080 (+0xAC into it) |
| 43 | 16 | 16 | 0x80085FD8 | 1 | 0x8007DF20 | 9 | SsUtGetDetVVol+0x0->0x80085FD8 | func_80085FD8 |
| 44 | 116 | 116 | 0x80085FE0 | 73 | 0x8008AE40 | 107 | SsUtGetDetVVol+0x0->0x80085FE0; SsUtSetDetVVol+0x3C->0x8008601C; SsUtGetVVol+0xA0->0x80086080; SsUtSetVVol+0x14C->0x8008612C | func_80085FD8 (+0x8 into it); func_80086014 (+0x8 into it); func_80086080; func_80086080 (+0xAC into it) |

**Best build 43 @0x80085FD8, mismatch 1/16 scored words. Differing words (lib | EXE):**

| addr | lib word | lib insn | EXE word | EXE insn |
|---|---|---|---|---|
| 0x80086014 | 00000000 | `nop` | 27BDFFF8 | `addiu	sp,sp,-8` |

**REL26 callees at the 43 placement (module says -> EXE jal target -> our name):**

- 0x80085FF8: module `sym:SpuGetVoiceVolume` -> EXE jal 0x8008BD88 = **SpuGetVoiceVolume** OK

## LIBSND/UT_VVOL3 — SsUtGetVVol claimed @0x80086080 (func_80086080 (C))

| ver | words | scored | best @ | mism | 2nd-best @ | mism | sym offset -> addr | our fn at addr |
|---|---:|---:|---|---:|---|---:|---|---|
| 40J | - | - | (module absent) | | | | | |
| 40U | - | - | (module absent) | | | | | |
| 41 | - | - | (module absent) | | | | | |
| 42 | - | - | (module absent) | | | | | |
| 43 | 44 | 44 | 0x80086080 | 2 | 0x80085FC8 | 38 | SsUtGetVVol+0x0->0x80086080 | func_80086080 |
| 44 | - | - | (module absent) | | | | | |

**Best build 43 @0x80086080, mismatch 2/44 scored words. Differing words (lib | EXE):**

| addr | lib word | lib insn | EXE word | EXE insn |
|---|---|---|---|---|
| 0x80086124 | 03E00008 | `jr	ra` | 27BD0028 | `addiu	sp,sp,40` |
| 0x80086128 | 27BD0028 | `addiu	sp,sp,40` | 03E00008 | `jr	ra` |

**REL26 callees at the 43 placement (module says -> EXE jal target -> our name):**

- 0x800860B8: module `sym:SpuGetVoiceVolume` -> EXE jal 0x8008BD88 = **SpuGetVoiceVolume** OK

## LIBSND/VM_SEQ_2 — _SsVmGetSeqLVol claimed @0x80087D10 (func_80087D10 (C))

| ver | words | scored | best @ | mism | 2nd-best @ | mism | sym offset -> addr | our fn at addr |
|---|---:|---:|---|---:|---|---:|---|---|
| 40J | - | - | (module absent) | | | | | |
| 40U | - | - | (module absent) | | | | | |
| 41 | - | - | (module absent) | | | | | |
| 42 | - | - | (module absent) | | | | | |
| 43 | 36 | 36 | 0x80087D10 | 6 | 0x80087C68 | 28 | _SsVmGetSeqLVol+0x0->0x80087D10; _SsVmGetSeqRVol+0x48->0x80087D58 | func_80087D10; func_80087D58 |
| 44 | - | - | (module absent) | | | | | |

**Best build 43 @0x80087D10, mismatch 6/36 scored words. Differing words (lib | EXE):**

| addr | lib word | lib insn | EXE word | EXE insn |
|---|---|---|---|---|
| 0x80087D18 | 3C030000 | `lui	v1,0x0` | 3C018010 | `lui	at,0x8010` |
| 0x80087D1C | 00621821 | `addu	v1,v1,v0` | 00220821 | `addu	at,at,v0` |
| 0x80087D20 | 8C630000 | `lw	v1,0(v1)` | 8C236F28 | `lw	v1,28456(at)` |
| 0x80087D60 | 3C030000 | `lui	v1,0x0` | 3C018010 | `lui	at,0x8010` |
| 0x80087D64 | 00621821 | `addu	v1,v1,v0` | 00220821 | `addu	at,at,v0` |
| 0x80087D68 | 8C630000 | `lw	v1,0(v1)` | 8C236F28 | `lw	v1,28456(at)` |

**REL26 callees at the 43 placement (module says -> EXE jal target -> our name):**

- data ref `sym:_ss_score` -> EXE hi/lo 0x80106F28 = g_seq_bank_table
- data ref `(0x14+sym:_svm_cur)` -> EXE hi/lo 0x80102806 = g_weapon_frame_col_plus_10

## LIBSND/VM_SEQ_2 — _SsVmGetSeqRVol claimed @0x80087D58 (func_80087D58 (C))

| ver | words | scored | best @ | mism | 2nd-best @ | mism | sym offset -> addr | our fn at addr |
|---|---:|---:|---|---:|---|---:|---|---|
| 40J | - | - | (module absent) | | | | | |
| 40U | - | - | (module absent) | | | | | |
| 41 | - | - | (module absent) | | | | | |
| 42 | - | - | (module absent) | | | | | |
| 43 | 36 | 36 | 0x80087D10 | 6 | 0x80087C68 | 28 | _SsVmGetSeqLVol+0x0->0x80087D10; _SsVmGetSeqRVol+0x48->0x80087D58 | func_80087D10; func_80087D58 |
| 44 | - | - | (module absent) | | | | | |

**Best build 43 @0x80087D10, mismatch 6/36 scored words. Differing words (lib | EXE):**

| addr | lib word | lib insn | EXE word | EXE insn |
|---|---|---|---|---|
| 0x80087D18 | 3C030000 | `lui	v1,0x0` | 3C018010 | `lui	at,0x8010` |
| 0x80087D1C | 00621821 | `addu	v1,v1,v0` | 00220821 | `addu	at,at,v0` |
| 0x80087D20 | 8C630000 | `lw	v1,0(v1)` | 8C236F28 | `lw	v1,28456(at)` |
| 0x80087D60 | 3C030000 | `lui	v1,0x0` | 3C018010 | `lui	at,0x8010` |
| 0x80087D64 | 00621821 | `addu	v1,v1,v0` | 00220821 | `addu	at,at,v0` |
| 0x80087D68 | 8C630000 | `lw	v1,0(v1)` | 8C236F28 | `lw	v1,28456(at)` |

**REL26 callees at the 43 placement (module says -> EXE jal target -> our name):**

- data ref `sym:_ss_score` -> EXE hi/lo 0x80106F28 = g_seq_bank_table
- data ref `(0x14+sym:_svm_cur)` -> EXE hi/lo 0x80102806 = g_weapon_frame_col_plus_10

## LIBSND/UT_VVOL2 — SsUtSetDetVVol claimed @0x80086018 (func_80086014 (starts 4 bytes EARLIER))

| ver | words | scored | best @ | mism | 2nd-best @ | mism | sym offset -> addr | our fn at addr |
|---|---:|---:|---|---:|---|---:|---|---|
| 40J | - | - | (module absent) | | | | | |
| 40U | - | - | (module absent) | | | | | |
| 41 | - | - | (module absent) | | | | | |
| 42 | - | - | (module absent) | | | | | |
| 43 | 28 | 28 | 0x80086018 | 7 | 0x80086154 | 21 | SsUtSetDetVVol+0x0->0x80086018 | func_80086014 (+0x4 into it) |
| 44 | - | - | (module absent) | | | | | |

**Best build 43 @0x80086018, mismatch 7/28 scored words. Differing words (lib | EXE):**

| addr | lib word | lib insn | EXE word | EXE insn |
|---|---|---|---|---|
| 0x80086048 | 3C040000 | `lui	a0,0x0` | 3C01800F | `lui	at,0x800f` |
| 0x8008604C | 00832021 | `addu	a0,a0,v1` | 00230821 | `addu	at,at,v1` |
| 0x80086050 | 90840000 | `lbu	a0,0(a0)` | 902465E0 | `lbu	a0,26080(at)` |
| 0x80086074 | 03E00008 | `jr	ra` | 27BD0008 | `addiu	sp,sp,8` |
| 0x80086078 | 00000000 | `nop` | 03E00008 | `jr	ra` |
| 0x80086080 | 00000000 | `?` | 27BDFFD8 | `addiu	sp,sp,-40` |
| 0x80086084 | 00000000 | `?` | AFB00018 | `sw	s0,24(sp)` |

**REL26 callees at the 43 placement (module says -> EXE jal target -> our name):**

- data ref `(0x2+sym:_svm_sreg_buf)` -> EXE hi/lo 0x80102A7A = g_voice_slot_24_arr_b
- data ref `sym:_svm_sreg_dirty` -> EXE hi/lo 0x800F65E0 = g_voice_slot_24_flags
- data ref `sym:_svm_sreg_buf` -> EXE hi/lo 0x80102A78 = g_voice_slot_24_arr_a

## LIBSPU/S_SAV — _SpuSetAnyVoice claimed @0x80089A54 (func_80089A48 (starts 12 bytes EARLIER))

| ver | words | scored | best @ | mism | 2nd-best @ | mism | sym offset -> addr | our fn at addr |
|---|---:|---:|---|---:|---|---:|---|---|
| 40J | 130 | 130 | 0x80089A70 | 64 | 0x8008161C | 121 | _SpuSetAnyVoice+0x0->0x80089A70 | func_80089A48 (+0x28 into it) |
| 40U | 130 | 130 | 0x80089A70 | 64 | 0x8008161C | 121 | _SpuSetAnyVoice+0x0->0x80089A70 | func_80089A48 (+0x28 into it) |
| 41 | 176 | 176 | 0x80089A54 | 31 | 0x8008159C | 165 | _SpuSetAnyVoice+0x0->0x80089A54 | func_80089A48 (+0xC into it) |
| 42 | 176 | 176 | 0x80089A54 | 31 | 0x8008159C | 165 | _SpuSetAnyVoice+0x0->0x80089A54 | func_80089A48 (+0xC into it) |
| 43 | 176 | 176 | 0x80089A54 | 31 | 0x8008159C | 165 | _SpuSetAnyVoice+0x0->0x80089A54 | func_80089A48 (+0xC into it) |
| 44 | 176 | 176 | 0x80089A54 | 31 | 0x8008159C | 165 | _SpuSetAnyVoice+0x0->0x80089A54 | func_80089A48 (+0xC into it) |

**Best build 41 @0x80089A54, mismatch 31/176 scored words. Differing words (lib | EXE):**

| addr | lib word | lib insn | EXE word | EXE insn |
|---|---|---|---|---|
| 0x80089A54 | 00804821 | `move	t1,a0` | 00000000 | `nop` |
| 0x80089A58 | 3C020000 | `lui	v0,0x0` | 30420001 | `andi	v0,v0,0x1` |
| 0x80089A5C | 8C420000 | `lw	v0,0(v0)` | 10400005 | `beqz	v0,0x80089a74` |
| 0x80089A60 | 00000000 | `nop` | 00A04021 | `move	t0,a1` |
| 0x80089A64 | 30420001 | `andi	v0,v0,0x1` | 3C04800F | `lui	a0,0x800f` |
| 0x80089A68 | 3C040000 | `lui	a0,0x0` | 24847298 | `addiu	a0,a0,29336` |
| 0x80089A6C | 2484FE78 | `addiu	a0,a0,-392` | 080226A0 | `j	0x80089a80` |
| 0x80089A70 | 14400003 | `bnez	v0,0x80089a80` | 00071040 | `sll	v0,a3,0x1` |
| 0x80089A74 | 00A04021 | `move	t0,a1` | 3C04800A | `lui	a0,0x800a` |
| 0x80089A78 | 3C040000 | `lui	a0,0x0` | 8C842CDC | `lw	a0,11484(a0)` |
| 0x80089A7C | 8C840000 | `lw	a0,0(a0)` | 00071040 | `sll	v0,a3,0x1` |
| 0x80089A80 | 00071040 | `sll	v0,a3,0x1` | 00441021 | `addu	v0,v0,a0` |
| 0x80089A84 | 00441021 | `addu	v0,v0,a0` | 94430000 | `lhu	v1,0(v0)` |
| 0x80089A88 | 94430000 | `lhu	v1,0(v0)` | 00061040 | `sll	v0,a2,0x1` |
| 0x80089A8C | 00061040 | `sll	v0,a2,0x1` | 00441021 | `addu	v0,v0,a0` |
| 0x80089A90 | 00441021 | `addu	v0,v0,a0` | 94420000 | `lhu	v0,0(v0)` |
| 0x80089A94 | 94420000 | `lhu	v0,0(v0)` | 306300FF | `andi	v1,v1,0xff` |
| 0x80089A98 | 306300FF | `andi	v1,v1,0xff` | 00031C00 | `sll	v1,v1,0x10` |
| 0x80089A9C | 00031C00 | `sll	v1,v1,0x10` | 240B0001 | `li	t3,1` |
| 0x80089AA0 | 240B0001 | `li	t3,1` | 112B000D | `beq	t1,t3,0x80089ad8` |
| 0x80089AA4 | 112B000C | `beq	t1,t3,0x80089ad8` | 00435025 | `or	t2,v0,v1` |
| 0x80089AA8 | 00435025 | `or	t2,v0,v1` | 29220002 | `slti	v0,t1,2` |
| 0x80089AAC | 29220002 | `slti	v0,t1,2` | 10400005 | `beqz	v0,0x80089ac4` |
| 0x80089AB0 | 10400005 | `beqz	v0,0x80089ac8` | 00000000 | `nop` |
| 0x80089AB4 | 24020008 | `li	v0,8` | 11200037 | `beqz	t1,0x80089b94` |
| 0x80089AB8 | 11200036 | `beqz	t1,0x80089b94` | 3C0200FF | `lui	v0,0xff` |
| 0x80089ABC | 3C0200FF | `lui	v0,0xff` | 08022741 | `j	0x80089d04` |
| 0x80089AC0 | 080000AD | `j	0x800002b4` | 00000000 | `nop` |
| 0x80089AC4 | 3442FFFF | `ori	v0,v0,0xffff` | 24020008 | `li	v0,8` |
| 0x80089AD4 | 3442FFFF | `ori	v0,v0,0xffff` | 00000000 | `nop` |
| 0x80089D10 | 00000000 | `nop` | 00801021 | `move	v0,a0` |

**REL26 callees at the 41 placement (module says -> EXE jal target -> our name):**

- data ref `sym:_spu_env` -> EXE hi/lo 0x00010005 = ?
- data ref `(0x188-sym:_spu_RQ)` -> EXE hi/lo 0x729826A0 = ?
- data ref `sym:_spu_RXX` -> EXE hi/lo 0x2CDC1040 = ?
- data ref `sym:_spu_env` -> EXE hi/lo 0x800A2CD4 = g_spu_pending_irq_mask
- data ref `(0x188-sym:_spu_RQ)` -> EXE hi/lo 0x800F7298 = g_spu_reg_voice_array
- data ref `sym:_spu_RQmask` -> EXE hi/lo 0x800A28A0 = g_spu_voice_mask
- data ref `sym:_spu_RXX` -> EXE hi/lo 0x800A2CDC = g_spu_base_addr

## LIBSND/VM_ALOC2 — _SsVmDoAllocate claimed @0x800861B8 (func_800861BC (starts 4 bytes LATER))

| ver | words | scored | best @ | mism | 2nd-best @ | mism | sym offset -> addr | our fn at addr |
|---|---:|---:|---|---:|---|---:|---|---|
| 40J | 133 | 133 | 0x800861B8 | 33 | 0x8006E618 | 113 | _SsVmDoAllocate+0x0->0x800861B8 | func_80086130 (+0x88 into it) |
| 40U | 133 | 133 | 0x800861B8 | 33 | 0x8006E618 | 113 | _SsVmDoAllocate+0x0->0x800861B8 | func_80086130 (+0x88 into it) |
| 41 | 120 | 120 | 0x800861C8 | 102 | 0x80034020 | 112 | _SsVmDoAllocate+0x0->0x800861C8 | func_800861BC (+0xC into it) |
| 42 | 120 | 120 | 0x800861C8 | 102 | 0x80034020 | 112 | _SsVmDoAllocate+0x0->0x800861C8 | func_800861BC (+0xC into it) |
| 43 | 116 | 116 | 0x800861E0 | 102 | 0x800820EC | 107 | _SsVmDoAllocate+0x0->0x800861E0 | func_800861BC (+0x24 into it) |
| 44 | 116 | 116 | 0x800861E0 | 102 | 0x800820EC | 107 | _SsVmDoAllocate+0x0->0x800861E0 | func_800861BC (+0x24 into it) |

**Best build 40J @0x800861B8, mismatch 33/133 scored words. Differing words (lib | EXE):**

| addr | lib word | lib insn | EXE word | EXE insn |
|---|---|---|---|---|
| 0x800861B8 | 27BDFFF8 | `addiu	sp,sp,-8` | 00000000 | `nop` |
| 0x800861BC | 00003021 | `move	a2,zero` | 27BDFFF8 | `addiu	sp,sp,-8` |
| 0x800861C0 | 3C030000 | `lui	v1,0x0` | 00003821 | `move	a3,zero` |
| 0x800861C4 | 24630000 | `addiu	v1,v1,0` | 3C038010 | `lui	v1,0x8010` |
| 0x800861C8 | 2468FFFE | `addiu	t0,v1,-2` | 2463280C | `addiu	v1,v1,10252` |
| 0x800861CC | 24070001 | `li	a3,1` | 2469FFFE | `addiu	t1,v1,-2` |
| 0x800861D0 | 3C040000 | `lui	a0,0x0` | 24080001 | `li	t0,1` |
| 0x800861D4 | 84840000 | `lh	a0,0(a0)` | 3C058010 | `lui	a1,0x8010` |
| 0x800861D8 | 3C050000 | `lui	a1,0x0` | 84A5280A | `lh	a1,10250(a1)` |
| 0x800861DC | 24A50000 | `addiu	a1,a1,0` | 3C068010 | `lui	a2,0x8010` |
| 0x800861E0 | 000410C0 | `sll	v0,a0,0x3` | 24C67898 | `addiu	a2,a2,30872` |
| 0x800861E4 | A4620000 | `sh	v0,0(v1)` | 000510C0 | `sll	v0,a1,0x3` |
| 0x800861E8 | 3C020000 | `lui	v0,0x0` | A4620000 | `sh	v0,0(v1)` |
| 0x800861EC | 90420000 | `lbu	v0,0(v0)` | 00451023 | `subu	v0,v0,a1` |
| 0x800861F0 | 3C030000 | `lui	v1,0x0` | 00021080 | `sll	v0,v0,0x2` |
| 0x800861F4 | 90630000 | `lbu	v1,0(v1)` | 00451023 | `subu	v0,v0,a1` |
| 0x800861F8 | 00021100 | `sll	v0,v0,0x4` | 00021040 | `sll	v0,v0,0x1` |
| 0x800861FC | 00621821 | `addu	v1,v1,v0` | 3C038010 | `lui	v1,0x8010` |
| 0x80086200 | 00041040 | `sll	v0,a0,0x1` | 906327F7 | `lbu	v1,10231(v1)` |
| 0x80086204 | 00441021 | `addu	v0,v0,a0` | 3C048010 | `lui	a0,0x8010` |
| 0x80086208 | 00021080 | `sll	v0,v0,0x2` | 908427FC | `lbu	a0,10236(a0)` |
| 0x8008620C | 00441021 | `addu	v0,v0,a0` | 00031900 | `sll	v1,v1,0x4` |
| 0x80086210 | 00021080 | `sll	v0,v0,0x2` | 00832021 | `addu	a0,a0,v1` |
| 0x80086214 | 3C010000 | `lui	at,0x0` | 24037FFF | `li	v1,32767` |
| 0x80086218 | A4230000 | `sh	v1,0(at)` | 3C018010 | `lui	at,0x8010` |
| 0x8008621C | 24037FFF | `li	v1,32767` | A424280E | `sh	a0,10254(at)` |
| 0x8008622C | 24C60001 | `addiu	a2,a2,1` | 24E70001 | `addiu	a3,a3,1` |
| 0x80086230 | 85020000 | `lh	v0,0(t0)` | 85220000 | `lh	v0,0(t1)` |
| 0x80086234 | 8CA30000 | `lw	v1,0(a1)` | 8CC30000 | `lw	v1,0(a2)` |
| 0x80086238 | 00471004 | `sllv	v0,a3,v0` | 00481004 | `sllv	v0,t0,v0` |
| 0x80086244 | ACA30000 | `sw	v1,0(a1)` | ACC30000 | `sw	v1,0(a2)` |
| 0x80086248 | 28C20010 | `slti	v0,a2,16` | 28E20010 | `slti	v0,a3,16` |
| 0x80086250 | 24A50004 | `addiu	a1,a1,4` | 24C60004 | `addiu	a2,a2,4` |

**REL26 callees at the 40J placement (module says -> EXE jal target -> our name):**

- data ref `(0x1c+sym:_svm_cur)` -> EXE hi/lo 0x38208010 = ?
- data ref `(0x1a+sym:_svm_cur)` -> EXE hi/lo 0x00008010 = ?
- data ref `sym:_svm_envx_hist` -> EXE hi/lo 0x28098010 = ?
- data ref `(0x7+sym:_svm_cur)` -> EXE hi/lo 0x00001023 = ?
- data ref `(0xc+sym:_svm_cur)` -> EXE hi/lo 0x10801023 = ?
- data ref `(0x1e+sym:_svm_cur)` -> EXE hi/lo 0x7FFE8010 = ?
- data ref `(0x6+sym:_svm_voice)` -> EXE hi/lo 0x800F4E1E = g_satan1_slot_state_field_at_4
- data ref `(0x18+sym:_svm_cur)` -> EXE hi/lo 0x80102808 = g_weapon_frame_player_flag
- data ref `sym:_svm_pg` -> EXE hi/lo 0x800FF6A0 = g_voice_state_ptr
- data ref `(0x1c+sym:_svm_cur)` -> EXE hi/lo 0x8010280C = g_weapon_frame_cursor
- data ref `(0x6+sym:_svm_sreg_buf)` -> EXE hi/lo 0x80102A7E = g_voice_slot_24_arr_b_plus_4
- data ref `(0x1a+sym:_svm_cur)` -> EXE hi/lo 0x8010280A = g_weapon_frame_idx
- data ref `sym:_svm_sreg_dirty` -> EXE hi/lo 0x800F65E0 = g_voice_slot_24_flags
- data ref `(0x7+sym:_svm_cur)` -> EXE hi/lo 0x801027F7 = g_weapon_frame_row
- data ref `(0xc+sym:_svm_cur)` -> EXE hi/lo 0x801027FC = g_weapon_frame_col
- data ref `sym:_svm_tn` -> EXE hi/lo 0x80101BC8 = g_weapon_frame_data_base
- data ref `(0x8+sym:_svm_sreg_buf)` -> EXE hi/lo 0x80102A80 = g_voice_slot_24_arr_b_plus_6
- data ref `sym:_svm_damper` -> EXE hi/lo 0x800F66F8 = g_weapon_frame_y_offset
- data ref `(0xa+sym:_svm_sreg_buf)` -> EXE hi/lo 0x80102A82 = g_voice_slot_24_arr_b_plus_8
