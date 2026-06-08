# WIP: func_800484A0 (src/text1b.c)

## TL;DR
HEAD honest pure-C distance: **5** (5 regfix rules). Candidate floor: **4**, build_insns matches target (36==36). One real lever found: the function semantically uses arg0 (the original input pointer) — NOT buf — as gpu_LoadImage's source when func_800486FC() returns 0 (no decompression). HEAD's body wires gpu_LoadImage to buf unconditionally, which is wrong in the false-decompress path. Candidate restores the correct semantic via `arg0 = (u8*)buf;` inside the if-true block, then `gpu_LoadImage(rect, (s32)arg0);` unconditionally. This matches what target's $s0 reuse does at runtime.

## Resume

1. Apply `candidate.c` to `src/text1b.c` (replace the body of `func_800484A0` starting around line 359).
2. `& tools/eng.ps1 sandbox func_800484A0 --disable all` → expect score 4 (down from HEAD's 5), build_insns 36.
3. Confirm the 4 remaining diffs match the breakdown in `meta.json.remaining_gap.summary` by disassembling `tmp/sandbox/func_800484A0/text1b.o` vs `build/src/text1b.o` (`mipsel-linux-gnu-objdump -d`).
4. Pick a hypothesis from `meta.json.next_hypotheses` and execute. Permuter from this base is the highest-leverage next move.

## Why the candidate is not a cheat

`arg0 = (u8 *)buf;` is a parameter reassignment, not a dead store / chain-extender / DImode trick. The 6-test vetting:

- **Semantic purpose: YES.** Target asm proves the gpu_LoadImage's $a1 is arg0 in the if-false path (where func_800486FC() returned 0). Without the reassignment, HEAD's `gpu_LoadImage(rect, (s32)buf)` would upload an uninitialized buf in that path — a behavior bug. The reassignment captures the if-true semantic ("after decompression, the source for upload is buf, not the input pointer"). A sibling function at line 280-296 of the same file uses the *exact* same `gpu_LoadImage(rect, decompressed_or_original)` pattern with a separate variable.
- **Human-programmer test: YES.** Reusing a parameter pointer once the original use ends (passed to a callee) is idiomatic — the parameter is now a free local. A real programmer might write this rather than introduce an `s32 *src;` local for one branch divergence. Less common than the `src` local form but unambiguously legitimate C.
- **GCC-internals justification: NO.** The construct's existence is justified by target's semantic behavior (the runtime behavior of the if-false path), not by RTL pseudo numbering / RA priority / combine fold.
- **Permuter-derived: NO.** Derived by reading target asm and recognizing the $s0 reuse pattern.
- **Family check: NOT IN ANY FORBIDDEN FAMILY.** Not dead-store (the value IS read at gpu_LoadImage). Not Lever D dead-param-assign (the param is referenced afterward). Not chain-extender. Not DImode chain. Not register-asm pin. Not volatile-coerce. Not lost-codegen-insert. Not inline-asm-injection. Not OR-tree shuffle.
- **Naming announces intent: NO.** `arg0` keeps its parameter name; no `_pad`/`_dummy`/`_spill`/etc.

The candidate is SAFE TO COMMIT as a partial-progress wip checkpoint. Whether it could be committed as part of a future COMPLETED-C close depends on whether the remaining 4 diffs can be eliminated; the cheat-reviewer agent should be invoked at that point.

## What remains (the 4 diffs)

Disassembly comparison (build vs target, both 36 insns):

| idx | build (candidate) | target | issue |
|---|---|---|---|
| 12 | `addiu s0,s0,12` (delay) | `addiu s0,s0,4` (delay) | GCC combined arg0 += 4; arg0 += 8 into +12 |
| 13 | `sh a1,16(sp)` | `addiu s0,s0,8` | (shifted by 1 from previous) |
| 14 | `sh a2,18(sp)` | `sh a1,0x10(sp)` | (shifted by 1) |
| 24 | `addiu v0,sp,24` | `addiu s0,sp,0x18` | target writes to $s0 directly |
| 25 | `move s0,v0` | (target's idx 25 is jal) | extra reg-move; cancels target's extra +8 |

So there are TWO independent structural issues, each adding 1 insn and removing 1 insn (net 0 to build_insns count, but creating 4 misaligned positions):

1. **+12 fold vs split.** GCC's combine pass merges `(set s0 (plus s0 4))(set s0 (plus s0 8))` into `(set s0 (plus s0 12))`. Target keeps them split. No pure-C structural change tried so far defeats combine here (intervening sh stores don't reference arg0; explicit goto-restructure folds back at jump-threading; no SOTN-sanctioned "break the basic block" lever applies to this specific pass).
2. **addiu+move vs addiu.** GCC stages the buf-address compute into $v0, then moves to $s0. Target writes directly to $s0. The parameter pseudo for arg0 should already be in $s0, so the reassignment ought to be a single insn — but GCC's RA evidently inserts a copy-via-temp here. Worth a `-da` dump to confirm whether this is RA reload or a combine artifact.

## Verified rejected forms (do NOT re-derive)

See `meta.json.rejected_forms`. Notably: introducing a separate `s16 *p_buf` local goes to $s1 (+1 callee-save, score 12 worse than HEAD). Assigning arg0=buf in both branches is +3 insns. Goto-form is bytewise identical to if-form. The +4/+8 reorder doesn't change combine.

## Related rules

- [[register-alloc-pure-c]] — the parent RA playbook; this candidate is a Lever A variant (parameter pseudo reuse for a new value within its callee-save slot).
- [[store-before-jal]] — the related delay-slot scheduling family; here we want target's `addu $a2,$s0,$zero` in the jal delay slot, which requires $s0 to hold buf BEFORE the jal. Candidate achieves this; the cost is the extra addiu+move pair.
- [[no-new-park-categories]] — the cheats-by-spelling policy the candidate was vetted against.
