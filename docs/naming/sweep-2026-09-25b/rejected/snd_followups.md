# snd_followups — ideas examined and dropped

## Premise correction: the Marionation heap DOES hold sound banks
The first sweep called the arena [0x800A9D10, 0x800EED10) a "model/data heap, not sound". That holds for
slots 7, 8 and 10. It does **not** hold for the whole arena:
- Slot 6: func_80045B68 allocates it and reads func_80044F50 = NDATA a1+0x83 / a1+0x10C into it. That range carries
  pBAV. func_80046020 frees slot 6 and then calls SsVabClose [VERIFIED] twice (func_8005B6AC).
- Player slots a0+3: func_80045878 reads func_80044ED8 = NDATA 0..0x1E (carries pBAV). func_80045A50 frees them next
  to SsVabClose (func_8005B644).
- The detector works. `magic_sum.py 0 897` finds pBAV in NDATA 0x00-0x24 and 0x83-0x12F, and finds none in
  0x4D-0x82 (slot 8) or 0x131-0x383 (slot 10).

What this means:
- Rows whose contradiction rests on slot 8 or 10 contents stand. So do rows that rest on pointer/vertex/matrix use.
- The applied snd_LoadSe RESET (0x8004695C, slot 9) leaned partly on "arena = not sound". Slot 9 is only touched by
  dead code. That RESET removes a name, so it adds no false positive, but its stated premise is too broad.
- snd_AllocSe and snd_SeNullCallback (slot 9) are filed MEDIUM for this reason.

## Dropped ideas

| addr | name | idea | why dropped |
|---|---|---|---|
| 0x800A9D10 | g_snd_stream_buffer | RESET | The arena holds VAB-bearing files in slots 3/4/6 as well as models and stage data. "Sound" is not contradicted. "Streaming" is wrong in spirit: it is a compacting file heap, with NDATA read whole files, overflow halt "Marionation over flow". But "stream" is too loose to call a concrete contradiction. |
| 0x800EED10 / 0x800EED14 / 0x800EED18 / 0x800EED1C | g_snd_stream_slot_table / _slot_field_4 / _slot_field_8 / _slot_callbacks | RESET | Same reason. The records are {s16 id, base, size, relocation cb}. The mechanical parts are right; "snd_stream" is unsupported, not contradicted. C uses only D_ spellings (src/text1a_c.c:11-20), so a future apply is registry-only. |
| 0x800A33A0 / A4 / A8 / AC | g_snd_stream_write_pos / _buffer_size / _pos_watermark / _count | RESET | Same reason. These are the heap cursor, free bytes (not a size), high-water mark and record count. Note: "buffer_size" is really *free bytes* (it decreases in func_80045600 and func_80045294). That is a candidate if the owner accepts "size vs remaining" as a contradiction; I would not bet on it. |
| 0x800A9CFC / 0x800A9CFE / 0x800A9D04 / 0x800A9D08 | g_snd_stream_stage / _header_state / _header_ptr / _header_field_8 | RESET | Members of the D_800A9CF8 header struct (include/game.h:36-58: +4 stage id, +6 entry count, +0xC entry table stride 0x68, +0x10). It sits 0x18 below the arena. Its consumer (func_8004473C) was not traced far enough to contradict "sound". |
| 0x800A3820 / 0x80102C00 / 0x800A3808 | g_sound_3d_cursor / _data_buffer / _dma_snapshot | RESET | Strong lead, not finished. D_800A3820 is a cursor into a list at D_80102C00. The list is reset in func_80046BF4 and text1a_c.c:1179, and gets transform-node pointers appended: &D_800EF070 (sound.c:885-887), &g_cam_bone_data2 (sound.c:606-608), plus code6cac_c2.c / text1a_pre.c / text1a_c.c sites. D_800A3808 = snapshot of g_gpu_ot_ptr (sound.c:284-285), not a DMA base. The list consumer was not traced, so "3d sound" is not yet shown false. Next pass. |
| 0x800A33C8 / 0x800A33CA | g_sound_3d_angle_h / _v | RESET | func_80047384 writes -ratan2(...) and a second angle, then returns &D_800A33C8. The consumer was not traced. Same lead as above. |
| 0x80044FA0 | marionation_GetFrameOffset | RENAME from the "Marionation over flow" string | The string only supports the overflow error path. Any full name (load / read-file) would be coined. RESET filed instead. |
| 0x800EF558 | g_snd_wave_phase_table | RENAME to drop only the prefix | No admitted class for data restatement. RESET filed. |
| 0x80047550 | saEft03Start_wrapper | RESET on the alias alone | The alias is unsupported (retired Kengo callee name), not contradicted. The row's RESET rests on the live link name game_SndCleanup. |
| 0x800455AC / 0x800453E0 | efc_particle_queue_entry / channel_helper | RESET at HIGH | Generic heap alloc/free. Some slots hold VAB files, and slot 6 is filled by the ex-"particle" Kengo function. Unsupported rather than contradicted. Kept as MEDIUM rows. |
| 0x80046934 / 0x80046954 | snd_AllocSe / snd_SeNullCallback (src spellings) | RESET at HIGH | Slot 9 is only allocated by dead code, and its contents cannot be known. See the premise correction above. Kept as MEDIUM rows. The desync could still be resolved toward the census as housekeeping (these spellings were never accepted; LEGACY_RENAMER_AUDIT.md:109). |
| 0x80047EC8 | snd_GetMaxFade | UPGRADE / RENAME to a size name | The body is `return 0xD00`. Its only use is as a byte count, but a coined "size" name adds nothing. RESET only. |
| 0x800F66A0 | g_anim_func_table / g_camera_rotation_data | RESET | Out of vein. It is the rotation-order builder table (math_RotMatrixZYX/ZXY/YXZ/XYZ). "anim func table" is loosely true. "camera_rotation_data" is doubtful but was not examined: another alias at the same address, owner/data vein. |
