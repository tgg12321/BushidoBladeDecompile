# Motion: move scripts, the frame decoder and the pose

A fighter's animation is driven by two things: the move script it is in
(`MoveScript`, include/game.h:1371; `Unk80101EC8Record.cur_move`), which says
which frames to play and what may follow, and the motion bitstream those
frames are decoded from (`motion_DecodeFrame`). Each frame two decoded
`MotionFrame`s are blended into the model's joint matrices. The bundle files
the data comes from are described in [../formats/BBM.md](../formats/BBM.md).

## Loading

- `motion_LoadCommonData` (src/main/9F9C.c:3242) reads file `cdrom_GetFileIndex(1, 1)`
  to `g_motion_common_buffer` and points the common record `g_motion_common_sections` (include/bb2.h:479)
  at its sections: the move streams at +0x14, then the sections named by the
  header words at +4, +8 and +0x10 (`unk_04`, `unk_08`, `unk_10`).
- `motion_LoadFighterData` (src/main/9F9C.c:3273) loads up to two character files
  (`cdrom_GetFileIndex(1, j + 2)`, `j` the motion data id's index in `g_motion_fighter_data_files`)
  into `g_motion_fighter_buffers[0..1]` (`Tbl800A3860Entry`, include/game.h:720-734) and points
  `g_motion_fighter_sections[i]` (include/bb2.h:482) at their sections, as the comment on
  `Unk801027B0Rec` describes (include/game.h:831). In modes 1, 4 and 6
  (`g_fight_mode_selector`) it also reads seven sectors per fighter from
  `cdrom_GetFileIndex(1, 0)` into `g_motion_frame_buffers[i]`, a table of ready
  `MotionFrame`s.
- `motion_InitStreams` (src/main/9F9C.c:3375) hands each loaded file's bitstream
  (`unk_10`) to `motion_InitStream`, decoder slots 0 (common), 1 and 2.

## Move scripts

`motion_ResolveMoveData(rec, id, is_common_out)` (src/main/9F9C.c:3454) turns a move id into a
`MoveScript *`:

- ids 0x7FF0..0x7FFF select through the character file's tables (`variant_move_script_index`,
  `next_variant_move_script_index`, `f66` of `g_motion_fighter_buffers[rec->motion_slot]`) using the record's cycling index
  `variant_index` (wrapped by `variant_count`; for some ids it skips the entries `object_variant_index` /
  `unk_8E` while their flags `object_variant_enabled` / `unk_90` are clear);
- other ids with bit 15 set index the character file's streams (the
  opponent's when `borrow_other_motion` is set);
- the rest index the common file's streams, and `*is_common_out` is set to 1.

`fighter_StartMoveScript(fighter_index, move_script, is_common)` (src/main/9F9C.c:3553) makes `move_script` the
current move of fighter `fighter_index`: `cur_move = move_script`, `motion_entry_index = move_script->motion_bank_entry_index`, and
`motion_entry` / `motion_metadata` point at the script's entry in the file's `unk_04` table
and at the byte stream it names in `unk_08` (the frame-window stream that
[combat.md](combat.md) describes). It also resets the frame counter `move_frame`
from `move_script->first_move_frame` and records the previous state in `prev_move_kind`.

The script's own fields (`event_move_table_id` / `fallback_move_id` move ids,
`last_move_frame` / `last_command_frame` frame bounds, `unk_09` flags, and the
command list `transition_commands`)
are walked by `fighter_Update`, as the comment on `MoveScript` records
(include/game.h:1364).

## The frame decoder

`g_motion_decoders[slot]` (`Unk800F1B18Rec`, include/game.h:1384) is one decoder per
loaded bitstream.

- `motion_InitStream(decoder_index, stream)` (src/main/9F9C.c:65) reads the 63 initial
  12-bit channel values and their 2-bit channel codes and empties the four
  cache slots (`frame = -2`).
- `motion_DecodeFrame(decoder_index, frame_index, out, work)` (src/main/9F9C.c:101) decodes frame `frame_index`
  into a 0x84-byte `MotionFrame` (include/game.h:1481). Frames come in groups
  of eight: a 24-bit table at stream + 0x70 gives each group's bit offset;
  the group start holds three optional 16-bit root values and the 63
  channels, and each later frame applies per-channel deltas coded by the
  channel's 2-bit code. Four cache slots keep recent frames and the reader
  state after them: a cached frame is copied out, and the frame after a cached
  one is decoded from it instead of from the group start (the comment above the function,
  src/main/9F9C.c:89, details it).

`MotionFrame.root_height` is the root offset, `root_offset_angle` /
`root_distance` the root heading and distance, `unk_06..unk_0A` the root rotation angles, and `node_channels[]` the
per-joint channels (the comment at include/game.h:1469).

## Per-frame update: `fighter_Update`

The fighter loop calls `fighter_BuildCpuOrMappedPadState(fighter_index, pad)` and then `fighter_Update(fighter_index, pad)`
for each fighter (src/main/9F9C.c:2214-2217); the variants at :2265 and :2390
call only `fighter_Update`, with a pad cleared by `pad_ClearStateBits`.
`fighter_Update` (src/main/9F9C.c:4651; its comment lists the whole sequence):

1. converts the pad bits, advances the frame `move_frame` and runs the move
   script's command list, switching moves through `motion_ResolveMoveData` /
   `fighter_StartMoveScript`;
2. decodes the frames at `move_frame` and the next one with `motion_DecodeFrame`
   (src/main/9F9C.c:5140-5149), or copies `g_motion_frame_buffers[fighter_index][move_frame]` in state
   0x33; the frame keys go to `blend_src_frame_code` / `blend_dst_frame_code` as `(slot << 14) | frame`
   (src/main/9F9C.c:5136-5139);
3. turns the two root offsets into a position delta (heading and distance
   through `Judge`, scaled by `size_scale`), blended by `move_frame_frac` or by
   `pose_blend_progress` against the held frame `saved_motion_frame`; the weight goes to `pose_interp_weight`;
4. integrates the velocities `base_velocity` / `decaying_xz_velocity`, resolves the position
   (`fighter_ResolveMovement`, src/main/9F9C.c:4164), and builds the pose:
   `slot_BlendTwoPosesIntoNodeMatrices(fighter_index, &pose_frames[0], &pose_frames[1], pose_interp_weight, ...)` (src/main/9F9C.c:5435) and
   `func_80040D48` (src/main/9F9C.c:5439).

## Building the pose

- `slot_BlendTwoPosesIntoNodeMatrices(i, f0, f1, w, m)` (src/main/31548.c:194): for joints 1..17
  it builds a rotation matrix from each frame's angle triple (the joint's
  offset into the frame comes from `g_node_pose_rotation_indices`) and blends the pair by `w`
  with `gte_LerpMatrixRowsAndRebuildOrthonormalBasis` into the model object's node `work` matrices
  (`g_slot_objects[i]`); joints 18 and 19 also blend a translation
  (`math_LerpShortVectorsNegateYZ`).
- `func_80040D48(i, mode, pos, rot, ...)` (src/main/31548.c:10) sets the root
  node's position and rotation, completes the joint nodes (mode 0 takes the
  angles from a channel array), runs `node_UpdateWorldMatrix` on each of the 18 nodes,
  and queues the nodes on `g_draw_queue_cursor`.

## Other users of the decoder

- `replay_PlayFrame` (src/main/28708.c:1370) rebuilds both fighters from the
  per-frame records `g_replay_fighter_frames` (`Rec1C`, include/game.h:1807, written by
  `replay_RecordFighter` from `cur_move`, `world_pos`, `blend_src_frame_code`, `blend_dst_frame_code`, `pose_interp_weight` and others): it
  decodes the two stored frame keys and calls `slot_BlendTwoPosesIntoNodeMatrices` /
  `func_80040D48` as the live path does.
- `motion_UpdatePlayback` (src/main/4510C.c) plays a loaded sequence: a camera
  stream plus per-object motion streams it registers with `motion_InitStream`
  and decodes each frame with `motion_DecodeFrame`.
