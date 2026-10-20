"""p3m11 rows: member renames in Unk80101DF0Record (transform node, include/game.h:1919, embedded
at offset 0 of Unk80045878Node / Unk800A6690Rec / Unk800A9CF8Entry and standalone in D_80101DF0,
D_800FF638, D_800EEDF0, D_800EF070) and MoveScript.unk_06; wave m11. Leads:
tmp/codex/m11-research.md. Every access re-derived at main 82a4b0fe0 from the compiler-driven
list (tools/naming_member_apply.py --dry-run tmp/p3m11/pairs.txt, output tmp/p3m11/dry.txt: 73
error lines in round 0 for the node members + 1 for MoveScript, 0 by round 2; accesses are in
2B344.c 309CC.c 31548.c 31CFC.c 31D3C.c 32D04.c 35000.c 368E4.c 9F9C.c) and by scanning every
INCLUDE_ASM body (71, asm/funcs) that touches a node pointer or the node tables: func_8004A4E0
(offsets 0/1/2/4 only), func_8004A940 ($s2 = queued node: +0xA light index, +0xC parent; its
$s6 reads at 6/8/A/C are the D_800FF610 MATRIX copy, not a node), func_8004DA74 /
func_8004DDB4 / func_8004C404 (D_800A3708 -> +0x2C = xf.mat.t only). The other 3AB48.c
handlers take other queue-record layouts. No asm reads +6/+8; +0xA/+0xC reads are below.

Dropped vs Codex: none of the five rows. MoveScript.unk_04 motion_entry_index excluded by
request (pending in another wave).

Names pending elsewhere are treated as taken; freshness: git grep -w of each new name over
src/, include/, asm/, tools/, engine/: 0 hits each.

APPLIER NOTE (comments to follow up): include/game.h:1904-1911 ('unk8 = 5', 'unk8 = 2',
'D_800F66A0[unk8]', 'reads unk6 as a state'); game.h:1925 '/* +0x08 D_800F66A0 index */';
game.h:1957-1967 ('a transform node's unk8 selects', 'set unk8 to 0, 2,', 'index it by unk8',
'D_800F66A0[unk8]'). The keycheck comment pass lists the rest.
"""
import csv
import os

VERDICT = ''
VERIFIER_NOTE = ''

N = 'Unk80101DF0Record'

MEMBERS = [
    (N, 'unk6', 'matrix_state',
     's16 at +0x06. WHY: node_UpdateWorldMatrix (31D3C.c:127-143) returns at 1, skips the angle '
     'conversion at 2 (work already supplied), builds work from xf.rot at 0, composes/copies to '
     'xf.mat and sets 1; it recurses into a parent whose value is 0 (:135). So 0 = xf.mat not '
     'current / work not built, 1 = xf.mat current, 2 = work supplied (31548.c:224 stores '
     'angle-built work then 2). Writers: 0 on init/invalidate (309CC.c:315, 31548.c:71/99/120, '
     '32D04.c:1217, 35000.c:48/81, 368E4.c:857/2057), 1 after xf.mat is filled directly '
     '(309CC.c:99/460 raw ent-0x52, 31548.c:254, 35000.c:140, 368E4.c:1987/2069). Only reader '
     'is node_UpdateWorldMatrix. No asm access. COUNTER-EVIDENCE: three values so not a '
     'bool; "state" is generic but the three values are enumerated and all concern the '
     'matrices (matrix_state = which of work / xf.mat is valid). ACCESSES: all C.'),
    (N, 'unk8', 'rotation_order',
     's16 at +0x08. WHY: every read is D_800F66A0[unk8](&xf.rot or negated copy, &work/&xf.mat) '
     '(2B344.c:2135, 31D3C.c:131/151, 35000.c:141, 368E4.c:628); func_80042E90 (32D04.c:399-403) '
     'installs [0] ZYX, [2] ZXY, [4] YXZ, [5] XYZ; stored values are 0, 2, 4 (368E4.c:673), 5 '
     '(2213), 2 (2229), i.e. only installed slots. So it picks the Euler-angle composition '
     'order of the rotation builder. COUNTER-EVIDENCE: it is a table index, not the order '
     'itself; the name says what the choice controls. Raw writes 309CC.c:458 (ent-0x50). No asm '
     'access (func_8003EDC0.s is not INCLUDE_ASM; C version at 2B344.c:2083). ACCESSES: all C.'),
    (N, 'unkA', 'light_setup_index',
     's16 at +0x0A. WHY: func_8004A940 (asm, $s2 = queued node) reads it at 8004AA28 / AC80+ / '
     'B004 / B0B0 / B164 / B318 / B564 and does idx*0x60 + D_800F62E0 (the 0x60-byte light-setup '
     'records, game.h light-setup typedef comment; light matrix at +0x18 passed to the GTE); at '
     'B0B0 it is the arg2 of func_800430E4, which does D_800F62E0[arg2] (32D04.c:495). '
     'func_8004C404 uses constant 4 = the default every constructor stores (2B344.c:2118, '
     '32D04.c:1210, 368E4.c:624/847/1926/2043) or 0 (309CC.c:101, 368E4.c:1989/2070). Other '
     'writers: 309CC.c:316 (a0->unk_10), raw 309CC.c:461, 368E4.c:1566 (arg0 slot id). '
     'COUNTER-EVIDENCE: some writers feed a slot/player-derived value; every reader treats it '
     'only as the D_800F62E0 index. ACCESSES: C writes as listed; asm reads in func_8004A940.'),
    (N, 'unkC', 'parent_node',
     'struct Unk80101DF0Record * at +0x0C. WHY: always a node pointer: &a0[6].node, '
     '&base[v1].node, &obj[-1].node, &slot_obj->unk_2C[12].node or 0 (309CC.c:98/305/307, '
     '368E4.c:856/1986/2056/2066); node_UpdateWorldMatrix composes work with parent->xf.mat, '
     'updating the parent first if its matrix_state is 0 (31D3C.c:134-139); 31CFC.c:17-19 '
     'rebases non-null values by delta; 368E4.c:1956-1962 adds parent->xf.mat.t; asm '
     'func_8004A940 (8004AE0C) loads it and passes parent +0x18 (= xf.mat) to '
     'math_ApplyLightPlaneProjection. COUNTER-EVIDENCE: pointer not index (template indices '
     'are converted at 309CC.c:305); the projection use is the same parent-transform role; '
     'not a sibling link. Raw write 309CC.c:459. ACCESSES: C + func_8004A940.s:314.'),
    ('MoveScript', 'unk_06', 'first_move_frame',
     'u8 at +0x06. WHY: sole access is func_80021A98 (9F9C.c:3387) reading it into a1_val and '
     'storing s0->move_frame = a1_val (:3395) on every move installation (move_frame is the '
     'current frame of the move, move_frame_frac is zeroed beside it); no other C or asm access '
     '(cur_move/MoveScript uses enumerated: unk_00/02/04/07/08/09/0A only). COUNTER-EVIDENCE: '
     'not necessarily 0; name says only the frame playback starts at. ACCESSES: R 9F9C.c:3387.'),
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
