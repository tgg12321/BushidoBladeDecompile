"""wave103 rows (func-second-pass-13): the command-slot route to the four NO_NAME 0x18/0x19 particle handlers of
eff_StartRecycledYawedParticles (51268.c:643-684): 0x80067130 / 0x80067164 (spawn halves) and 0x80067830 (draw) of command
0x18, 0x8006786C (draw) of command 0x19. The five NO_NAME ledger rounds (w58, w72, w77, w83, w94; no_name.tsv:81,82,91,92)
tried the particle_mode route (every mode test is a range test); this draft keys on the issuer's command slot and the handler
table instead (d_8BC50.c:200-203, :323-328). Research input: tmp/fsp13-research.md (4 RENAME; untrusted; every use, table
index, issuer line and knockout pixel number re-derived at HEAD b823eb6a9; the research's line cites for the 0x17 trio were
off by a few lines and are corrected here). Q124 runtime (7ff0439d1) corroborates three rows and nothing about 0x8006786C:
that row is CODE-ONLY (its stub showed no difference). Verdicts PENDING.
(DATA is empty.)
"""
import glob
import os
import re

VERDICT = "PENDING"
VERIFIER_NOTE = ""
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x80067130": ("eff_SpawnRecycledYawedParticlesFirstSlotFirstHalf", "eff_SpawnRecycledYawedParticlesFirstSlotFirstHalf", ""),
    "0x80067164": ("eff_SpawnRecycledYawedParticlesFirstSlotSecondHalf", "eff_SpawnRecycledYawedParticlesFirstSlotSecondHalf", ""),
    "0x80067830": ("eff_DrawAndAdvanceRecycledYawedParticlesFirstSlot", "eff_DrawAndAdvanceRecycledYawedParticlesFirstSlot", ""),
    "0x8006786C": ("eff_DrawAndAdvanceRecycledYawedParticlesSecondSlot", "eff_DrawAndAdvanceRecycledYawedParticlesSecondSlot", ""),
}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-b")

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


def R(addr, new, fn, body, callers, why, counter, prior, trace):
    assert ADJUSTED[addr][1] == new, (addr, new)
    ev = (f"BODY: {B(fn, '51268.c')}: {body}. CALLERS: {callers}. WHY: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Prior manifests: {prior}. {trace}")
    return (addr, new, "RENAME", ev)


_DISPATCH = ("the only issuer of command words 0x10018 / 0x10019 is eff_StartRecycledYawedParticles (src/main/51268.c:643-684, "
             "CORROBORATED wave83; git grep 0x1001[89] over src/asm/include hits only 51268.c:678 and :681 and the asm "
             "asm/funcs/eff_StartRecycledYawedParticles.s:64-65, :76-77): it builds a yawed command_vector (:650-668), "
             "clears status[24] and status[25] when both are busy (:672-675), then status[24] == 0 -> command 0x10018 with "
             "status_dest &status[24] (:676-678), else status[25] == 0 -> 0x10019 with &status[25] (:679-682), then "
             "eff_StartCommand (:683); its only caller is fight_RecordReplayEventAndDispatchSoundOrEffect case 13 "
             "(src/main/17AFC.c:5087-5090, after snd_QueuePositionalSound(0x79)); eff_StartCommand (51268.c:84-115) clears "
             "g_eff_command_phase[idx] (:89) and calls g_eff_command_handlers[g_eff_command_handler_bases[idx] + phase] "
             "(:105-108) storing the result in status_dest, eff_StepCommand (:117-143) repeats the call each frame while "
             "status != 0; the per-frame loop func_80061064 (:268-282) only re-issues live indices. Tables: "
             "g_eff_command_handler_bases (src/main/d_8BC50.c:200-203) [24] = 0x36 = 54, [25] = 0x39 = 57; "
             "g_eff_command_handlers (d_8BC50.c:268-330, index 0 at :269) [54] func_80067130 (:323), [55] func_80067164 (:324), "
             "[56] func_80067830 (:325), [57] func_80067198 (:326), [58] func_800671CC (:327), [59] func_8006786C (:328). "
             "Lifecycle by the code: phase 0 handler (spawn half 0, phase := 1, returns 1), next frame the phase 1 handler "
             "(spawn half 1, phase := 2, returns 1), then the phase 2 handler every frame until eff_LinkParticleQuads returns "
             "0 (no quads), which zeroes the status and frees the slot")

_NOUN = ("eff_ and Particles = unchanged from the landed siblings (eff_SpawnParticles 51268.c:2628, eff_PrepareParticles :2779, "
         "eff_UpdateParticles :2893; census function-names.csv:887, :896, :897); the Spawn / DrawAndAdvance split follows "
         "eff_SpawnRotatedQuadCluster / eff_DrawAndAdvanceRotatedQuadClusters (function-names.csv:829-830); "
         "RecycledYawedParticles = the issuer's own noun (eff_StartRecycledYawedParticles; Recycled = it clears both slots "
         "when both are busy, 51268.c:672-675; Yawed = the yaw-rotated command_vector :650-668), used here to identify the "
         "command pair 0x18/0x19, not an action of the handler; FirstSlot / SecondSlot = the command word the issuer picks "
         "(status[24] first free = 0x18, else status[25] = 0x19), the precedent of eff_SpawnFighter0YawedSlotQuad / "
         "eff_SpawnFighter1YawedSlotQuad (function-names.csv:833-834). The sibling trios do not share the pair: 0x16/0x17 "
         "(spawns 51268.c:2571-2594, draws :2750-2760) belong to a different issuer, "
         "eff_StartRecycledYawedParticlesWithTextureSelection (:594-641, same slot logic plus "
         "g_eff_yawed_particle_alternate_texture_flags[0|1], read for modes 4/5 at :2818, :3135), so the mode-free word "
         "'YawedParticlesFirstSlot' would also fit 0x16 and fails the sibling test, while 'RecycledYawedParticles' covers "
         "only 0x18/0x19")

_PRIOR = ("docs/naming/phase3/no_name.tsv ledger rounds w58, w72, w77, w83, w94 ({row}), every round NO_NAME on the particle_mode "
          "route (every mode test in eff_SpawnParticles / eff_PrepareParticles / eff_UpdateParticles is a range test: the code "
          "shows no per-mode difference); the command-slot route (issuer slot -> handler table) was not taken in any round, "
          "and the issuer itself was named only in wave83")

_COV = ("docs/naming/runtime/matrix/coverage.tsv (header line 1): eff_StartRecycledYawedParticles :832, func_80067130 :891, "
        "func_80067164 :892, func_80067830 :902, func_8006786C :903; the first four are hit in exactly the same phases "
        "(lib-versus-2p-kimigaura-kick-2p-1 kick2p_3:1+, lib-versus-2p-kimigaura-shore-kick-1 kick_3:1+ .. kick_7:1+), zero in "
        "every other capture; func_8006786C is hit only in kick2p_3:1+ (the phase where two kicks overlap: commit 7ff0439d1 "
        "body, P2 starts command 0x18 at frame 9148, P1 starts 0x19 at 9150), consistent with first-free slot allocation; "
        "both hitting captures are Kimigaura kick scenarios, so the trace adds no stage, move or visual word, and none is "
        "claimed")

_KO = ("docs/naming/runtime/versus-2p-kimigaura-kick-particle-stub-2/ (baseline capture.manifest.json SHA1 "
       "924cc770b7e6f019f645f49aa88161904bde81f8; scenario versus-2p-kimigaura-kick-2p, stubs from phase fight frame 8230, "
       "ret 0, the game ran to end_frame 10330, knockout.md status ok)")

FUNCS = [
    R("0x80067130", "eff_SpawnRecycledYawedParticlesFirstSlotFirstHalf", "func_80067130",
      "spawn_result = eff_SpawnParticles(6, 3, 0); g_eff_command_phase[24] = 1; return spawn_result. eff_SpawnParticles "
      "(51268.c:2628-2720, comment :2618-2626) with half_index 0 resets g_eff_particle_group_fill_indices[3], latches "
      "g_eff_command_position / g_eff_command_vector (the issuer's yawed vector) into the group position and "
      "g_eff_particle_emitter_vectors[3] (:2662-2675), and spawns records 0..23 (loop bounds (0x30 >> 1) * half_index .. "
      "* (half_index + 1), :2694-2696), each with a random velocity from the emitter vector; returns 1",
      "none by name: the one address-taken use is g_eff_command_handlers[54] = g_eff_command_handler_bases[24] + phase 0 "
      "(src/main/d_8BC50.c:323; prototype :259); all other occurrences of func_80067130 are the C definition "
      "src/main/51268.c:2595, the asm glabel/endlabel asm/funcs/func_80067130.s:1,15 and the census "
      "docs/naming/function-names.csv:883 (git grep over src include asm symbol_addrs.txt named_syms.txt "
      "function-names.csv; no symbol_addrs.txt line; named_syms.txt:1692 is a RESET note naming only the retired "
      "efc_spawn_80067130). Dispatch: " + _DISPATCH,
      "Spawn = it fills the particle records (the effect of the body: records 0..23 get velocities, the first half latches "
      "the emitter); RecycledYawedParticles FirstSlot = the command it serves: phase[24] is written and the handler sits at "
      "base[24] + 0, i.e. the phase 0 handler of command 0x18, the issuer's first slot; FirstHalf = half_index 0, the half "
      "that also latches the emitter; the pair FirstHalf / SecondHalf differ only in half_index (0 / 1) and the phase "
      "value written (1 / 2). " + _NOUN,
      "(1) 'Recycled' is the issuer's word and names the command pair here, not an action of this handler (no recycling "
      "happens in the body); (2) the name does not claim the particle row: g_eff_particles row 3 is shared with command "
      "0x17's trio (mode 5 row 3, 51268.c:2583-2593 spawns, :2756-2760 draw), 'slot' is the command slot of the issuer; "
      "(3) the phase write is part of the name's effect only as the handler's own phase advance, the same shape as every "
      "sibling spawn handler (e.g. func_80066FF8 :2559-2563); (4) the long spawn names are needed to tell the pair apart "
      "(FirstHalf also latches the emitter); (5) no runtime stub: the trace only shows the same hit phases as the issuer, "
      "which corroborates that it runs with it and fixes nothing about the name",
      _PRIOR.format(row="no_name.tsv:81"),
      "TRACE (Q124, corroborating only; no stub of this function): " + _COV + "."),

    R("0x80067164", "eff_SpawnRecycledYawedParticlesFirstSlotSecondHalf", "func_80067164",
      "spawn_result = eff_SpawnParticles(6, 3, 1); g_eff_command_phase[24] = 2; return spawn_result. half_index 1 "
      "(eff_SpawnParticles 51268.c:2628-2720): records 24..47 of g_eff_particles[3] with the emitter vector latched by the "
      "half 0 call (the half_index == 0 latch at :2661 is not taken); returns 1",
      "none by name: the one address-taken use is g_eff_command_handlers[55] = g_eff_command_handler_bases[24] + phase 1 "
      "(src/main/d_8BC50.c:324; prototype :260); all other occurrences of func_80067164 are the C definition "
      "src/main/51268.c:2601, the asm glabel/endlabel asm/funcs/func_80067164.s:1,15 and the census "
      "docs/naming/function-names.csv:884 (no symbol_addrs.txt line; named_syms.txt:1693 is a RESET note naming only the "
      "retired efc_spawn_80067164). Dispatch: " + _DISPATCH,
      "Spawn / RecycledYawedParticles FirstSlot as the half 0 handler (phase[24] written, table slot base[24] + 1 = the phase "
      "1 handler of command 0x18); SecondHalf = half_index 1, the second 24 of the 48 records, spawned the frame after the "
      "first half (phase 1 is reached once phase 0 set it to 1). " + _NOUN,
      "(1) as func_80067130: 'Recycled' names the command pair; (2) the row is shared with command 0x17's trio and is not "
      "claimed; (3) SecondHalf is the second half of the spawn only because the phase handlers run one per frame in the "
      "order 0, 1, 2, which the code shows (phase := 1, := 2); (4) no runtime stub, only the same hit phases as the issuer",
      _PRIOR.format(row="no_name.tsv:82"),
      "TRACE (Q124, corroborating only; no stub of this function): " + _COV + "."),

    R("0x80067830", "eff_DrawAndAdvanceRecycledYawedParticlesFirstSlot", "func_80067830",
      "eff_PrepareParticles(6, 3); eff_UpdateParticles(6, 3); return eff_LinkParticleQuads(6, 3). Prepare (51268.c:2779) "
      "sets the sprite and texture state (modes 6/7 share the `< 8` branch, :2829-2837); Update (:2893, comment "
      ":2883-2892) steps each live record's frame_age, kills records, and emits up to three POLY_FT4 per record; Link "
      "(:3158-3201) adds the new quads to g_gpu_ot_ptr and returns has_quads, which becomes status[24] and ends the "
      "command at 0",
      "none by name: the one address-taken use is g_eff_command_handlers[56] = g_eff_command_handler_bases[24] + phase 2 "
      "(src/main/d_8BC50.c:325; prototype :261); all other occurrences of func_80067830 are the C definition "
      "src/main/51268.c:2762, the asm glabel/endlabel asm/funcs/func_80067830.s:1,17 and the census "
      "docs/naming/function-names.csv:894 (no symbol_addrs.txt line; named_syms.txt:1438 is a RESET note naming only the "
      "retired efc_helper_80067830). Dispatch: " + _DISPATCH,
      "DrawAndAdvance = the body advances the records (frame_age, kills) and draws them (POLY_FT4 quads linked into the OT) "
      "and keeps the command alive until no quads remain, the verb pair of eff_DrawAndAdvanceJitteredBillboards and the "
      "other landed siblings; RecycledYawedParticles FirstSlot = phase 2 handler of command 0x18 (base[24] + 2). " + _NOUN,
      "(1) particle row 3 is shared with command 0x17's trio, so FirstSlot is the command slot, not the particle storage; "
      "mode 6 is reached only through this trio and its spawns; (2) the stub changed pixels only where the lost output is "
      "plausibly this function's own drawing: " + _KO + "; with func_80067830 stubbed (called while stubbed in kick2p_3, "
      "kick_4 .. kick_7; knockout.tsv) frames 9150, 9152, 9154, 9155, 9156 are 0 changed px of 153600, 9157 and 9158 are 3 "
      "px (bbox 469,149-470,150), 9160 is 87 px (bbox 449,138-471,151), 9162 is 50025 px and 9165 43298 px "
      "(whole-screen divergence), so only 9150-9160 are evidence; the commit body reads the lost pixels as a small white "
      "puff at P2's front foot, a viewed-zoomed reading this row does not repeat and no name claims; from 9162 the "
      "divergence is plausibly the skipped eff_UpdateParticles rand() draws shifting the shared rand() sequence (a likely "
      "cause, not established), which is why the later frames are not used; (3) the draw name claims only that it draws, "
      "no visual",
      _PRIOR.format(row="no_name.tsv:91"),
      "TRACE (Q124, corroborating): " + _COV + ". Stub: " + _KO + "; func_80067830/capture.manifest.json SHA1 "
      "eaa48d39c071e733b23598f69b0e0428802b9abe; knockout.tsv numbers above; the lost effect is the function's own output "
      "(a drawn effect absent), consistent with Draw."),

    R("0x8006786C", "eff_DrawAndAdvanceRecycledYawedParticlesSecondSlot", "func_8006786C",
      "eff_PrepareParticles(7, 2); eff_UpdateParticles(7, 2); return eff_LinkParticleQuads(7, 2), the same shape as "
      "func_80067830 with mode 7 / row 2 (same `< 8` Prepare branch, 51268.c:2829-2837; Update :2893; Link :3158-3201)",
      "none by name: the one address-taken use is g_eff_command_handlers[59] = g_eff_command_handler_bases[25] + phase 2 "
      "(src/main/d_8BC50.c:328; prototype :264); all other occurrences of func_8006786C are the C definition "
      "src/main/51268.c:2768, the asm glabel/endlabel asm/funcs/func_8006786C.s:1,17, the census "
      "docs/naming/function-names.csv:895 and two comments in include/game.h: :782 ('particle_row_index 0..3 from the "
      "callers func_800676C8..func_8006786C', a generic range end) and :1118 ('each of the wrappers func_800676C8 .. "
      "func_8006786C calls the three in turn', likewise a range end over all particle wrappers; both still name the old "
      "spelling and the dry run does NOT list include/game.h among its file edits, so the apply must hand-edit them in the "
      "wave, e.g. to the new name or to a range-end wording, and the keycheck must be run on that); no symbol_addrs.txt line; "
      "named_syms.txt:1439 is a RESET note naming only the retired efc_helper_8006786C. Dispatch: " + _DISPATCH,
      "same as func_80067830 for the body (advance and draw, return has_quads); SecondSlot = phase 2 handler of command "
      "0x19, the issuer's second slot (status[25] free when status[24] is busy, 51268.c:679-682; base[25] + 2). THE ROW IS "
      "CODE-ONLY: the Q124 stub of this function showed no difference, so the runtime corroborates nothing here; the name "
      "stands on the code argument (the body is the func_80067830 shape and the table ties it to the issuer's second "
      "slot), which per naming-runtime-evidence is what establishes a name (the trace only corroborates). " + _NOUN,
      "(1) NO RUNTIME CORROBORATION: " + _KO + "; func_8006786C/capture.manifest.json SHA1 "
      "3b0ef2fc22793ad804564163f6593100a9886286; knockout.tsv: 0 changed px at 9150, 9152, 9154, 9155, 9156, 9157, 9158 and "
      "9160, 50113 px at 9162 and 43347 px at 9165 (whole-screen divergence, not evidence); the trace neither shows nor "
      "contradicts a draw (no particle need be on screen in that window), while the sibling draw func_80067830 did lose "
      "a few pixels in the same window; this row therefore claims no visual and relies on the code; (2) a hit only in "
      "kick2p_3 (coverage.tsv:903), so one phase; the second slot is used only when both kicks overlap, which the code "
      "predicts (first-free allocation) and the shore-kick run never needs; (3) particle row 2 is shared with command "
      "0x16's trio (mode 4 row 2, 51268.c:2571-2581 spawns, :2750-2754 draw), so SecondSlot is the command slot, not "
      "the particle storage",
      _PRIOR.format(row="no_name.tsv:92"),
      "TRACE (Q124, hit map only; the stub showed no difference): " + _COV + "."),
]

DATA = []
