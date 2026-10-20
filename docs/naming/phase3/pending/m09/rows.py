"""p3m09 rows: member renames in Unk80101EC8Record (fighter record, include/game.h), offsets
0x200-0x3FF, wave m09. Leads: tmp/codex/m09-research.md (14 RENAME proposals); every access
re-derived at main 440589c3f from the compiler-driven list (tools/naming_member_apply.py
--dry-run tmp/p3m09/pairs.txt, output tmp/p3m09/dry.txt: 93 error lines in round 0, 0 in round
1, all accesses are in 17AFC.c / 4510C.c / 9F9C.c) and by scanning the 137 remaining INCLUDE_ASM
bodies for loads/stores at these offsets: none (no asm access).

Changed from Codex (N3: an unshown claim or another rename's word is not evidence):
  unk_290 / unk_314 cached_motion_frame / cached_pose_yaw -> saved_motion_frame /
    saved_frame_offset_yaw ('cached' implies a derived value kept for reuse; the code only
    stores fighter_Update's frame 0 at the end of the update and reads it back; 'pose_yaw'
    drops what the angle is: the angle that rotated that frame's polar root offsets);
  unk_318 pose_blend_weight -> pose_blend_progress (m08 pose_interp_weight is the weight in
    use, rec->unk_68 = var_14 every update, and equals unk_318 only during a blend; two
    'weight' names for two members would hide that unk_318 is the running 0..0x1000 state);
  unk_320 pose_blend_root_correction -> pose_blend_base_shift (the same difference is added to
    base_pos and to unk_320, 9F9C.c:4853-4856, so it is the shift applied to base_pos since
    the blend began; 'correction' is a claimed purpose);
  unk_330 / unk_332 queued_kind_count / queued_object_kinds -> carried_object_count /
    carried_object_kinds ('queued' for what: the entries are object kinds taken from
    g_object_pool by object_PickUpNearby, 17AFC.c:3972-3976, and leave by throw / drop as
    object_SpawnFromFighter's kind);
  unk_3B8 cpu_pad_script_delay -> cpu_pad_script_delay_left (it counts down);
  unk_3C8 cpu_script_held -> cpu_pad_script_held (same prefix as the cursor);
  unk_3D0 cpu_pad_state -> last_cpu_pad ('state' is generic and PadState is the type; the
    value is the last pad fighter_BuildCpuPad built: its held bits give the next pad's
    pressed / released and it is copied to the caller);
  unk_3E8 cpu_update_count -> cpu_pad_build_count (rec->update_count is the fighter update
    count; this one counts fighter_BuildCpuPad calls, which run only for records with
    unk_06 != 0).

Dropped (NO_NAME): unk_3A4 cpu_move_list_base / unk_3A8 cpu_move_row_lists (the list is the
motion section unk_0C block passed to fighter_InitCpuControl; its entries (distance bytes
x40, a 0x40 character-mask header, e[0] & 0x80 tested against unk_26C) are not shown to be
'moves'; the row a CPU picks is unk_86 but 'move' is a guess). Codex's other NO_NAME rows
are kept neutral.

Names pending in the struct are treated as taken: m08 (pose_interp_weight, root_offset_yaw,
use_root_offset_yaw, cpu_controlled, motion_*), m04 (cpu_profile, cpu_probes).

APPLIER NOTE (comments to follow up): include/game.h: 'the motion frame fighter_Update last
used' (unk_290, :1593) and the game.h:1344 comment '(Unk80101EC8Record.unk_290 holds ...)';
'queued kinds, unk_330 of them (object_InitKindQueue / object_QueueKind)' (:1604);
'script pointer (...)' (:1633, unk_3B4); 'the pad record fighter_BuildCpuPad builds for
func_8001BE20' (:1646, unk_3D0); 'script list' / 'per-row cursor into unk_3A4' (:1629-1631)
mention only unk_3A4, which stays. include/bb2.h:195 '[unk_0A][i] -> ...unk_332[i]'.
9F9C.c: the fighter_Update header (:4375-4390) cites unk_318 / unk_290 / unk_320 (and
unk_31A, which stays); any comment naming unk_330. 4510C.c: comments naming unk_3B4 /
unk_3B8 / unk_3C8 / unk_3E8. The keycheck comment pass lists the rest.

Freshness (naming-bar step 5): git grep -w of each new name over src/, include/, tools/,
engine/ and asm/: 0 hits each.
"""
import csv
import os

VERDICT = ''
VERIFIER_NOTE = ''

T = 'Unk80101EC8Record'

MEMBERS = [
    ('unk_290', 'saved_motion_frame',
     'MotionFrame at +0x290. WHY: fighter_Update stores its frame 0 (var_0[0]) here after the '
     'per-frame work (9F9C.c:5073) and, when a blend finishes, frame 1 (:5083); the next update '
     'reads it back as the pose to blend away from (:4839) and compares its three rotation '
     'halfwords with the new frame to carry the heading across move kind 6 (:4795). While a '
     'blend runs it is rewritten with itself, so it stays the frozen source. COUNTER-EVIDENCE: '
     '"previous_frame" is false during a blend; "cached" suggests a derived value kept for '
     'speed, which is not shown; "saved" claims only the store-then-reload. ACCESSES (all C): '
     'R 9F9C.c:4795 (x3 fields), 4839; W 9F9C.c:5073, 5083.'),
    ('unk_314', 'saved_frame_offset_yaw',
     "u16 at +0x314. WHY: ang[0] is the angle (unk_154 or orientation.vy) that rotates frame 0's "
     'polar root offset (9F9C.c:4809-4817); it is stored beside the frame (:5074; ang[1] at '
     "blend end :5084) and restored as the source frame's angle (:4840), where it rotates the "
     'source offset (:4857-4863). COUNTER-EVIDENCE: not necessarily orientation.vy (it may be '
     'the other angle), so "orientation" or "heading" names are false; the name says only whose '
     "angle it is (the saved frame's offsets) and what it does. ACCESSES (all C): R "
     '9F9C.c:4840; W 9F9C.c:5074, 5084.'),
    ('unk_318', 'pose_blend_progress',
     's16 at +0x318. WHY: fighter_RequestPoseBlend sets it to its argument on the first '
     'request, when unk_31A becomes 1 (9F9C.c:4302-4305); fighter_Update uses it as the Q12 weight '
     'of the current pose against the saved one (var_14, :4841; root_offset = source * (0x1000 - '
     'w) + current * w, :4876-4878), adds unk_31C each update (:5080) and ends the blend at '
     '>= 0x1000 (:5081). COUNTER-EVIDENCE: it equals m08 pose_interp_weight (unk_68 = var_14, '
     ':4848) only while a blend runs, so it is a distinct member; it starts at the step, not 0. '
     'ACCESSES (all C): W 9F9C.c:4305; R 9F9C.c:4841, 5081; RW 9F9C.c:5080.'),
    ('unk_31C', 'pose_blend_step',
     's16 at +0x31C. WHY: fighter_RequestPoseBlend stores its argument (9F9C.c:4309; callers '
     'pass 0x200 etc.); the only read adds it to unk_318 each update (:5080). COUNTER-EVIDENCE: '
     'the same argument also seeds unk_318 on a first request, which the name does not deny. '
     'ACCESSES (all C): W 9F9C.c:4309; R 9F9C.c:5080.'),
    ('unk_320', 'pose_blend_base_shift',
     'Vec3i32 at +0x320; only x and z are accessed. WHY: the first blend request zeroes x/z '
     '(9F9C.c:4306-4307); during a blend, when move_started, fighter_Update adds '
     "unk_1F8 - v[0] to base_pos.x/z and the same amount to unk_320 (:4853-4856), then "
     "subtracts unk_320 from the source frame's root offset (:4868-4869). So it is the total "
     'shift put onto base_pos since the blend began. COUNTER-EVIDENCE: y is never accessed, so '
     'the name claims no vertical shift; it is not cleared when the blend ends (harmless: '
     'cleared at the next first request). ACCESSES (all C): W 9F9C.c:4306, 4307; RW 4855, '
     '4856; R 4868, 4869.'),
    ('unk_330', 'carried_object_count',
     's16 at +0x330. WHY: object_InitKindQueue counts the non -1 entries of D_8008E338[unk_0A] '
     '(17AFC.c:3751-3757); object_QueueKind (used by object_PickUpNearby) refuses at 0xC and '
     'increments (:3924-3934); object_ThrowQueuedKind and object_DropQueuedHeadAt decrement '
     '(:3861, 3896); readers test emptiness or take it as a count: 9F9C.c:2606, 5043, 5054 '
     '(unk_62 flags), 4510C.c:837, 2554, 2617 (CPU), fighter_Init clears or sets 1 per game '
     'mode (9F9C.c:3832-3842). COUNTER-EVIDENCE: the initial fill writes five slots, sentinels '
     'included, but only non-sentinels are counted. ACCESSES (all C): W 17AFC.c:3751, 3861, '
     '3896, 3932, 3934; RW 3757; R 3847, 3857, 3893, 3924, 3928; R 4510C.c:837, 2554, 2617; R '
     '9F9C.c:2606, 5043, 5054; W 9F9C.c:3832, 3836, 3842.'),
    ('unk_332', 'carried_object_kinds',
     's16[12] at +0x332 (capacity 0xC = the object pool size). WHY: entries are g_object_pool '
     'kinds: object_PickUpNearby passes p->kind to object_QueueKind (17AFC.c:3972); object_Throw'
     'QueuedKind and object_DropQueuedHeadAt pass an entry to object_SpawnFromFighter as the '
     "new object's kind (:3856, 3878) and shift the rest down; object_QueueKind puts a kind "
     'equal to unk_14 at the front (:3926-3931). Init copies D_8008E338[unk_0A][i] (:3755). '
     'COUNTER-EVIDENCE: the first five initial slots may be -1 sentinels beyond the live '
     'count; unk_332[0] is also compared with unk_14 (9F9C.c:3825, 5043). ACCESSES (all C): '
     'W 17AFC.c:3755, 3931, 3934; R 3853, 3855, 3878; RW 3858, 3894, 3929; R 9F9C.c:2606, '
     '3825, 5043, 5054; W 9F9C.c:3829, 3841.'),
    ('unk_3B4', 'cpu_pad_script_cursor',
     'u8 * at +0x3B4. WHY: fighter_StartCpuPadScript stores the script pointer (4510C.c:527); '
     'fighter_StepCpuPadScript resumes from it (:448), stores the position after a delay '
     'opcode (:511) and clears it on the 0x80 terminator or a cancel (:499, 502); '
     'fighter_BuildCpuPad runs the script while it is nonzero (:864); fighter_InitCpuControl '
     'clears it (:407). COUNTER-EVIDENCE: it only advances at delay opcodes, so "pointer to '
     'the current command" is false; "script" comes from the bytes (button set/clear, delay, '
     'end) and the Start/Step function names. ACCESSES (all C): W 4510C.c:407, 499, 502, 511, '
     '527; R 448, 864.'),
    ('unk_3B8', 'cpu_pad_script_delay_left',
     'u8 at +0x3B8. WHY: the Start function stores its argument (4510C.c:529); a delay opcode '
     'stores (t & 0x7F) - 1 (:512); while nonzero fighter_StepCpuPadScript decrements it unless '
     'unk_46 is set and returns the held mask unchanged (:447-452). COUNTER-EVIDENCE: the '
     'decrement is per step call and is suppressed by unk_46, so it is not real frames. '
     'ACCESSES (all C): R 4510C.c:447; W 451, 512, 529.'),
    ('unk_3C8', 'cpu_pad_script_held',
     's32 at +0x3C8. WHY: the Start function zeroes it (4510C.c:530); each non-delay opcode '
     'sets or clears bit (t & 0xF) (:517-519); the Step function returns it (:523), and '
     'fighter_BuildCpuPad stores the result as pad.held (:865-872). COUNTER-EVIDENCE: the '
     'other poll, func_80058580, does not use it, and unk_3CC (CPU decision output) and '
     'unk_3D0 (built pad) are different members. ACCESSES (all C): RW 4510C.c:517, 519; R '
     '523; W 530.'),
    ('unk_3D0', 'last_cpu_pad',
     'PadState at +0x3D0. WHY: fighter_BuildCpuPad derives pressed and released from the new '
     'held bits against this member (4510C.c:885, 887), stores the new pad here (:889) and '
     'copies it to the caller (:890); fighter_InitCpuControl clears held/pressed/released and '
     'sets unheld = -1 (:408-410, 437). COUNTER-EVIDENCE: it is the previous pad only until '
     ':889 and the new one afterwards, so "previous" is false; "last" holds either way. '
     'ACCESSES (all C): W 4510C.c:408, 409, 410, 437, 889; R 885, 887, 890.'),
    ('unk_3E8', 'cpu_pad_build_count',
     'u16 at +0x3E8. WHY: fighter_InitCpuControl zeroes it (4510C.c:427); '
     'fighter_BuildCpuPad (called only from func_8001BE20 for records with unk_06 != 0) '
     'increments it once per call (:627); every other read is a threshold or a parity / '
     'modulo test on it: warm-up and scatter (:574, 596-604), probe slot (:905), periodic '
     'work (:1031, 1806, 1849, 1872, 1879, 1927, 1982, 2390). COUNTER-EVIDENCE: u16 wraps; '
     'it is not the fighter update count (update_count). ACCESSES (all C): W 4510C.c:427; RW '
     '627; R 574, 596, 599, 601, 604, 905, 1031, 1806, 1849, 1872, 1879, 1927, 1982, 2390.'),
]


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    with open(os.path.join(here, 'member_manifest.csv'), 'w', newline='\n') as f:
        w = csv.writer(f, lineterminator='\n')
        w.writerow(['addr', 'current_names', 'proposed_name', 'kind', 'evidence', 'verdict',
                    'verifier_note'])
        for m, n, ev in MEMBERS:
            w.writerow(['member', '%s.%s' % (T, m), n, 'member', ev, VERDICT, VERIFIER_NOTE])
    with open(os.path.join(here, 'pairs.txt'), 'w', newline='\n') as f:
        for m, n, ev in MEMBERS:
            f.write('%s.%s=%s\n' % (T, m, n))


if __name__ == '__main__':
    main()
