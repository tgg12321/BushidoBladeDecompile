"""wave104 rows (func-second-pass-14): the particle command trios 0x16..0x19 of
eff_StartRecycledYawedParticlesWithTextureSelection (51268.c:594-641, commands 0x16 / 0x17) and
eff_StartRecycledYawedParticles (51268.c:643-684, commands 0x18 / 0x19), named together so that a word separates
every trio from its siblings (the func-second-pass-13 reviewer FAIL: 'RecycledYawedParticles' fitted both issuers).
Spawns func_80067060 / 80067094 / 800670C8 / 800670FC / 80067130 / 80067164 / 80067198 / 800671CC (d_8BC50.c:317-328
handlers 48,49,51,52,54,55,57,58), draws func_800677B8 / 800677F4 / 80067830 / 8006786C (handlers 50,53,56,59).
Research input tmp/codex/func-second-pass-14-research.md (untrusted; every claim re-derived at HEAD 73a7a1e56, see
notes.txt). Words: TextureSelected (modes 4/5 pick the texture record by age and issuer flag, eff_PrepareParticles
51268.c:2812-2829), AdditiveSingleQuad (modes 6/7: one semi-transparent 0x2E quad per particle, tpage 0x2E ABR 1,
eff_UpdateParticles :3144-3149; modes 0-3 emit 2-3 quads, modes 4/5 one opaque 0x2C quad), FirstSlot / SecondSlot (the
status byte the issuer takes: status[22]/[24] first, [23]/[25] second). Verdicts CONFIRM (pre-apply PASS 12/12). (DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply 2026-10-31 (naming-c, HEAD 73a7a1e56): PASS 12/12. TextureSelected holds only for modes 4/5 (eff_PrepareParticles 51268.c:2812-2829; one opaque 0x2C quad, :3129-3143); AdditiveSingleQuad only for modes 6/7 (fixed D_8009B8B0, tpage 0x2E ABR=1, one 0x2E quad then break, :3144-3149); modes 0-3 draw 2-3 quads from fixed D_8009B890. Spawn nouns are the command's call-graph role (mode arg + phase index), admitted as for eff_InitQuadPair*. 0x8006786C code-only, admitted."
NOTES = {}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-c")

_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")


def B(fn, f):
    """src/main/<file>:<first>-<last> of the C definition of fn."""
    lines = _SRC[f]
    for k, l in enumerate(lines):
        if not re.match(r"^[A-Za-z_][\w \*]*\b%s\(" % fn, l) or l.startswith("extern"):
            continue
        j = k
        while j < len(lines) and lines[j] != "}" and not (j == k and l.rstrip().endswith("}")):
            j += 1
        return f"src/main/{f}:{k + 1}-{j + 1}" if j != k else f"src/main/{f}:{k + 1}"
    raise SystemExit(f"no C definition of {fn}")


# ---- per-command facts (all re-derived at HEAD; see notes.txt) ----------------------------------------------------
# cmd: (word, status slot, mode, row, issuer, issuer lines, flag index or None, 'first'/'second', coverage lines)
CMD = {
    0x16: dict(word="0x10016", slot=22, mode=4, row=2, nth="FirstSlot", issuer="eff_StartRecycledYawedParticlesWithTextureSelection",
               il="51268.c:594-641, status[22] first-free at :631-634", base=0x30, noun="TextureSelectedParticles"),
    0x17: dict(word="0x10017", slot=23, mode=5, row=3, nth="SecondSlot", issuer="eff_StartRecycledYawedParticlesWithTextureSelection",
               il="51268.c:594-641, status[23] when status[22] is busy at :635-638", base=0x33, noun="TextureSelectedParticles"),
    0x18: dict(word="0x10018", slot=24, mode=6, row=3, nth="FirstSlot", issuer="eff_StartRecycledYawedParticles",
               il="51268.c:643-684, status[24] first-free at :676-678", base=0x36, noun="AdditiveSingleQuadParticles"),
    0x19: dict(word="0x10019", slot=25, mode=7, row=2, nth="SecondSlot", issuer="eff_StartRecycledYawedParticles",
               il="51268.c:643-684, status[25] when status[24] is busy at :679-682", base=0x39, noun="AdditiveSingleQuadParticles"),
}

# addr: (old, cmd, phase, handler idx, d_8BC50 proto line, d_8BC50 table line, asm endlabel line, census csv line, named_syms line)
FN = {
    "0x80067060": ("func_80067060", 0x16, 0, 48, 253, 317, 15, 879, 1688, 2571),
    "0x80067094": ("func_80067094", 0x16, 1, 49, 254, 318, 15, 880, 1689, 2577),
    "0x800677B8": ("func_800677B8", 0x16, 2, 50, 255, 319, 17, 892, 1436, 2750),
    "0x800670C8": ("func_800670C8", 0x17, 0, 51, 256, 320, 15, 881, 1690, 2583),
    "0x800670FC": ("func_800670FC", 0x17, 1, 52, 257, 321, 15, 882, 1691, 2589),
    "0x800677F4": ("func_800677F4", 0x17, 2, 53, 258, 322, 17, 893, 1437, 2756),
    "0x80067130": ("func_80067130", 0x18, 0, 54, 259, 323, 15, 883, 1692, 2595),
    "0x80067164": ("func_80067164", 0x18, 1, 55, 260, 324, 15, 884, 1693, 2601),
    "0x80067830": ("func_80067830", 0x18, 2, 56, 261, 325, 17, 894, 1438, 2762),
    "0x80067198": ("func_80067198", 0x19, 0, 57, 262, 326, 15, 885, 1694, 2607),
    "0x800671CC": ("func_800671CC", 0x19, 1, 58, 263, 327, 15, 886, 1695, 2613),
    "0x8006786C": ("func_8006786C", 0x19, 2, 59, 264, 328, 17, 895, 1439, 2768),
}
COVLINE = {"0x80067060": 887, "0x80067094": 888, "0x800677B8": 900, "0x800670C8": 889, "0x800670FC": 890, "0x800677F4": 901,
           "0x80067130": 891, "0x80067164": 892, "0x80067830": 902, "0x80067198": 893, "0x800671CC": 894, "0x8006786C": 903}
COVHIT = {
    0x16: "story:1+ (lib-story-final-fight-retail-1), kick2p_0..2 (lib-versus-2p-kimigaura-kick-2p-1), kick_0..2 "
          "(lib-versus-2p-kimigaura-shore-kick-1), sel12 (story-final-fight-stages-1)",
    0x17: "kick2p_0..2 only (lib-versus-2p-kimigaura-kick-2p-1), consistent with second-slot use when two kicks overlap",
    0x18: "kick2p_3, kick_3..7",
    0x19: "kick2p_3 only",
}

_SIB = {
    0x16: "vs the 0x17 trio: FirstSlot (status[22] / phase[22]) vs SecondSlot (status[23] / phase[23]); vs the 0x18 / 0x19 trios: "
          "TextureSelected (modes 4/5 read a table entry chosen by age and flag, eff_PrepareParticles 51268.c:2812-2829; one opaque 0x2C "
          "quad, eff_UpdateParticles :3127-3143) vs AdditiveSingleQuad (modes 6/7 fixed D_8009B8B0, one semi-transparent 0x2E quad); vs "
          "the mode 0-3 trios (commands 0x02 / 0x03 / 0x14 / 0x15, issuers eff_StartRecycledVectorCommandWithRedLight and "
          "eff_StartRecycledVectorCommandAndJitteredBillboardsWithPaleRedLight): those read the fixed record D_8009B890[0] (:2799, :2806), "
          "so neither word is true of them",
    0x17: "vs the 0x16 trio: SecondSlot vs FirstSlot; vs 0x18 / 0x19: TextureSelected vs AdditiveSingleQuad; vs modes 0-3: fixed "
          "D_8009B890[0], no texture selection",
    0x18: "vs the 0x19 trio: FirstSlot vs SecondSlot; vs the 0x16 / 0x17 trios: AdditiveSingleQuad vs TextureSelected (their one quad is "
          "opaque 0x2C and the texture record is picked, :2812-2829, :3127-3143); vs the mode 0-3 trios: those emit 2-3 additive quads per "
          "particle (the p_n loop falls through :3151-3153), so 'SingleQuad' is false of them",
    0x19: "vs the 0x18 trio: SecondSlot vs FirstSlot; vs 0x16 / 0x17: AdditiveSingleQuad vs TextureSelected; vs modes 0-3: SingleQuad",
}

_MODEFACT = {
    4: "mode 4 row 2: Prepare takes the `< 6` arm: record index D_800EFC8A[2 * 0x1E0] >> 3 (= g_eff_particles[2][0].frame_age / 8, "
       "D_800EFC8A = g_eff_particles + 0x12 = the frame_age member of the row's record 0, rows are 48 * 20 bytes = 0x1E0 halfwords), capped "
       "at 3, table D_8009B9B8 if g_eff_yawed_particle_alternate_texture_flags[0] != 0 else D_8009B998 (51268.c:2812-2823), extent 0x1F x 0x20, "
       "base 0xC0 x 0x30, tpage 0xF, 1 group position; Update emits one opaque 0x2C quad per particle then breaks (:3127-3143)",
    5: "mode 5 row 3: the same `< 6` arm as mode 4 but flag [1] (g_eff_yawed_particle_alternate_texture_flags[particle_mode - 4], :2818, "
       ":3135), written by the issuer's second slot (:638); one opaque 0x2C quad per particle (:3127-3143)",
    6: "mode 6 row 3: Prepare takes the `< 8` arm: tpage 0x2E, fixed D_8009B8B0, extent 0xF x 0xF, base 0xC0 x 0x60, 2 group positions "
       "(:2830-2837); Update adds a rand() offset to the second projected vertex (:3077-3079) and emits ONE quad with word 0x2EFF8080 "
       "(command byte 0x2E = textured quad with the semi-transparent bit, tpage word 0x2E = ABR bits 5-6 = 1, B+F additive) then breaks "
       "(:3144-3149)",
    7: "mode 7 row 2: the same `< 8` arm as mode 6 (:2830-2837, :3144-3149)",
}

_DISPATCH = ("dispatch: eff_StartCommand (51268.c:84-115) clears g_eff_command_phase[command_word.h] (:89) and calls "
             "g_eff_command_handlers[g_eff_command_handler_bases[idx] + phase] (:105-109), storing the result in status_dest; "
             "eff_StepCommand (:117-144) makes the same call once per step and func_80061064 (:268-282) steps every index whose status is nonzero each frame (command_word.w := index, :277); tables d_8BC50.c:200-203 (bases) and :268-330 "
             "(handlers, index 0 at :269). Lifecycle by the code: phase 0 handler (spawn half 0, phase := 1, returns 1), next frame the "
             "phase 1 handler (spawn half 1, phase := 2, returns 1), then the phase 2 handler every frame until eff_LinkParticleQuads "
             "returns 0 (no quads), which zeroes the status")

_NOUN = ("eff_ and Particles = unchanged from the landed siblings (eff_SpawnParticles 51268.c:2628, eff_PrepareParticles :2779, "
         "eff_UpdateParticles :2893, eff_LinkParticleQuads :3158); the Spawn / DrawAndAdvance split follows eff_SpawnRotatedQuadCluster / "
         "eff_DrawAndAdvanceRotatedQuadClusters and eff_DrawAndAdvanceJitteredBillboards; FirstSlot / SecondSlot = the status byte the "
         "issuer takes (the precedent of eff_SpawnFighter0YawedSlotQuad / eff_SpawnFighter1YawedSlotQuad); 'Recycled' is dropped (the "
         "issuer's slot-clearing word, :627-630 / :672-675, not an action of these handlers); no visual word is claimed")

_PRIOR = ("docs/naming/phase3/no_name.tsv ledger rounds w58, w72, w77, w83, w94 (every round NO_NAME on the particle_mode route: "
          "they read the mode tests as range tests only; the per-arm content, texture-record selection and command byte / quad count, "
          "was not weighed) and func-second-pass-13 (wave103, dropped: slot route alone confirmed, 'RecycledYawedParticles' fitted both "
          "issuers)")


def _evidence(addr):
    old, cmd, phase, hidx, proto, tbl, endl, csvl, nsl, defl = FN[addr]
    c = CMD[cmd]
    draw = phase == 2
    nm = f"eff_{'DrawAndAdvance' if draw else 'Spawn'}{c['noun']}{c['nth']}" + ("" if draw else ("FirstHalf" if phase == 0 else "SecondHalf"))
    mode, row, slot = c["mode"], c["row"], c["slot"]
    if draw:
        body = (f"eff_PrepareParticles({mode}, {row}); eff_UpdateParticles({mode}, {row}); return eff_LinkParticleQuads({mode}, {row}); "
                f"{_MODEFACT[mode]}; Link (:3158-3205) adds the new quads to g_gpu_ot_ptr and returns has_quads, which becomes status[{slot}] "
                "(0 ends the command)")
    else:
        body = (f"spawn_result = eff_SpawnParticles({mode}, {row}, {phase}); g_eff_command_phase[{slot}] = {phase + 1}; return spawn_result. "
                f"eff_SpawnParticles (51268.c:2628-2720) half_index {phase}: " +
                ("resets the row fill index, latches g_eff_command_position / g_eff_command_vector (the issuer's yawed vector) into the group "
                 "position and g_eff_particle_emitter_vectors[row] (:2651-2666), then spawns records 0..23 (:2685-2687); returns 1"
                 if phase == 0 else
                 "records 24..47 with the emitter vector latched by the half 0 call (the half_index == 0 latch at :2653 is not taken); returns 1") +
                f". THE SPAWN BODY IS IDENTICAL FOR MODES 4-7 (the `< 6` and `< 8` arms of angle_mask / angle_offset are equal, :2675-2681, the mode "
                f"is not stored in the record, :2706-2717): the noun comes from the command this phase belongs to, whose phase-2 handler "
                f"({'func_800677B8' if cmd == 0x16 else 'func_800677F4' if cmd == 0x17 else 'func_80067830' if cmd == 0x18 else 'func_8006786C'}) "
                f"renders the records these halves fill")
    callers = (f"none by name: the one address-taken use is g_eff_command_handlers[{hidx}] = g_eff_command_handler_bases[{slot}] + phase {phase} "
               f"(src/main/d_8BC50.c:{tbl}; prototype :{proto}); all other occurrences of {old} are the C definition src/main/51268.c:{defl}, "
               f"the asm glabel/endlabel asm/funcs/{old}.s:1,{endl}, the census docs/naming/function-names.csv:{csvl} and the named_syms.txt:{nsl} "
               "RESET note (retired efc_ name); no symbol_addrs.txt line" +
               ("; plus two comments in include/game.h, :782 ('particle_row_index 0..3 from the callers func_800676C8..func_8006786C', a range "
                "end) and :1117-1118 ('each of the wrappers func_800676C8 .. func_8006786C calls the three in turn', likewise a range end): the "
                "dry run does NOT edit include/game.h, so the apply must hand-edit both comments in the wave and rerun the keycheck "
                "--sub-comments" if addr == "0x8006786C" else "") +
               f". Command {hex(cmd)} (word {c['word']}) is issued only by {c['issuer']} ({c['il']}); " + _DISPATCH)
    why = (f"{'DrawAndAdvance = the body steps frame_age, kills records and draws POLY_FT4 quads linked into the OT, keeps the command alive until no quads remain' if draw else 'Spawn = it fills the particle records (velocities, ages 0, life_state 1); FirstHalf / SecondHalf = half_index 0 / 1 (records 0..23 / 24..47, the first also latches the emitter), the phase := 1 / 2 written'}; "
           f"{c['noun']} = {'TextureSelected: modes 4/5 alone read a table entry by index and flag' if c['noun'].startswith('Texture') else 'AdditiveSingleQuad: modes 6/7 alone emit exactly one semi-transparent 0x2E quad per particle'}"
           f"{' (the draw reads them itself)' if draw else ' (call-graph role: this handler is phase ' + str(phase) + ' of the command whose draw does)'}; "
           f"{c['nth']} = status[{slot}] ({'first' if c['nth'] == 'FirstSlot' else 'second'} free byte of the issuer). Siblings: {_SIB[cmd]}. " + _NOUN)
    cov = COVLINE[addr]
    other = hex({0x16: 0x19, 0x19: 0x16, 0x17: 0x18, 0x18: 0x17}[cmd])
    if c["noun"].startswith("Additive"):
        c1 = ("(1) 'Additive' rests on the GP0 semi-transparent command bit (0x2E vs 0x2C) plus the tpage word 0x2E (ABR = 1, B+F), a hardware "
              "role; for a textured primitive the blend applies per texel only where the CLUT entry has its STP bit set, and the CLUT "
              "(0x380, 0xFF) is VRAM data not in the repo, so additive is the primitive's mode, not a pixel claim; ")
    else:
        c1 = ("(1) 'TextureSelected' is a code fact of eff_PrepareParticles (:2812-2823), true of the draw's own reads; the spawns only fill "
              "records, so their noun is the call-graph role; ")
    c2 = (f"(2) particle row {row} is shared with command {other}'s trio "
          "(row 2 = modes 4 and 7, row 3 = modes 5 and 6), so 'Slot' is the command slot, not the particle row; "
          "(3) the pair FirstHalf / SecondHalf and the three phases differ only by half_index and the phase value written; "
          "(4) the eight spawns have no body-level difference between modes 4-7 (see BODY), so the spawn names rest on the command; the "
          "fallback if the reviewer rejects command-derived nouns on the spawns is NO_NAME for the eight spawns with the four draws kept")
    counter = c1 + c2
    trace = (f"TRACE (Q124, corroborating only): docs/naming/runtime/matrix/coverage.tsv:{cov} hits {COVHIT[cmd]} (header line 1; issuers "
             "func_80061C00 :831, func_80061D74 :832); the trace adds no stage, move or visual word, none is claimed")
    if addr == "0x80067830":
        trace += ("; stub-and-observe docs/naming/runtime/versus-2p-kimigaura-kick-particle-stub-2/knockout.md (func_80067830, SHA1 "
                  "eaa48d39c071e733b23598f69b0e0428802b9abe, baseline 924cc770b7e6f019f645f49aa88161904bde81f8): 0 changed px at 9150-9156, 3 px at "
                  "9157/9158, 87 px at 9160 (whole-screen divergence from 9162 is not evidence), consistent with Draw only")
    if addr == "0x8006786C":
        trace += ("; the Q124 stub of func_8006786C (SHA1 3b0ef2fc22793ad804564163f6593100a9886286) changed 0 px at every compared frame up to "
                  "9160, so it neither corroborates nor contradicts: CODE-ONLY row")
    return nm, body, callers, why, counter, trace


FUNCS = []
for _addr in FN:
    _nm, _body, _callers, _why, _counter, _trace = _evidence(_addr)
    _ev = (f"BODY: {B(FN[_addr][0], '51268.c')}: {_body}. CALLERS: {_callers}. WHY: {_why}. COUNTER-EVIDENCE: {_counter}. "
           f"Prior manifests: {_PRIOR}. {_trace}")
    FUNCS.append((_addr, _nm, "RENAME", _ev))

DATA = []
