"""wave47 rows: the effect-command core of 51268.c (eff_) and the motion-stream decoder of 9F9C.c
(motion_), re-derived from the current tree at main aa7566f06 (the old draft cited 032e23f8c; its
names were leads only). 6 RENAME functions + 4 RENAME data symbols.

Family: 0x80061064 keeps its sotn-review KEEP name eff_FrameUpdate (N5, an alias; not renamed
here) and 0x80061178 is eff_StopCommands (wave58); eff_StartCommand / eff_StepCommand /
eff_ResetCommands read beside them (FrameUpdate steps each running command with StepCommand;
StopCommands = ResetCommands + light off + countdown clear). Retired first-pass names
(anim_helper_80060A68, eff_helper_80060B70, mode_helper_8001979C / 800198D0, mario_test_init_
characters) are RESET history (wave02 / wave05) and are not reused.

Not named: nothing dropped from the old draft; its 0x80061064 eff_UpdateCommands is replaced by the
existing eff_FrameUpdate (KEEP). 0x80060CB8 eff_LoadImagesAndSetup (wave58) is untouched.

Pre-apply review pending (VERDICT / VERIFIER_NOTE empty).
"""

VERDICT = 'CONFIRM'
VERIFIER_NOTE = 'naming-reviewer pre-apply (round 1, 10/10): all 6 function rows (eff_StartCommand, eff_StepCommand, eff_ResetCommands, motion_InitStream, motion_DecodeFrame, motion_InitStreams) and 4 data rows (g_eff_command_phase, g_eff_command_status, g_eff_command_block, g_motion_decoders) PASS on its own enumeration at aa7566f06; R6 must also check engine/test_engine.py, engine/deferred.json and the 7 moved grind ledgers'

FUNCS = [
    ('0x80060A68', 'eff_StartCommand', 'RENAME',
     'BODY src/main/51268.c:80-102 (void; doc comment :70-79): for the command block D_800A3468 '
     'points at (idx = its low halfword unk00.h) clears D_800F10D0[idx]; copies the block\'s three '
     'words unk0C into unk20 and three halfwords unk10 into unk18, pointing D_800A347C / D_800A3478 '
     'at the copies; calls the handler D_8009BA7C[D_8009BA60[idx] + D_800F10D0[idx]] (phase 0, the '
     'command\'s first handler) and stores its result through unk14 (a D_800F1150 byte); sets '
     'D_800A32BC = 0xA when bit 21 of unk00.w is set. All 20 call sites (51268.c:298, :321, :335, '
     ':349, :363, :377, :400, :425, :450, :473, :497, :510, :518, :537, :552, :558, :562, :615, :658, '
     ':676 = asm jal in the 18 wrappers func_800611A4 .. func_80061EC0) first set D_800A3468 = '
     '&D_800F116C and fill its command word (0x..XXXX, idx in the low halfword), sources unk0C / '
     'unk10 and status slot unk14. Handlers advance D_800F10D0[idx] themselves (e.g. func_80064E90 '
     'sets D_800F10D0[4] = 1, 51268.c:1600; func_80066EC0 sets [2] = 1, :2294), so phase 0 is the '
     'start. WHY: begins one effect command: resets its phase to 0, hands it its position / angle '
     'copies and runs its first handler; eff_ rests on the module\'s own eff_ names (eff_Init '
     '6CF8.c:248 -> eff_LoadImagesAndSetup, eff_StopCommands, eff_BindScratchpad, eff_FrameUpdate) '
     'and the "eff prim over" message (51268.c:264). COUNTER-EVIDENCE: what each of the 28 command '
     'ids draws is not established, so the name claims only the start; the D_800A32BC side effect '
     '(light countdown on bit 21) is a secondary write not covered by the verb. PRIOR: wave02 '
     'RESET anim_helper_80060A68 -> func_80060A68 (first-pass tag, no semantic claim); no other '
     'proposal or rejection under docs/naming/phase3.'),
    ('0x80060B70', 'eff_StepCommand', 'RENAME',
     'BODY src/main/51268.c:104-128 (void): copies the block\'s s16 triple unk04[0..2] and s32 '
     'triple unk08[0..2] (the two arrays eff_BindScratchpad stored there from func_80061064\'s '
     'arguments) to D_800A346C / D_800A3470, calls eff_SetRotMatrix(dst16, dst_s32, D_800A3474) (a '
     'MATRIX; 51268.c:686), then calls the handler D_8009BA7C[D_8009BA60[idx] + D_800F10D0[idx]] '
     'for the command\'s CURRENT phase (idx = unk00.h) and stores the result through unk14. Single '
     'caller: func_80061064 (51268.c:261, asm/funcs/func_80061064.s:20), once per frame for each '
     'slot i whose status byte D_800F1150[i] is non-zero (it sets the block word to i first, '
     ':259-261). WHY: runs one frame of a running effect command: sets up the frame\'s rotation and '
     'runs the handler of the phase the command is in; eff_ as for eff_StartCommand, and it is '
     'eff_FrameUpdate\'s per-slot step. COUNTER-EVIDENCE: what a handler does per frame depends on '
     'the command; the name does not claim an animation. PRIOR: wave02 RESET eff_helper_80060B70 '
     '(first-pass tag, no semantic claim); no other proposal or rejection.'),
    ('0x80060C60', 'eff_ResetCommands', 'RENAME',
     'BODY src/main/51268.c:132-150 (void): for i in 0..0x1B zeroes D_800F10D0[i] (phase) and '
     'D_800F1150[i] (status); then zeroes D_800A345C[0..1], D_800A3454[0..1], D_800A344C[0..1], '
     'D_800A3460, D_800A3444, D_800A3448. Callers: eff_LoadImagesAndSetup (51268.c:187, '
     'asm/funcs/eff_LoadImagesAndSetup.s:73, the loader eff_Init calls) and eff_StopCommands '
     '(51268.c:279, asm/funcs/eff_StopCommands.s:4, itself eff_Reset + light_DisablePointLight + '
     'D_800A32BC = 0). WHY: returns the command tables to the not-running state (status 0 means '
     'func_80061064 steps no slot, :259) together with the seven per-effect statics the commands '
     'fill (D_800A3448 is the flare slot mask 51268.c:1044-1049; D_800A3460 the 6-bit slot mask '
     '8006288C uses). COUNTER-EVIDENCE: Reset vs eff_StopCommands: both clear the tables; Stop adds '
     'the light and countdown, so Reset is the table-clearing part, and the verb "Reset" does not '
     'claim the light. The seven non-table statics are unnamed. PRIOR: no earlier proposal; wave58 '
     'eff_LoadImagesAndSetup / eff_StopCommands evidence calls it func_80060C60.'),
    ('0x8001979C', 'motion_InitStream', 'RENAME',
     'BODY src/main/9F9C.c:64-84 (s32 arg0, u32 *ptr): base = &D_800F1B18[arg0] (Unk800F1B18Rec, '
     'include/game.h:1381-1387); stores base->stream = ptr; reads 63 12-bit values into pose[3..65] '
     'and 63 2-bit codes into code[3..65] from the stream (GETBITS, :43-57); sets the four cache '
     'slots\' frame = -2 (empty) and ctr = 0. Callers: fight_Init via func_80021210 (9F9C.c:3138, '
     ':3140, :3143), func_8003CF84 (2B344.c:1100-1102, asm/funcs/func_8003CF84.s:175/179/183, '
     'unconditionally), motion_UpdatePlayback (4510C.c:66, per object stream j, asm/funcs/'
     'motion_UpdatePlayback.s:56). The ptr arguments are g_motion_common_sections.unk_10 / '
     'g_motion_fighter_sections[i].unk_10 (set by motion_LoadFighterData, named_syms.txt:2220; '
     'game.h:755 "func_8001979C\'s motion bitstream") and motion_UpdatePlayback\'s unk34[j] '
     'per-object streams. WHY: initialises decoder slot arg0 on a motion bitstream: one word is '
     'the stream pointer, the rest the initial channel values and codes; motion_ is the admitted '
     'prefix of the sibling motion_LoadFighterData / motion_UpdatePlayback that supply and consume '
     'the same streams, and of the MotionFrame type (game.h:1342-1360) the sibling decodes into. '
     'COUNTER-EVIDENCE: the 63 channels are unnamed and "stream" is the bitstream (not a stream '
     'object of the PsyQ sense); the name claims init of the decoder on a stream, not the format. '
     'PRIOR: wave02 RESET mode_helper_8001979C -> func_8001979C; no other proposal.'),
    ('0x800198D0', 'motion_DecodeFrame', 'RENAME',
     'BODY src/main/9F9C.c:99-329 (s32 obj, s32 arg1, MotionFrame *out, u16 *work; doc comment '
     ':88-98): decodes entry arg1 of D_800F1B18[obj]\'s stream into the 0x84-byte MotionFrame out '
     '(COPY33), from its group of 8 (offset table at stream + 0x70) or from a cached predecessor, '
     'keeping the result in one of the record\'s four cache slots; fills work (pose +0x00, rates '
     '+0x84, codes +0x108). Callers: fighter_Update (9F9C.c:4785-4786, asm via fighter_Update.s:'
     '1194/1200, two frames at move_frame and next), replay_PlayFrame (28708.c:1391-1392, the '
     'recorded frame indices hE / h10 with the slot in the top bits), motion_UpdatePlayback '
     '(4510C.c:132, per object, result then scales a position). Every caller passes a slot index '
     'and a frame index and consumes a MotionFrame. WHY: decodes one motion frame; MotionFrame is '
     'the output type (game.h:1342). COUNTER-EVIDENCE: it also updates the record\'s cache and the '
     'work buffer; the verb Decode covers that as part of decoding. PRIOR: wave02 RESET '
     'mode_helper_800198D0 -> func_800198D0; no other proposal.'),
    ('0x80021210', 'motion_InitStreams', 'RENAME',
     'BODY src/main/9F9C.c:3137-3145 (void): func_8001979C(0, g_motion_common_sections.unk_10); '
     'then func_8001979C(1, g_motion_fighter_sections[0].unk_10) if D_800A38C4[0] != 0 and '
     'func_8001979C(2, g_motion_fighter_sections[1].unk_10) if D_800A38C4[1] != 0 (D_800A38C4: '
     'model id loaded per fighter slot, named_syms.txt:725). Single caller: fight_Init (9F9C.c:1860, '
     'asm/funcs/fight_Init.s:334), right before func_80021280(0) / (1). func_8003CF84 '
     '(2B344.c:1100-1102) makes the same three calls inline, unconditionally. WHY: sets up the '
     'decoder slots on the loaded motion data streams (the common one and each fighter\'s '
     'that is present), i.e. calls motion_InitStream per slot. COUNTER-EVIDENCE: the guard '
     'D_800A38C4 is a loaded-model test, not a stream test; the name claims the streams of the '
     'loaded data, which is what the guard selects. PRIOR: wave05 RESET mario_test_init_characters '
     '-> func_80021210; no other proposal.'),
]

DATA = [
    ('0x800F10D0', 'g_eff_command_phase',
     'RENAME. s32 D_800F10D0[0x1C] (extern include/bb2.h:595; table of 28 phases, one per '
     'command id). ACCESSES: C src/main/51268.c: eff_StartCommand W 0 and R (:85, :96) = handler '
     'index; eff_StepCommand R (:125); eff_ResetCommands W 0 (:134); handler writes of 1 or 2 at '
     'fixed indices (:876 [26]=1, :1027 [26]=0 on completion, :1042 [8]=1, :1241 [0], :1247 [1], '
     ':1444 [7], :1600-1716 [4..6, 9..19]=1, :2294-2384 [2], [3], [20..25] = 1 / 2). asm: the same '
     'stores in the INCLUDE_ASM handlers (func_8006288C [26]=1, func_8006295C [26]=0, '
     'func_80062FEC [8], func_80063AF0 [0], func_80063B34 [1], func_800645B0 [7], func_80064E90 .. '
     'func_800652AC [4..19], func_80066EC0 .. func_800671CC [2], [3], [20..25]; each `sw` of a '
     '0 / 1 / 2 constant to D_800F10D0 + 4*idx), and the reads in func_80060A68 / func_80060B70 / '
     'func_80060C60 (asm/funcs). Every access is an s32 index by command id of a small phase '
     'number. WHY: each effect command\'s phase, added to its handler base D_8009BA60[idx] to select '
     'the D_8009BA7C handler (51268.c:95, :125; named_syms.txt:1344, :1815). eff_ as for the '
     'functions. COUNTER-EVIDENCE: the values beyond 0 / 1 / 2 are not decoded; the high indices '
     '(22..25) are written 1 / 2 in pairs, consistent with phase. Retired element aliases '
     '(D_800F10E0 etc., named_syms.txt:914, :1295, :2198, :2252, :2382) are inside this array and '
     'are covered by the array name.'),
    ('0x800F1150', 'g_eff_command_status',
     'RENAME. u8 D_800F1150[0x1C] (extern src/main/51268.c:130). ACCESSES: eff_ResetCommands W 0 '
     '(:137); func_80061064 R (:259, step only non-zero slots) and its address as the block\'s '
     'result slot unk14 (:258); each wrapper sets D_800F116C.unk14 = &D_800F1150[n] (31 sites, '
     'e.g. :296, asm func_80061A3C.s:17/18 and +1 for the second slot via D_800F1151); eff_StartCommand '
     'and eff_StepCommand store the handler result through unk14 (51268.c:97, :127); wrappers test '
     'and clear pairs of slots (:310-317 [9]/[10], :389 [5]/[6], :462 [16]/[17], :486 [2]/[3], '
     ':545 [20]/[21], :602 [22]/[23], :647 [24]/[25], :665 [18]/[19]) and clear single slots '
     '(:415 [12], :420 [13], :440 [14], :445 [15]). Every access is a u8 per command slot. Offset '
     'aliases inside the array (D_800F1151, D_800F1152, D_800F1154, D_800F1158, D_800F1159, '
     'D_800F115B, D_800F115C, D_800F1160, D_800F1164, D_800F1168, D_800F116A in named_syms.txt) '
     'stay as they are. WHY: the per-command status byte the handler result is stored to; 0 is '
     'the state func_80061064 skips, so a slot is stepped only while its result is non-zero. '
     'COUNTER-EVIDENCE: the non-zero values are the handler\'s return, not decoded further, so '
     '"status" claims only role; handlers that return void leave whatever the compiler left in '
     '$v0. Old evidence file docs/naming/data_evidence/D_800F1150.md (g_satan1_gauge_buf) is the '
     'retired first-pass guess and contradicts the code (never applied).'),
    ('0x800F116C', 'g_eff_command_block',
     'RENAME. Unk1F800000Unk00 (0x2C bytes; extern src/main/51268.c:284; game.h:875-912 doc). '
     'ACCESSES: the 18 wrappers func_800611A4 .. func_80061EC0 set D_800A3468 = &D_800F116C '
     '(51268.c:292 ... :663) or take %lo(D_800F116C) in asm (func_800611A4.s:12 ...), then fill '
     'its command word unk00.w, sources unk0C / unk10 and status pointer unk14 before '
     'eff_StartCommand; the block\'s fields are read back only through D_800A3468 / D_800A3478 / '
     'D_800A347C (eff_StartCommand, the handlers). The per-frame steps use the scratchpad block '
     '(eff_BindScratchpad) instead. WHY: the command block a new effect command is started from. '
     'COUNTER-EVIDENCE: the type is shared with the scratchpad block, so "command block" is the '
     'role of this instance. Sub-field aliases D_800F1178 / D_800F117C stay.'),
    ('0x800F1B18', 'g_motion_decoders',
     'RENAME. Unk800F1B18Rec (0x570 bytes each) array D_800F1B18[] (extern include/bb2.h:599, '
     'three records, D_800F2B68 follows at +0x1050). ACCESSES: motion_InitStream (9F9C.c:71, '
     '&D_800F1B18[arg0]) and motion_DecodeFrame (9F9C.c:130, &D_800F1B18[obj]) only, '
     'asm/funcs/func_8001979C.s and func_800198D0.s; slot indices come from fight_Init / '
     'func_8003CF84 (0 = common, 1..2 = the loaded fighters), motion_UpdatePlayback (0..1) and '
     'the recorded/fighter slots of replay_PlayFrame / fighter_Update (motion_slot + 1 or 0). '
     'WHY: the motion-stream decoder state, one record per decoder slot. COUNTER-EVIDENCE: the '
     'prefix D_800F1B14 (4 bytes before it) is a separate object and keeps its name.'),
]
