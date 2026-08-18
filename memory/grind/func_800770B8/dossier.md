# func_800770B8 — recon dossier (2026-08-17)

## 1. IDENTITY
- **Body:** `src/text1b.c:7189` — `void func_800770B8(s32 arg0_in, s32 arg1, long arg2)`.
  Re-declared inconsistently at `src/text1b.c:7420` as `void func_800770B8(s32, s32 *, s32);` and called at
  `src/text1b.c:7423` as `func_800770B8(a0, (s32 *)&D_8009BD24, D_800A35E8);` — a live prototype contradiction
  worth noting before any signature change (see `memory/reference/sotn-prototype-struct-precedent-2026-08-10.md`:
  bytes decide the declaration).
- **Named:** `replay_camera_helper_800770B8` (`named_syms.txt:3385`, called from
  `replay_camera_apply_transform_and_flip_80077A80`).
- **What it does:** match/round setup. Clears the OT (`ClearOTagR(D_800A374C, 0x1008)`), stops all sound, allocates
  a new context (`func_8006E950(6, …)` → `func_80076FF8` → `func_8006E49C`) and publishes it in `D_800A36A0`,
  linking the previous block at `+4`. Then, for each of 2 players, zeroes ~8 s16 fields plus a `D_800A35D0` pair,
  stamps the player index at `+0x68`, fills a 5-entry `-1` / `0` table pair at `+0x6A` / `+0x7E`, sets `+0x5C = 0`
  and `+0x60 = 5`, and walks 10 slots of `D_8009BCE4` masking with `0xF2` and OR-ing 1 wherever the corresponding
  bit of `arg2` is set, counting hits per player. Finally it writes `min(count) - 3` (clamped to 2) at `+0x64`,
  stores `arg1` at `+0`, sets `+0x65 = 0`, `+0x67 = 1`, `+0x66 = D_8009BD21[+0x67 * 2]`, and `D_800A35DC = 1`.
- **Size:** target 175 insns; `asm/funcs/func_800770B8.s` = 184 lines.

## 2. MEASUREMENT
- `canonical func_800770B8` → **verdict `C`**, `asm_insns 0`, `total 175`, `distance 32`.
- `sandbox func_800770B8 --disable all` → **score 32**, `target_insns 175`, `build_insns 174`, `rules_dropped 25`.
- Honest alignment (`tmp/sandbox/func_800770B8/text1b.o` vs `build/src/text1b.o`, `mask=False`;
  full listing in `tmp/recon/func_800770B8.hdiff.txt`): **equal 140 / replace 35 / delete 9 / insert 3**.
  Roughly 10 of the raw replaces are branch-address-only artifacts. Five residual clusters:
  1. **Prologue interleave (T1–T13):** target saves/moves `s0` first (`sw s0,40(sp)` / `move s0,a0`), then `sw ra`,
     `li a1,4104`, and `addiu s1,s0,88` at T12. Ours emits `addiu s1,s0,88` at H8, `li a1,4104` at H11 and
     `sw ra,56(sp)` at H12 — same instructions, different interleave.
  2. **Hoisted symbol address (T49–T51 vs H31–H32, H51):** target materialises `D_800A35D0` **inside the loop**
     (`lui v0,%hi` / `addiu v0,%lo` / `addu v0,v1,v0`); our honest build **hoists it out of the loop** into `t4`
     (`lui t4` / `addiu t4` at H31–H32) and then only does `addu v0,v1,t4`. This is the single biggest structural
     divergence and accounts for the +3 inserted instructions.
  3. **Base-register reuse (T35–T37):** target re-loads the context pointer into `v0` and stores through it
     (`sw zero,48(v0)` / `sh zero,52(v0)`) and then does `move a2,zero` *after*; ours keeps `s1` live and stores
     through `s1`, with `move a2,zero` *before*.
  4. **Missing delay-slot fill (T103–T104):** target has `beqz v0,…` followed by **`addiu v0,a2,1`** in the delay
     slot — the per-player hit-count increment computed speculatively before the branch resolves. We have no such
     instruction (this is the entire 175-vs-174 shortfall) and compute the increment after the branch.
  5. **Tail block (T146–T166):** target re-loads `lw v0,0(gp)` twice, keeps the context pointer in `v0`, and holds
     a single `li a1,1` alive across **both** the `sb a1,103(v0)` store and the final `sb a1,0(gp)`. Ours keeps the
     pointer in `a0`, materialises the constant `1` twice (`li v0,1` and separately for `D_800A35DC`), and emits an
     extra load-delay `nop` before the `sll`.
  Commutative-operand differences also appear at T54 (`addu v0,a0,v1` vs `addu v0,v1,a0`) and T62–T64
  (target accumulates into `v1`, we accumulate into `v0`).

## 3. RULE INVENTORY (25 regfix, 0 asmfix — `regfix.txt:2885-2940`)
| Lines | Kind | Papers over |
|---|---|---|
| 2885 (1) | reorder `5,6,1,2,3,4,9,7,11,10,8,12` @1-12 | the **prologue interleave** (cluster 1) |
| 2888 (1) | `delete @ 30` | removes the hoisted `la $12, D_800A35D0` (cluster 2) |
| 2891-2893 (3) | two `insert` (`lui`/`addiu` %hi/%lo) + one subst @48-49 | re-synthesises the in-loop symbol materialisation (cluster 2) — **an inline-address insert**, see §4 |
| 2898 (1) | reorder `30,…,36,29` @29-36 | sinks `move a2,zero` past the two stores (cluster 3) |
| 2902-2903 (2) | subst `$17`→`$2` on the `48`/`52` stores | base-register reuse (cluster 3) |
| 2907-2908, 2910, 2912, 2914-2915 (6) | subst end-of-function register renames | tail-block allocation (cluster 5) |
| 2918-2921 (4) | subst `$4`→`$2` on the `D_800A36A0` reload group | tail-block pointer register (cluster 5) |
| 2925, 2927, 2929-2930 (4) | subst commutative operand order / dest change @52,60,61,62 | operand-order and accumulator-register choice |
| 2934 (1) | `insert_after "addu $2,$6,1" @ 100` | **the missing delay-slot increment** (cluster 4) — a lost-codegen insert; see §4 |
| 2939-2940 (2) | `insert_label ".L_post988:"` + branch retarget @108/@99 | keeps the branch target one instruction further along after the insert above |

Cheat-asm in the body: `register s32 arg0 asm("$16") = arg0_in;` (line 7191) and a memory-clobber barrier
`__asm__ volatile("" : : "r"(arg1) : "memory");` (line 7198). Both are stripped by the cheat-invisible sandbox and
both must go for COMPLETED-C.

## 4. RESIDUAL DIAGNOSIS
Two mechanisms dominate; both have a named technique rule.

- **Loop-invariant hoisting of a symbol address (cluster 2, 4 rules).** The C forms an intermediate pointer inside
  the loop (`s16 *p_d0 = &(&D_800A35D0)[t0 * 2];`), and `loop.c`'s `move_movables` lifts the `%hi/%lo` address pair
  out of the loop into a long-lived pseudo (`t4`). Target rematerialises it every iteration. Rules:
  **`defeat-licm-hoist-var-reuse`** and **`defeat-combine-symbol-fold`**. This is the *same* mechanism already
  solved once in this repo — see `memory/wip/func_800858D0/notes.md`, where a multi-set scratch local defeated an
  identical `move_movables` hoist and retired two callee-saves.
- **A value not available for the branch delay slot (cluster 4, 3 rules).** Target computes `count + 1` *before*
  the conditional branch resolves and commits it only on the taken path; we compute it inside the arm, leaving the
  delay slot empty. Rules: **`staged-value-reused-variable`** and
  **`defer-store-past-later-compute-into-jal-delay`**. Regfix line 2934 is a genuine
  **lost-codegen insert** (`.claude/rules/lost-codegen-insert-cheat.md`) — close it in C, never as `__asm__`.

The remaining three clusters are ordinary **scheduling** (prologue interleave — reload save placement plus list
scheduling) and **allocation/CSE** (base-register reuse; the duplicated `li 1` in the tail where target CSEs one
constant across two stores). None of the residual is frame-shaped: neither side spills differently and the frame
size never appears in a rule.

## 5. FRONTIER (no `memory/wip/func_800770B8/` and no `memory/grind/func_800770B8/` — nothing killed on record)
1. **Kill the symbol-address hoist** (highest value: 4 rules, and it is the only cluster contributing inserted
   instructions). Drop the `s16 *p_d0` intermediate and write the two zero stores directly as indexed accesses off
   the global, so there is no single-set loop-invariant pseudo for `move_movables` to promote. If a direct indexed
   form still hoists, apply the proven `func_800858D0` shape: route the address through a **multi-set** scratch
   local so it is not a `move_movables` candidate (a movable must be set exactly once in the loop).
2. **Stage the hit counter so its increment is branch-independent** (3 rules, and it closes the 174→175 shortfall).
   Compute the incremented value into a named local before the `if`, and assign it inside the arm, rather than
   doing `sp[t0] += 1;` in the arm. That is exactly the `staged-value-reused-variable` shape and it is what puts
   `addiu v0,a2,1` in target's delay slot.
3. **Share one constant `1` across the two tail stores** (up to 6 rules in cluster 5). Introduce a single local
   holding `1` and use it for both `*((u8 *)(p + 0x67)) = 1;` and `D_800A35DC = …`, so GCC CSEs one `li` into `a1`
   and keeps it live — target's exact shape. Do this **together with removing the `asm("$16")` pin on `arg0`**,
   since the pin currently forces the final assignment through `$16` and is what the `addiu $16,$zero,1` → `$2`
   rules (2910, 2912) exist to undo.
4. **Re-measure after 1–3 before touching the prologue interleave (rule 2885).** Prologue save order is downstream
   of how many pseudos survive to reload; three structural changes above will move it, so solving it first would
   very likely be wasted work.

## 6. SIBLING LEADS
- **`func_800858D0` / `title_mv_exec2`** (`src/main.c:857`, also in this dossier set) is the closest match in kind:
  the same `move_movables` loop-invariant hoist, already analysed to the RTL level, with a working C-level lever
  written up in `memory/wip/func_800858D0/notes.md`. Read that before attempting hypothesis 1.
- `D_800A36A0` is re-read at `src/text1b.c:7266` and in `func_80077098` / `func_80077374` immediately below this
  function; the matched ones show the accepted spelling for "reload the global into a fresh local per statement
  group", which is the shape cluster 3 needs.
- `src/text1b.c:1818` carries a `asm volatile("" : : "r"(partial));` barrier in a neighbouring function — same
  cheat family, worth sweeping together if this file gets a cleanup pass.
