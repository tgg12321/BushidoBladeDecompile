"""sweep-368e4-r3.3 rows (unit sweep-368e4-r3.3, kind function): the getter func_80046780 (src/main/368E4.c:332).
Research input tmp/codex/sweep-368e4-r3.3-research.md ROW 0x80046780 (untrusted; every claim re-derived at HEAD ed9268cc2, see notes.txt). The research proposed
resource_GetStagePointPairRecords; the global that actually landed (sweep-368e4-r3.1, 55d64368a) is g_resource_stage_counted_record_list, so the research's ALT name is used.
Verdict PENDING (the session sets CONFIRM after the naming-reviewer pass). The row is a NO_NAME ledger row (no_name.tsv:18) revived by that global's name. (DATA is empty.)
"""
VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS (1/1, 2026-11-07, naming-c): pure Get of g_resource_stage_counted_record_list; not dead / forwarder; revive valid (55d64368a)"
NOTES = {}
DROPPED = {}

FUNCS = [
    ("0x80046780", "resource_GetStageCountedRecordList", "RENAME",
     "BODY: src/main/368E4.c:332 `s32 func_80046780(void) { return g_resource_stage_counted_record_list; }` "
     "(asm/funcs/func_80046780.s:1-5: one `lw $v0, %gp_rel(g_resource_stage_counted_record_list)($gp)` at :2, `jr $ra` :3, delay-slot nop :4). "
     "The global (was D_800A33B0, named in sweep-368e4-r3.1, 55d64368a; static s32 at 368E4.c:24; symbol_addrs.txt:152) holds the loaded stage block's header section 3 "
     "(u32 count + 20-byte records, docs/formats/NDATA.md 'Stage header section 3'); it is stored at 368E4.c:188 and :260 and rebased by the move callback at :279. "
     "CALLERS / USES: none. git grep -n -w func_80046780 over src include asm symbol_addrs.txt named_syms.txt gives only the def (368E4.c:332) and the glabel/endlabel (asm/funcs/func_80046780.s:1, :5); "
     "in docs it appears in NDATA.md:224 (states no jal/j/data word/%hi-%lo references it), the census (function-names.csv:551), no_name.tsv:18, runtime facts.tsv:813 (never_hit) and :2036 (reach, hit []), and older wave notes / manifests; "
     "named_syms.txt:120 and symbol_addrs.txt:152 carry the retired/old name only as history. Not dead: it reads a named global. Not forwarder: no call, no argument. "
     "SIBLINGS: the adjacent getters fight_GetPositionRecordList (368E4.c:334, returns g_fight_position_records) and resource_GetLoadedSelector (:336, returns g_loaded_resource_selector) are landed: "
     "Get + the global's noun. This takes the resource_ prefix of its global and TU (the global is resource_-owned, loaded by resource_LoadOrReuseSelectedBlock). "
     "WHY: verb Get + the returned global's noun phrase 'stage counted record list' (stage block header section 3, a count followed by records). "
     "COUNTER-EVIDENCE: (1) the getter is never called (runtime never_hit across the targeted captures), so there is no consumer showing what the records mean; the name states only the structure the global is already named for; "
     "(2) it returns the pointer as s32, so a reader might expect a typed pointer; the name does not claim a type; "
     "(3) the research's first choice resource_GetStagePointPairRecords rejected: 'point pair' was a guess, the same guess the pre-apply review FAILed for the global in sweep-368e4-r3.1; "
     "(4) the ledger row's recorded objections (no caller, placeholder global) are answered by the global's landed name, the revive evidence."),
]

DATA = []