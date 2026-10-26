"""wave83 rows (pilot-effects-51268): Q132 mechanical names for 17 of the 67 neutral effect functions of
main/51268 (the command-start wrappers of the eff_ command block, the rotated-quad-cluster, billboard-pair and
jittered-billboard slot spawners/drawers, and the two-quad advance wrappers). Research input:
tmp/codex/pilot-effects-51268-research.md (17 RENAME, 50 NO_NAME over 67 targets; untrusted proposals). Every row was
re-derived from the C bodies (src/main/51268.c) and every caller (src/main/17AFC.c func_80032854, the handler table
src/main/d_8BC50.c, asm/funcs/func_80032854.s); names changed against the research say CHANGED FROM RESEARCH.
The 50 research NO_NAMEs (80063AF0 80063B34 80063B78 80063BA4 80064ED8 80064F20 80064F68 80064FB4 8006505C 800650A4
800650EC 80065134 8006517C 800651F0 80065264 800652AC 80065344 80065394 800653E4 80065434 80065540 80065590 800655E0
80065630 80065760 800657B0 80066EC0 80066EF4 80066F28 80066F5C 80066F90 80066FC4 80066FF8 8006702C 80067060 80067094
800670C8 800670FC 80067130 80067164 80067198 800671CC 800676C8 80067704 80067740 8006777C 800677B8 800677F4 80067830
8006786C) stay neutral and are not rows: the lane wrappers, quad initializers, single-counter draw wrappers and
particle half-spawn / frame wrappers each have an instruction-identical or selector-only twin. The research's data
suggestions (D_800A3464, D_800F1140, D_800A32BC, D_800A3440, D_800A34F0, D_800A3460, D_800A3448, D_800A3444,
D_800F0C04, D_800F0BEC, D_800F0BCC ...) need separate all-access data manifests and are not part of this wave (DATA
is empty). The eff_ prefix is the TU's: every function here starts, spawns or draws a command of the eff_StartCommand /
eff_StepCommand block (51268.c:80-133; handler table D_8009BA7C, d_8BC50.c:262-324) or its slot/quad pools, as the
already-named eff_SpawnBillboardPair, eff_SpawnJitteredBillboards, eff_StartRecycledCommandWithPaleRedLight and
eff_DrawAndAdvanceCosineScaledQuad do. TRACE_USED marks the three rows whose evidence cites a runtime hit map; it is
reach-only corroboration and no role rests on it (Q124).
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = 'naming-reviewer pre-apply (round 1): 16/17 PASS as proposed; 1 FAIL (80061D74) replaced by the reviewer-suggested name (NOTES).'

NOTES = {'0x80061D74': 'suggested: naming-reviewer pre-apply round 1 failed eff_StartRecycledYawedParticlesWithJitteredQuads and suggested eff_StartRecycledYawedParticles; R6 must judge it fresh at every use.'}

# rows whose evidence cites a runtime hit map (reach only)
TRACE_USED = {
    "0x80061C00": True,
    "0x80061D74": True,
    "0x80061EC0": True,
}

# names changed against the research (research name -> name used)
ADJUSTED = {
    "0x800611A4": ("eff_StartRotatedQuadClusterWithPaleYellowLight", "eff_StartRotatedQuadClusterWithNearWhiteLight",
                   "0xFFFFEF is white with one channel 0x10 low, not a yellow"),
    "0x80061ACC": ("eff_StartRecycledVectorAndJitteredBillboardCommandsWithPaleRedLight",
                   "eff_StartRecycledVectorCommandAndJitteredBillboardsWithPaleRedLight",
                   "'Vector' is one command (the 20/21 pair, spelled like the existing eff_StartRecycledVectorCommandWithRedLight); "
                   "'Commands' plural attached the wrong noun"),
    "0x80065680": ("eff_DrawAndAdvanceLayeredQuadsUntilTrailingCounterExpires",
                   "eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires",
                   "'Layered' is not shown (two modes of eff_DrawAnimatedQuads at their own positions); Second/First follow draw order"),
    "0x800656EC": ("eff_DrawAndAdvanceLayeredQuadsUntilLeadingCounterExpires",
                   "eff_DrawAndAdvanceTwoAnimatedQuadsUntilFirstCounterExpires",
                   "same as 80065680"),
}

# research RENAMEs not carried
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-b")

# ---------------------------------------------------------------------------------------------------------
# helpers that read the tree so every line number below is the live one
_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")

_COV = {}
_covp = os.path.join(WT, "docs", "naming", "runtime", "matrix", "coverage.tsv")
_rd = list(csv.reader(open(_covp, encoding="utf-8"), delimiter="\t"))
_CAPS = _rd[0][5:]
for _r in _rd[1:]:
    _COV[_r[0].lower()] = [_CAPS[i] for i in range(len(_CAPS)) if _r[5 + i] not in (".", "")]


def B(fn, f="51268.c"):
    """src/main/<file>:<first>-<last> of the C definition of fn."""
    lines = _SRC[f]
    for k, l in enumerate(lines):
        if not re.match(r"^[A-Za-z_][\w \*]*\b%s\(" % fn, l) or l.startswith("extern"):
            continue
        j = k
        while j < len(lines) and lines[j] != "}":
            j += 1
        return f"src/main/{f}:{k + 1}-{j + 1}"
    raise SystemExit(f"no C definition of {fn}")


def L(f, pat, nth=0):
    """src/main/<f>:<line> of the nth line matching regex pat."""
    hits = [k + 1 for k, l in enumerate(_SRC[f]) if re.search(pat, l)]
    if len(hits) <= nth:
        raise SystemExit(f"{f}: no line {nth} matching {pat}")
    return f"src/main/{f}:{hits[nth]}"


def CL(fn):
    """src/main/17AFC.c:<line> of the call of fn."""
    return L("17AFC.c", r"^\s+(\w+ = )?%s\(" % fn)


def TB(fn):
    """src/main/d_8BC50.c:<line> of fn's slot in the handler table D_8009BA7C."""
    return L("d_8BC50.c", r"^\s+\(s32\)%s[,}]" % fn)


def JAL(fn):
    ls = open(os.path.join(WT, "asm", "funcs", "func_80032854.s"), encoding="utf-8").read().split("\n")
    for i, l in enumerate(ls):
        if re.search(r"\bjal\s+%s\b" % fn, l):
            return f"asm/funcs/func_80032854.s:{i + 1}"
    raise SystemExit(f"no jal {fn}")


def T(addr):
    """Reach clause from coverage.tsv (verified at build time); no role rests on it."""
    h = _COV[addr.lower()]
    return (f"TRACE: not used; coverage.tsv lists the function in {len(h)} of {len(_CAPS)} registered captures "
            f"(docs/naming/runtime/matrix/coverage.tsv), and no part of the name rests on a runtime observation (Q132 mechanical name).")


_RT = "docs/naming/runtime/"
_SK = _RT + "lib-versus-2p-kimigaura-shore-kick-1/"
_FF = _RT + "lib-story-final-fight-retail-1/"

# the shared start-wrapper contract (51268.c, eff_StartCommand)
START = ("COMMON CONTRACT: the wrapper points D_800A3468 at g_eff_command_block, fills the block (unk0C = the position, "
         "unk10 = a rotation/vector triple if any, unk14 = the address of one byte of g_eff_command_status, unk00.w = the "
         "command word) and calls eff_StartCommand (51268.c:80-103), which copies the triples, clears "
         "g_eff_command_phase[unk00.h], calls the D_8009BA7C handler selected by D_8009BA60[unk00.h] + phase, stores its "
         "result through unk14 and, when command bit 0x200000 is set, sets the point-light countdown D_800A32BC = 0xA. "
         "The 0x21xxxx commands carry that bit, so the wrapper's D_800F1140 / D_800A3464 stores (position, RGB) are the light "
         "the step loop (51268.c:268) hands to light_SetPointLight.")


FUNCS = [
    ("0x800611A4", "eff_StartRotatedQuadClusterWithNearWhiteLight", "RENAME",
     f"CHANGED FROM RESEARCH: eff_StartRotatedQuadClusterWithPaleYellowLight; 0xFFFFEF is 255/255/239, white with one channel low, so NearWhite is what the literal shows. "
     f"BODY: {B('func_800611A4')}: copies the three s16 of arg1 into the local svec, points block.unk10 at it and unk0C at arg0, status byte 26, command word 0x21001A, eff_StartCommand; "
     f"copies the position arg0 into D_800F1140; D_800A3464 = 0xFFFFEF. {START} Command 26 is table slot D_8009BA60[26] = 0x3C = 60, the pair {TB('func_8006288C')} (func_8006288C) and "
     f"{TB('func_8006295C')} (func_8006295C): the spawner copies position and the full rotation triple into a slot, the drawer emits three POLY_FT4 quads per slot through that rotation. "
     f"CALLER: {CL('func_800611A4')} (func_80032854 case 11, arg3 = the s16 triple; asm {JAL('func_800611A4')}). RETURN USE: void. "
     f"WHY: Start = starts a command, RotatedQuadCluster = the command whose slot holds a rotation triple and draws a cluster of quads (the only wrapper that starts command 26 and the only one that passes a rotation "
     f"triple through unk10 and copies it), WithNearWhiteLight = the stored point-light colour. COUNTER-EVIDENCE: no impact, flash or weapon identity is claimed; allocation may find no free slot "
     f"(func_8006288C then does nothing, still returning 1) and the command is started regardless. Prior manifests: wave02 RESET (splat placeholder); no_name.tsv w58 (identity unproved, the 'three quads' mechanics "
     f"were not disputed; Q132 removes the identity requirement). {T('0x800611A4')}"),
    ("0x800613C8", "eff_StartSlowRateCosineScaledQuadWithPaleBlueLight", "RENAME",
     f"BODY: {B('func_800613C8')}: status byte 11, command word 0x21000B, eff_StartCommand, position to D_800F1140, D_800A3464 = 0x8080FF, returns 16. {START} Command 11 is slot D_8009BA60[11] = 0x18: "
     f"{TB('eff_InitRateSelectedCosineScaledQuad')} (init) then {TB('eff_DrawAndAdvanceRateSelectedCosineScaledQuad')} (step). The init stores D_800A3440 = (command word >> 19) & 3 ({L('51268.c', r'D_800A3440 = ')}); "
     f"0x21000B >> 19 = 4, & 3 = 0. The step ({B('eff_DrawAndAdvanceRateSelectedCosineScaledQuad')}) switches on that value and adds 0x1FF (0), 0x3FE (1) or 0x5FD (2) to the timer D_800F0BA8[5] each call "
     f"and ends it at 0x2001. CALLER: {CL('func_800613C8')} (func_80032854 case 6; asm {JAL('func_800613C8')}). RETURN USE: D_800A3910 = the result (same line), the countdown of 16 steps; 0x2001 / 0x1FF is about 16, "
     f"matching the timer rate. WHY: Start = starts the command, SlowRate = the 0x1FF increment, the slowest of the three wrappers 800613C8 / 80061454 (0x3FE, returns 8) / 800614E0 (0x5FD, returns 5), "
     f"CosineScaledQuad = the quad whose size is scaled by rcos of the timer (eff_DrawAnimatedQuads case 5 here, rcos of the timer in its case 0 for the existing eff_StartCosineScaledQuadWithPaleBlueLight), PaleBlue = 0x8080FF. "
     f"COUNTER-EVIDENCE: Slow is relative to this trio and says nothing of the existing fixed-rate eff_StartCosineScaledQuadWithPaleBlueLight (command 4, also 0x1FF, but ends at 0x1000 and returns 1); the rate is "
     f"followed through the extracted bits, not read off the command literal. Prior manifests: wave02 RESET; no_name.tsv w58/w72/w77/w81 (selector-only distinction; this row names the consumer's arithmetic). {T('0x800613C8')}"),
    ("0x80061454", "eff_StartMediumRateCosineScaledQuadWithPaleBlueLight", "RENAME",
     f"BODY: {B('func_80061454')}: the same body as 800613C8 with command word 0x29000B and return 8. 0x29000B >> 19 = 5, & 3 = 1, so D_800A3440 = 1 and the step adds 0x3FE per call "
     f"({B('eff_DrawAndAdvanceRateSelectedCosineScaledQuad')}); the middle of 0x1FF / 0x3FE / 0x5FD. {START} CALLER: {CL('func_80061454')} (func_80032854 case 7; asm {JAL('func_80061454')}). "
     f"RETURN USE: D_800A3910 = 8 (0x2001 / 0x3FE is about 8). WHY: see 800613C8; Medium = the middle increment. COUNTER-EVIDENCE: relative to the trio, no event identity. "
     f"Prior manifests: wave05 RESET; no_name.tsv w58/w72/w77/w81 (selector-only; answered by the consumer's increments). {T('0x80061454')}"),
    ("0x800614E0", "eff_StartFastRateCosineScaledQuadWithPaleBlueLight", "RENAME",
     f"BODY: {B('func_800614E0')}: the same body with command word 0x31000B and return 5. 0x31000B >> 19 = 6, & 3 = 2, so D_800A3440 = 2 and the step adds 0x5FD per call, the fastest of the three. {START} "
     f"CALLER: {CL('func_800614E0')} (func_80032854 case 8; asm {JAL('func_800614E0')}). RETURN USE: D_800A3910 = 5 (0x2001 / 0x5FD is about 5). WHY: see 800613C8; Fast = the largest increment. "
     f"COUNTER-EVIDENCE: relative to the trio. Prior manifests: wave02 RESET; no_name.tsv w58/w72/w77/w81. {T('0x800614E0')}"),
    ("0x80061710", "eff_StartArgSelectedCommandWithGreenLight", "RENAME",
     f"BODY: {B('func_80061710')}: switch on arg1: 0 clears g_eff_command_status[14] and selects status 14 with command word 0x21000E, 1 clears status[15] and selects it with 0x21000F, any other value leaves the block as it was; "
     f"eff_StartCommand; position to D_800F1140; D_800A3464 = 0x10FF10. {START} CALLER: {CL('func_80061710')} (func_80032854 case 15, second argument = that function's arg0; asm {JAL('func_80061710')}). RETURN USE: void. "
     f"WHY: Start = starts a command, ArgSelected = the argument chooses which of two commands (and clears its status byte first), WithGreenLight = 0x10FF10 (R 0x10, G 0xFF, B 0x10). The existing "
     f"{B('eff_StartArgSelectedCommandWithCyanLight')} has the identical shape with status 12/13 and 0x10FFFF, so the light colour and command pair tell the two apart. "
     f"COUNTER-EVIDENCE: the name promises no validation of arg1 (the stale block of the previous call is used for other values); no fighter or side is claimed for the argument. "
     f"Prior manifests: wave02 RESET; no_name.tsv w58/w72/w77 (only slot and colour distinguish it; Q132 accepts the fixed output as the mechanical distinction). {T('0x80061710')}"),
    ("0x800619A4", "eff_StartBillboardPairCommand", "RENAME",
     f"BODY: {B('func_800619A4')}: unk0C = arg0, status byte 8, command word 0x10008, eff_StartCommand; no light store. {START} Command 8 is slot D_8009BA60[8] = 0x12 = 18: "
     f"{TB('eff_SpawnBillboardPair')} (eff_SpawnBillboardPair, claims a slot of D_800A3448 and copies the position) then {TB('func_80063084')} (func_80063084, draws and ages the paired billboards). "
     f"CALLER: {CL('func_800619A4')} (func_80032854 case 9; asm {JAL('func_800619A4')}). RETURN USE: void. WHY: Start...Command = starts a command, BillboardPair = the pool that command's handler pair "
     f"spawns and draws (the already-named eff_SpawnBillboardPair); it differs from 800619F0 by command 8 versus 7 and the handler pool. COUNTER-EVIDENCE: it sets no light and makes no claim about "
     f"allocation succeeding or two visible quads per frame. Prior manifests: wave02 RESET; no_name.tsv w58/w72/w77 (a bare numeric command; this row follows the command into its handlers). {T('0x800619A4')}"),
    ("0x800619F0", "eff_StartJitteredBillboardsCommand", "RENAME",
     f"BODY: {B('func_800619F0')}: unk0C = arg0, status byte 7, command word 0x10007, eff_StartCommand; no light store. {START} Command 7 is slot D_8009BA60[7] = 0x10 = 16: "
     f"{TB('eff_SpawnJitteredBillboards')} (eff_SpawnJitteredBillboards, jitters the position with rand() into a free slot of D_800A3444) then {TB('func_800646E8')} (func_800646E8, draws and ages the pool). "
     f"CALLER: {CL('func_800619F0')} (func_80032854 case 10; asm {JAL('func_800619F0')}). RETURN USE: void. WHY: the standalone starter of command 7 (see 800619A4). COUNTER-EVIDENCE: "
     f"func_80061ACC also starts command 7 but additionally starts a recycled vector command and sets a light, so this name covers the standalone wrapper and does not say the command has no other starter. "
     f"Prior manifests: wave02 RESET; no_name.tsv w58/w72/w77 (slot 7 is also started inside func_80061ACC; the name does not claim exclusivity). {T('0x800619F0')}"),
    ("0x80061ACC", "eff_StartRecycledVectorCommandAndJitteredBillboardsWithPaleRedLight", "RENAME",
     f"CHANGED FROM RESEARCH: eff_StartRecycledVectorAndJitteredBillboardCommandsWithPaleRedLight; the recycled command is one (20 or 21), spelled as the existing eff_StartRecycledVectorCommandWithRedLight is. "
     f"BODY: {B('func_80061ACC')}: unk0C = arg0, unk10 = arg1 (a vector triple); when both status bytes 20 and 21 are busy it clears both; if 20 is free it selects status 20 / command word 0x210014, runs "
     f"eff_StartCommand, then re-points the block at status 7 / command word 0x10007; else if 21 is free the same with 0x210015; after the if chain a final eff_StartCommand runs the command left in the block "
     f"(command 7), then position to D_800F1140 and D_800A3464 = 0xFF8080. {START} Commands 20 / 21 are the particle emitters (D_8009BA60[20] = 0x2A, [21] = 0x2D: eff_SpawnParticles modes 2 and 3), command 7 "
     f"is the jittered-billboard pool (see 800619F0). CALLER: {CL('func_80061ACC')} (func_80032854 case 14; asm {JAL('func_80061ACC')}). RETURN USE: void. WHY: Start = starts, RecycledVectorCommand = the "
     f"free-of-two particle command taking the vector (the existing RedLight sibling's first half, but with commands 20 / 21), AndJitteredBillboards = the second start (command 7), WithPaleRedLight = 0xFF8080, "
     f"the same literal as the existing eff_StartRecycledCommandWithPaleRedLight. The name covers both command starts and the light. COUNTER-EVIDENCE: no combined visual identity is claimed; "
     f"which of 20 / 21 runs depends on the busy status, which the Recycled word states. Prior manifests: wave02 RESET; no_name.tsv w58/w72/w77 (composite established, noun missing; the mechanical composition "
     f"is now the name). {T('0x80061ACC')}"),
    ("0x80061C00", "eff_StartRecycledYawedParticlesWithTextureSelection", "RENAME",
     f"BODY: {B('func_80061C00')}: arg2 is normalised (arg2 != 1 becomes 0); the fixed vector (0, -0xA00, 0xA00) is rotated about Y by arg1 (RotMatrix of (0, arg1, 0), RotTrans) into sp10, which becomes block.unk10; "
     f"both status bytes 22 and 23 are cleared when busy; the free one is selected with command word 0x10016 / 0x10017 and the normalised arg2 is stored into D_800A34F0[0] / [1] ({L('51268.c', r'D_800A34F0\[0\] = arg2')}, "
     f":{L('51268.c', r'D_800A34F0\[1\] = arg2').split(':')[-1]}); eff_StartCommand; no light store. Commands 22 / 23 are D_8009BA60[22] = 0x30 / [23] = 0x33: eff_SpawnParticles modes 4 and 5 "
     f"({TB('func_80067060')} ff.), updated by eff_PrepareParticles / eff_UpdateParticles (4, 2) and (5, 3). Those consumers read D_800A34F0[mode - 4] to choose the texture table (D_8009B9B8 versus D_8009B998, "
     f"{L('51268.c', r'D_800A34F0\[arg0 - 4\] != 0', 0)}) and the colour ({L('51268.c', r'D_800A34F0\[arg0 - 4\] != 0', 1)}). CALLER: {CL('func_80061C00')} (func_80032854 case 12; asm {JAL('func_80061C00')}). "
     f"RETURN USE: void. WHY: Start = starts, Recycled = free-of-two status bytes, Yawed = the vector is rotated about Y by arg1, Particles = commands 22 / 23 drive eff_SpawnParticles, WithTextureSelection = "
     f"the extra output that tells it from 80061D74: it stores the normalised selector the particle texture table choice reads. COUNTER-EVIDENCE: the C caller passes two arguments to this three-argument "
     f"function (17AFC.c:27-32; asm sets only a0 and a1 before the jal, func_80032854.s:204-207), so the stored selector is not a caller-meaningful property and no material or kick kind is claimed; the name states the store only. "
     f"Prior manifests: wave02 RESET; no_name.tsv w58/w72/w77 (identity unproved; Q132 removes the requirement). "
     f"TRACE (reach only, corroborates nothing about the role): {_SK}hitmap.tsv:831 hits kick_0, kick_1, kick_2 (shore-kick capture, manifest {_SK}capture.manifest.json SHA1 b78f56a408dda8f4ca4a5d48659060f1eb437e3f, "
     f"capture.jsonl SHA1 4a533c05b4be0947df28bfb64dae07dd0719e79a recorded in it, raw log not in the tree) and {_FF}hitmap.tsv:831 hits story (manifest SHA1 8934de3f30bbf6544ef08437d9bfab369c5b5aa9, "
     f"capture.jsonl SHA1 2c1def7be909e4540374643dcdb40608285b34e6). Neither capture is in the coverage matrix yet; no only-X or knockout claim is made and no part of the name uses these hits."),
    ("0x80061D74", "eff_StartRecycledYawedParticles", "RENAME",
     f"SUGGESTED NAME: naming-reviewer pre-apply round 1 FAILED eff_StartRecycledYawedParticlesWithJitteredQuads (N1: eff_UpdateParticles "
     f"jitters only projected vertex 1, and modes 6 / 7 particles drawn at vertex 2 get unjittered quads) and suggested eff_StartRecycledYawedParticles; "
     f"80061C00's WithTextureSelection suffix tells the two apart. The jitter clauses below describe the consumer and do not support the name. BODY:{B('func_80061D74')}: the same yaw-rotated vector as 80061C00 (arg1 is s16 here) as unk10; status bytes 24 / 25 recycled with command words 0x10018 / 0x10019; no D_800A34F0 store, no light. "
     f"Commands 24 / 25 are D_8009BA60[24] = 0x36 / [25] = 0x39: eff_SpawnParticles modes 6 and 7 ({TB('func_80067130')} ff.), updated by eff_PrepareParticles / eff_UpdateParticles (6, 3) and (7, 2). In the "
     f"update, modes below 8 and at least 6 add rand() jitter to the projected screen coordinates (sxy[2] += rand() * 20 / 32768, sxy[3] += rand() * 10 / 32768, "
     f"{L('51268.c', r'sxy\[2\] \+= rand')}-{L('51268.c', r'sxy\[3\] \+= rand').split(':')[-1]}) before the quads are written; modes 4 / 5 do not. CALLER: {CL('func_80061D74')} "
     f"(func_80032854 case 13; asm {JAL('func_80061D74')}). RETURN USE: void. WHY: the same Start / Recycled / Yawed / Particles as 80061C00; WithJitteredQuads = the consumer of these commands jitters the "
     f"projected quad coordinates, which separates it from the 80061C00 modes (4 / 5) that select a texture table. COUNTER-EVIDENCE: the random initial rotation in eff_SpawnParticles is common to every "
     f"mode and is not what the name says; the caller's sound selector is not an effect identity. Prior manifests: wave02 RESET; no_name.tsv w58/w72/w77 (relationship to 80061C00 mechanical, identity "
     f"missing). TRACE (reach only): {_SK}hitmap.tsv:832 hits kick_3 through kick_7 (same capture and SHA1s as the 80061C00 row); "
     f"no role is taken from it."),
    ("0x80061EC0", "eff_StartRecycledCommandWithMagentaLight", "RENAME",
     f"BODY: {B('func_80061EC0')}: unk0C = arg0; when status bytes 18 and 19 are both busy it clears both; if 18 is free it selects status 18 / command word 0x210012, else if 19 is free status 19 / "
     f"0x210013; eff_StartCommand; position to D_800F1140; D_800A3464 = 0xFF00FF (R 0xFF, G 0, B 0xFF: magenta). {START} Commands 18 / 19 are D_8009BA60[18] = 0x26 / [19] = 0x28 (handlers "
     f"{TB('func_80065264')} and {TB('func_800652AC')}, the pairs behind the single-counter neutral wrappers). CALLER: {CL('func_80061EC0')} (func_80032854 case 18, followed by a positional sound; asm {JAL('func_80061EC0')}). RETURN USE: void. "
     f"WHY: Start = starts, Recycled = free-of-two status bytes, Command = whatever those two commands are, WithMagentaLight = 0xFF00FF; it is the fourth of a family with the existing "
     f"{B('eff_StartRecycledCommandWithRoseLight')} (status 9 / 10, 0xFF0060), eff_StartRecycledCommandWithPaleRedLight (5 / 6, 0xFF8080) and eff_StartRecycledCommandWithOrangeLight (16 / 17, 0xC06013), "
     f"told apart by the light literal. COUNTER-EVIDENCE: no story-final, character or move identity is claimed. Prior manifests: wave02 RESET; no_name.tsv w58 (a fixed colour does not establish a role; "
     f"none is claimed). TRACE (reach only): {_FF}hitmap.tsv:833 hits final_fight (retail story capture; manifest SHA1 8934de3f30bbf6544ef08437d9bfab369c5b5aa9, capture.jsonl SHA1 "
     f"2c1def7be909e4540374643dcdb40608285b34e6); the dispatcher is generic and the function is not hit in the shore-kick or effects captures, so no only-X role exists and none is taken."),
    ("0x8006288C", "eff_SpawnRotatedQuadCluster", "RENAME",
     f"BODY: {B('func_8006288C')}: g_eff_command_phase[26] = 1; the first clear bit of D_800A3460 (of 6) is claimed; the slot gets the position D_800A347C[0..2] (D_800F0FB8[i]) and the rotation triple D_800A3478[0..2] "
     f"(D_800F10A0[i]) and its age D_800F0C04[i] = 0; returns 1 whether or not a slot was free. CALLERS: handler table {TB('func_8006288C')} only (phase 0 of command 26, started by func_800611A4); "
     f"no jal. RETURN USE: stored as the command's status byte by eff_StartCommand. WHY: Spawn = claims a slot and copies its state, RotatedQuadCluster = the slot carries a rotation triple and "
     f"its drawer func_8006295C emits three POLY_FT4 quads per slot through RotMatrixZYX of it ({B('func_8006295C')}); the existing eff_SpawnBillboardPair / eff_SpawnJitteredBillboards are the other "
     f"slot spawners of the TU, the billboard ones carry no rotation. COUNTER-EVIDENCE: no impact or flash noun is claimed; with all six slots busy it silently does nothing. "
     f"Prior manifests: no_name.tsv w58 (slot mechanics clear, imagery missing; Q132 drops the imagery requirement). {T('0x8006288C')}"),
    ("0x8006295C", "eff_DrawAndAdvanceRotatedQuadClusters", "RENAME",
     f"BODY: {B('func_8006295C')}: for each live slot of D_800A3460 (6): the scale from rsin of the age D_800F0C04[i] plus age << 12 / 6, RotMatrixZYX of the stored triple D_800F10A0[i], ScaleMatrix, the translation "
     f"D_800F0FB8[i] less the camera D_800A3470, CompMatrix with the camera matrix D_800A3474; three POLY_FT4 quads (j = 0..2) from the table D_8009BB84 are projected (RotTransPers4), depth-mapped through "
     f"gpu_DepthToOtIndex and filled with colour and texture; the age is incremented and the slot's bit cleared at age 7; the new quads are linked into the OT (AddPrim) and the quad cursor advances. "
     f"Returns 1 when quads were added, 0 (and g_eff_command_phase[26] = 0) once every slot is dead, else the live mask. CALLERS: handler table {TB('func_8006295C')} only (phase 1 of command 26). "
     f"WHY: DrawAndAdvance = it both renders the quads and increments the age / retires slots (the same verb pair as the existing eff_DrawAndAdvanceCosineScaledQuad), RotatedQuadClusters = the slots "
     f"eff_SpawnRotatedQuadCluster fills (rotation triple, three quads each). COUNTER-EVIDENCE: eff_ rather than gpu_ because it owns the slot lifetime and command phase, not just primitive output; "
     f"quads can be suppressed by the depth range or the buffer limit while the age still advances. Prior manifests: wave10 RESET; no_name.tsv w58 (specific noun missing; not a generic GPU renderer). {T('0x8006295C')}"),
    ("0x80063084", "eff_DrawAndAdvanceBillboardPairs", "RENAME",
     f"BODY: {B('func_80063084')}: for each live slot of D_800A3448 (12) and j = 0, 1: while the age D_800F0BEC[i] is below 16 it takes the slot position D_800F0E38[i] less the camera (the second layer is lowered by "
     f"an rsin of the age once the age reaches 3), projects it (RotTransPers), picks a texture (D_8009B940 for j = 0, D_8009B948 / D_8009B950 for j = 1 by age), sizes the quad from the depth and shapes it with "
     f"rsin / rcos of the age, colours it (j = 0: 0x808080 or 0xFF8080 by D_8009BD24.unk20_0; j = 1: fading with age), writes a POLY_FT4 and links it into the OT; at age 16 the slot's bit is cleared; "
     f"every live slot's age is incremented. Returns whether any slot is live. CALLERS: handler table {TB('func_80063084')} only (phase 1 of command 8). WHY: DrawAndAdvance = draws and ages (as above), "
     f"BillboardPairs = the 12 slots eff_SpawnBillboardPair claims ({B('eff_SpawnBillboardPair')}), each drawn as two screen-facing quads (j = 0 and 1). COUNTER-EVIDENCE: Pair names the two-layer slot "
     f"structure, not a promise of two visible quads per call (depth range and buffer limit skip quads); the colour alternatives (unk20_0) are not named; no flare or halo imagery from comments is used. "
     f"Prior manifests: no_name.tsv w58 (comment-derived imagery and fixed-colour overclaim rejected; neither is used). {T('0x80063084')}"),
    ("0x800646E8", "eff_DrawAndAdvanceJitteredBillboards", "RENAME",
     f"BODY: {B('func_800646E8')}: for each live slot of D_800A3444 (16): the counter D_800F0BCC[i] is incremented, its quarter selects the frame D_8009B8E8[counter / 4] and ends the slot at 7; "
     f"the slot position D_800F0D78[i] less the camera is projected (ApplyRotMatrixLV, RotTransPers), depth-mapped, culled outside the range, sized from the depth and grown with the counter, written as a "
     f"POLY_FT4 (colour 0x808080 or 0xA08C50 by D_8009BD24.unk20_0) and the counter is incremented a second time when the slot passed the depth cull; the new quads are linked into the OT. Returns 1 while any slot "
     f"is live, else 0. CALLERS: handler table {TB('func_800646E8')} only (phase 1 of command 7). WHY: DrawAndAdvance = renders and steps the counters, JitteredBillboards = the 16 slots "
     f"eff_SpawnJitteredBillboards fills with rand()-jittered positions ({B('eff_SpawnJitteredBillboards')}), drawn as screen-facing quads. COUNTER-EVIDENCE: the name does not promise one tick per "
     f"call (culling decides whether the second increment happens); no dust or smoke identity and no generic-GPU claim. Prior manifests: wave10 RESET; no_name.tsv w58 "
     f"(visual identity and the double advance; Q132 drops identity, the name says 'Advance' without a rate). {T('0x800646E8')}"),
    ("0x80065680", "eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires", "RENAME",
     f"CHANGED FROM RESEARCH: eff_DrawAndAdvanceLayeredQuadsUntilTrailingCounterExpires; 'Layered' is not shown (two modes of eff_DrawAnimatedQuads at their own position records), and Trailing is replaced by the plain draw order. "
     f"BODY: {B('func_80065680')}: eff_DrawAnimatedQuads(0xC) (result discarded), D_800F0BA8[12] += 1; v0 = eff_DrawAnimatedQuads(0xE), D_800F0BA8[14] += 1; v0 &= 0xFF; returns 0 once D_800F0BA8[14] >= 11, "
     f"else v0. The paired initializer func_8006517C seeds both position records and zeroes both counters. CALLERS: handler table {TB('func_80065680')} only (D_8009BA7C[35], phase 1 of command 16, which eff_StartRecycledCommandWithOrangeLight starts as 0x210010 / 0x210011); "
     f"no jal. RETURN USE: the command's status byte, stored by eff_StepCommand (51268.c:127-129). WHY: DrawAndAdvance = two draws and two counter increments, TwoAnimatedQuads = modes 0xC and 0xE of eff_DrawAnimatedQuads, "
     f"UntilSecondCounterExpires = the result is zeroed by the counter of the second draw (D_800F0BA8[14]). COUNTER-EVIDENCE: the sibling func_800656EC (modes 0xD / 0xF) tests the first counter "
     f"(D_800F0BA8[13]) instead, which is the stated difference; the counters start together and rise together, so runtime expiry coincides and the name claims the tested operand only. "
     f"Prior manifests: wave03 RESET; no_name.tsv w58/w72/w77/w81 (sibling separation; the tested counters differ in the code). {T('0x80065680')}"),
    ("0x800656EC", "eff_DrawAndAdvanceTwoAnimatedQuadsUntilFirstCounterExpires", "RENAME",
     f"CHANGED FROM RESEARCH: eff_DrawAndAdvanceLayeredQuadsUntilLeadingCounterExpires (see 80065680). BODY: {B('func_800656EC')}: eff_DrawAnimatedQuads(0xD), D_800F0BA8[13] += 1; "
     f"v0 = eff_DrawAnimatedQuads(0xF), D_800F0BA8[15] += 1; returns v0 & 0xFF while D_800F0BA8[13] < 11, else 0. The paired initializer func_800651F0 seeds records 13 / 15 and zeroes counters 13 / 15. "
     f"CALLERS: handler table {TB('func_800656EC')} only (D_8009BA7C[37], phase 1 of command 17, the other half of the orange-light pair); no jal. WHY: the mirror of 80065680: the same two draws and increments, but the lifetime test reads the FIRST draw's counter. "
     f"COUNTER-EVIDENCE: as in 80065680. Prior manifests: wave03 RESET; no_name.tsv w58/w72/w77 (termination tests the first counter here, the second in the sibling: the distinction this name states). "
     f"{T('0x800656EC')}"),
]

DATA = []
