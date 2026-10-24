"""wave78 rows (pilot-menus-trace): 2 RENAMEs (0x80069898 gpu_AddShadedHorizontalLine dropped at
pre-apply, R4: three flat TILEs, no LINE/Gouraud primitive) re-derived from live code (Codex w78 research; 58 other
596CC.c / 64FD8.c functions stay neutral). TRACE_USED marks rows whose evidence cites a runtime trace
note (Q124: code supports, trace only corroborates).
"""

VERDICT = "CONFIRM"
VERIFIER_NOTE = ("naming-reviewer pre-apply (round 1, 1/3 PASS): menu_GetSelPrimBuffer PASS (pure parity-indexed "
                 "getter of D_800A35D8, set only by menu_InitSelScreen via gpu_InitPrimBufferPair, read only by "
                 "menu_StepSelScreen). gpu_AddShadedHorizontalLine FAIL (dropped).")

NOTES = {
    "0x8006CFBC": ("suggested: naming-reviewer pre-apply round 1 failed menu_DrawRowBitsAndFindEmptyRows (N1/R4: "
                   "the result is a per-side mask read per side at 596CC.c:1983) and suggested "
                   "menu_DrawRowBitsGetEmptyRowSides; R6 must judge it fresh at every use."),
}

TRACE_USED = {
    '0x8006CFBC': True,
    '0x80077098': True,
}

FUNCS = [
    ('0x8006CFBC', 'menu_DrawRowBitsGetEmptyRowSides', 'RENAME',
     'BODY: src/main/596CC.c:2047-2145 loops var_3 = 0..2 over three row bytes D_800A3524->unk17[var_3] (low nibble = side 0, high nibble = side 1, include/game.h:485); for each of 4 columns and 2 sides it draws sheet table[column + 8] when the bit is set (gpu_AddSpriteCells, x advanced by 23 per drawn sheet via counts.count[side]); when a side drew nothing on that row it ORs 1 << side into result and draws the empty-row sheet table[12]; then per side it draws a status sheet (table[17] if D_800A34FC->unk_28.word == 0x50005, else table[19] if that side bit is set in result, else table[18]); then a DR_MODE for table[14] at OT 8 and func_80069898(arg0, rect, 0x11); returns result. The return is a 2-bit mask: bit i = side i has at least one empty row (not one bit per row, not "side empty"). '
     'CALLER / RETURN USE: sole caller menu_UpdateRowBitMenu (src/main/596CC.c:2155, asm/funcs/menu_UpdateRowBitMenu.s:31 jal) does r = func_8006CFBC(sp10) and passes (r << 16) >> 16 as arg2 of menu_HandleRowBitInput (:2156). There, :1983 lim = ((arg2 >> i) & 1) ? 4 : 5 caps the side-i cursor unk_28.half[i] at 4 when that side has an empty row, so the 5 position (the 0x50005 accept state tested at :1973) cannot be reached; the cursor steps at :1985/:1992 wrap within 0..lim. '
     'WHY: Draw = the set-bit sheets, empty-row and status sheets, DR_MODE and line; RowBits = the unk17 row bytes it reads; GetEmptyRowSides = it computes and returns a per-side mask (bit i = side i has at least one empty row), which its only consumer reads per side at :1983. Reviewer-suggested name (pre-apply round 1, replacing menu_DrawRowBitsAndFindEmptyRows, which failed N1/R4 because the result is per side, not per row). menu_ rests on the sole caller menu_UpdateRowBitMenu (an existing menu_ name) and on the input path it feeds: menu_HandleRowBitInput holds the cursor-plus-accept input (cursor steps :1985/:1992, accept bits 0x400040 at :1973 and 0x40 at :2012/:2023 with sound, menu_SetExclusiveRowBit at :2008). The body itself has no input; the prefix is justified by that single menu path, not by shared plumbing. '
     'COUNTER-EVIDENCE: wave03/func_manifest.csv:129 rejected draw_anim_obj (unsupported animation, omitted the status result); this name claims no animation and includes the mask result. The name does not list the status sheet or the trailing line; Draw covers them, GetEmptyRowSides names the non-draw effect. The loop variable called row in the body indexes sides, which the name does not reuse ("Rows" = the three unk17 bytes). Only one caller, so no other path contradicts menu_. '
     'PRIOR: wave03 draw_anim_obj rejected (above). '
     'TRACE (corroborates only, Q124: code supports, trace corroborates): docs/naming/runtime/evidence/main__596CC.tsv:33 hit in 1/18 captures (menu-tour-hitmap-1 phases t_key, key_config, t_key_exit; capture b328b6f997bb), armed and never hit in the other 17, status capture_only+phase_only. It shows the function runs in the key-config input menu path; it does not establish button or action semantics, and the name stands on the code alone.'),
    ('0x80077098', 'menu_GetSelPrimBuffer', 'RENAME',
     'BODY: src/main/64FD8.c:1065 `return &D_800A35D8[a0];`; asm/funcs/func_80077098.s is a stride-0x2C index (a0*11*4) added to the gp-relative D_800A35D8 with no writes and no calls: a pure indexed getter. D_800A35D8 (static Unk8006E49CRec *, 64FD8.c:18) is stored only in menu_InitSelScreen (64FD8.c:1080, asm menu_InitSelScreen.s:18) from its arg0 and passed to gpu_InitPrimBufferPair (:1086), which (596CC.c:2558-2586) fills elements [0] and [1] each with unk_00..unk_20 sub-buffer pointers (a pair of primitive buffers); no other reader exists (grep over src/, include/, asm/funcs, named_syms.txt, symbol_addrs.txt). '
     'CALLER / CONSUMPTION: sole caller menu_StepSelScreen (64FD8.c:1237, asm menu_StepSelScreen.s:25 jal): p = func_80077098(temp_v1 & 1) (frame parity), stored in SELWORK->f2C, then :1239-1247 unpack p->unk_00/04/08/10/0C/14/18/1C/20 into s.unk_04 (primitive cursors) and s.unk_24, and :1248 passes &s to menu_RunSelPhases; the draw helpers it dispatches advance them (e.g. 64FD8.c:208-212 unk_04.unk_0C sprite cursor and unk_04.unk_14 DR_MODE cursor, :269-270 unk_04.unk_00). '
     'WHY: Get = pure getter; SelPrimBuffer = the primitive-buffer record of the Sel screen (the array menu_InitSelScreen sets up through gpu_InitPrimBufferPair, indexed by parity). menu_ and Sel come from the path, not a family prefix: the only user is menu_StepSelScreen and the data is created in menu_InitSelScreen, both existing Sel-screen names; menu_RunSelPhases drives the screen input (64FD8.c:1188 calls func_80075670, whose own path has a directional step through func_800692C0 at :472 and an accept bit 0x40<<16*side with sound at :481/:488, back bit 0x10 at :491). '
     'COUNTER-EVIDENCE: wave02/func_manifest.csv:137 reset setup_helper (unsupported prefix); this name is not that alias, it derives from initializer and consumer. It makes no group-only or character-only claim. It returns the address of a record (one element of the buffer-pair array), not a buffer; "PrimBuffer" refers to that element as gpu_InitPrimBufferPair fills it. '
     'PRIOR: wave02 setup_helper reset (above). '
     'TRACE (corroborates only, Q124: code supports, trace corroborates): docs/naming/runtime/evidence/main__64FD8.tsv:16 hit in 1/18 captures (group-battle-hitmap-1 phases t_group, group_settings, school_select, t_char, char_select, t_weapon, weapon_select, t_roster; capture 59c43080d1a3), armed and never hit in the other 17, status capture_only+phase_only. It shows the getter runs throughout the selection path; the name stands on the code alone.'),
]

DATA = []
