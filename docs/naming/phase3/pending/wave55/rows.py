"""wave55 rows: 32D04.c (23 func_* in the census at 11fbe8743: 22 with a 32D04.c location plus
func_800433E4, whose old-style definition at 32D04.c:557 leaves its census location blank),
researched by lane-debt at main 11fbe8743, citations refreshed at ddc49ffd3. No wave47 / wave48 row covers this range (tmp/p3w47
has no 0x80042..0x80044 address; there is no tmp/p3w48), and waves 51-54 do not touch it.

Existing prefixes used: `math_` (math_RgbToHsv, math_RotMatrixZXY / YXZ / XYZ in this file) and
`gpu_` (gpu_OffsetTexPrimStream and the gpu_OffsetTexPoly* / TPage / Clut helpers here).

PREFIX `reloc_` (naming-bar N4; admitted by this wave's pre-apply review and already landed by
wave57 for 596CC.c's reloc_RelocateOffsetList / reloc_RelocateSelfOffsets, stored offsets
turned into addresses). These rows use it for the relocatable offset blocks of the table D_80103608 /
D_80103658 (include/bb2.h:487-496; 20 entries, func_80044498 clears 20 counts). A block is a
count word followed by block-relative word offsets (the entries' starts) and an end offset; the
data follows. The prefix is explained by the data and by every user of the table:
- func_80044010 (reloc_RelocateBlock) turns a block's offsets into addresses by adding its
  base (unless header bit 15 already marks it relocated) and records it as table entry i;
  func_80044098 subtracts the base
  again; func_80044100 adds a byte delta to entry i's addresses when its memory moves. Its
  callers are the arena move callbacks (func_80046048, set by arena_SetMoveCallback(6) at
  35000.c:763; func_800466C0, set at 368E4.c:174; func_800468DC, set at 368E4.c:350;
  func_80045AA4, set at 35000.c:544 / :567) and func_80044C70, which func_800466C0 calls
  (368E4.c:267), all with the delta arena_ShiftBlocks hands a callback (35000.c:370).
- func_80044378 / func_8004428C build such a block out of a selection of another block's entries,
  and func_80045B68 then relocates the result (35000.c:720-726: func_80044378(...) then
  func_80044010(dst, 6); func_8004428C(...) then func_80044010(last, 6)); func_80044170 is the
  varargs form of the same compaction (no caller).
- Readers: gpu_OffsetTexPrimStream walks entry i's addresses as primitive streams (via
  func_800432A0 / func_800433E4), func_8003FA24 / 2B344.c:2508 read u16 data through them, and
  the canonical-asm bodies func_8004A4E0 / func_8004A940 / func_8004C404 / func_8004DA74 read
  D_80103608 (2 references each).
`reloc_` claims only the relocation and the block format, not what the entries hold.
(`reloc` is also a word in tools/libscan for ELF relocations; no identifier is shared.)

Not named (neutral):
- 0x8004283C / 0x80042864: set / return the flag D_800F6650 (cleared at boot, 368E4.c:442); no
  caller of either and no reader of the flag, so its meaning is not established.
- 0x800430E4: toggles scratchpad word 8 (1 <-> 2), copies D_800FF610 into a scratchpad MATRIX with
  row 1 halved, selects D_800F62E0[arg2] and one of the primitive handler tables D_80095280 /
  D_800951D8 by arg3[1] bit 0, and calls the unnamed func_8004DDB4; its single caller
  func_8004A940 is unnamed, so what it draws is not established.
- 0x80043244 (maps a value to 0..3 at the thresholds 0x5A01 / 0xB501 / 0x16A0A, steps of
  sqrt 2) and 0x80043278 (decodes a value scaled by scratchpad word 8 with an 11-bit mantissa):
  no callers; roles not established.
- 0x800444BC / 0x800444E0 (identical wrappers of func_80044504(D_800A378C)) and 0x80044504 (builds
  D_800FF610 from the D_800A3678 angles and the D_800A3708 / D_800A370C transform records, sets
  the scratchpad render parameters, then runs the unnamed func_8004A4E0 / func_80046E54): the
  pass it runs is not established from named callees.
- 0x80044650: the default entry of the primitive handler table D_800951D8 (asm/data/7D920.data.s
  :10275..), which calls func_80052C10 (a store to 0x1F800400, one past scratchpad); the trap's
  purpose is not established.
- 0x80044670 / 0x8004473C: set up the D_800A9CF8 node table (count by the func_80046798 id: 7 ->
  0x11, 4 -> 2, 0x12 -> 0xA) and initialise its nodes; the record and the id are unnamed, so the
  subsystem is not established.

APPLIER: the dry run also substitutes 6 sites in tools/test_board_cards.py:321-326, where
func_80044378 is a synthetic disassembly-fixture label (any name works there); named_syms.txt
gets 1 substitution. Comments that name the old functions and need a post-apply check:
named_syms.txt:205-206, include/bb2.h:487-494 and :814, include/game.h:1932 and :1993.

Pre-apply review round 1: 10 PASS / 2 FAIL; the reloc_ prefix passes. The two FAILs are renamed
per the reviewer: 0x80044010 reloc_SetBlock -> reloc_RelocateBlock ('Set' missed the
offset-to-address rewrite, 32D04.c:909-911) and 0x80044100 reloc_ShiftBlock -> reloc_RebaseBlock
(it only rebases pointers after arena_ShiftBlocks has moved the bytes).
"""

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS (round 1, 10/12; 2 renamed per reviewer)"

FUNCS = [
    ('0x80042504', 'math_HsvToRgb', 'RENAME',
     'BODY 32D04.c:11-78 (s32 *hsv, s32 *rgb): reads H, S, V (4.12); forces S = 0 when '
     'func_800486FC() (the grayscale flag g_color_mode, 368E4.c:1408-1415) is set; S = 0 gives '
     'r = g = b = V; otherwise the six-sector HSV -> RGB formula (sector = H * 6 >> 12, p = '
     'V(1 - S), q = V(1 - S f), t = V(1 - S (1 - f))) and writes r, g, b. Callers: none in '
     'code or data. WHY: the inverse of math_RgbToHsv (32D04.c:84, same 4.12 layout, its '
     'comment at :80 says so); the formula is the standard conversion. COUNTER-EVIDENCE: the '
     'grayscale override is a mode check on top of the conversion; it does not change what the '
     'function computes for S = 0. CENSUS NAME (draft time): func_80042504 (AUTO, splat-auto).'),
    ('0x80042E90', 'math_InitRotMatrixFuncs', 'RENAME',
     'BODY 32D04.c:398-404: stores math_RotMatrixZYX, math_RotMatrixZXY, math_RotMatrixYXZ and '
     'math_RotMatrixXYZ into D_800F66A0[0], [2], [4], [5]. Caller: func_80046B44 (368E4.c:426, '
     "the boot init sequence). D_800F66A0 is the RotMatrixFunc table a transform node's unk8 "
     'indexes (include/game.h:1931-1937; callers at 2B344.c:2134, 31D3C.c:131/151, '
     '35000.c:141, 368E4.c:624). WHY: initialises the rotation-to-matrix function table. '
     'COUNTER-EVIDENCE: [1] and [3] are not written; no node uses them. CENSUS NAME (draft '
     'time): func_80042E90 (AUTO, splat-auto).'),
    ('0x800432A0', 'gpu_OffsetBlockTexPrimStreams', 'RENAME',
     'BODY 32D04.c:539-547 (block, du, dv, clut dx, clut dy): for each of the '
     'D_80103658[block] entries of D_80103608[block], stores the entry address to the '
     'scratchpad cursor and calls gpu_OffsetTexPrimStream(du, dv, clut dx, clut dy). Callers: '
     'func_8003E164 / func_8003E22C (2B344.c:1669-1689), func_80040594 (309CC.c:234, :242), '
     'func_80043398 (32D04.c:550). WHY: shifts the texture page and CLUT of every primitive '
     "stream of one relocated block (gpu_OffsetTexPrimStream's own comment, 32D04.c:577-582). "
     'COUNTER-EVIDENCE: none. CENSUS NAME (draft time): func_800432A0 (AUTO, splat-auto).'),
    ('0x80043398', 'gpu_OffsetBlockTexPrimStreamsByPages', 'RENAME',
     'BODY 32D04.c:549-552: calls func_800432A0(block, a1 << 6, a2 << 8, a3 << 6, a4 << 8). '
     'gpu_OffsetTPage takes pixel offsets and shifts the page by x >> 6 and y >> 8 '
     '(32D04.c:872-879), so a1 / a2 count 64-pixel / 256-line texture pages; the CLUT offset '
     'is scaled by the same 64 / 256. Caller: func_80040594 (309CC.c:247, (block, 2, 0, 2, '
     "0)). WHY: the block offset in page units. COUNTER-EVIDENCE: the CLUT's 64 / 256 scale "
     "matches a page step but a CLUT has no page of its own; 'Pages' describes the shared "
     'unit. CENSUS NAME (draft time): func_80043398 (AUTO, splat-auto).'),
    ('0x800433E4', 'gpu_OffsetBlockTexPrimStream', 'RENAME',
     'BODY 32D04.c:557-566 (block, entry, du, dv, clut dx, clut dy; old-style definition): '
     'stores D_80103608[block][entry] to the scratchpad cursor and calls '
     'gpu_OffsetTexPrimStream(du, dv, clut dx, clut dy). Caller: func_80045B68 '
     '(35000.c:737-746, block 6, entries y * 2). WHY: the single-entry form of func_800432A0. '
     'COUNTER-EVIDENCE: none. Census location is blank (old-style definition); the definition '
     'is 32D04.c:557. CENSUS NAME (draft time): func_800433E4 (AUTO, splat-auto).'),
    ('0x80044010', 'reloc_RelocateBlock', 'RENAME',
     'BODY 32D04.c:896-913 (s32 *block, s16 i): sets header bit 15 (keeping the low halfword); '
     'records block + 1 in D_80103608[i] and the count (header & 0x7FFF) in D_80103658[i]; if '
     "bit 15 was clear, adds the block's base to each of the count words. Callers: "
     'func_80040594 (309CC.c:219-220), func_80045B68 (35000.c:723, :726), func_800460E4 '
     '(368E4.c:144, :156, :166), func_800464C4 (:206), func_8004659C (:240), func_800467B8 '
     '(:345). WHY: relocates block i in place, rewriting each block-relative offset as an '
     'address (32D04.c:909-911), and records it as table entry i (name per the pre-apply '
     'review). COUNTER-EVIDENCE: an already-relocated block is only re-recorded. CENSUS NAME '
     '(draft time): func_80044010 (AUTO, splat-auto).'),
    ('0x80044098', 'reloc_UnrelocateBlock', 'RENAME',
     "BODY 32D04.c:915-940 (s16 i): when entry i's header has bit 15, clears it and subtracts "
     "the block's base from each of the count words, turning addresses back into offsets. "
     'Callers: func_8004659C (368E4.c:239, before re-recording block 7 at :240). WHY: the '
     'inverse of func_80044010 (its comment, 32D04.c:895). COUNTER-EVIDENCE: the table entry '
     'itself is left in place. CENSUS NAME (draft time): func_80044098 (AUTO, splat-auto).'),
    ('0x80044100', 'reloc_RebaseBlock', 'RENAME',
     'BODY 32D04.c:942-954 (s32 i, s32 delta): moves D_80103608[i] by delta / 4 words and adds '
     'delta to each of its D_80103658[i] addresses. Callers: the arena move callbacks '
     'func_80046048 (35000.c:774, block 6), func_800466C0 (368E4.c:258, 7; :270, 8), '
     'func_800468DC (368E4.c:361, 9), func_80045AA4 (35000.c:610-611) and func_80044C70 '
     '(35000.c:150, D_800A9CF8.unk0; called by func_800466C0 at 368E4.c:267), each passing the '
     "delta arena_ShiftBlocks hands its callback (35000.c:370). WHY: rebases the block's "
     'addresses after arena_ShiftBlocks has already moved its bytes; it moves no memory itself '
     '(name per the pre-apply review). COUNTER-EVIDENCE: func_800466C0 passes only the index '
     'at 368E4.c:270 (include/bb2.h:813-815); its own a1, the delta, is still in place. CENSUS '
     'NAME (draft time): func_80044100 (AUTO, splat-auto).'),
    ('0x80044170', 'reloc_KeepListedEntries', 'RENAME',
     'BODY 32D04.c:960-1004 (s32 *block, count, index...): sets the header to count; for each '
     "listed index (func_80052C10 when it is not below the old count) copies that entry's "
     'bytes (offset[index]..offset[index + 1]) down to the write cursor with func_800520B8 '
     '(forward word copy), stores the new offset and advances; writes the end offset and '
     'returns the end address. Callers: none in code or data. WHY: compacts a block in place '
     'to the listed entries, keeping the block format. COUNTER-EVIDENCE: never called; named '
     'by its body. CENSUS NAME (draft time): func_80044170 (AUTO, splat-auto).'),
    ('0x8004428C', 'reloc_KeepSelectedEntries', 'RENAME',
     'BODY 32D04.c:1006-1056 (s32 *block, s16 *sel): walks sel to its -2 terminator; for each '
     'sel entry >= 0 copies that block entry down to the write cursor (func_800520B8), stores '
     'its new offset and counts it; writes the count to the header and the end offset, returns '
     'the end address. Caller: func_80045B68 (35000.c:725, then func_80044010(last, 6) at '
     ':726), also asm func_80045B68.s:213. WHY: compacts a block in place to the selected '
     'entries. COUNTER-EVIDENCE: none. CENSUS NAME (draft time): func_8004428C (AUTO, splat- '
     'auto).'),
    ('0x80044378', 'reloc_CopySelectedEntries', 'RENAME',
     'BODY 32D04.c:1058-1118 (src, s32 *dst, s16 *sel): counts the sel entries >= 0 before the '
     '-2 terminator, writes that count to dst[0], then copies each selected src entry into dst '
     'after a (count + 2)-word header with func_800520B8, writing the dst-relative offsets and '
     'the end offset; returns the end address. Caller: func_80045B68 (35000.c:722, then '
     'func_80044010(dst, 6) at :723), also asm func_80045B68.s:206. WHY: builds a new block '
     'from the selected entries of another. COUNTER-EVIDENCE: none. CENSUS NAME (draft time): '
     'func_80044378 (AUTO, splat-auto).'),
    ('0x80044498', 'reloc_ClearCounts', 'RENAME',
     'BODY 32D04.c:1122-1128: zeroes 20 halfwords down from D_8010367E, i.e. '
     'D_80103658[0..19], the entry counts of the table (named_syms.txt:206). Caller: '
     'func_80046B44 (368E4.c:427, boot init, after math_InitRotMatrixFuncs). WHY: empties '
     'every table entry; with count 0 func_800432A0 and func_80044100 touch no entry. COUNTER- '
     "EVIDENCE: the D_80103608 pointers are left as they are; 'Counts' says what is cleared. "
     'CENSUS NAME (draft time): func_80044498 (AUTO, splat-auto).'),
]

DATA = []
