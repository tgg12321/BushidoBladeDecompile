# func_800693CC evidence

## 2026-09-24 Codex session

- Canonical route: `C`; target length: 307 instructions.
- Recovered behavior: menu-selection input handler.  It merges the two controller
  halves, decodes directional/action input, cycles through the eight-entry
  availability mask at `D_8009BC04`, updates the selected entry's mode bit,
  updates the visibility mask, renders the menu, and returns the accepted entry
  or `-1`/`-2` for the two cancellation paths.
- Best honest candidate: score 13, 307/307 instructions.  Apart from relocation
  cascades, the residual consisted of a 0x78 generated frame versus the target's
  0x60 frame and one tail address hoist.  The entire functional body through the
  four render calls and both action paths otherwise aligned.
- Target stack facts: context base is `sp+0x10`; the decoder word is at `sp+0x40`;
  the saved visibility word is at `sp+0x48`; saves are at `sp+0x50..0x58`.
  `func_8006E390` initializes ten words from the context base.
- Measured variants (sandbox score): natural structured C 104; m2c-shaped 83;
  control-flow/goto shape 41; unsigned availability mask 27; corrected action
  control and stack offsets 17; best loop spelling 13; ten-word context plus
  separate input/mask locals 16-30 depending on loop spelling (0x68 frame);
  parameter-carrier experiments 53; direct-expression form 37; shared loop
  temporaries 27/30; `static __inline__` loop extraction 13 (frame unchanged).
- A 15-word aggregate preserves both required stack stores but GCC 2.7.2 retains
  24 bytes of additional phantom local allocation for this CFG.  A ten-word
  context reduces the frame to 0x68 but places the decoder word at `sp+0x38` and
  optimizes away the otherwise-dead `sp+0x48` store.  Shrinking the aggregate and
  indexing beyond it would be undefined C; fake padding, volatile steering,
  dead parameter assignments, and register pins were rejected under
  `docs/DECOMP_WORKFLOW.md`.
- The old decomp-permuter import produced a scorer baseline of 789 rather than
  the engine's baseline and was not used as evidence for a match.

Disposition: rotated with this record after semantic recovery and measured lever
exhaustion.  This remains a pure-C problem; it is not proposed as a new park or
infrastructure category.

## 2026-09-28 Codex return from rotation

Canonical gate: C, LOW hand-coded tier, 307 target instructions. Isolated worktree branch `codex/func-800693cc` at 840a5f5af; no source edits on Claude's main checkout.

Recovered the menu navigator from the old scratch permuter base. The first compilable form scored 18/307. Making the controller merge an unsigned right shift lowered it to 17. The key whole-cluster simplification was spelling the two-byte row index as a shift (`index << 1`); it removed the address hoist and the excess frame, reaching 4/307. A nested assignment that gives the incremented index to both the stored flags and the range test, paired with compound clearing of the stored flags, produced score 0/307. Both actions are real operations: the low nibble selects an entry, and the range check wraps it.

Fresh adversarial review FAILed the first score-zero body: separate `D_8009BC0C[]` and `D_8009BC0D[]` declarations overlapped the same 16-byte record table. Replaced them with one `MenuOption D_8009BC0C[8]` whose `{u8 state,u8 mode}` elements are proven by asm/data/7D920.data.s at 0x8009BC0C..0x8009BC1B. This revised body also measures score 0/307; no source-level or operand differences remain. The scorer's relocation-only hunks are expected from the sandbox object location, and the full linked oracle is authoritative.

Pointer-alias ablation on the revised, typed-record chassis (`sandbox --disable all --candidate`): both availability pointers present, 0/307; first pointer replaced by a direct global read, 5/307; second replaced, 5/307; both replaced, 10/307. The observed mechanism is loop-invariant motion in GCC `loop.c`: on the direct-global form the compiled loop receives the **word value** once in its preheader, while the pointer spelling keeps only the address in a register and reloads the word each iteration, as the target does. This pass attribution is inferred from the emitted preheader/body difference; the score and instruction differences are measured. Saved ablation bodies: `rejected/direct-first.c`, `direct-second.c`, and `v13.c` (both direct). The two declarations carry inline `FAKE` annotations under pointer-alias-fake-exception.md; the aliases are the only Tier-2 constructs in the body. The `mask_ptr` local is a genuine read-modify-write through one handle.

Context ablation on that chassis: replacing `context[15]` with a ten-word draw context plus standalone input and saved-mask locals scores 11/306, changes the frame from target 0x60 to 0x50, moves the input stack address, and eliminates the `sp+0x48` saved-mask store. Whole-cluster plain form (both pointers removed as well) scores 21/306. Saved forms: `rejected/context-ablate.c` and `plain-cluster.c`. The 15-word scratch array has genuine live uses at indices 12 and 14, and the target contains the word-14 store. No unwritten dead array was introduced.

After the exact revised body (including annotations and two-parameter signature) was installed in the isolated worktree: `sandbox func_800693CC --disable all --diff` returned score 0, 307/307; `verify-oracle --rebuild` returned `ok: true`, `build_matches: true`, SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`. Fresh review then found the stale zero-argument cast in the only caller. `func_80077894(s32 held, s32 pressed)` now forwards the two values explicitly to `func_800693CC`; the upstream call in `src/code6cac_b2_post.c` already supplies them. The reviewer also required `s32 *available` to match the global declaration, with `(u32)*available` for the logical shift. Both final bodies score 0 (`func_800693CC`: 307/307; `func_80077894`: 28/28). After these exact source edits, a fresh `verify-oracle --rebuild` returned `ok: true`, `build_matches: true`, SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`. Fresh default-FAIL adversarial review by `review_693cc` passed the exact final source and ledger; no source body edits followed.

Completion gate: `queue done func_800693CC` returned `ok: true`, `completion: COMPLETED-C`, oracle SHA1. `check_completion_integrity.py` exited 0 and reported all category invariants satisfied. `audit_asm_cheats.py --check-new` exited 1 on pre-existing canonical evidence/self-authorization findings in the branch baseline; its output names neither `func_800693CC` nor `func_80077894` (captured in ignored `tmp/asm-audit.log`). No new cheat construct was introduced in either body.

## 2026-10-02 port to main under owner ruling Q91 (093ae1db8)

Ported the branch body (0cf578443) to main, where the function now lives in src/text1b_tu1c.c. On main 5954c47d6 the branch body still scored 0/307. A fresh layer-2 FAILed it under the pre-Q91 rules: the context[15] object model, the always-true `state >= 0`, the D_8010278C/E alias symbols, and the block-scope MenuOption. The fixes:
- `state >= 0` ladder and bare block replaced by two `switch` statements, still 0/307. Plain-if variant vA2: 0. Switch in one arm only (vC/vD): same score before the clear_mask fix.
- Pad-state reads are now `D_80102788.unk_00[2]/[3]` (PadState, include/code6cac.h). The alias rows stay because asm/data and other asm functions still reference those symbols.
- `MenuOption D_8009BC0C[8]` is declared in include/game.h.
- `D_800A350C` is passed without `&` (it is an `s16[2]` in this TU).
- clear_mask written as the literal `~0xF` scores 2: `li -16` moves two slots (rejected/literal-clear-mask-2.c). It is kept as a labelled FAKE.
- mask_ptr replaced by direct global reads scores 9/305 (rejected/direct-mask-9.c). It is kept as a labelled FAKE.
- context[15] keeps its "FAKE: frame layout" label, now with a truthful comment; the separate-locals ablations above score 11/21.
- Caller func_80077894(s32 held, s32 pressed) compiled out of tree is byte-identical to main's object.
