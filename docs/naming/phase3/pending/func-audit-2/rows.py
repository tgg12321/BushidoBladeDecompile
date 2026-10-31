"""wave109 rows (func-audit-2): function-name correction wave, second batch after func-audit-1 (wave105).
2 already-admitted function names whose name-audit findings (overclaims / wrong) were re-derived, each renamed to
a truer name. Research input: tmp/codex/func-audit-2-research.md (2 RENAME, 0 RESET, 4 KEEP; untrusted proposals);
every home and use re-derived at HEAD 891427c44 (git grep -w over src include asm named_syms.txt symbol_addrs.txt
inline_asm_canonical.txt known_psyq_stdlib.txt tools engine docs/engine .claude docs/naming/function-names.csv).
0 changed, 0 dropped. The 4 KEEP functions are not rows (reasons in notes.txt). (DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS 2/2 (2026-10-31, naming-b); 4 audited names KEEP, each PASS (notes.txt)"
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x80041988": ("gpu_LoadRequestedMaskedTimGroupsForSlot", "gpu_LoadRequestedMaskedTimGroupsForSlot", ""),
    "0x80049E4C": ("node_InitViewRootNodePair", "node_InitViewRootNodePair", ""),
}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-b")

_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")


def B(fn, f):
    """src/main/<file>:<first>-<last> of the C definition of fn (INCLUDE_ASM line when no C body)."""
    lines = _SRC[f]
    for k, l in enumerate(lines):
        if l.startswith("INCLUDE_ASM") and re.search(r"\b%s\b" % fn, l):
            return f"src/main/{f}:{k + 1}"
        if not re.match(r"^[A-Za-z_][\w \*]*\b%s\(" % fn, l) or l.startswith("extern"):
            continue
        j = k
        while j < len(lines) and lines[j] != "}" and not (j == k and l.rstrip().endswith("}")):
            j += 1
        return f"src/main/{f}:{k + 1}-{j + 1}" if j != k else f"src/main/{f}:{k + 1}"
    raise SystemExit(f"no C definition of {fn}")


def R(addr, new, fn, cfile, audit, body, why, counter):
    assert ADJUSTED[addr][1] == new, (addr, new)
    ev = (f"AUDIT: {audit}. BODY: {B(fn, cfile)}: {body}. WHY RENAME: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Research input tmp/codex/func-audit-2-research.md, every use re-derived at HEAD (no change against the research).")
    return (addr, new, "RENAME", ev)


FUNCS = [
    R("0x80041988", "gpu_LoadRequestedMaskedTimGroupsForSlot",
      "gpu_LoadRequestedAvailableTimGroupsForSlot", "31D3C.c",
      "audit-99 #26 (overclaims; suggested gpu_LoadRequestedMaskedTimGroupsForSlot)",
      "returns unless slot_index < 2; load_mask = g_gpu_asset_tim_group_load_masks[asset_index]; for bit 0x10..0x01 (i = 0..4) loads TIM group i+1 "
      "only when the bit is set in BOTH load_mask and tim_group_request; slot 0 calls gpu_LoadTimGroupSplitOffset(.., 0, 0, -0x140, 0xE8) when "
      "gpu_GetFogClutObject() == 0, else gpu_LoadTimGroup; slot 1 the 0x80-offset variants (gpu_LoadTimGroupSplitOffset / gpu_LoadTimGroupOffset). "
      "One caller: slot_FinalizeObjectLoadSoundTexturesAndArena (309CC.c:274; asm jal .s:194) with (arena_id, asset_index, "
      "g_slot_tim_group_requests[arena_id], tim_groups); afterwards the caller stores the whole request (not the intersection) into load_flags bits "
      "16..20 and clears it (309CC.c:279-282); the request is written by slot_RequestTimGroupsAndMarkReloadOnChange (31D3C.c:40-49). The mask "
      "g_gpu_asset_tim_group_load_masks (31D3C.c:525, u8[64], .data, read only here: .s:18/:20) is all-or-nothing: 0x1F for asset indices 0..12, "
      "15..27, 30, 63, else 0",
      "the body intersects the request with a constant per-asset load mask; nothing in the code shows the mask records which groups an asset has "
      "(no read of the asset's own data, no count, no presence test), so 'Available' claims a meaning the code does not show. 'Masked' names exactly "
      "what the body tests and agrees with the admitted global g_gpu_asset_tim_group_load_masks. Kept parts all hold at the one use: gpu_ (callees are "
      "gpu TIM-group loaders), Requested (g_slot_tim_group_requests[slot]), TimGroups, ForSlot (slot_index selects the VRAM offsets)",
      "the all-or-nothing mask pattern hints it may mean 'asset carries groups 1..5', which would make 'Available' a fair reading, but no asset-side "
      "evidence ties it; the narrower word wins. A reviewer may prefer KEEP as defensible; the rename is small"),
    R("0x80049E4C", "node_InitViewRootNodePair",
      "node_InitRootNodePairWithNegatedRotationMatrices", "368E4.c",
      "audit-103 #5 (wrong; suggested node_InitRootNodePairFromNegatedRotationAngles)",
      "for g_view_camera_root_node (node_kind 0x64, rotation_order 5) and g_view_identity_root_node (node_kind 0x65, rotation_order 2): "
      "primitive_table_select = 0, xf.rot = {0,0,0}, work.t = {0,0,0}, parent_node = 0, then node_SetMatrixFromNegatedRot(node); finally "
      "g_view_camera_root_ptr and g_view_identity_root_ptr are set to the two nodes. One call, func_80046B44 (368E4.c:434, the engine reset; "
      "asm jal func_80046B44.s:12), itself called from 2B344.c:691 and 6CF8.c:300; local extern 368E4.c:313. The callee node_SetMatrixFromNegatedRot "
      "(31D3C.c:155-161) negates a stack copy of xf.rot (the angles), passes it to g_math_rot_matrix_funcs[rotation_order] into &work and copies "
      "work to xf.mat; with rot = 0 the result is the identity rotation",
      "no matrix is negated anywhere: the callee negates angles and here they are zero, so 'WithNegatedRotationMatrices' is false. The audit's "
      "'FromNegatedRotationAngles' restates the callee's mechanics but misleads here (zero angles; the negation is a no-op on this path). The body's "
      "whole effect is to initialise the two view root nodes (the admitted g_view_camera_root_node / g_view_identity_root_node), including their "
      "matrices, and publish them through g_view_*_ptr, which 'Init...Pair' covers. node_ is the prefix of the Unk80101DF0Record node routines and of "
      "the callee node_SetMatrixFromNegatedRot; 'Root' (both get parent_node = 0) is kept from the current name; 'View' comes from the globals' names",
      "the name does not list the pointer publication or the matrix build; both are part of initialising the pair, so it claims no more than shown. "
      "A reviewer may prefer node_InitViewRootNodePairAndPointers; not needed to remove the false word"),
]

DATA = []
