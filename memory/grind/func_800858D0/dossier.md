# func_800858D0 — recon dossier (2026-08-17)

## 1. IDENTITY
- **Body:** `src/main.c:857` — `void func_800858D0(void)`. Known in the ledger as **`title_mv_exec2`**.
  Note a live prototype contradiction: `src/text1b.c:1928` declares `extern s32 func_800858D0(s32);` and
  `:1950` / `:1985` call it as `func_800858D0(0)`.
- **What it does:** title-screen / demo mover setup. Builds a 64-byte GPU-primitive buffer on the stack
  (`buf[1] = 0x60093`, `+0x14 = 0x1000`, `+0x1C = 0x1000`, `+0x3A = 0x80FF`, `+0x3C = 0x4000`, `+0x08/+0x0A = 0`),
  then for each of `D_80101BCC` entries writes six s16 fields at a 54-byte stride into the `D_800F4E1x` block
  (`0x18, 0, 0xFF, 0, 0, 0xFF`), sets `buf[0] = 1 << i`, and calls `func_8008B488(buf)` followed by
  `D_8010280A = i` and `func_800871D4(1)`.
- **Size:** target 72 insns; `asm/funcs/func_800858D0.s` = 76 lines. **The smallest of the six.**

## 2. MEASUREMENT
- `canonical func_800858D0` → **verdict `C`**, `asm_insns 0`, `total 72`, `distance 27`.
- `sandbox func_800858D0 --disable all` → **score 27**, `target_insns 72`, `build_insns 76`, `rules_dropped 18`.
  This is the **committed HEAD** floor. A banked candidate at **22** already exists — see §5.
- Honest alignment (`tmp/sandbox/func_800858D0/main.o` vs `build/src/main.o`;
  `tmp/recon/func_800858D0.hdiff.txt`): **equal 53 / replace 19 / delete 6 / insert 4**. Clusters:
  1. **Frame and callee-saves (T0, T15, T66, T69):** target `addiu sp,sp,-96` with `sw ra,88(sp)` and **3**
     callee-saves; ours `addiu sp,sp,-104` with `sw ra,96(sp)` plus **`sw s3,92(sp)` / `sw s2,88(sp)`** — i.e.
     **5** saves — and matching extra restores in the epilogue.
  2. **Hoisted loop constants (H24–H25):** ours emits `li s3,24` / `li s2,1` **before** the loop; target has
     neither, instead rematerialising `li v1,24` at T30 and `li v0,1` / `sllv v0,v0,a1` at T49–T50 **inside** the
     loop body. This is the direct cause of cluster 1.
  3. **Register naming in the stride computation (T23–T33):** target holds the sign-extended index in `a1`
     (`sll a1,s0,0x10` / `sra a1,a1,0x10` / `sll v0,a1,0x3` / `subu v0,v0,a1`); ours holds it in `v1`.
  4. **Prologue/entry scheduling (T5–T21):** the `sw s0` / `move s0,zero` pair and the `sh v0,76(sp)` / `beqz v1`
     ordering differ by a few slots.

## 3. RULE INVENTORY (18 regfix, 0 asmfix — `regfix.txt:192-209`)
The committed comment is explicit: *"GCC hoists 0x18 and 1 into s2/s3, target only hoists 0xFF into s1. Remove
s2/s3 saves, change sign-ext reg $3->$5 in loop, fresh load 0x18 and 1<<a1."*
| Lines | Kind | Papers over |
|---|---|---|
| 192 (1) | register swap `$3 <-> $5 @ 26-32` | **cluster 3** — the sign-extend register |
| 193, 194, 198, 199 (4) | subst frame size `104`→`96` and save offsets `96(`→`88(` | **cluster 1** — the two extra callee-saves |
| 200-205 (6) | six `delete` rules @15, 16, 23, 24, 54, 55 | deletes the `sw s2` / `sw s3` saves and their restores (cluster 1) |
| 195-197 (3) | subst `sll $3,$18,$3`→`addiu $3,$zero,24`, `$19,`→`$3,`, `sw $3,16`→`sw $2,16` | **cluster 2** — rewrites the hoisted constant back into an in-loop materialisation |
| 206-207 (2) | two `insert` (`sllv $2,$2,$5` + `.set noreorder`, and `addiu $2,$zero,1`) | **cluster 2** — re-injects the in-loop `1 << i`; a **lost-codegen insert** (`.claude/rules/lost-codegen-insert-cheat.md`) |
| 208-209 (2) | `reorder 3,4,5,1,2 @ 1-5` and `reorder 17,19,18,16 @ 16-19` | **cluster 4** — prologue/entry scheduling |

No asmfix, no cheat-asm in the body, no `prologue_config` / `frame_fix` / maspsx-gate entries (grep-verified in the
existing ledger).

## 4. RESIDUAL DIAGNOSIS
**This function is the best-characterised of the six and the diagnosis is already settled and recorded.** The root
cause is `loop.c`'s **`move_movables` hoisting the two loop-invariant constants** `0x18` and `1` into callee-saved
registers, which forces two extra saves and a larger frame. Target rematerialises both inside the loop. It is
explicitly **not** a phantom-frame case (`memory/project/phantom-frame-slots-gcc272.md` does not apply — both sides
have `vars=64`). Governing rule: **`.claude/rules/defeat-licm-hoist-var-reuse.md`**.

Two follow-on residuals are quantified to the RTL level in `memory/wip/func_800858D0/meta.json`:
- The `$a1`/`$v1` swap on the sign-extend is decided by **local allocation**, not global. `tools/ra_solver`
  Phase 5 reproduces the block exactly (2/2 ordering, 10/10 assignment) and gives a **two-part** requirement,
  neither part sufficient alone: (1) an extra local quantity overlapping span `[6,38)` with priority > 22500, **and**
  (2) hard `$a0` live across that range. Target supplies (1) with `li v1,24` at line 31 consumed at 34, and (2) with
  `addiu a0,sp,16` emitted early at line 23.
- **Scheduling is CLOSED.** After the banked A+B statement moves, `sched_solver`'s goal-mapper reports
  `goal == identity` for all three blocks; the remaining gap is `replace 12` (naming) plus `delete 5 / insert 1`.

## 5. FRONTIER — **BANKED at 22; resume from the checkpoint, not from HEAD**
`memory/wip/func_800858D0/` holds `candidate.c` + `notes.md` + `meta.json` from three 2026-08-04/05 sessions.
The candidate scores **22** (vs HEAD's 27) with **frame 96 == target, 3 saves == target, 72 insns == target**.
Per `.claude/rules/decomp-loop.md`, resume from that checkpoint.

**Open question the ledger leaves as the single next move:** *why does `loop.c`'s `move_movables` not hoist
target's single-set loop constants, when it hoists ours?* Answering it makes the `24` a short-lived local quantity
in `$v1` (satisfying RA part 1), removes the need for the shared multi-set scratch, and reduces part 2 to a
placement question the call-argument family has already half-explored.

1. **Fold the banked A+B statement moves into `candidate.c` and re-measure** (the ledger's own explicit resume
   step; both were measured individually — honest differing insns 19 → 17 (A) → 16 (B) → **14 (A+B)** — and they
   compose). A = put `buf[1] = 0x60093;` **before** `var_s0 = 0;`. B = move
   `*(s16 *)((u8 *)buf + 0x3C) = 0x4000;` to the **end** of the init block.
2. **Attack the `move_movables` question directly** — find a single-set spelling of the `24` that is not a movable
   (e.g. making it depend on something loop-varying that folds away, or giving it a use that blocks promotion),
   so both RA parts can land together. The two requirements currently pull against each other: the shared
   multi-set `t` defeats the hoist but destroys part 1.
3. **Do not re-run any placement sweep.** See the killed list below.

### Levers already KILLED — do not re-propose
All at score 22 unless noted, from `memory/wip/func_800858D0/meta.json`:
- Named sign-extend local (`s32 idx` / `s16 idx`) reused as the shift amount — register map **bit-identical**.
- `t` at loop-body block scope; **every** `t` placement sweep (loop top, at each use, store reordered,
  shift-first, idx+shift-first — shift-first is strictly worse, 42 differing lines vs 40).
- Call-argument naming per `hoist-call-arg-local-flips-jal-delay` in all four shapes (`s32 *p = buf;`
  block-wrapped, at loop-body top, `u8 *p`, the rule's exact shape) — inert at 22, and **at function scope it
  regresses to 35**.
- Statement order around the call.
- `t` for the `0x18` only, or the `1` only — still hoisted, back to 27.
- Reusing `offset` for the `1` (score 20 but 74 insns); `offset` reuse + `t` for `0x18` (20, 74 insns);
  `s16 t` (16 but an extra save and 74 insns); `t = (s16)var_s0; buf[0] = 1 << t;` (74 insns).
- `store-before-jal` does not apply — the pre-call `sw …,16(sp)` is already in the jal delay slot on both sides.
- **cc1psx chooses as we do** (`.claude/rules/cc1psx-calibration-only.md`) — no compiler-config win here.
- `ra_solver` global-only `perturb.py` with spec `{75: 3}`: **no vector**, and that is the structurally correct
  answer, not a search failure (the residual is local-alloc).

**Build gate warning recorded in the ledger:** applying the candidate as-is breaks the oracle (SHA1 `31fe8c21`) —
the 18 regfix rules are calibrated to HEAD's emission shape, so they must be retired in the same change.

## 6. SIBLING LEADS
- **`func_8003DBE4`** already ships the multi-set scratch shape this function needs
  (`.claude/rules/defeat-licm-hoist-var-reuse.md` cites it) — the reference spelling for hypothesis 2.
- **`func_800770B8`** (also in this dossier set) has the *same* `move_movables` hoist mechanism, applied to a
  symbol address rather than an integer constant. The two should be worked in the same session: whichever is
  solved first supplies the spelling for the other.
- `memory/wip/_frame_census_2026-08-04.md` and `memory/wip/_g8_census_2026-08-05.md` contain the cross-function
  frame/save-count survey this function's analysis came out of.
