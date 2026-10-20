"""p3m10 rows: member renames in Unk80101EC8Record (fighter record, include/game.h), offsets
0x404-0x442, wave m10. Leads: tmp/codex/m10-research.md (15 RENAME proposals); every access
re-derived at main 82a4b0fe0 from the compiler-driven list (tools/naming_member_apply.py
--dry-run tmp/p3m10/pairs.txt, output tmp/p3m10/dry.txt: the 25 tail members 0x404-0x442
renamed to throwaway names -> 'OK: every member access renamed'; every access is in
src/main/4510C.c, none in another TU) and by scanning the 137 remaining INCLUDE_ASM bodies
for loads/stores at +0x404..+0x442: none (no asm access).

Changed from Codex (N3: an unshown claim or another rename's word is not evidence):
  unk_40A cpu_range_adjustment -> scale_distance_offset ('range' is a guess; the code shows an
    offset added to distances, computed from the record's unk_1A, which scales geometry).
  unk_40C / unk_40D cpu_last_script_entry / _row -> last_chosen_script_index /
    last_chosen_script_list_index ('row' only restates the unk_86 value; its use is the index
    of the list p->unk_3A8[unk_86] walked).
  unk_40E / 410 / 412 cpu_script_origin_x / z / radius -> chosen_script_start_x / z /
    chosen_script_radius ('origin' = where this fighter stood when the script was chosen).
  unk_414 / unk_424 cpu_opponent_tally / cpu_tally_threshold -> opponent_move_tally /
    opponent_move_tally_threshold (the key is the opponent's unk_5C = its cur_move->unk_04,
    9F9C.c:3364, so the tally is per opponent move record).
  unk_42A / 42C / 42E cpu_response_origin_x / z / radius -> opponent_snapshot_x / z /
    opponent_snapshot_radius ('response' is a purpose the code does not state: the reader only
    tests a distance from the stored opponent position).
  unk_434 cpu_nav_crossing_distance -> nearest_nav_crossing_distance (it is the return of
    fighter_FindNearestNavCrossing, the nearest edge crossing).
  unk_43A / unk_43C cpu_opponent_yaw_delta / _yaw_magnitude -> opponent_angle_from_orientation /
    opponent_angle_from_orientation_abs ('yaw' is not in the code; the angle is relative to
    orientation.vy).
  unk_43E cpu_target_distance -> interpolated_separation ('target' is a purpose; the code
    interpolates two stored distance bounds and compares the result with the fighters'
    separation).

Dropped by the pre-apply review (NO_NAME): unk_414 opponent_move_tally / unk_424
opponent_move_tally_threshold (the 0xFF key is this record's own kind-0x11 state,
4510C.c:744-745, not an opponent move); unk_43E interpolated_separation (a distance bound
compared against the separation, not the separation itself).

Dropped (NO_NAME, as Codex): unk_404 (the distance bound over a subset of list entries: subset
not shown), unk_425 / 426 / 427 / 428 (classification and key members of the opponent-move
latch: written with sentinels and rewritten by later handlers, no single shown role),
unk_430 (heterogeneous flag word), unk_438 (parameter feeding probabilities, intervals and
angles alike), unk_440 / 441 (metadata nibble and its conditional latch: table indices
without a shown category), unk_442 (three-way sign class of the unnamed unk_148 difference).

APPLIER NOTE (comments to follow up): include/game.h:1668 'fighter_BuildCpuPad: 8 (target id,
count) pairs' (unk_414: the id is the opponent's unk_5C, hand-edit with the rename);
:1658 '[unk_86] distance bounds' stays. 4510C.c comments naming members: :553 ('D_800A387C -
unk_43E'), :1001 ('its unk_40A'), :1618 ('the unk_43A angle'), and any naming unk_41x /
42x / 43x; the Q76 notes citing unk_438 stay. The keycheck comment pass lists the rest.
Wording: m09 'cpu_pad_script_*' is pending; this wave uses 'chosen_script_*' so the two stand
or fall independently ('script' here rests on the Start/Step uses below, not on m09).

Freshness (naming-bar step 5): git grep -w of each new name over src/, include/, tools/,
engine/ and asm/: 0 hits each.
"""
import csv
import os

VERDICT = 'CONFIRM'
VERIFIER_NOTE = 'naming-reviewer pre-apply round 1: 12/15 PASS'

T = 'Unk80101EC8Record'

MEMBERS = [
    ('unk_40A', 'scale_distance_offset',
     's16 at +0x40A. WHY: fighter_InitCpuControl sets it per record to (unk_1A - 0x1000) * 225 '
     '>> 11 (4510C.c:345); unk_1A is a geometry scale (0x1000 = unity: bone and root-offset '
     'vectors are multiplied by it >> 12, 9F9C.c:4828-4833; fighter_ScaleBoneHitRecords gets '
     'it, 9F9C.c:3805). Every read adds it to a distance that is then compared with the '
     "fighters' separation D_800A387C: the +100 base of the list-entry bounds (:390), the "
     'opponent-range result of func_80056FE8 (:1010, with +0x12C), the D_8009A850 range '
     'limit (:1861) and the two list-entry limits (:2522-2523). COUNTER-EVIDENCE: written '
     'for both records, so it is not CPU-owned; the 1010 read uses the other record\'s value '
     '(its own scale); "range", "reach" and "adjustment" claim a purpose and are not used. '
     'ACCESSES (all 4510C.c): W 345; R 390, 1010, 1861, 2522, 2523 (1001 is a comment).'),
    ('unk_40C', 'last_chosen_script_index',
     's8 at +0x40C. WHY: set to -1 by fighter_InitCpuControl (4510C.c:403); the selection walk '
     'counts list entries in work4 (:2400-2579, incremented at next:), and after '
     'fighter_StartCpuPadScript for the best entry stores that ordinal (:2583); the only read '
     'skips an entry whose ordinal and unk_40D match the stored pair unless profile flags & '
     '0x80 (:2483). The entry is a script: the pointer given to Start is the entry +4 (+5 '
     'past the 0x40 mask header, :2421-2422, 2491), the bytes fighter_StepCpuPadScript '
     'interprets. COUNTER-EVIDENCE: the other Start callers (1755, 2031, 2253, 2640, 2673) '
     'do not update it, so it is the last chosen by this walk, not the last started. '
     'ACCESSES (all 4510C.c): W 403, 2583; R 2483.'),
    ('unk_40D', 'last_chosen_script_list_index',
     's8 at +0x40D. WHY: -1 at init (4510C.c:402); stored from unk_86 right after the chosen '
     'entry is started (:2584); the entry walk reads the list p->unk_3A8[p->unk_86] (:2400 '
     'region), so the value says which of the three lists the ordinal unk_40C indexes; read '
     'only in the repeat test (:2483). COUNTER-EVIDENCE: unk_86 also indexes the distance '
     'bounds, which this name does not claim; same other-callers caveat as unk_40C. ACCESSES '
     '(all 4510C.c): W 402, 2584; R 2483.'),
    ('unk_40E', 'chosen_script_start_x',
     's16 at +0x40E. WHY: stores p->world_pos.x when a script entry is chosen and started '
     '(4510C.c:2585). fighter_StepCpuPadScript applies the position test only when unk_3BC '
     '== 0; the only Start call with a2 == 0 is that site (the others pass 4, 4, 2, 1 or '
     'mode 3 / 4). The test subtracts it from the OTHER record\'s world_pos.x (:484); with '
     'unk_410 and unk_412 the script ends (unk_3B4 = 0, return -1) when the opponent is '
     'outside the circle: unk_412^2 >= dx^2 + dz^2 continues (:484-487). COUNTER-EVIDENCE: '
     'the point tested is the opponent\'s, the centre is this fighter\'s start position '
     '(named "start", not "origin" or "target"); the local receiving the z difference is '
     'called dy. ACCESSES (all 4510C.c): W 2585; R 484.'),
    ('unk_410', 'chosen_script_start_z',
     's16 at +0x410. WHY: p->world_pos.z at the same store (4510C.c:2586); subtracted from the '
     'opponent\'s world_pos.z in the Step circle test (:485). COUNTER-EVIDENCE: as unk_40E. '
     'ACCESSES (all 4510C.c): W 2586; R 485.'),
    ('unk_412', 'chosen_script_radius',
     's16 at +0x412. WHY: stores var_5, the chosen entry\'s upper distance (hi = e[2] * 40 at '
     ':2497, replaced by 100000 at :2519 or raised by work5 + unk_40A at :2523, var_5 = hi at '
     ':2573) at :2588; the Step test squares it (dz = unk_412, dz * dz >= dx * dx + dy * dy, '
     ':486-487). COUNTER-EVIDENCE: it is a radius only in that test; the selection compared '
     'the same number with the separation as an upper limit. ACCESSES (all 4510C.c): W 2588; '
     'R 486.'),
    ('unk_42A', 'opponent_snapshot_x',
     's16 at +0x42A. WHY: stores rec->other->world_pos.x when unk_428 takes a new opponent '
     'key (4510C.c:766, same block as unk_42C / 42E); the one reader subtracts this '
     "record's world_pos.x from it (:1709) in a circle test against unk_42E. "
     "COUNTER-EVIDENCE: an old position by design (not the opponent's current one); taken "
     'only when this record is not in move kind 0x11. ACCESSES (all 4510C.c): W 766; R 1709.'),
    ('unk_42C', 'opponent_snapshot_z',
     's16 at +0x42C. WHY: rec->other->world_pos.z at the same store (4510C.c:767); the same '
     "circle test subtracts this record's world_pos.z (:1710). COUNTER-EVIDENCE: as unk_42A. "
     'ACCESSES (all 4510C.c): W 767; R 1710.'),
    ('unk_42E', 'opponent_snapshot_radius',
     's16 at +0x42E. WHY: stores func_80056FE8(rec) + 800 (temp3, :747, :768), the same '
     'limit the unk_426 distance bands compare with D_800A387C (:758-761); the reader is '
     '"unk_42E * unk_42E < CPU_SQ(unk_42A - x) + CPU_SQ(unk_42C - z)" (:1707-1710): a '
     'circle around the snapshot position, outside which this record sets unk_426 / unk_425 '
     'to 4. COUNTER-EVIDENCE: the stored number is a distance limit first, a radius only at '
     'the one read; s16 truncation of the limit is possible. ACCESSES (all 4510C.c): W 768; '
     'R 1708.'),
    ('unk_434', 'nearest_nav_crossing_distance',
     's32 at +0x434. WHY: the only write caches fighter_FindNearestNavCrossing (every 8th '
     'build, !(unk_3E8 & 7), 4510C.c:1927-1928), which returns the smallest horizontal '
     'distance from this record to a nav-polygon edge crossed by the segment to (tx, tz), or '
     '100000 when none (:1261-1303); every reader tests it against 100000 only (1942, 1970, '
     '1974, 1982, 1989, 2624). COUNTER-EVIDENCE: before the first call it holds 0 (record '
     'cleared), which a reader sees as a crossing at distance 0; the value is up to 7 builds '
     'old; the segment target is the route target, not necessarily the opponent. ACCESSES '
     '(all 4510C.c): W 1928; R 1942, 1970, 1974, 1982, 1989, 2624.'),
    ('unk_43A', 'opponent_angle_from_orientation',
     's16 at +0x43A. WHY: (ratan2(other.x - x, other.z - z) - orientation.vy) & 0xFFF, minus '
     "0x1000 when above 0x800 (4510C.c:582-587), the signed angle from the record's own "
     'orientation.vy to the other fighter; readers use its sign or value (1730, 1764, 2139, '
     '2300); cleared in fighter_InitCpuControl (:416, 418). COUNTER-EVIDENCE: the build '
     "refreshes only one record's pair per call (chosen by unk_3E8 & 1, :574-581), so it can "
     'be a call old; opponent_bearing is the absolute bearing, this is relative to '
     'orientation.vy. ACCESSES (all 4510C.c): W 416, 418, 582; R 586, 589, 1730, 1764, 2139, '
     '2300 (1618 is a comment); RW 587.'),
    ('unk_43C', 'opponent_angle_from_orientation_abs',
     's16 at +0x43C. WHY: -unk_43A if negative else unk_43A (4510C.c:589); every other read '
     'compares it with an angle constant (0x10, 0x80, 0x100, 0x200, 0x300, 0x400, 0x600, '
     '0x800, or the unk_438-scaled 0x200 term). COUNTER-EVIDENCE: same refresh caveat as '
     'unk_43A; often read through the other record (the other record\'s angle to this one). '
     'ACCESSES (all 4510C.c): W 415, 417, 589; R 467, 469, 640, 749, 788, 845, 1787, 1864, '
     '1953, 1955, 1957, 2120, 2123, 2230, 2310, 2530, 2531, 2539, 2594, 2650.'),
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
