"""wave70 rows: the unnamed functions of 368E4.c that Codex researched (tmp/codex/w70-research.md:
8 RENAME leads, 32 NO_NAME), re-derived from the code at main f00e4da2b. Codex names were leads only.
No kept address is another tmp/p3w* draft's. 5 RENAME. No new prefix (motion_, camera_, node_, snd_
are admitted).

Pre-apply review pending (VERDICT / VERIFIER_NOTE empty).

Not named (dropped Codex leads):
- 0x80046E18 (node_GetSlotNodes): the slot record holds two Unk80045878Node arrays (unk_2C[21] and
  unk_8B4[20], game.h:2062-2063), so "SlotNodes" does not single out the unk_2C one; its twin
  func_80046DEC returns unk_1994 the same way; no caller anywhere (src, asm/funcs, asm/data), so no
  use bears a distinguishing noun out.
- 0x80047D94 (wave_SampleOuterRow) / 0x80047E5C (wave_SampleOuterRowDelta): no caller in src, asm
  or data; the coordinate is scaled by 3200 while wave_Draw spaces the grid by 2000 (368E4.c:947),
  the weight is the parity bit (&1), not a fraction, and the read runs past the row into
  D_800EF800; "Outer" is a guess for row 8 ("last" is the same claim). The sampled quantity (height?
  offset?) is unproved, so the wave_ prefix rests on table proximity only.
- The 32 NO_NAME rows stay neutral (not re-mined; Codex's reasons stand).
"""

VERDICT = "CONFIRM"
VERIFIER_NOTE = 'naming-reviewer pre-apply (round 1, 2/5): motion_FreePlaybackBlock and node_InitIdentity CONFIRM; 0x80047210 / 0x800472B0 FAIL (HalfY is a partial mechanism detail, the matrix D_800A3778 has no reader) and 0x80049C24 FAIL (nothing selects the D_800A33E8 pair) -- left neutral'

FUNCS = [
    ('0x80046A60', 'motion_FreePlaybackBlock', 'RENAME',
     'BODY 368E4.c:397: arena_Free(0xA). The only user of that arena block is func_800469C4 '
     '(368E4.c:380-394: arena_OpenBlock, ndata_ReadChecked, set end, move callback '
     'func_80046A80 = motion_RebasePlayback(a1), 368E4.c:399); its only caller is '
     'motion_StartPlayback (43500.c:585), which stores the returned block as the playback source '
     's->unk2C and reads the header from it (43500.c:586-588). Only caller of func_80046A60: '
     'func_800548DC (43500.c:646, from 2B344.c:556/:748/:1093), after DrawSync and '
     'func_8004659C(-1). WHY: frees the block motion_StartPlayback loaded; motion_ is the playback '
     'subsystem that loads, rebases and owns it. COUNTER-EVIDENCE: motion_StartPlayback can also '
     'be given an external buffer (a6 != 0, 43500.c:577-579), in which case this block is not '
     'involved; the name says "the playback block", not "the playback source", and it does not '
     'claim the function resets playback state (the caller does DrawSync and another call). The '
     'neighbouring arena_Free(8) (func_80046914) frees a different block that '
     'motion_StartPlayback also frees, so its name stays neutral. FRESH: motion_FreePlaybackBlock '
     'absent from src/, include/, named_syms.txt, symbol_addrs.txt and every tmp/p3w* draft. '
     'CENSUS NAME (draft time): func_80046A60 (AUTO, splat-auto).'),
    ('0x800472C0', 'node_InitIdentity', 'RENAME',
     'BODY 368E4.c:613-631 (Unk80101DF0Record *node): sets the header (unk0/1/2 = 0, unk4 = 8, '
     'unk8 = 0, unkC = 0, unkA = 4), xf.rot = 0, calls D_800F66A0[unk8 == 0](&rot, &work) (the '
     'ZYX handler, game.h:1954; zero angles give the identity matrix), work.t = 0, then '
     'xf.mat = work. Only caller: func_80047570 (368E4.c:668), which then sets unk8 = 4. WHY: the '
     'transform it leaves is the identity (rotation and translation zero) on the node-record '
     'layout node_UpdateWorldMatrix / node_SetMatrixFromNegatedRot use; node_ is that record. '
     'COUNTER-EVIDENCE: it also writes header constants (unk4, unkA) and leaves unk6 and padding '
     'untouched, so "Init" is the whole start state, not a memset; the other node initialisers '
     '(32D04.c:1210, 368E4.c:843, :1922, :2039) copy source transforms or set up a wave or draw '
     'node and do not run the zero-angle handler. FRESH: node_InitIdentity absent from src/, '
     'include/, named_syms.txt, symbol_addrs.txt and every tmp/p3w* draft. CENSUS NAME (draft '
     'time): func_800472C0 (AUTO, splat-auto).'),
]

DATA = []
