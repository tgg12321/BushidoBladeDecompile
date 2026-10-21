"""p3m12 rows: member renames in MoveScript (header of one move record of a character's move
script, include/game.h:1402; pointed to by Unk80101EC8Record.cur_move / queued_move, Rec1C.w0,
and the returns of motion_Get*VariantScript / func_800219E4 / func_80021A3C); wave m12. Leads:
tmp/codex/m12-research.md (untrusted). Every access re-derived at main ca52c1d92 from the
compiler-driven list (tools/naming_member_apply.py --dry-run pairs.txt, output dry.txt:
22 renamed access lines in 17AFC.c 4510C.c 9F9C.c plus 28708.c:1429/1434, which the tool
reports UNRESOLVED because the same lines also reach g_motion_*_sections.unk_04 (another type);
see APPLIER NOTE) and by scanning every INCLUDE_ASM body (137 listed in src, asm/funcs) for
loads of cur_move (+0x50), calls of the move getters / motion_ResolveMoveData /
motion_FindPairedMoveEntry, and lbu/lhu at 0x7-0x9 / 0x0-0x4 offsets: the only hits are matrix
and vertex code (CompMatrix, MulMatrix*, math_RotMatrixZYX, gte_*, func_8004A4E0/A940/C404/
DA74/DDB4, func_80054440), none dereferences a MoveScript. No member is written anywhere in C
or asm (the records are ROM data). 9F9C.c:2191 'entry->unk_0A' belongs to another type
(Unk8001FAE4Entry-adjacent, not MoveScript) and is not touched.

Dropped vs Codex: unk_09 stays unk_09 (Codex NO_NAME agreed): its uses are four independent
bit tests (9F9C.c:3408 '& 3' -> unk_70; 4525 0x20 borrow-motion; 4594 0x80 buffer the command
move; 4760 0x40 motion-frame bank / paired offset), so any one-meaning name fails another use.
Not a row. unk_04 is NOT named motion_entry_index (taken by the fighter field).

Names pending elsewhere are treated as taken; freshness: git grep -w of each new name over
src/, include/, asm/, tools/, engine/: 0 hits each.

APPLIER NOTE (hand edits and comments): src/main/28708.c:1429 and :1434 read 'p->w0->unk_04'
on lines that also use '.unk_04' of g_motion_*_sections (another type); the applier leaves
them UNRESOLVED (dry.txt exit=1): hand-edit only the 'p->w0->unk_04' token to
'p->w0->motion_bank_entry_index' (the sections' own unk_04 stays). Comments: include/game.h:
1391-1394 ('unk_00 / unk_02 are follow-up move ids, unk_07 / unk_08 frame bounds ..., unk_09
flag bits ...; unk_0A starts the command list': unk_00 is the id of an event-indexed move-id
TABLE, not a follow-up move, and unk_02 is the direct fallback move), game.h:1405 'row of the
bank's 4-byte entry table', game.h:1453 '/* current move; unk_08 bounds move_frame
(func_80058580) */'. The keycheck comment pass lists the rest.
"""
import csv
import os

VERDICT = ''
VERIFIER_NOTE = ''

N = 'MoveScript'

MEMBERS = [
    (N, 'unk_00', 'event_move_table_id',
     'u16 at +0x00. WHY: all five reads pass it to motion_ResolveMoveData(rec, id, '
     '&move_is_common) and then INDEX the returned u16 table: table[pending_move_event] '
     '(9F9C.c:4479-4481, event ids set in 175A4.c), table[0xD] (4497), table[0x17/0x18] '
     '(4624), table[0x18] (4645) and table[0x1D] (17AFC.c:232-233); each selected value is '
     'resolved again and installed with func_80021A98. So it is the (resolver-encoded) id of '
     'the per-move table mapping an event / condition index to the move id to switch to. '
     'Read-only: 17AFC.c:232, 9F9C.c:4479, 4497, 4624, 4645. COUNTER-EVIDENCE: it does not '
     'name the next move itself (an extra table lookup intervenes); the table indices are '
     'partly events (pending_move_event) and partly fixed condition slots, so "event" is the '
     'common sense (what keys the lookup); the id may be a resolver special id (0x7FF0..), '
     'still an id. No asm access (INCLUDE_ASM scan). ACCESSES: all C.'),
    (N, 'unk_02', 'fallback_move_id',
     'u16 at +0x02. WHY: both reads resolve it directly (no table) to the move installed when '
     'nothing more specific applies: 9F9C.c:4529 inside the "move_frame > last frame" branch '
     '(no queued move, first command is not the 0x2000 special transition) and 9F9C.c:4632 '
     'when func_80023DB8 succeeds, the height test fails and unk_72 == 0 (landing with no '
     'pending jump). Both then call func_80021A98. Read-only. COUNTER-EVIDENCE: the second '
     'path is not strictly an end-of-move path, so an "end/next" name would overstate; '
     '"fallback" holds for both (default when no event/command redirected). ACCESSES: all C.'),
    (N, 'unk_04', 'motion_bank_entry_index',
     'u16 at +0x04. WHY: func_80021A98 (9F9C.c:3363) copies it to the fighter field '
     'motion_entry_index and indexes the common or fighter section 4-byte entry table with '
     '[v1 * 2] (3366-3372), taking motion_entry and motion_metadata = base + entry[1]; '
     '28708.c:1429 and :1434 (Rec1C.w0 = cur_move) do the identical [w0->unk_04 * 2] / '
     'entry[1] to restore motion_metadata. Read-only. COUNTER-EVIDENCE: an entry index, not a '
     'byte offset, move id or frame; the fighter copy of the same value is motion_entry_index '
     '(that name is taken, hence the bank qualifier: the bank is the common / per-slot '
     'fighter section chosen by the move_is_common flag). ACCESSES: all C.'),
    (N, 'unk_07', 'last_move_frame',
     'u8 at +0x07, compared with Unk80101EC8Record.move_frame. WHY: 9F9C.c:4507 '
     '"unk_07 < move_frame" is the end-of-move test that installs the queued move, the '
     'special transition or the fallback (so unk_07 is the last frame the move plays); '
     '4712-4713: for move kinds 0x17/0x18/0x28/0xA, move_frame == unk_07 with rising vy '
     'rewinds move_frame to unk_07 - 1 (holds the last frame while airborne). Read-only; '
     'these are the only three uses. COUNTER-EVIDENCE: not necessarily the last frame of the '
     'animation data (motion_metadata[1] bounds decoding separately); other transitions '
     '(commands, events) can replace the move earlier; it is the last frame of the move '
     'script. ACCESSES: all C.'),
    (N, 'unk_08', 'last_command_frame',
     'u8 at +0x08, compared with move_frame. WHY: fighter_Update scans the command list only '
     'while "unk_08 >= move_frame" (9F9C.c:4551) and activates an already queued move once '
     '"unk_08 < move_frame" (4536); fighter_BuildCpuPad tests it against motion_metadata[1] '
     '- 1 and move_frame (4510C.c:616, 700) and func_80058580 plans around it (1833: frame == '
     'it - 1, 1871: within 2 frames of it). All use it as the last frame at which commands '
     'count. Read-only. COUNTER-EVIDENCE: not the first command frame; queued commands take '
     'effect after it, so it is the end of the command window rather than of the move '
     '(unk_07 is the move own last frame; the two are independent bytes). ACCESSES: all C '
     '(9F9C.c:4536, 4551; 4510C.c:616, 700, 1833, 1871).'),
    (N, 'unk_0A', 'transition_commands',
     'u16 [1] at +0x0A, start of a variable-length zero-terminated halfword stream. WHY: '
     'motion_FindPairedMoveEntry walks it (2 halfwords per entry, 4 when flags & 0xC000, '
     'returning the first 0x4000 paired entry; 9F9C.c:2579); fighter_Update reads entry 0 '
     'directly (4515, 4516, 4522: 0x2000 special transition, 0x1000 borrow-motion, [1] = move '
     'id) and walks the stream testing pad conditions and fighter masks, then installs / '
     'queues [1] (4552-4600); fight_UpdatePairedMoves consumes the returned paired entry '
     '(2684-2709). Every consumer reads it as a list of (flags, move id, ...) entries that '
     'trigger move transitions. Read-only (address-of use at 2579, 4552). COUNTER-EVIDENCE: '
     'entries are not only pad commands (automatic 0x2000 and paired 0x4000 entries exist), '
     'hence "transition"; the declared [1] is only the stream start. ACCESSES: all C.'),
]


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    with open(os.path.join(here, 'member_manifest.csv'), 'w', newline='\n') as f:
        w = csv.writer(f, lineterminator='\n')
        w.writerow(['addr', 'current_names', 'proposed_name', 'kind', 'evidence', 'verdict',
                    'verifier_note'])
        for t, m, n, ev in MEMBERS:
            w.writerow(['member', '%s.%s' % (t, m), n, 'member', ev, VERDICT, VERIFIER_NOTE])
    with open(os.path.join(here, 'pairs.txt'), 'w', newline='\n') as f:
        for t, m, n, ev in MEMBERS:
            f.write('%s.%s=%s\n' % (t, m, n))


if __name__ == '__main__':
    main()
